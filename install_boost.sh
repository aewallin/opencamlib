#!/usr/bin/env bash

set -e

boost_url="https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.gz"
project_dir=$(pwd)

print_help() {
cat << EOF
Usage: ./install_boost.sh [OPTIONS]

OPTIONS:
  --platform                  Set the platform, for when auto-detection doesn't work (one of: windows, macos, linux)
  --boost-with-python         Compile Boost.Python
  --boost-address-model       Set the address model for Boost (one of: 32, 64) (only valid when using --boost-with-python)
  --boost-architecture        Set the architecture for Boost (one of: x86, ia64, sparc, power, loongarch, mips, mips1, mips2, mips3, mips4, mips32, mips32r2, mips64, parisc, arm, riscv, s390x, arm+x86) (only valid when using --boost-with-python)
  --boost-python-version      Set the python version to look for when compiling Boost (only valid when using --boost-with-python)
  --python-executable         Set a custom path (or name of) the Python executable
  --help                      Shows this help page
EOF
    exit 1
}

original_args="$*"
while [[ "$#" -gt 0 ]]; do
    case $1 in
        --platform) OCL_PLATFORM="$2"; shift ;;
        --boost-with-python) OCL_BOOST_WITH_PYTHON="1"; ;;
        --boost-address-model) OCL_BOOST_ADDRESS_MODEL="$2"; shift ;;
        --boost-architecture) OCL_BOOST_ARCHITECTURE="$2"; shift ;;
        --boost-python-version) OCL_BOOST_PYTHON_VERSION="$2"; shift ;;
        --python-executable) OCL_PYTHON_EXECUTABLE="$2"; shift ;;
        --help|--*)
            echo $1
            print_help ;;
    esac
    shift
done

if [ -n "${OCL_BOOST_WITH_PYTHON}" ] && [ -z "${OCL_BOOST_ARCHITECTURE}" ]; then
    echo "WARN: Setting --boost-with-python without setting --boost-architecture. add --boost-architecture or remove the --boost-with-python option"
fi

get_os() {
    if [ "${OCL_PLATFORM}" ]; then
        echo "${OCL_PLATFORM}"
    else
        if [[ "${OSTYPE}" =~ ^darwin.* ]]; then
            echo "macos"
        elif [[ "${OSTYPE}" =~ ^linux.* ]]; then
            echo "linux"
        else
            echo "windows"
        fi
    fi
}
determined_os=$(get_os)

primary='\033[1;34m'
secondary='\033[1;35m'
nc='\033[0m'
prettyprint() {
    printf "${primary}${1}${nc}${secondary}${2}${nc}\n"
}

download_boost() {
    if [ ! -f "${TMPDIR:-"/tmp"}/boost.tar.gz" ]; then
        prettyprint "Downloading boost.tar.gz"
        curl "${boost_url}" --output "${TMPDIR:-"/tmp"}/boost.tar.gz" --silent --location
    else
        prettyprint "boost.tar.gz found, re-using..."
    fi
    prettyprint "Extracting boost.tar.gz..."
    tar -zxf "${TMPDIR:-"/tmp"}/boost.tar.gz" -C .

    prettyprint "Applying boost-python-3.11.patch"
    git apply --ignore-space-change --ignore-whitespace --directory "boost_1_80_0/libs/python" "${project_dir}/.github/patches/boost-python-3.11.patch"
}

compile_boost_python() {
    boost_variant="${build_type_lower}"
    cd "${project_dir}/boost_1_80_0"
    if [ -n "${OCL_BOOST_WITH_PYTHON}" ]; then
        if [ -n "${OCL_PYTHON_EXECUTABLE}" ]; then
            python_version=$(${OCL_PYTHON_EXECUTABLE} -c 'import sys; version=sys.version_info[:3]; print("{0}.{1}".format(*version))')
            python_include_dir=$(${OCL_PYTHON_EXECUTABLE} -c 'from sysconfig import get_paths as gp; print(gp()["include"])')
            if [ "${determined_os}" = "windows" ]; then
                python_include_dir=$(cygpath -w "${python_include_dir}")
            fi
            echo "using python : ${python_version} : ${OCL_PYTHON_EXECUTABLE//\\/\\\\} : ${python_include_dir//\\/\\\\} ;" > user-config.jam
        elif [ -n "${OCL_BOOST_PYTHON_VERSION}" ]; then
            echo "using python : ${OCL_BOOST_PYTHON_VERSION} ;" > user-config.jam
        else
            echo "using python ;" > user-config.jam
        fi
        cat user-config.jam
        prettyprint "Bootstrapping boost"
        if [ "${determined_os}" = "windows" ]; then
            ./bootstrap.bat
        else
            ./bootstrap.sh
        fi
        prettyprint "Compiling boost " "${OCL_BOOST_ADDRESS_MODEL:-"64"}-bit ${OCL_BOOST_ARCHITECTURE}"
        ./b2 \
            ${OCL_CLEAN:+"-a"} \
            -j2 \
            --layout="system" \
            --with-python \
            --user-config="user-config.jam" \
            threading="multi" \
            variant="${boost_variant}" \
            link="static" \
            cxxflags="-fPIC" \
            address-model="${OCL_BOOST_ADDRESS_MODEL:-"64"}" \
            ${OCL_BOOST_ARCHITECTURE:+"architecture=${OCL_BOOST_ARCHITECTURE}"} \
            stage
    fi
}

install_boost () {
    cd "${project_dir}"
    if [ -d boost_1_80_0 ]; then
        # boost folder already exists, re-unsing
        prettyprint "Boost already found, re-using..."
    elif [ -f boost-precompiled.tar.gz ]; then
        # boost-precompiled.tar.gz found, re-using
        prettyprint "Found cached precompiled boost, installing..."
        tar -zxf boost-precompiled.tar.gz -C .
    elif [ -n "${OCL_BOOST_ARCHITECTURE}" ] && [ -n "${OCL_BOOST_WITH_PYTHON}" ]; then
        # got enough information to try and download a pre-compiled boost with python
        boost_precompiled_url="https://github.com/vespakoen/boost-python-precompiled/releases/download/1.80.0/boost-python-precompiled-${determined_os}-${OCL_BOOST_ARCHITECTURE}-${OCL_BOOST_ADDRESS_MODEL:-"64"}-bit.tar.gz"
        if curl --output /dev/null --silent --head --fail "$boost_precompiled_url"; then
            prettyprint "Downloading boost-precompiled.tar.gz for ${OCL_BOOST_ARCHITECTURE} ${OCL_BOOST_ADDRESS_MODEL:-"64"}-bit..."
            curl "${boost_precompiled_url}" --output "${TMPDIR:-"/tmp"}/boost-precompiled.tar.gz" --silent --location
            prettyprint "Extracting boost-precompiled.tar.gz..."
            tar -zxf "${TMPDIR:-"/tmp"}/boost-precompiled.tar.gz" -C .
        else
            # precompiled boost python not available for given architecture and address model, installing from source
            download_boost
            if [ -n "${OCL_BOOST_WITH_PYTHON}" ]; then
                compile_boost_python
            fi
        fi
    else
        download_boost
        if [ -n "${OCL_BOOST_WITH_PYTHON}" ]; then
            compile_boost_python
        fi
    fi
}

install_boost