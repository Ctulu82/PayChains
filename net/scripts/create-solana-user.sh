#!/usr/bin/env bash
set -ex

[[ $(uname) = Linux ]] || exit 1
[[ $USER = root ]] || exit 1

if grep -q paychains /etc/passwd ; then
  echo "User paychains already exists"
else
  adduser paychains --gecos "" --disabled-password --quiet
  adduser paychains sudo
  adduser paychains adm
  echo "paychains ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
  id paychains

  [[ -r /paychains-scratch/id_ecdsa ]] || exit 1
  [[ -r /paychains-scratch/id_ecdsa.pub ]] || exit 1

  sudo -u paychains bash -c "
    echo 'PATH=\"/home/paychains/.cargo/bin:$PATH\"' > /home/paychains/.profile
    mkdir -p /home/paychains/.ssh/
    cd /home/paychains/.ssh/
    cp /paychains-scratch/id_ecdsa.pub authorized_keys
    umask 377
    cp /paychains-scratch/id_ecdsa id_ecdsa
    echo \"
      Host *
      BatchMode yes
      IdentityFile ~/.ssh/id_ecdsa
      StrictHostKeyChecking no
    \" > config
  "
fi
