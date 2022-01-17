/**
 * @brief Example C-based BPF program uses pay_log_data
 */
#include <paychains_sdk.h>

static const uint8_t return_data[] = { 0x08, 0x01, 0x44 };

extern uint64_t entrypoint(const uint8_t *input) {
  PayAccountInfo ka[1];
  PayParameters params = (PayParameters) { .ka = ka };
  PayBytes fields[2];

  if (!pay_deserialize(input, &params, PAY_ARRAY_SIZE(ka))) {
    return ERROR_INVALID_ARGUMENT;
  }

  // Generate two fields, split at the first 0 in the input
  fields[0].addr = params.data;
  fields[0].len = pay_strlen((char*)fields[0].addr);
  fields[1].addr = fields[0].addr + fields[0].len + 1;
  fields[1].len = params.data_len - fields[0].len - 1;

  pay_set_return_data(return_data, sizeof(return_data));

  pay_log_data(fields, 2);

  return SUCCESS;
}
