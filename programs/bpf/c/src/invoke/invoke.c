/**
 * @brief Example C-based BPF program that tests cross-program invocations
 */
#include "../invoked/instruction.h"
#include <sol/entrypoint.h>
#include <sol/cpi.h>
#include <sol/pubkey.h>
#include <sol/log.h>
#include <sol/assert.h>
#include <sol/deserialize.h>
#include <sol/return_data.h>

static const uint8_t TEST_SUCCESS = 1;
static const uint8_t TEST_PRIVILEGE_ESCALATION_SIGNER = 2;
static const uint8_t TEST_PRIVILEGE_ESCALATION_WRITABLE = 3;
static const uint8_t TEST_PPROGRAM_NOT_EXECUTABLE = 4;
static const uint8_t TEST_EMPTY_ACCOUNTS_SLICE = 5;
static const uint8_t TEST_CAP_SEEDS = 6;
static const uint8_t TEST_CAP_SIGNERS = 7;
static const uint8_t TEST_ALLOC_ACCESS_VIOLATION = 8;
static const uint8_t TEST_INSTRUCTION_DATA_TOO_LARGE = 9;
static const uint8_t TEST_INSTRUCTION_META_TOO_LARGE = 10;
static const uint8_t TEST_RETURN_ERROR = 11;
static const uint8_t TEST_PRIVILEGE_DEESCALATION_ESCALATION_SIGNER = 12;
static const uint8_t TEST_PRIVILEGE_DEESCALATION_ESCALATION_WRITABLE = 13;
static const uint8_t TEST_WRITABLE_DEESCALATION_WRITABLE = 14;
static const uint8_t TEST_NESTED_INVOKE_TOO_DEEP = 15;
static const uint8_t TEST_EXECUTABLE_LAMPORTS = 16;
static const uint8_t TEST_CALL_PRECOMPILE = 17;
static const uint8_t ADD_LAMPORTS = 18;
static const uint8_t TEST_RETURN_DATA_TOO_LARGE = 19;

static const int MINT_INDEX = 0;
static const int ARGUMENT_INDEX = 1;
static const int INVOKED_PROGRAM_INDEX = 2;
static const int INVOKED_ARGUMENT_INDEX = 3;
static const int INVOKED_PROGRAM_DUP_INDEX = 4;
static const int ARGUMENT_DUP_INDEX = 5;
static const int DERIVED_KEY1_INDEX = 6;
static const int DERIVED_KEY2_INDEX = 7;
static const int DERIVED_KEY3_INDEX = 8;
static const int SYSTEM_PROGRAM_INDEX = 9;
static const int FROM_INDEX = 10;
static const int ED25519_PROGRAM_INDEX = 11;
static const int INVOKE_PROGRAM_INDEX = 12;

uint64_t do_nested_invokes(uint64_t num_nested_invokes,
                           PayAccountInfo *accounts, uint64_t num_accounts) {
  pay_assert(accounts[ARGUMENT_INDEX].is_signer);

  *accounts[ARGUMENT_INDEX].lamports -= 5;
  *accounts[INVOKED_ARGUMENT_INDEX].lamports += 5;

  PayAccountMeta arguments[] = {
      {accounts[INVOKED_ARGUMENT_INDEX].key, true, true},
      {accounts[ARGUMENT_INDEX].key, true, true},
      {accounts[INVOKED_PROGRAM_INDEX].key, false, false}};
  uint8_t data[] = {NESTED_INVOKE, num_nested_invokes};
  const PayInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                      arguments, PAY_ARRAY_SIZE(arguments),
                                      data, PAY_ARRAY_SIZE(data)};

  pay_log("First invoke");
  pay_assert(SUCCESS == pay_invoke(&instruction, accounts, num_accounts));
  pay_log("2nd invoke from first program");
  pay_assert(SUCCESS == pay_invoke(&instruction, accounts, num_accounts));

  pay_assert(*accounts[ARGUMENT_INDEX].lamports ==
             42 - 5 + (2 * num_nested_invokes));
  pay_assert(*accounts[INVOKED_ARGUMENT_INDEX].lamports ==
             10 + 5 - (2 * num_nested_invokes));

  return SUCCESS;
}

