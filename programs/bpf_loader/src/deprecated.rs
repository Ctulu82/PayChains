paychains_sdk::declare_builtin!(
    paychains_sdk::bpf_loader_deprecated::ID,
    paychains_bpf_loader_deprecated_program,
    paychains_bpf_loader_program::process_instruction,
    deprecated::id
);
