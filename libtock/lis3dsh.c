#include "lis3dsh.h"

// struct to hold values send with the callback
typedef struct lis3dsh_response {
  int data1;
  int data2;
  int data3;
  bool done;  // the callback has been called
} LIS3DSHResponse;

static void command_callback_yield(int data1, int data2, int data3, void* ud) {
  LIS3DSHResponse *response = (LIS3DSHResponse*)ud;
  if (response) {
    response->data1 = data1;
    response->data2 = data2;
    response->data3 = data3;
    response->done  = true;
  }
}

static int lis3dsh_subscribe(subscribe_cb cb, void *userdata) {
  return subscribe(DRIVER_NUM_LIS3DSH, 0, cb, userdata);
}

static int lis3dshc_command(uint32_t command_num, uint32_t data1, uint32_t data2) {
  return command(DRIVER_NUM_LIS3DSH, command_num, data1, data2);
}

// Accelerometer Scale Factor
// see manual page 12
const float SCALE_FACTOR[5] = { 
  0.06 / 1000.0,
  0.12 / 1000.0,
  0.18 / 1000.0,
  0.24 / 1000.0,
  0.73 / 1000.0
};

unsigned char scale_factor = 0;

bool lis3dsh_is_present(int *id) {
  LIS3DSHResponse response;
  response.data1 = 0;
  response.done  = false;
  int ret;
  // subscribe
  lis3dsh_subscribe(command_callback_yield, &response);
  if ((ret = lis3dshc_command(1, 0, 0)) == TOCK_SUCCESS) {
    yield_for(&(response.done));
    if (id != NULL) {
      *id = response.data2;
    }
  }
  return response.data1 ? true : false;
}

bool lis3dsh_set_power_mode(unsigned char power_mode) {
  int evalue;
  LIS3DSHResponse response;
  response.done  = false;

  if (power_mode > 9) power_mode = 6;
  // subscribe
  lis3dsh_subscribe(command_callback_yield, &response);
  evalue = lis3dshc_command(2, power_mode, 0);
  if (evalue == TOCK_SUCCESS) {
    yield_for(&(response.done));
  }
  return evalue ? false : true; // TOCK_SUCCESS == 0
}

bool lis3dsh_set_scale_and_filter(unsigned char scale, unsigned char filter) {
  int evalue;
  LIS3DSHResponse response;
  response.done  = false;

  if (scale > 4) scale = 4;
  if (filter > 3) filter = 0;
  response.done = false;
  // subscribe
  lis3dsh_subscribe(command_callback_yield, &response);
  evalue = lis3dshc_command(3, scale, filter);
  if (evalue == TOCK_SUCCESS) {
    scale_factor = scale;
    yield_for(&(response.done));
  }
  return evalue ? false : true;
}

int lis3dsh_read_xyz (LIS3DSHXYZ *xyz) {
  int evalue;
  LIS3DSHResponse response;
  response.done  = false;

  // subscribe
  lis3dsh_subscribe(command_callback_yield, &response);
  evalue = lis3dshc_command(4, 0, 0);
  if (evalue == TOCK_SUCCESS) {
    yield_for(&(response.done));
    if (xyz != NULL) {
      xyz->x_r = response.data1;
      xyz->y_r = response.data2;
      xyz->z_r = response.data3;
      xyz->x = (float)response.data1 * SCALE_FACTOR[scale_factor];
      xyz->y = (float)response.data2 * SCALE_FACTOR[scale_factor];
      xyz->z = (float)response.data3 * SCALE_FACTOR[scale_factor];
    }
  }
  return evalue;
}

int lis3dsh_read_temperature(int *temperature) {
  int evalue;
  LIS3DSHResponse response;
  response.done = false;
  // subscribe
  lis3dsh_subscribe(command_callback_yield, &response);
  evalue = lis3dshc_command(5, 0, 0);
  if (evalue == TOCK_SUCCESS) {
    yield_for(&(response.done));
    if (temperature != NULL) {
      *temperature = response.data1;
    }
  }
  return evalue;
}

int lis3dsh_read_value(unsigned char offset, int *value) {
  int evalue;
  LIS3DSHResponse response;
  response.done = false;

  if (offset > 5) offset = 0;

  // subscribe
  lis3dsh_subscribe(command_callback_yield, &response);
  evalue = lis3dshc_command(6, offset, 0);
  if (evalue == TOCK_SUCCESS) {
    yield_for(&(response.done));
    if (value != NULL) {
      *value = response.data1;
    }
  }
  return evalue;
}
