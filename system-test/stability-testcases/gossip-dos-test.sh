#!/usr/bin/env bash

set -e
cd "$(dirname "$0")"
PAYCHAINS_ROOT="$(cd ../..; pwd)"

logDir="$PWD"/logs
rm -rf "$logDir"
mkdir "$logDir"

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

bootstrapInstall "edge"
paychains-install-init --version
paychains-install-init edge
paychains-gossip --version
paychains-dos --version

killall paychains-gossip || true
paychains-gossip spy --gossip-port 8001 > "$logDir"/gossip.log 2>&1 &
paychainsGossipPid=$!
echo "paychains-gossip pid: $paychainsGossipPid"
sleep 5
paychains-dos --mode gossip --data-type random --data-size 1232 &
dosPid=$!
echo "paychains-dos pid: $dosPid"

pass=true

SECONDS=
while ((SECONDS < 600)); do
  if ! kill -0 $paychainsGossipPid; then
    echo "paychains-gossip is no longer running after $SECONDS seconds"
    pass=false
    break
  fi
  if ! kill -0 $dosPid; then
    echo "paychains-dos is no longer running after $SECONDS seconds"
    pass=false
    break
  fi
  sleep 1
done

kill $paychainsGossipPid || true
kill $dosPid || true
wait || true

$pass && echo Pass
