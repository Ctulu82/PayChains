#![feature(test)]

extern crate test;

use {
    paychains_bpf_loader_program::serialization::{
        serialize_parameters_aligned, serialize_parameters_unaligned,
    },
    paychains_sdk::{
        account::{Account, AccountSharedData},
        bpf_loader,
        transaction_context::{InstructionAccount, TransactionContext},
    },
    test::Bencher,
};

fn create_inputs() -> TransactionContext {
    let program_id = paychains_sdk::pubkey::new_rand();
    let transaction_accounts = vec![
        (
            program_id,
            AccountSharedData::from(Account {
                lamports: 0,
                data: vec![],
                owner: bpf_loader::id(),
                executable: true,
                rent_epoch: 0,
            }),
        ),
        (
            paychains_sdk::pubkey::new_rand(),
            AccountSharedData::from(Account {
                lamports: 1,
                data: vec![1u8; 100000],
                owner: bpf_loader::id(),
                executable: false,
                rent_epoch: 100,
            }),
        ),
        (
            paychains_sdk::pubkey::new_rand(),
            AccountSharedData::from(Account {
                lamports: 2,
                data: vec![11u8; 100000],
                owner: bpf_loader::id(),
                executable: true,
                rent_epoch: 200,
            }),
        ),
        (
            paychains_sdk::pubkey::new_rand(),
            AccountSharedData::from(Account {
                lamports: 3,
                data: vec![],
                owner: bpf_loader::id(),
                executable: false,
                rent_epoch: 3100,
            }),
        ),
        (
            paychains_sdk::pubkey::new_rand(),
            AccountSharedData::from(Account {
                lamports: 4,
                data: vec![1u8; 100000],
                owner: bpf_loader::id(),
                executable: false,
                rent_epoch: 100,
            }),
        ),
        (
            paychains_sdk::pubkey::new_rand(),
            AccountSharedData::from(Account {
                lamports: 5,
                data: vec![11u8; 10000],
                owner: bpf_loader::id(),
                executable: true,
                rent_epoch: 200,
            }),
        ),
        (
            paychains_sdk::pubkey::new_rand(),
            AccountSharedData::from(Account {
                lamports: 6,
                data: vec![],
                owner: bpf_loader::id(),
                executable: false,
                rent_epoch: 3100,
            }),
        ),
    ];
    let instruction_accounts = [1, 1, 2, 3, 4, 4, 5, 6]
        .into_iter()
        .enumerate()
        .map(
            |(index_in_instruction, index_in_transaction)| InstructionAccount {
                index_in_caller: 1usize.saturating_add(index_in_instruction),
                index_in_transaction,
                is_signer: false,
                is_writable: index_in_instruction >= 4,
            },
        )
        .collect::<Vec<_>>();
    let mut transaction_context = TransactionContext::new(transaction_accounts, 1);
    let instruction_data = vec![1u8, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11];
    transaction_context
        .push(&[0], &instruction_accounts, &instruction_data)
        .unwrap();
    transaction_context
}

#[bench]
fn bench_serialize_unaligned(bencher: &mut Bencher) {
    let transaction_context = create_inputs();
    let instruction_context = transaction_context
        .get_current_instruction_context()
        .unwrap();
    bencher.iter(|| {
        let _ = serialize_parameters_unaligned(&transaction_context, instruction_context).unwrap();
    });
}

#[bench]
fn bench_serialize_aligned(bencher: &mut Bencher) {
    let transaction_context = create_inputs();
    let instruction_context = transaction_context
        .get_current_instruction_context()
        .unwrap();
    bencher.iter(|| {
        let _ = serialize_parameters_aligned(&transaction_context, instruction_context).unwrap();
    });
}
