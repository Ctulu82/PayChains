/**
 * @brief test program that generates BPF PC relative call instructions
 */
#include <paychains_sdk.h>

void __attribute__ ((noinline)) helper() {
  pay_log(__func__);
}

extern uint64_t entrypoint(const uint8_t *input) {
  pay_log(__func__);
  helper();
  return SUCCESS;
}
