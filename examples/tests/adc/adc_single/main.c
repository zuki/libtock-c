#include <stdio.h>

#include <adc.h>
#include <tock.h>

int main(void) {
  printf("[Tock] ADC Simple Test\n");

  // Check if ADC driver exists.
  if (!adc_is_present()) {
    printf("No ADC driver!\n");
    return -1;
  }
  printf("ADC driver exists with %d channels\n", adc_channel_count());

  // Take the actual ADC sample.
  uint16_t sample;
  int err = adc_sample_sync(0, &sample);
  if (err < 0) {
    printf("Error taking ADC sample: %i\n", err);
    return -1;
  }
  printf("ADC Reading: %i counts\n", sample);

  return 0;
}
