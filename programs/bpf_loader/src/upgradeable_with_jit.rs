paychains_sdk::declare_builtin!(
    paychains_sdk::bpf_loader_upgradeable::ID,
    paychains_bpf_loader_upgradeable_program_with_jit,
    paychains_bpf_loader_program::process_instruction_jit,
    upgradeable_with_jit::id
);
