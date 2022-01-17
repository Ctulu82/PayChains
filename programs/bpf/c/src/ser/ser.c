/**
 * @brief Example C-based BPF sanity rogram that prints out the parameters
 * passed to it
 */
#include <paychains_sdk.h>

extern uint64_t entrypoint(const uint8_t *input) {
  PayAccountInfo ka[2];
  PayParameters params = (PayParameters){.ka = ka};

  pay_log(__FILE__);

  if (!pay_deserialize(input, &params, PAY_ARRAY_SIZE(ka))) {
    return ERROR_INVALID_ARGUMENT;
  }

  char ka_data[] = {1, 2, 3};
  PayPubkey ka_owner;
  pay_memset(ka_owner.x, 0, SIZE_PUBKEY); // set to system program

  pay_assert(params.ka_num == 2);
  for (int i = 0; i < 2; i++) {
    pay_assert(*params.ka[i].lamports == 42);
    pay_assert(!pay_memcmp(params.ka[i].data, ka_data, 4));
    pay_assert(PayPubkey_same(params.ka[i].owner, &ka_owner));
    pay_assert(params.ka[i].is_signer == false);
    pay_assert(params.ka[i].is_writable == false);
    pay_assert(params.ka[i].executable == false);
  }

  char data[] = {4, 5, 6, 7};
  pay_assert(params.data_len = 4);
  pay_assert(!pay_memcmp(params.data, data, 4));

  return SUCCESS;
}
