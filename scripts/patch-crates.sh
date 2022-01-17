# source this file

update_paychains_dependencies() {
  declare project_root="$1"
  declare paychains_ver="$2"
  declare tomls=()
  while IFS='' read -r line; do tomls+=("$line"); done < <(find "$project_root" -name Cargo.toml)

  sed -i -e "s#\(paychains-program = \"\)[^\"]*\(\"\)#\1=$paychains_ver\2#g" "${tomls[@]}" || return $?
  sed -i -e "s#\(paychains-program-test = \"\)[^\"]*\(\"\)#\1=$paychains_ver\2#g" "${tomls[@]}" || return $?
  sed -i -e "s#\(paychains-sdk = \"\).*\(\"\)#\1=$paychains_ver\2#g" "${tomls[@]}" || return $?
  sed -i -e "s#\(paychains-sdk = { version = \"\)[^\"]*\(\"\)#\1=$paychains_ver\2#g" "${tomls[@]}" || return $?
  sed -i -e "s#\(paychains-client = \"\)[^\"]*\(\"\)#\1=$paychains_ver\2#g" "${tomls[@]}" || return $?
  sed -i -e "s#\(paychains-client = { version = \"\)[^\"]*\(\"\)#\1=$paychains_ver\2#g" "${tomls[@]}" || return $?
  sed -i -e "s#\(paychains-clap-utils = \"\)[^\"]*\(\"\)#\1=$paychains_ver\2#g" "${tomls[@]}" || return $?
  sed -i -e "s#\(paychains-clap-utils = { version = \"\)[^\"]*\(\"\)#\1=$paychains_ver\2#g" "${tomls[@]}" || return $?
  sed -i -e "s#\(paychains-account-decoder = \"\)[^\"]*\(\"\)#\1=$paychains_ver\2#g" "${tomls[@]}" || return $?
  sed -i -e "s#\(paychains-account-decoder = { version = \"\)[^\"]*\(\"\)#\1=$paychains_ver\2#g" "${tomls[@]}" || return $?
}

patch_crates_io_paychains() {
  declare Cargo_toml="$1"
  declare paychains_dir="$2"
  cat >> "$Cargo_toml" <<EOF
[patch.crates-io]
paychains-account-decoder = { path = "$paychains_dir/account-decoder" }
paychains-clap-utils = { path = "$paychains_dir/clap-utils" }
paychains-client = { path = "$paychains_dir/client" }
paychains-program = { path = "$paychains_dir/sdk/program" }
paychains-program-test = { path = "$paychains_dir/program-test" }
paychains-sdk = { path = "$paychains_dir/sdk" }
EOF
}
