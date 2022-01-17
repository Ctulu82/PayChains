paychains_sdk::declare_builtin!(
    paychains_sdk::bpf_loader_upgradeable::ID,
    paychains_bpf_loader_upgradeable_program,
    paychains_bpf_loader_program::process_instruction,
    upgradeable::id
);
