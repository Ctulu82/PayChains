---
title: Connecting to a Cluster
---

See [PayChains Clusters](../clusters.md) for general information about the
available clusters.

## Configure the command-line tool

You can check what cluster the PayChains command-line tool (CLI) is currently targeting by
running the following command:

```bash
paychains config get
```

Use `paychains config set` command to target a particular cluster. After setting
a cluster target, any future subcommands will send/receive information from that
cluster.

For example to target the Devnet cluster, run:

```bash
paychains config set --url https://api.devnet.paychains.com
```

## Ensure Versions Match

Though not strictly necessary, the CLI will generally work best when its version
matches the software version running on the cluster. To get the locally-installed
CLI version, run:

```bash
paychains --version
```

To get the cluster version, run:

```bash
paychains cluster-version
```

Ensure the local CLI version is greater than or equal to the cluster version.
