/* vim: set sw=2 expandtab tw=80: */

#include <lis3dsh.h>
#include <math.h>
#include <stdio.h>
#include <timer.h>

static int decimals(float f, int n)
{
  return (int)((fabs(f - (int)(f))) * pow(10, n));
}

int main(void) {
  LIS3DSHXYZ xyz;
  int id; 
  int i;
  char cx, cy, cz;

  if (lis3dsh_is_present(&id)) {
    printf ("LIS3DSH sensor is present: id = %X\r\n", id);
    if (lis3dsh_set_power_mode(LIS3DSH_100HZ)) {
      printf ("LIS3DSH device set power mode\r\n");
      if (lis3dsh_set_scale_and_filter(LIS3DSH_SCALE_2G, LIS3DSH_FILTER_800HZ)) {
        printf ("LIS3DSH device set scale\r\n");
        for (i=0; i<10; i++)
        {
          if (lis3dsh_read_xyz(&xyz) == TOCK_SUCCESS) {
            cx = ((int)xyz.x == 0 && xyz.x < 0) ? '-' : ' ';
            cy = ((int)xyz.y == 0 && xyz.y < 0) ? '-' : ' ';
            cz = ((int)xyz.z == 0 && xyz.z < 0) ? '-' : ' ';

            printf("x: (%d, %c%d.%03d) y: (%d, %c%d.%03d) z: (%d, %c%d.%03d)\r\n", xyz.x_r, cx, (int)xyz.x, decimals(xyz.x, 3),
                    xyz.y_r, cy, (int)xyz.y, decimals(xyz.y, 3), xyz.z_r, cz, (int)xyz.z, decimals(xyz.z, 3));
            //printf("0x%04X, 0x%04X, 0x%04X\r\n", (short)xyz.x_r, (short)xyz.y_r, (short)xyz.z_r);
          } else {
            printf("Error while reading gyroscope and temperature\r\n");
          }
          delay_ms(100);
        }
      } else {
        printf("LIS3DSH device set scale failed\r\n");
      }
    } else {
      printf("LIS3DSH device set power mode failed\r\n");
    }
  } else {
    printf("LIS3DSH sensor driver not present: id = %d\r\n", id);
  }
  return 0;
}
