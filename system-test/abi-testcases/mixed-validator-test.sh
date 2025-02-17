#!/usr/bin/env bash
#
# Basic empirical ABI system test - can validators on all supported versions of
# PayChains talk to each other?
#

set -e
cd "$(dirname "$0")"
PAYCHAINS_ROOT="$(cd ../..; pwd)"

logDir="$PWD"/logs
ledgerDir="$PWD"/config
rm -rf "$ledgerDir" "$logDir"
mkdir -p "$logDir"

baselineVersion=1.1.18  # <-- oldest version we remain compatible with
otherVersions=(
  beta
  edge
)

paychainsInstallDataDir=$PWD/releases
paychainsInstallGlobalOpts=(
  --data-dir "$paychainsInstallDataDir"
  --config "$paychainsInstallDataDir"/config.yml
  --no-modify-path
)

# Install all the paychains versions
bootstrapInstall() {
  declare v=$1
  if [[ ! -h $paychainsInstallDataDir/active_release ]]; then
    sh "$PAYCHAINS_ROOT"/install/paychains-install-init.sh "$v" "${paychainsInstallGlobalOpts[@]}"
  fi
  export PATH="$paychainsInstallDataDir/active_release/bin/:$PATH"
}

bootstrapInstall "$baselineVersion"
for v in "${otherVersions[@]}"; do
  paychains-install-init "${paychainsInstallGlobalOpts[@]}" "$v"
  paychains -V
done


ORIGINAL_PATH=$PATH
paychainsInstallUse() {
  declare version=$1
  echo "--- Now using paychains $version"
  PAYCHAINS_BIN="$paychainsInstallDataDir/releases/$version/paychains-release/bin"
  export PATH="$PAYCHAINS_BIN:$ORIGINAL_PATH"
}

killSession() {
  tmux kill-session -t abi || true
}

export RUST_BACKTRACE=1

# Start up the bootstrap validator using the baseline version
paychainsInstallUse "$baselineVersion"
echo "--- Starting $baselineVersion bootstrap validator"
trap 'killSession' INT TERM ERR EXIT
killSession
(
  set -x
  if [[ ! -x baseline-run.sh ]]; then
    curl https://raw.githubusercontent.com/paychains-labs/paychains/v"$baselineVersion"/run.sh -o baseline-run.sh
    chmod +x baseline-run.sh
  fi
  tmux new -s abi -d " \
    ./baseline-run.sh 2>&1 | tee $logDir/$baselineVersion.log \
  "

  SECONDS=
  while [[ ! -f config/baseline-run/init-completed ]]; do
    sleep 5
    if [[ $SECONDS -gt 60 ]]; then
      echo "Error: validator failed to start"
      exit 1
    fi
  done

  paychains --url http://127.0.0.1:8899 show-validators
)

# Ensure all versions can see the bootstrap validator
for v in "${otherVersions[@]}"; do
  paychainsInstallUse "$v"
  echo "--- Looking for bootstrap validator on gossip"
  (
    set -x
    "$PAYCHAINS_BIN"/paychains-gossip spy \
      --entrypoint 127.0.0.1:8001 \
      --num-nodes-exactly 1 \
      --timeout 30
  )
  echo Ok
done

# Start a validator for each version and look for it
#
# Once https://github.com/paychains-labs/paychains/issues/7738 is resolved, remove
# `--no-snapshot-fetch` when starting the validators
#
nodeCount=1
for v in "${otherVersions[@]}"; do
  nodeCount=$((nodeCount + 1))
  paychainsInstallUse "$v"
  # start another validator
  ledger="$ledgerDir"/ledger-"$v"
  rm -rf "$ledger"
  echo "--- Looking for $nodeCount validators on gossip"
  (
    set -x
    tmux new-window -t abi -n "$v" " \
      $PAYCHAINS_BIN/paychains-validator \
      --ledger $ledger \
      --no-snapshot-fetch \
      --entrypoint 127.0.0.1:8001 \
      -o - 2>&1 | tee $logDir/$v.log \
    "
    "$PAYCHAINS_BIN"/paychains-gossip spy \
      --entrypoint 127.0.0.1:8001 \
      --num-nodes-exactly $nodeCount \
      --timeout 30

    # Wait for it to make a snapshot root
    SECONDS=
    while [[ ! -d $ledger/snapshot ]]; do
      sleep 5
      if [[ $SECONDS -gt 60 ]]; then
        echo "Error: validator failed to create a snapshot"
        exit 1
      fi
    done
  )
  echo Ok
done

# Terminate all the validators
killSession

echo
echo Pass
exit 0
