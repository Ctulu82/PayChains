---
title: Rust API
---

PayChains's Rust crates are [published to crates.io][crates.io] and can be found
[on docs.rs with the "paychains-" prefix][docs.rs].

[crates.io]: https://crates.io/search?q=paychains-
[docs.rs]: https://docs.rs/releases/search?query=paychains-

Some important crates:

- [`paychains-program`] &mdash; Imported by programs running on PayChains, compiled
  to BPF. This crate contains many fundamental data types and is re-exported from
  [`paychains-sdk`], which cannot be imported from a PayChains program.

- [`paychains-sdk`] &mdash; The basic off-chain SDK, it re-exports
  [`paychains-program`] and adds more APIs on top of that. Most PayChains programs
  that do not run on-chain will import this.

- [`paychains-client`] &mdash; For interacting with a PayChains node via the
  [JSON RPC API](jsonrpc-api).

- [`paychains-cli-config`] &mdash; Loading and saving the PayChains CLI configuration
  file.

- [`paychains-clap-utils`] &mdash; Routines for setting up a CLI, using [`clap`],
  as used by the main PayChains CLI. Includes functions for loading all types of
  signers supported by the CLI.

[`paychains-program`]: https://docs.rs/paychains-program
[`paychains-sdk`]: https://docs.rs/paychains-sdk
[`paychains-client`]: https://docs.rs/paychains-client
[`paychains-cli-config`]: https://docs.rs/paychains-cli-config
[`paychains-clap-utils`]: https://docs.rs/paychains-clap-utils
[`clap`]: https://docs.rs/clap
