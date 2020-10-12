/*LIS3DSH 3 axis accelerometer and temperature sensor
*
* <https://www.st.com/resource/en/datasheet/lis3dsh.pdf>
*
*/

#pragma once

#include "tock.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DRIVER_NUM_LIS3DSH 0x70223

// Accelerometer Data Rate
// Manual page Table 20, page 25
#define LIS3DSH_OFF 0
#define LIS3DSH_3_125HZ 1
#define LIS3DSH_6_26HZ 2
#define LIS3DSH_12_5HZ 3
#define LIS3DSH_25HZ	4
#define LIS3DSH_50HZ 5
#define LIS3DSH_100HZ 6
#define LIS3DSH_400HZ 7
#define LIS3DSH_800HZ 8
#define LIS3DSH_1600HZ 9

#define LIS3DSH_SCALE_2G 0
#define LIS3DSH_SCALE_4G 1
#define LIS3DSH_SCALE_6G 2
#define LIS3DSH_SCALE_8G 3
#define LIS3DSH_SCALE_16G 4

#define LIS3DSH_FILTER_800HZ 0
#define LIS3DSH_FILTER_400HZ 1
#define LIS3DSH_FILTER_200HZ 2
#define LIS3DSH_FILTER_50HZ 3

typedef struct lis3dshxyz {
	int x_r;
	int y_r;
	int z_r;
	float x;
	float y;
	float z;
} LIS3DSHXYZ;

bool lis3dsh_is_present(int *id);
bool lis3dsh_set_power_mode(unsigned char mode);
bool lis3dsh_set_scale_and_filter(unsigned char scale, unsigned char filter);
int lis3dsh_read_xyz(LIS3DSHXYZ *xyz);
int lis3dsh_read_temperature(int *temperature);

#ifdef __cplusplus
}
#endif
