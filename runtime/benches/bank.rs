#![feature(test)]
#![allow(clippy::integer_arithmetic)]

extern crate test;

use {
    log::*,
    paychains_program_runtime::invoke_context::InvokeContext,
    paychains_runtime::{bank::*, bank_client::BankClient, loader_utils::create_invoke_instruction},
    paychains_sdk::{
        client::{AsyncClient, SyncClient},
        clock::MAX_RECENT_BLOCKHASHES,
        genesis_config::create_genesis_config,
        instruction::InstructionError,
        message::Message,
        pubkey::Pubkey,
        signature::{Keypair, Signer},
        transaction::Transaction,
    },
    paychains_vote_program::{
        vote_instruction,
        vote_state::{Vote, VoteInit},
    },
    std::{sync::Arc, thread::sleep, time::Duration},
    test::Bencher,
};

const BUILTIN_PROGRAM_ID: [u8; 32] = [
    98, 117, 105, 108, 116, 105, 110, 95, 112, 114, 111, 103, 114, 97, 109, 95, 105, 100, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
];

const NOOP_PROGRAM_ID: [u8; 32] = [
    98, 117, 105, 108, 116, 105, 110, 95, 112, 114, 111, 103, 114, 97, 109, 95, 105, 100, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
];

#[allow(clippy::unnecessary_wraps)]
fn process_instruction(
    _first_instruction_account: usize,
    _data: &[u8],
    _invoke_context: &mut InvokeContext,
) -> Result<(), InstructionError> {
    Ok(())
}

pub fn create_builtin_transactions(
    bank_client: &BankClient,
    mint_keypair: &Keypair,
) -> Vec<Transaction> {
    let program_id = Pubkey::new(&BUILTIN_PROGRAM_ID);

    (0..4096)
        .map(|_| {
            // Seed the signer account
            let rando0 = Keypair::new();
            bank_client
                .transfer_and_confirm(10_000, mint_keypair, &rando0.pubkey())
                .unwrap_or_else(|_| panic!("{}:{}", line!(), file!()));

            let instruction = create_invoke_instruction(rando0.pubkey(), program_id, &1u8);
            let blockhash = bank_client.get_latest_blockhash().unwrap();
            let message = Message::new(&[instruction], Some(&mint_keypair.pubkey()));
            Transaction::new(&[&rando0], message, blockhash)
        })
        .collect()
}

pub fn create_vote_transactions(
    bank_client: &BankClient,
    mint_keypair: &Keypair,
) -> Vec<Transaction> {
    let blockhash = bank_client.get_latest_blockhash().unwrap();
    (0..4096)
        .map(|_| {
            // Seed the signer account
            let payer_keypair = Keypair::new();
            bank_client
                .transfer_and_confirm(27_000_000, mint_keypair, &payer_keypair.pubkey())
                .unwrap_or_else(|_| panic!("{}:{}", line!(), file!()));

            // Setup vote
            let vote_keypair = Keypair::new();
            let instructions = vote_instruction::create_account(
                &payer_keypair.pubkey(),
                &vote_keypair.pubkey(),
                &VoteInit {
                    node_pubkey: payer_keypair.pubkey(),
                    authorized_voter: payer_keypair.pubkey(),
                    authorized_withdrawer: payer_keypair.pubkey(),
                    commission: 100u8,
                },
                26_858_640,
            );
            let message = Message::new(&instructions, Some(&payer_keypair.pubkey()));
            bank_client
                .send_and_confirm_message(&[&payer_keypair, &vote_keypair], message)
                .unwrap();

            let vote_ix = vote_instruction::vote(
                &vote_keypair.pubkey(),
                &payer_keypair.pubkey(),
                Vote {
                    slots: vec![0],
                    hash: blockhash,
                    timestamp: None,
                },
            );
            let message = Message::new(&[vote_ix], Some(&payer_keypair.pubkey()));
            Transaction::new(&[&payer_keypair], message, blockhash)
        })
        .collect()
}

pub fn create_native_loader_transactions(
    bank_client: &BankClient,
    mint_keypair: &Keypair,
) -> Vec<Transaction> {
    let program_id = Pubkey::new(&NOOP_PROGRAM_ID);

    (0..4096)
        .map(|_| {
            // Seed the signer account©41
            let rando0 = Keypair::new();
            bank_client
                .transfer_and_confirm(10_000, mint_keypair, &rando0.pubkey())
                .unwrap_or_else(|_| panic!("{}:{}", line!(), file!()));

            let instruction = create_invoke_instruction(rando0.pubkey(), program_id, &1u8);
            let blockhash = bank_client.get_latest_blockhash().unwrap();
            let message = Message::new(&[instruction], Some(&mint_keypair.pubkey()));
            Transaction::new(&[&rando0], message, blockhash)
        })
        .collect()
}

