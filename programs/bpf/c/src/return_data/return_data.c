/**
 * @brief return data Syscall test
 */
#include <paychains_sdk.h>

#define DATA "the quick brown fox jumps over the lazy dog"

extern uint64_t entrypoint(const uint8_t *input) {
  uint8_t buf[1024];
  PayPubkey me;

  // There should be no return data on entry
  uint64_t ret = pay_get_return_data(NULL, 0, NULL);

  pay_assert(ret == 0);

  // set some return data
  pay_set_return_data((const uint8_t*)DATA, sizeof(DATA));

  // ensure the length is correct
  ret = pay_get_return_data(NULL, 0, &me);
  pay_assert(ret == sizeof(DATA));

  // try getting a subset
  ret = pay_get_return_data(buf, 4, &me);

  pay_assert(ret == sizeof(DATA));

  pay_assert(!pay_memcmp(buf, "the ", 4));

  // try getting the whole thing
  ret = pay_get_return_data(buf, sizeof(buf), &me);

  pay_assert(ret == sizeof(DATA));

  pay_assert(!pay_memcmp(buf, (const uint8_t*)DATA, sizeof(DATA)));

  // done
  return SUCCESS;
}
