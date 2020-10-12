#include <limits.h>
#include <stdio.h>

#include <led.h>
#include <ninedof.h>

int main(void) {
  printf("[App] Accelerometer -> LEDs\r\n");

  while (1) {
    int x, y, z; // 単位はmg: 1G = 1000
    ninedof_read_acceleration_sync(&x, &y, &z);

    // LED: [Green, Orange, Red, Blue]
    if (x > 100) {
      led_off(0);
      led_on(2);
    } else if( x < -100) {
      led_on(0);
      led_off(2);
    } else {
      led_off(0);
      led_off(2);
    }
    if (y > 100) {
      led_on(1);
      led_off(3);
    } else if (y < -100) {
      led_off(1);
      led_on(3);
    } else {
      led_off(1);
      led_off(3);
    }
  }

  return 0;
}