extern uint64_t entrypoint(const uint8_t *input) {
  pay_log("Invoke C program");

  PayAccountInfo accounts[13];
  PayParameters params = (PayParameters){.ka = accounts};

  if (!pay_deserialize(input, &params, PAY_ARRAY_SIZE(accounts))) {
    return ERROR_INVALID_ARGUMENT;
  }

  uint8_t bump_seed1 = params.data[1];
  uint8_t bump_seed2 = params.data[2];
  uint8_t bump_seed3 = params.data[3];

  switch (params.data[0]) {
  case TEST_SUCCESS: {
    pay_log("Call system program create account");
    {
      uint64_t from_lamports = *accounts[FROM_INDEX].lamports;
      uint64_t to_lamports = *accounts[DERIVED_KEY1_INDEX].lamports;
      PayAccountMeta arguments[] = {
          {accounts[FROM_INDEX].key, true, true},
          {accounts[DERIVED_KEY1_INDEX].key, true, true}};
      uint8_t data[4 + 8 + 8 + 32];
      *(uint64_t *)(data + 4) = 42;
      *(uint64_t *)(data + 4 + 8) = MAX_PERMITTED_DATA_INCREASE;
      pay_memcpy(data + 4 + 8 + 8, params.program_id, SIZE_PUBKEY);
      const PayInstruction instruction = {accounts[SYSTEM_PROGRAM_INDEX].key,
                                          arguments, PAY_ARRAY_SIZE(arguments),
                                          data, PAY_ARRAY_SIZE(data)};
      uint8_t seed1[] = {'Y', 'o', 'u', ' ', 'p', 'a', 's', 's',
                         ' ', 'b', 'u', 't', 't', 'e', 'r'};
      const PaySignerSeed seeds1[] = {{seed1, PAY_ARRAY_SIZE(seed1)},
                                      {&bump_seed1, 1}};
      const PaySignerSeeds signers_seeds[] = {{seeds1, PAY_ARRAY_SIZE(seeds1)}};
      pay_assert(SUCCESS == pay_invoke_signed(&instruction, accounts,
                                              PAY_ARRAY_SIZE(accounts),
                                              signers_seeds,
                                              PAY_ARRAY_SIZE(signers_seeds)));
      pay_assert(*accounts[FROM_INDEX].lamports == from_lamports - 42);
      pay_assert(*accounts[DERIVED_KEY1_INDEX].lamports == to_lamports + 42);
      pay_assert(PayPubkey_same(accounts[DERIVED_KEY1_INDEX].owner,
                                params.program_id));
      pay_assert(accounts[DERIVED_KEY1_INDEX].data_len ==
                 MAX_PERMITTED_DATA_INCREASE);
      pay_assert(
          accounts[DERIVED_KEY1_INDEX].data[MAX_PERMITTED_DATA_INCREASE - 1] ==
          0);
      accounts[DERIVED_KEY1_INDEX].data[MAX_PERMITTED_DATA_INCREASE - 1] = 0x0f;
      pay_assert(
          accounts[DERIVED_KEY1_INDEX].data[MAX_PERMITTED_DATA_INCREASE - 1] ==
          0x0f);
      for (uint8_t i = 0; i < 20; i++) {
        accounts[DERIVED_KEY1_INDEX].data[i] = i;
      }
    }

    pay_log("Call system program transfer");
    {
      uint64_t from_lamports = *accounts[FROM_INDEX].lamports;
      uint64_t to_lamports = *accounts[DERIVED_KEY1_INDEX].lamports;
      PayAccountMeta arguments[] = {
          {accounts[FROM_INDEX].key, true, true},
          {accounts[DERIVED_KEY1_INDEX].key, true, false}};
      uint8_t data[] = {2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0};
      const PayInstruction instruction = {accounts[SYSTEM_PROGRAM_INDEX].key,
                                          arguments, PAY_ARRAY_SIZE(arguments),
                                          data, PAY_ARRAY_SIZE(data)};
      pay_assert(SUCCESS ==
                 pay_invoke(&instruction, accounts, PAY_ARRAY_SIZE(accounts)));
      pay_assert(*accounts[FROM_INDEX].lamports == from_lamports - 1);
      pay_assert(*accounts[DERIVED_KEY1_INDEX].lamports == to_lamports + 1);
    }

    pay_log("Test data translation");
    {
      for (int i = 0; i < accounts[ARGUMENT_INDEX].data_len; i++) {
        accounts[ARGUMENT_INDEX].data[i] = i;
      }

      PayAccountMeta arguments[] = {
          {accounts[ARGUMENT_INDEX].key, true, true},
          {accounts[INVOKED_ARGUMENT_INDEX].key, true, true},
          {accounts[INVOKED_PROGRAM_INDEX].key, false, false},
          {accounts[INVOKED_PROGRAM_DUP_INDEX].key, false, false}};
      uint8_t data[] = {VERIFY_TRANSLATIONS, 1, 2, 3, 4, 5};
      const PayInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                          arguments, PAY_ARRAY_SIZE(arguments),
                                          data, PAY_ARRAY_SIZE(data)};

      pay_assert(SUCCESS ==
                 pay_invoke(&instruction, accounts, PAY_ARRAY_SIZE(accounts)));
    }

    pay_log("Test no instruction data");
    {
      PayAccountMeta arguments[] = {{accounts[ARGUMENT_INDEX].key, true, true}};
      uint8_t data[] = {};
      const PayInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                          arguments, PAY_ARRAY_SIZE(arguments),
                                          data, PAY_ARRAY_SIZE(data)};

      pay_assert(SUCCESS ==
                 pay_invoke(&instruction, accounts, PAY_ARRAY_SIZE(accounts)));
    }

    pay_log("Test return data");
    {
      PayAccountMeta arguments[] = {{accounts[ARGUMENT_INDEX].key, true, true}};
      uint8_t data[] = { SET_RETURN_DATA };
      uint8_t buf[100];

      const PayInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                          arguments, PAY_ARRAY_SIZE(arguments),
                                          data, PAY_ARRAY_SIZE(data)};

      // set some return data, so that the callee can check it is cleared
      pay_set_return_data((uint8_t[]){1, 2, 3, 4}, 4);

      pay_assert(SUCCESS ==
                 pay_invoke(&instruction, accounts, PAY_ARRAY_SIZE(accounts)));

      PayPubkey setter;

      uint64_t ret = pay_get_return_data(data, sizeof(data), &setter);

      pay_assert(ret == sizeof(RETURN_DATA_VAL));

      pay_assert(pay_memcmp(data, RETURN_DATA_VAL, sizeof(RETURN_DATA_VAL)));
      pay_assert(PayPubkey_same(&setter, accounts[INVOKED_PROGRAM_INDEX].key));
    }

    pay_log("Test create_program_address");
    {
      uint8_t seed1[] = {'Y', 'o', 'u', ' ', 'p', 'a', 's', 's',
                         ' ', 'b', 'u', 't', 't', 'e', 'r'};
      const PaySignerSeed seeds1[] = {{seed1, PAY_ARRAY_SIZE(seed1)},
                                      {&bump_seed1, 1}};
      PayPubkey address;
      pay_assert(SUCCESS ==
                 pay_create_program_address(seeds1, PAY_ARRAY_SIZE(seeds1),
                                            params.program_id, &address));
      pay_assert(PayPubkey_same(&address, accounts[DERIVED_KEY1_INDEX].key));
    }

    pay_log("Test try_find_program_address");
    {
      uint8_t seed[] = {'Y', 'o', 'u', ' ', 'p', 'a', 's', 's',
                        ' ', 'b', 'u', 't', 't', 'e', 'r'};
      const PaySignerSeed seeds[] = {{seed, PAY_ARRAY_SIZE(seed)}};
      PayPubkey address;
      uint8_t bump_seed;
      pay_assert(SUCCESS == pay_try_find_program_address(
                                seeds, PAY_ARRAY_SIZE(seeds), params.program_id,
                                &address, &bump_seed));
      pay_assert(PayPubkey_same(&address, accounts[DERIVED_KEY1_INDEX].key));
      pay_assert(bump_seed == bump_seed1);
    }

    pay_log("Test derived signers");
    {
      pay_assert(!accounts[DERIVED_KEY1_INDEX].is_signer);
      pay_assert(!accounts[DERIVED_KEY2_INDEX].is_signer);
      pay_assert(!accounts[DERIVED_KEY3_INDEX].is_signer);

      PayAccountMeta arguments[] = {
          {accounts[INVOKED_PROGRAM_INDEX].key, false, false},
          {accounts[DERIVED_KEY1_INDEX].key, true, true},
          {accounts[DERIVED_KEY2_INDEX].key, true, false},
          {accounts[DERIVED_KEY3_INDEX].key, false, false}};
      uint8_t data[] = {DERIVED_SIGNERS, bump_seed2, bump_seed3};
      const PayInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                          arguments, PAY_ARRAY_SIZE(arguments),
                                          data, PAY_ARRAY_SIZE(data)};
      uint8_t seed1[] = {'Y', 'o', 'u', ' ', 'p', 'a', 's', 's',
                         ' ', 'b', 'u', 't', 't', 'e', 'r'};
      const PaySignerSeed seeds1[] = {{seed1, PAY_ARRAY_SIZE(seed1)},
                                      {&bump_seed1, 1}};
      const PaySignerSeeds signers_seeds[] = {{seeds1, PAY_ARRAY_SIZE(seeds1)}};
      pay_assert(SUCCESS == pay_invoke_signed(&instruction, accounts,
                                              PAY_ARRAY_SIZE(accounts),
                                              signers_seeds,
                                              PAY_ARRAY_SIZE(signers_seeds)));
    }

    pay_log("Test readonly with writable account");
    {
      PayAccountMeta arguments[] = {
          {accounts[INVOKED_ARGUMENT_INDEX].key, true, false}};
      uint8_t data[] = {VERIFY_WRITER};
      const PayInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                          arguments, PAY_ARRAY_SIZE(arguments),
                                          data, PAY_ARRAY_SIZE(data)};

      pay_assert(SUCCESS ==
                 pay_invoke(&instruction, accounts, PAY_ARRAY_SIZE(accounts)));
    }

    pay_log("Test nested invoke");
    {
      pay_assert(SUCCESS == do_nested_invokes(4, accounts, params.ka_num));
    }

    pay_log("Test privilege deescalation");
    {
      pay_assert(true == accounts[INVOKED_ARGUMENT_INDEX].is_signer);
      pay_assert(true == accounts[INVOKED_ARGUMENT_INDEX].is_writable);
      PayAccountMeta arguments[] = {
          {accounts[INVOKED_ARGUMENT_INDEX].key, false, false}};
      uint8_t data[] = {VERIFY_PRIVILEGE_DEESCALATION};
      const PayInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                          arguments, PAY_ARRAY_SIZE(arguments),
                                          data, PAY_ARRAY_SIZE(data)};
      pay_assert(SUCCESS ==
                 pay_invoke(&instruction, accounts, PAY_ARRAY_SIZE(accounts)));
    }

    pay_log("Verify data values are retained and updated");
    for (int i = 0; i < accounts[ARGUMENT_INDEX].data_len; i++) {
      pay_assert(accounts[ARGUMENT_INDEX].data[i] == i);
    }
    for (int i = 0; i < accounts[INVOKED_ARGUMENT_INDEX].data_len; i++) {
      pay_assert(accounts[INVOKED_ARGUMENT_INDEX].data[i] == i);
    }

    pay_log("Verify data write before ro cpi call");
    {
      for (int i = 0; i < accounts[ARGUMENT_INDEX].data_len; i++) {
        accounts[ARGUMENT_INDEX].data[i] = 0;
      }

      PayAccountMeta arguments[] = {
          {accounts[ARGUMENT_INDEX].key, false, false}};
      uint8_t data[] = {VERIFY_PRIVILEGE_DEESCALATION};
      const PayInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                          arguments, PAY_ARRAY_SIZE(arguments),
                                          data, PAY_ARRAY_SIZE(data)};
      pay_assert(SUCCESS ==
                 pay_invoke(&instruction, accounts, PAY_ARRAY_SIZE(accounts)));

      for (int i = 0; i < accounts[ARGUMENT_INDEX].data_len; i++) {
        pay_assert(accounts[ARGUMENT_INDEX].data[i] == 0);
      }
    }
    break;
  }
  case TEST_PRIVILEGE_ESCALATION_SIGNER: {
    pay_log("Test privilege escalation signer");
    PayAccountMeta arguments[] = {
        {accounts[DERIVED_KEY3_INDEX].key, false, false}};
    uint8_t data[] = {VERIFY_PRIVILEGE_ESCALATION};
    const PayInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, PAY_ARRAY_SIZE(arguments),
                                        data, PAY_ARRAY_SIZE(data)};
    pay_assert(SUCCESS ==
               pay_invoke(&instruction, accounts, PAY_ARRAY_SIZE(accounts)));

    // Signer privilege escalation will always fail the whole transaction
    instruction.accounts[0].is_signer = true;
    pay_invoke(&instruction, accounts, PAY_ARRAY_SIZE(accounts));
    break;
  }
  case TEST_PRIVILEGE_ESCALATION_WRITABLE: {
    pay_log("Test privilege escalation writable");
    PayAccountMeta arguments[] = {
        {accounts[DERIVED_KEY3_INDEX].key, false, false}};
    uint8_t data[] = {VERIFY_PRIVILEGE_ESCALATION};
    const PayInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, PAY_ARRAY_SIZE(arguments),
                                        data, PAY_ARRAY_SIZE(data)};
    pay_assert(SUCCESS ==
               pay_invoke(&instruction, accounts, PAY_ARRAY_SIZE(accounts)));

    // Writable privilege escalation will always fail the whole transaction
    instruction.accounts[0].is_writable = true;
    pay_invoke(&instruction, accounts, PAY_ARRAY_SIZE(accounts));
    break;
  }
  case TEST_PPROGRAM_NOT_EXECUTABLE: {
    pay_log("Test program not executable");
    PayAccountMeta arguments[] = {
        {accounts[DERIVED_KEY3_INDEX].key, false, false}};
    uint8_t data[] = {VERIFY_PRIVILEGE_ESCALATION};
    const PayInstruction instruction = {accounts[ARGUMENT_INDEX].key, arguments,
                                        PAY_ARRAY_SIZE(arguments), data,
                                        PAY_ARRAY_SIZE(data)};
    return pay_invoke(&instruction, accounts, PAY_ARRAY_SIZE(accounts));
  }
  case TEST_EMPTY_ACCOUNTS_SLICE: {
    pay_log("Empty accounts slice");

    PayAccountMeta arguments[] = {
        {accounts[INVOKED_ARGUMENT_INDEX].key, false, false}};
    uint8_t data[] = {};
    const PayInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, PAY_ARRAY_SIZE(arguments),
                                        data, PAY_ARRAY_SIZE(data)};

    pay_assert(SUCCESS == pay_invoke(&instruction, 0, 0));
    break;
  }
  case TEST_CAP_SEEDS: {
    pay_log("Test cap seeds");
    PayAccountMeta arguments[] = {};
    uint8_t data[] = {};
    const PayInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, PAY_ARRAY_SIZE(arguments),
                                        data, PAY_ARRAY_SIZE(data)};
    uint8_t seed[] = {"seed"};
    const PaySignerSeed seeds[] = {
        {seed, PAY_ARRAY_SIZE(seed)}, {seed, PAY_ARRAY_SIZE(seed)},
        {seed, PAY_ARRAY_SIZE(seed)}, {seed, PAY_ARRAY_SIZE(seed)},
        {seed, PAY_ARRAY_SIZE(seed)}, {seed, PAY_ARRAY_SIZE(seed)},
        {seed, PAY_ARRAY_SIZE(seed)}, {seed, PAY_ARRAY_SIZE(seed)},
        {seed, PAY_ARRAY_SIZE(seed)}, {seed, PAY_ARRAY_SIZE(seed)},
        {seed, PAY_ARRAY_SIZE(seed)}, {seed, PAY_ARRAY_SIZE(seed)},
        {seed, PAY_ARRAY_SIZE(seed)}, {seed, PAY_ARRAY_SIZE(seed)},
        {seed, PAY_ARRAY_SIZE(seed)}, {seed, PAY_ARRAY_SIZE(seed)},
        {seed, PAY_ARRAY_SIZE(seed)},
    };
    const PaySignerSeeds signers_seeds[] = {{seeds, PAY_ARRAY_SIZE(seeds)}};
    pay_assert(SUCCESS == pay_invoke_signed(
                              &instruction, accounts, PAY_ARRAY_SIZE(accounts),
                              signers_seeds, PAY_ARRAY_SIZE(signers_seeds)));
    break;
  }
  case TEST_CAP_SIGNERS: {
    pay_log("Test cap signers");
    PayAccountMeta arguments[] = {};
    uint8_t data[] = {};
    const PayInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, PAY_ARRAY_SIZE(arguments),
                                        data, PAY_ARRAY_SIZE(data)};
    uint8_t seed[] = {"seed"};
    const PaySignerSeed seed1[] = {{seed, PAY_ARRAY_SIZE(seed)}};
    const PaySignerSeed seed2[] = {{seed, PAY_ARRAY_SIZE(seed)}};
    const PaySignerSeed seed3[] = {{seed, PAY_ARRAY_SIZE(seed)}};
    const PaySignerSeed seed4[] = {{seed, PAY_ARRAY_SIZE(seed)}};
    const PaySignerSeed seed5[] = {{seed, PAY_ARRAY_SIZE(seed)}};
    const PaySignerSeed seed6[] = {{seed, PAY_ARRAY_SIZE(seed)}};
    const PaySignerSeed seed7[] = {{seed, PAY_ARRAY_SIZE(seed)}};
    const PaySignerSeed seed8[] = {{seed, PAY_ARRAY_SIZE(seed)}};
    const PaySignerSeed seed9[] = {{seed, PAY_ARRAY_SIZE(seed)}};
    const PaySignerSeed seed10[] = {{seed, PAY_ARRAY_SIZE(seed)}};
    const PaySignerSeed seed11[] = {{seed, PAY_ARRAY_SIZE(seed)}};
    const PaySignerSeed seed12[] = {{seed, PAY_ARRAY_SIZE(seed)}};
    const PaySignerSeed seed13[] = {{seed, PAY_ARRAY_SIZE(seed)}};
    const PaySignerSeed seed14[] = {{seed, PAY_ARRAY_SIZE(seed)}};
    const PaySignerSeed seed15[] = {{seed, PAY_ARRAY_SIZE(seed)}};
    const PaySignerSeed seed16[] = {{seed, PAY_ARRAY_SIZE(seed)}};
    const PaySignerSeed seed17[] = {{seed, PAY_ARRAY_SIZE(seed)}};
    const PaySignerSeeds signers_seeds[] = {
        {seed1, PAY_ARRAY_SIZE(seed1)},   {seed2, PAY_ARRAY_SIZE(seed2)},
        {seed3, PAY_ARRAY_SIZE(seed3)},   {seed4, PAY_ARRAY_SIZE(seed4)},
        {seed5, PAY_ARRAY_SIZE(seed5)},   {seed6, PAY_ARRAY_SIZE(seed6)},
        {seed7, PAY_ARRAY_SIZE(seed7)},   {seed8, PAY_ARRAY_SIZE(seed8)},
        {seed9, PAY_ARRAY_SIZE(seed9)},   {seed10, PAY_ARRAY_SIZE(seed10)},
        {seed11, PAY_ARRAY_SIZE(seed11)}, {seed12, PAY_ARRAY_SIZE(seed12)},
        {seed13, PAY_ARRAY_SIZE(seed13)}, {seed14, PAY_ARRAY_SIZE(seed14)},
        {seed15, PAY_ARRAY_SIZE(seed15)}, {seed16, PAY_ARRAY_SIZE(seed16)},
        {seed17, PAY_ARRAY_SIZE(seed17)}};
    pay_assert(SUCCESS == pay_invoke_signed(
                              &instruction, accounts, PAY_ARRAY_SIZE(accounts),
                              signers_seeds, PAY_ARRAY_SIZE(signers_seeds)));
    break;
  }
  case TEST_ALLOC_ACCESS_VIOLATION: {
    pay_log("Test resize violation");
    PayAccountMeta arguments[] = {
        {accounts[FROM_INDEX].key, true, true},
        {accounts[DERIVED_KEY1_INDEX].key, true, true}};
    uint8_t data[4 + 8 + 8 + 32];
    *(uint64_t *)(data + 4) = 42;
    *(uint64_t *)(data + 4 + 8) = MAX_PERMITTED_DATA_INCREASE;
    pay_memcpy(data + 4 + 8 + 8, params.program_id, SIZE_PUBKEY);
    const PayInstruction instruction = {accounts[SYSTEM_PROGRAM_INDEX].key,
                                        arguments, PAY_ARRAY_SIZE(arguments),
                                        data, PAY_ARRAY_SIZE(data)};
    uint8_t seed1[] = {'Y', 'o', 'u', ' ', 'p', 'a', 's', 's',
                       ' ', 'b', 'u', 't', 't', 'e', 'r'};
    const PaySignerSeed seeds1[] = {{seed1, PAY_ARRAY_SIZE(seed1)},
                                    {&bump_seed1, 1}};
    const PaySignerSeeds signers_seeds[] = {{seeds1, PAY_ARRAY_SIZE(seeds1)}};

    PayAccountInfo derived_account = {
        .key = accounts[DERIVED_KEY1_INDEX].key,
        .lamports = accounts[DERIVED_KEY1_INDEX].lamports,
        .data_len = accounts[DERIVED_KEY1_INDEX].data_len,
        // Point to top edge of heap, attempt to allocate into unprivileged
        // memory
        .data = (uint8_t *)0x300007ff8,
        .owner = accounts[DERIVED_KEY1_INDEX].owner,
        .rent_epoch = accounts[DERIVED_KEY1_INDEX].rent_epoch,
        .is_signer = accounts[DERIVED_KEY1_INDEX].is_signer,
        .is_writable = accounts[DERIVED_KEY1_INDEX].is_writable,
        .executable = accounts[DERIVED_KEY1_INDEX].executable,
    };
    const PayAccountInfo invoke_accounts[] = {
        accounts[FROM_INDEX], accounts[SYSTEM_PROGRAM_INDEX], derived_account};
    pay_assert(SUCCESS ==
               pay_invoke_signed(&instruction,
                                 (const PayAccountInfo *)invoke_accounts, 3,
                                 signers_seeds, PAY_ARRAY_SIZE(signers_seeds)));
    break;
  }
  case TEST_INSTRUCTION_DATA_TOO_LARGE: {
    pay_log("Test instruction data too large");
    PayAccountMeta arguments[] = {};
    uint8_t *data = pay_calloc(1500, 1);
    const PayInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, PAY_ARRAY_SIZE(arguments),
                                        data, 1500};
    const PaySignerSeeds signers_seeds[] = {};
    pay_assert(SUCCESS == pay_invoke_signed(
                              &instruction, accounts, PAY_ARRAY_SIZE(accounts),
                              signers_seeds, PAY_ARRAY_SIZE(signers_seeds)));

    break;
  }
  case TEST_INSTRUCTION_META_TOO_LARGE: {
    pay_log("Test instruction meta too large");
    PayAccountMeta *arguments = pay_calloc(40, sizeof(PayAccountMeta));
    pay_log_64(0, 0, 0, 0, (uint64_t)arguments);
    pay_assert(0 != arguments);
    uint8_t data[] = {};
    const PayInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, 40, data,
                                        PAY_ARRAY_SIZE(data)};
    const PaySignerSeeds signers_seeds[] = {};
    pay_assert(SUCCESS == pay_invoke_signed(
                              &instruction, accounts, PAY_ARRAY_SIZE(accounts),
                              signers_seeds, PAY_ARRAY_SIZE(signers_seeds)));

    break;
  }
  case TEST_RETURN_ERROR: {
    pay_log("Test return error");
    PayAccountMeta arguments[] = {{accounts[ARGUMENT_INDEX].key, false, true}};
    uint8_t data[] = {RETURN_ERROR};
    const PayInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, PAY_ARRAY_SIZE(arguments),
                                        data, PAY_ARRAY_SIZE(data)};

    pay_invoke(&instruction, accounts, PAY_ARRAY_SIZE(accounts));
    break;
  }
  case TEST_PRIVILEGE_DEESCALATION_ESCALATION_SIGNER: {
    pay_log("Test privilege deescalation escalation signer");
    pay_assert(true == accounts[INVOKED_ARGUMENT_INDEX].is_signer);
    pay_assert(true == accounts[INVOKED_ARGUMENT_INDEX].is_writable);
    PayAccountMeta arguments[] = {
        {accounts[INVOKED_PROGRAM_INDEX].key, false, false},
        {accounts[INVOKED_ARGUMENT_INDEX].key, false, false}};
    uint8_t data[] = {VERIFY_PRIVILEGE_DEESCALATION_ESCALATION_SIGNER};
    const PayInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, PAY_ARRAY_SIZE(arguments),
                                        data, PAY_ARRAY_SIZE(data)};
    pay_assert(SUCCESS ==
               pay_invoke(&instruction, accounts, PAY_ARRAY_SIZE(accounts)));
    break;
  }
  case TEST_PRIVILEGE_DEESCALATION_ESCALATION_WRITABLE: {
    pay_log("Test privilege deescalation escalation writable");
    pay_assert(true == accounts[INVOKED_ARGUMENT_INDEX].is_signer);
    pay_assert(true == accounts[INVOKED_ARGUMENT_INDEX].is_writable);
    PayAccountMeta arguments[] = {
        {accounts[INVOKED_PROGRAM_INDEX].key, false, false},
        {accounts[INVOKED_ARGUMENT_INDEX].key, false, false}};
    uint8_t data[] = {VERIFY_PRIVILEGE_DEESCALATION_ESCALATION_WRITABLE};
    const PayInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, PAY_ARRAY_SIZE(arguments),
                                        data, PAY_ARRAY_SIZE(data)};
    pay_assert(SUCCESS ==
               pay_invoke(&instruction, accounts, PAY_ARRAY_SIZE(accounts)));
    break;
  }
  case TEST_WRITABLE_DEESCALATION_WRITABLE: {
    pay_log("Test writable deescalation");
    uint8_t buffer[10];
    for (int i = 0; i < 10; i++) {
      buffer[i] = accounts[INVOKED_ARGUMENT_INDEX].data[i];
    }
    PayAccountMeta arguments[] = {
        {accounts[INVOKED_ARGUMENT_INDEX].key, false, false}};
    uint8_t data[] = {WRITE_ACCOUNT, 10};
    const PayInstruction instruction = {accounts[INVOKED_PROGRAM_INDEX].key,
                                        arguments, PAY_ARRAY_SIZE(arguments),
                                        data, PAY_ARRAY_SIZE(data)};
    pay_invoke(&instruction, accounts, PAY_ARRAY_SIZE(accounts));

    for (int i = 0; i < 10; i++) {
      pay_assert(buffer[i] == accounts[INVOKED_ARGUMENT_INDEX].data[i]);
    }
    break;
  }
  case TEST_NESTED_INVOKE_TOO_DEEP: {
    do_nested_invokes(5, accounts, params.ka_num);
    break;
  }
  case TEST_EXECUTABLE_LAMPORTS: {
    pay_log("Test executable lamports");
    accounts[ARGUMENT_INDEX].executable = true;
    *accounts[ARGUMENT_INDEX].lamports -= 1;
    *accounts[DERIVED_KEY1_INDEX].lamports +=1;
    PayAccountMeta arguments[] = {
      {accounts[ARGUMENT_INDEX].key, true, false},
      {accounts[DERIVED_KEY1_INDEX].key, true, false},
    };
    uint8_t data[] = {ADD_LAMPORTS, 0, 0, 0};
    PayPubkey program_id;
    pay_memcpy(&program_id, params.program_id, sizeof(PayPubkey));
    const PayInstruction instruction = {&program_id,
					arguments, PAY_ARRAY_SIZE(arguments),
					data, PAY_ARRAY_SIZE(data)};
    pay_invoke(&instruction, accounts, PAY_ARRAY_SIZE(accounts));
    *accounts[ARGUMENT_INDEX].lamports += 1;
    break;
  }
  case TEST_CALL_PRECOMPILE: {
    pay_log("Test calling precompile from cpi");
    PayAccountMeta arguments[] = {};
    uint8_t data[] = {};
    const PayInstruction instruction = {accounts[ED25519_PROGRAM_INDEX].key,
					arguments, PAY_ARRAY_SIZE(arguments),
					data, PAY_ARRAY_SIZE(data)};
    pay_invoke(&instruction, accounts, PAY_ARRAY_SIZE(accounts));
    break;
  }
  case ADD_LAMPORTS: {
    *accounts[0].lamports += 1;
     break;
  }
  case TEST_RETURN_DATA_TOO_LARGE: {
    pay_log("Test setting return data too long");
    // The actual buffer doesn't matter, just pass null
    pay_set_return_data(NULL, 1027);
    break;
  }

  default:
    pay_panic();
  }

  return SUCCESS;
}
