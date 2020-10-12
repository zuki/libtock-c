#include <stdbool.h>
#include <stdio.h>
#include <ninedof.h>
#include <tock.h>

int main(void) {
  bool ninedof_accel = false;
  int ninedof_accel_x = 0, ninedof_accel_y = 0, ninedof_accel_z = 0;

  printf("[Sensors] Starting Ninedorf App.\r\n");

  if (driver_exists(DRIVER_NUM_NINEDOF)) {
    ninedof_accel = (ninedof_read_acceleration_sync(&ninedof_accel_x, &ninedof_accel_y, &ninedof_accel_z) == TOCK_SUCCESS);
    if (ninedof_accel) {
      printf("Acceleration: X: %d Y: %d Z: %d\r\n", ninedof_accel_x, ninedof_accel_y, ninedof_accel_z);
    } else {
      printf("Ninedorf read acceleration failed\r\n");
    }
  } else {
    printf("Ninedorf driver does not exits\r\n");
  }

  return 0;
}