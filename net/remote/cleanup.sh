#!/usr/bin/env bash

set -x
! tmux list-sessions || tmux kill-session
declare sudo=
if sudo true; then
  sudo="sudo -n"
fi

echo "pwd: $(pwd)"
for pid in paychains/*.pid; do
  pgid=$(ps opgid= "$(cat "$pid")" | tr -d '[:space:]')
  if [[ -n $pgid ]]; then
    $sudo kill -- -"$pgid"
  fi
done
if [[ -f paychains/netem.cfg ]]; then
  paychains/scripts/netem.sh delete < paychains/netem.cfg
  rm -f paychains/netem.cfg
fi
paychains/scripts/net-shaper.sh cleanup
for pattern in validator.sh boostrap-leader.sh paychains- remote- iftop validator client node; do
  echo "killing $pattern"
  pkill -f $pattern
done
