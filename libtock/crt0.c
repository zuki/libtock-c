#include <tock.h>

#if defined(STACK_SIZE)
#warning Attempt to compile libtock with a fixed STACK_SIZE.
#warning
#warning Instead, STACK_SIZE should be a variable that is linked in,
#warning usually at compile time via something like this:
#warning   `gcc ... -Xlinker --defsym=STACK_SIZE=2048`
#warning
#warning This allows applications to set their own STACK_SIZE.
#error Fixed STACK_SIZE.
#endif

extern int main(void);

// Allow _start to go undeclared
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wmissing-prototypes"

/*
 * The structure populated by the linker script at the very beginning of the
 * text segment. It represents sizes and offsets from the text segment of
 * sections that need some sort of loading and/or relocation.
 */
struct hdr {
  //  0: Offset of GOT symbols in flash
  uint32_t got_sym_start;
  //  4: Offset of GOT section in memory
  uint32_t got_start;
  //  8: Size of GOT section
  uint32_t got_size;
  // 12: Offset of data symbols in flash
  uint32_t data_sym_start;
  // 16: Offset of data section in memory
  uint32_t data_start;
  // 20: Size of data section
  uint32_t data_size;
  // 24: Offset of BSS section in memory
  uint32_t bss_start;
  // 28: Size of BSS section
  uint32_t bss_size;
  // 32: First address offset after program flash, where elf2tab places
  //     .rel.data section
  uint32_t reldata_start;
  // 36: The size of the stack requested by this application
  uint32_t stack_size;
};

struct reldata {
  uint32_t len;
  uint32_t data[];
};

__attribute__ ((section(".start"), used))
__attribute__ ((weak))
__attribute__ ((naked))
__attribute__ ((noreturn))
void _start(void* app_start __attribute__((unused)),
            void* mem_start __attribute__((unused)),
            void* memory_len __attribute__((unused)),
            void* app_heap_break __attribute__((unused))) {
#if defined(__thumb__)
  // Assembly written to adhere to any modern thumb arch

  // Allocate stack and data. `brk` to stack_size + got_size + data_size +
  // bss_size from start of memory. Also make sure that the stack starts on an
  // 8 byte boundary per section 5.2.1.2 here:
  // http://infocenter.arm.com/help/topic/com.arm.doc.ihi0042f/IHI0042F_aapcs.pdf

  asm volatile (
    // Compute the stack top
    //
    // struct hdr* myhdr = (struct hdr*)app_start;
    // uint32_t stacktop = (((uint32_t)mem_start + myhdr->stack_size + 7) & 0xfffffff8);
    "ldr  r4, [r0, #36]\n"      // r4 = myhdr->stack_size
    "add  r4, #7\n"             // r4 = myhdr->stack_size + 7
    "add  r4, r4, r1\n"         // r4 = mem_start + myhdr->stack_size + 7
    "movs r5, #7\n"
    "bic  r4, r4, r5\n"         // r4 = (mem_start + myhdr->stack_size + 7) & ~0x7
    //
    // Compute the heap size
    //
    // uint32_t heap_size = myhdr->got_size + myhdr->data_size + myhdr->bss_size;
    "ldr  r5, [r0, #8]\n"       // r5 = myhdr->got_size
    "ldr  r6, [r0, #20]\n"      // r6 = myhdr->data_size
    "ldr  r7, [r0, #28]\n"      // r6 = myhdr->bss_size
    "add  r5, r5, r6\n"         // r5 = got_size + data_size
    "add  r5, r5, r7\n"         // r5 = got_size + data_size + bss_size
    //
    // Move registers we need to keep over to callee-saved locations
    "movs r6, r0\n"
    "movs r7, r1\n"
    //
    // Call `brk` to set to requested memory
    //
    // memop(0, stacktop + heap_size);
    "movs r0, #0\n"
    "add  r1, r4, r5\n"
    "svc 4\n"                   // memop
    //
    // Debug support, tell the kernel the stack location
    //
    // memop(10, stacktop);
    "movs r0, #10\n"
    "movs r1, r4\n"
    "svc 4\n"                   // memop
    //
    // Debug support, tell the kernel the heap location
    //
    // memop(11, stacktop + heap_size);
    "movs r0, #11\n"
    "add  r1, r4, r5\n"
    "svc 4\n"                   // memop
    //
    // Setup initial stack pointer for normal execution
    "mov  sp, r4\n"
    "mov  r9, sp\n"
    //
    // Call into the rest of startup.
    // This should never return, if it does, trigger a breakpoint (which will
    // promote to a HardFault in the absence of a debugger)
    "movs r0, r6\n"             // first arg is app_start
    "movs r1, r4\n"             // second arg is stacktop
    "bl _c_start\n"
    "bkpt #255\n"
    );
#elif defined(__ricsv)
  // FIXME: Written blindly without RISC-V test hardware, but should be mostly correct.
  //
  // RISC-V does not specify a syscall calling convention for passing syscall number.
  // Currently, this code does not pass it at all! So when Tock makes a decision this
  // will need to be updated, but this is the majority of what will need to be here.

  // Allocate stack and data. `brk` to stack_size + got_size + data_size +
  // bss_size from start of memory. Also make sure that the stack starts on an
  // 32 byte boundary (possibly conservative) per:
  // https://github.com/riscv/riscv-elf-psabi-doc/pull/50/files

  asm volatile (
    // Compute the stack top
    //
    // struct hdr* myhdr = (struct hdr*)app_start;
    // uint32_t stacktop = (((uint32_t)mem_start + myhdr->stack_size + 31) & 0xffffffe0);
    "lw   a4, a0, 9\n"          // a4 = myhdr->stack_size
    "addi a4, a4, 31\n"         // a4 = myhdr->stack_size + 31
    "add  a4, a4, a1\n"         // a4 = mem_start + myhdr->stack_size + 31
    "subi a5, zero, 32\n"       // a5 = 0xffffffe0
    "and  a4, a4, a5\n"         // a4 = (mem_start + myhdr->stack_size + 7) & 0xffffffe0
    //
    // Compute the heap size
    //
    // uint32_t heap_size = myhdr->got_size + myhdr->data_size + myhdr->bss_size;
    "lw   a5, a0, 2\n"          // a5 = myhdr->got_size
    "lw   a6, a0, 5\n"          // a6 = myhdr->data_size
    "lw   a7, a0, 7\n"          // a6 = myhdr->bss_size
    "add  a5, a5, a6\n"         // a5 = got_size + data_size
    "add  a5, a5, a7\n"         // a5 = got_size + data_size + bss_size
    //
    // Move registers we need to keep over to callee-saved locations
    "add  s0, a0, zero\n"
    "add  s1, a1, zero\n"
    //
    // Call `brk` to set to requested memory
    //
    // memop(0, stacktop + heap_size);
    "add  a0, zero, zero\n"
    "add  a1, a4, a5\n"
    "ecall\n"                   // memop // FIXME: Tock Calling Convention?
    //
    // Debug support, tell the kernel the stack location
    //
    // memop(10, stacktop);
    "addi a0, zero, 10\n"
    "add  a1, a1, a4\n"
    "ecall\n"                   // memop // FIXME: Tock Calling Convention?
    //
    // Debug support, tell the kernel the heap location
    //
    // memop(11, stacktop + heap_size);
    "addi a0, zero, 11\n"
    "add  a1, a4, a5\n"
    "ecall\n"                   // memop // FIXME: Tock Calling Convention?
    //
    // Setup initial stack pointer for normal execution
    "add  sp, a4, zero\n"
    "mov  gp, a4, zero\n"       // FIXME: GOT Register?
    //
    // Call into the rest of startup.
    // This should never return, if it does, trigger a breakpoint (which will
    // promote to a HardFault in the absence of a debugger)
    "add  a0, s0, zero\n"       // first arg is app_start
    "add  a1, s1, zero\n"       // second arg is stacktop
    "jal  _c_start\n"
    "ebreak\n"
    );
#else
#error Missing initial stack setup trampoline for current arch.
#endif
}

