/**
 * @brief Example C++ based BPF program that prints out the parameters
 * passed to it
 */
#include <paychains_sdk.h>

extern uint64_t entrypoint(const uint8_t *input) {
  PayAccountInfo ka[1];
  PayParameters params = (PayParameters) { .ka = ka };

  if (!pay_deserialize(input, &params, PAY_ARRAY_SIZE(ka))) {
    return ERROR_INVALID_ARGUMENT;
  }

  return SUCCESS;
}
