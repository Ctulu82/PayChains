/**
 * @brief Example C-based BPF program that prints out the parameters
 * passed to it
 */
#include <paychains_sdk.h>

extern uint64_t entrypoint(const uint8_t *input) {
  pay_panic();
  return SUCCESS;
}
