#!/usr/bin/env bash
set -ex

[[ $(uname) = Linux ]] || exit 1
[[ $USER = root ]] || exit 1

[[ -d /home/paychains/.ssh ]] || exit 1

if [[ ${#PAYCHAINS_PUBKEYS[@]} -eq 0 ]]; then
  echo "Warning: source paychains-user-authorized_keys.sh first"
fi

# paychains-user-authorized_keys.sh defines the public keys for users that should
# automatically be granted access to ALL testnets
for key in "${PAYCHAINS_PUBKEYS[@]}"; do
  echo "$key" >> /paychains-scratch/authorized_keys
done

sudo -u paychains bash -c "
  cat /paychains-scratch/authorized_keys >> /home/paychains/.ssh/authorized_keys
"