__attribute__((noreturn))
void _c_start(uint32_t* app_start, uint32_t stacktop) {
  struct hdr* myhdr = (struct hdr*)app_start;

  // fix up GOT
  volatile uint32_t* got_start     = (uint32_t*)(myhdr->got_start + stacktop);
  volatile uint32_t* got_sym_start = (uint32_t*)(myhdr->got_sym_start + (uint32_t)app_start);
  for (uint32_t i = 0; i < (myhdr->got_size / (uint32_t)sizeof(uint32_t)); i++) {
    if ((got_sym_start[i] & 0x80000000) == 0) {
      got_start[i] = got_sym_start[i] + stacktop;
    } else {
      got_start[i] = (got_sym_start[i] ^ 0x80000000) + (uint32_t)app_start;
    }
  }

  // load data section
  void* data_start     = (void*)(myhdr->data_start + stacktop);
  void* data_sym_start = (void*)(myhdr->data_sym_start + (uint32_t)app_start);
  memcpy(data_start, data_sym_start, myhdr->data_size);

  // zero BSS
  char* bss_start = (char*)(myhdr->bss_start + stacktop);
  memset(bss_start, 0, myhdr->bss_size);

  struct reldata* rd = (struct reldata*)(myhdr->reldata_start + (uint32_t)app_start);
  for (uint32_t i = 0; i < (rd->len / (int)sizeof(uint32_t)); i += 2) {
    uint32_t* target = (uint32_t*)(rd->data[i] + stacktop);
    if ((*target & 0x80000000) == 0) {
      *target += stacktop;
    } else {
      *target = (*target ^ 0x80000000) + (uint32_t)app_start;
    }
  }

  main();
  while (1) {
    yield();
  }
}

