#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"/..
cargo="$(readlink -f "./cargo")"

"$cargo" build --package paychains-install
export PATH=$PWD/target/debug:$PATH

echo "\`\`\`manpage"
paychains-install --help
echo "\`\`\`"
echo ""

commands=(init info deploy update run)

for x in "${commands[@]}"; do
    echo "\`\`\`manpage"
    paychains-install "${x}" --help
    echo "\`\`\`"
    echo ""
done