fn sync_bencher(bank: &Arc<Bank>, _bank_client: &BankClient, transactions: &[Transaction]) {
    let results = bank.process_transactions(transactions.iter());
    assert!(results.iter().all(Result::is_ok));
}

fn async_bencher(bank: &Arc<Bank>, bank_client: &BankClient, transactions: &[Transaction]) {
    for transaction in transactions.to_owned() {
        bank_client.async_send_transaction(transaction).unwrap();
    }
    for _ in 0..1_000_000_000_u64 {
        if bank
            .get_signature_status(transactions.last().unwrap().signatures.get(0).unwrap())
            .is_some()
        {
            break;
        }
        sleep(Duration::from_nanos(1));
    }
    if bank
        .get_signature_status(transactions.last().unwrap().signatures.get(0).unwrap())
        .unwrap()
        .is_err()
    {
        error!(
            "transaction failed: {:?}",
            bank.get_signature_status(transactions.last().unwrap().signatures.get(0).unwrap())
                .unwrap()
        );
        panic!();
    }
}

fn do_bench_transactions(
    bencher: &mut Bencher,
    bench_work: &dyn Fn(&Arc<Bank>, &BankClient, &[Transaction]),
    create_transactions: &dyn Fn(&BankClient, &Keypair) -> Vec<Transaction>,
) {
    paychains_logger::setup();
    let ns_per_s = 1_000_000_000;
    let (mut genesis_config, mint_keypair) = create_genesis_config(100_000_000_000_000);
    genesis_config.ticks_per_slot = 100;

    let bank = Bank::new_for_benches(&genesis_config);
    // freeze bank so that slot hashes is populated
    bank.freeze();

    let mut bank = Bank::new_from_parent(&Arc::new(bank), &Pubkey::default(), 1);
    bank.add_builtin(
        "builtin_program",
        &Pubkey::new(&BUILTIN_PROGRAM_ID),
        process_instruction,
    );
    bank.add_builtin_account("paychains_noop_program", &Pubkey::new(&NOOP_PROGRAM_ID), false);
    let bank = Arc::new(bank);
    let bank_client = BankClient::new_shared(&bank);
    let transactions = create_transactions(&bank_client, &mint_keypair);

    // Do once to fund accounts, load modules, etc...
    let results = bank.process_transactions(transactions.iter());
    assert!(results.iter().all(Result::is_ok));

    bencher.iter(|| {
        // Since bencher runs this multiple times, we need to clear the signatures.
        bank.clear_signatures();
        bench_work(&bank, &bank_client, &transactions);
    });

    let summary = bencher.bench(|_bencher| {}).unwrap();
    info!("  {:?} transactions", transactions.len());
    info!("  {:?} ns/iter median", summary.median as u64);
    assert!(0f64 != summary.median);
    let tps = transactions.len() as u64 * (ns_per_s / summary.median as u64);
    info!("  {:?} TPS", tps);
}

#[bench]
#[ignore]
fn bench_bank_sync_process_builtin_transactions(bencher: &mut Bencher) {
    do_bench_transactions(bencher, &sync_bencher, &create_builtin_transactions);
}

#[bench]
#[ignore]
fn bench_bank_sync_process_native_loader_transactions(bencher: &mut Bencher) {
    do_bench_transactions(bencher, &sync_bencher, &create_native_loader_transactions);
}

#[bench]
#[ignore]
fn bench_bank_sync_process_vote_transactions(bencher: &mut Bencher) {
    do_bench_transactions(bencher, &sync_bencher, &create_vote_transactions);
}

#[bench]
#[ignore]
fn bench_bank_async_process_builtin_transactions(bencher: &mut Bencher) {
    do_bench_transactions(bencher, &async_bencher, &create_builtin_transactions);
}

#[bench]
#[ignore]
fn bench_bank_async_process_native_loader_transactions(bencher: &mut Bencher) {
    do_bench_transactions(bencher, &async_bencher, &create_native_loader_transactions);
}

#[bench]
#[ignore]
fn bench_bank_update_recent_blockhashes(bencher: &mut Bencher) {
    let (genesis_config, _mint_keypair) = create_genesis_config(100);
    let mut bank = Arc::new(Bank::new_for_benches(&genesis_config));
    goto_end_of_slot(Arc::get_mut(&mut bank).unwrap());
    let genesis_hash = bank.last_blockhash();
    // Prime blockhash_queue
    for i in 0..(MAX_RECENT_BLOCKHASHES + 1) {
        bank = Arc::new(Bank::new_from_parent(
            &bank,
            &Pubkey::default(),
            (i + 1) as u64,
        ));
        goto_end_of_slot(Arc::get_mut(&mut bank).unwrap());
    }
    // Verify blockhash_queue is full (genesis hash has been kicked out)
    assert_eq!(
        Some(false),
        bank.check_hash_age(&genesis_hash, MAX_RECENT_BLOCKHASHES)
    );
    bencher.iter(|| {
        bank.update_recent_blockhashes();
    });
}
