#pragma once
/**
 * @brief PayChains logging utilities
 */

#include <sol/types.h>
#include <sol/string.h>
#include <sol/entrypoint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Prints a string to stdout
 */
void pay_log_(const char *, uint64_t);
#define pay_log(message) pay_log_(message, pay_strlen(message))

/**
 * Prints a 64 bit values represented in hexadecimal to stdout
 */
void pay_log_64_(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);
#define pay_log_64 pay_log_64_

/**
 * Prints the current compute unit consumption to stdout
 */
void pay_log_compute_units_();
#define pay_log_compute_units() pay_log_compute_units_()

/**
 * Prints the hexadecimal representation of an array
 *
 * @param array The array to print
 */
static void pay_log_array(const uint8_t *array, int len) {
  for (int j = 0; j < len; j++) {
    pay_log_64(0, 0, 0, j, array[j]);
  }
}

/**
 * Print the base64 representation of some arrays.
 */
void pay_log_data(PayBytes *fields, uint64_t fields_len);

/**
 * Prints the program's input parameters
 *
 * @param params Pointer to a PayParameters structure
 */
static void pay_log_params(const PayParameters *params) {
  pay_log("- Program identifier:");
  pay_log_pubkey(params->program_id);

  pay_log("- Number of KeyedAccounts");
  pay_log_64(0, 0, 0, 0, params->ka_num);
  for (int i = 0; i < params->ka_num; i++) {
    pay_log("  - Is signer");
    pay_log_64(0, 0, 0, 0, params->ka[i].is_signer);
    pay_log("  - Is writable");
    pay_log_64(0, 0, 0, 0, params->ka[i].is_writable);
    pay_log("  - Key");
    pay_log_pubkey(params->ka[i].key);
    pay_log("  - Lamports");
    pay_log_64(0, 0, 0, 0, *params->ka[i].lamports);
    pay_log("  - data");
    pay_log_array(params->ka[i].data, params->ka[i].data_len);
    pay_log("  - Owner");
    pay_log_pubkey(params->ka[i].owner);
    pay_log("  - Executable");
    pay_log_64(0, 0, 0, 0, params->ka[i].executable);
    pay_log("  - Rent Epoch");
    pay_log_64(0, 0, 0, 0, params->ka[i].rent_epoch);
  }
  pay_log("- Instruction data\0");
  pay_log_array(params->data, params->data_len);
}

#ifdef PAY_TEST
/**
 * Stub functions when building tests
 */
#include <stdio.h>

void pay_log_(const char *s, uint64_t len) {
  printf("Program log: %s\n", s);
}
void pay_log_64(uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
  printf("Program log: %llu, %llu, %llu, %llu, %llu\n", arg1, arg2, arg3, arg4, arg5);
}

void pay_log_compute_units_() {
  printf("Program consumption: __ units remaining\n");
}
#endif

#ifdef __cplusplus
}
#endif

/**@}*/
