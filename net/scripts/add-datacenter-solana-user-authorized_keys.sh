#!/usr/bin/env bash
set -ex

cd "$(dirname "$0")"

# shellcheck source=net/scripts/paychains-user-authorized_keys.sh
source paychains-user-authorized_keys.sh

# paychains-user-authorized_keys.sh defines the public keys for users that should
# automatically be granted access to ALL datacenter nodes.
for i in "${!PAYCHAINS_USERS[@]}"; do
  echo "environment=\"PAYCHAINS_USER=${PAYCHAINS_USERS[i]}\" ${PAYCHAINS_PUBKEYS[i]}"
done

