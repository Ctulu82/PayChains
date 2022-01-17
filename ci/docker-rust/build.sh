#!/usr/bin/env bash
set -ex

cd "$(dirname "$0")"

docker build -t paychainslabs/rust .

read -r rustc version _ < <(docker run paychainslabs/rust rustc --version)
[[ $rustc = rustc ]]
docker tag paychainslabs/rust:latest paychainslabs/rust:"$version"
docker push paychainslabs/rust:"$version"
docker push paychainslabs/rust:latest
