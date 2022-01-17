paychains_sdk::declare_builtin!(
    paychains_sdk::bpf_loader::ID,
    paychains_bpf_loader_program_with_jit,
    paychains_bpf_loader_program::process_instruction_jit
);
