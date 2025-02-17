#!/usr/bin/env bash
#
# Starts an instance of paychains-faucet
#
here=$(dirname "$0")

# shellcheck source=multinode-demo/common.sh
source "$here"/common.sh

[[ -f "$PAYCHAINS_CONFIG_DIR"/faucet.json ]] || {
  echo "$PAYCHAINS_CONFIG_DIR/faucet.json not found, create it by running:"
  echo
  echo "  ${here}/setup.sh"
  exit 1
}

set -x
# shellcheck disable=SC2086 # Don't want to double quote $paychains_faucet
exec $paychains_faucet --keypair "$PAYCHAINS_CONFIG_DIR"/faucet.json "$@"
