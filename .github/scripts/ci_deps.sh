#!/usr/bin/env bash

set -e

primary='\033[1;34m'
secondary='\033[1;35m'
nc='\033[0m'
prettyprint() {
    printf "${primary}${1}${nc}${secondary}${2}${nc}\n"
}

command_exists() {
    command -v "${1}" >/dev/null 2>&1;
}

is_root() {
    [ "${EUID:-$(id -u)}" -eq 0 ];
}

get_os() {
    if [[ "${OSTYPE}" =~ ^darwin.* ]]; then
        echo "macos"
    elif [[ "${OSTYPE}" =~ ^linux.* ]]; then
        echo "linux"
    else
        echo "windows"
    fi
}
determined_os=$(get_os)

install_ci_dependencies() {
    if [ "${determined_os}" = "windows" ]; then
        if command_exists choco; then
            choco install curl --no-progress
        fi
    elif [ "${determined_os}" = "linux" ]; then
        if command_exists apt; then
            if ! is_root; then
                maybe_sudo="sudo"
            fi
            ${maybe_sudo} apt update
            ${maybe_sudo} apt install -y --no-install-recommends curl
        elif command_exists yum; then
            if ! is_root; then
                maybe_sudo="sudo"
            fi
            ${maybe_sudo} yum install curl
        fi
    elif [ "${determined_os}" = "macos" ]; then
        prettyprint "Downloading libomp for: " "${OCL_MACOS_ARCHITECTURE}"
        if [ "${OCL_MACOS_ARCHITECTURE}" = "arm64" ]; then
            libomp_tar_loc=$(brew fetch --bottle-tag=arm64_big_sur libomp | grep -i downloaded | grep tar.gz | cut -f2 -d ":" | xargs echo)
        else
            libomp_tar_loc=$(brew fetch --bottle-tag=big_sur libomp | grep -i downloaded | grep tar.gz | cut -f2 -d ":" | xargs echo)
        fi
        temp_dir="/tmp"
        cp "${libomp_tar_loc}" "${temp_dir}/libomp.tar.gz"
        mkdir "${temp_dir}/libomp" || true
        tar -xzf "${temp_dir}/libomp.tar.gz" -C "${temp_dir}/libomp"
        libomp_prefix=$(find "${temp_dir}/libomp/libomp" -depth 1 | head -1)
        export OPENMP_PREFIX_MACOS="${temp_dir}/libomp/libomp/fixed"
        mv "${libomp_prefix}" "${OPENMP_PREFIX_MACOS}"
    fi
}

install_ci_dependencies