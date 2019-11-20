#!/usr/bin/env python3

import itertools
import sys

TOCK_ARCHS = sys.argv[1]
platforms = TOCK_ARCHS.split('|')

# ARCH = sys.argv[1]
# TOOLCHAIN = sys.argv[2]

ARCH = platforms[0]
TOOLCHAIN = platforms[1]

# ARCH='a'
# TOOLCHAIN = 'b'
# print(TOCK_ARCHS)
# print('ARCH')
# print(ARCH)

flash = 0x4043002c
ram = 0x80004000


def get_location_pairs (arch):
	if arch.startswith('cortex'):
		# The Cortex-M cores support PIC, so no need to compile for multiple
		# locations.
		flash = 0x80000000
		ram = 0
		return [(flash, ram)]

	elif arch.startswith('rv32'):
		# The RISC-V compile does not support FDPIC, so we need to compile for
		# multiple locations.
		start = 0x40430000
		end = 0x40440000
		inc = 0x2000
		# Need to add the TBF header offset to each flash location since that
		# is where the actual application will be.
		flash_locations = list(map(lambda x: x + 256, range(start,  end, inc)))

		# list(map(lambda x : x + K, test_list))

		start = 0x80004000
		end = 0x80010000
		inc = 0x1000
		ram_locations = list(range(start,  end, inc))

		return list(itertools.product(flash_locations, ram_locations))





linking='''
# Collect all desired built output.
$(BUILDDIR)/{arch}/{arch}.{flash:#x}.{ram:#x}.elf: $(OBJS_{arch}) $(LIBS_{arch}) $(LEGACY_LIBS_{arch}) $(LAYOUT) | $(BUILDDIR)/{arch}
	$(TRACE_LD)
	$(Q){toolchain}$(CC) $(CFLAGS) $(CPPFLAGS) $(CPPFLAGS_{arch})\\
	    --entry=_start\\
	    -Xlinker --defsym=STACK_SIZE=$(STACK_SIZE)\\
	    -Xlinker --defsym=APP_HEAP_SIZE=$(APP_HEAP_SIZE)\\
	    -Xlinker --defsym=KERNEL_HEAP_SIZE=$(KERNEL_HEAP_SIZE)\\
	    -Xlinker --defsym=FLASH_ORIGIN={flash:#x}\\
	    -Xlinker --defsym=RAM_ORIGIN={ram:#x}\\
	    -T $(LAYOUT)\\
	    -nostdlib\\
	    -Wl,--start-group $(OBJS_{arch}) $(LIBS_{arch}) $(LEGACY_LIBS_{arch}) -Wl,--end-group\\
	    -Wl,-Map=$(BUILDDIR)/{arch}/{arch}.{flash:#x}.{ram:#x}.Map\\
	    -o $@
'''

# Generate all of the rules for each of the N x K locations.
out = ''
for flash,ram in get_location_pairs(ARCH):
	out += linking.format(arch=ARCH, toolchain=TOOLCHAIN, flash=flash, ram=ram)


print(out.replace('\n', 'NEWLINE'))

