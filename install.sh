#!/usr/bin/env bash

set -e

boost_url="https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.gz"
boost_additional_versions="1.81.0;1.80.0;1.79.0;1.78.0;1.77.0;1.76.0;1.75.0;1.74.0;1.73.0;1.72.0;1.71.0;1.70.0"
project_dir=$(pwd)

print_help() {
cat << EOF
Usage: ./install.sh [OPTIONS]

Options:
  --clean                     Clean the build folder before compiling a library (only valid with --build-library)
  --build-library             Compile a library with CMake (one of: cxx, nodejs, python, emscripten)
  --build-type                Choose the build type (one of: debug, release) (only valid with --build-library)
  --disable-openmp            Disable OpenMP in the build. (only valid with --build-library)
  --install-system-deps       Install dependencies for compiling libraries (only aware of apt, brew and choco at the moment)
  --install-ci-deps           Install curl and, when the platform is macos, installs OpenMP for the given architecture (see: --macos-architecture)
  --install-boost             Install Boost from source
  --install-boost-from-repo   Install Boost (pre-compiled) from platform specific package repositories

  --install                   Install the CMake install targets to the prefix (see: --install-prefix)
  --sudo-install              Install the CMake install targets to the prefix with root privileges (see: --install-prefix)
  --install-prefix            Set the install prefix location for CMake installs (only valid when using --install)

  --boost-prefix              Set a custom path where to look for Boost
  --boost-with-python         Compile Boost.Python (only valid when using --install-boost)
  --boost-address-model       Set the address model for Boost (one of: 32, 64) (only valid when using --install-boost and --boost-with-python)
  --boost-architecture        Set the architecture for Boost (one of: x86, ia64, sparc, power, loongarch, mips, mips1, mips2, mips3, mips4, mips32, mips32r2, mips64, parisc, arm, riscv, s390x, arm+x86) (only valid when using --install-boost and --boost-with-python)
  --boost-python-version      Set the python version to look for when compiling Boost (only valid when using --install-boost and --boost-with-python)

  --python-executable         Set a custom path (or name of) the Python executable (only valid when using --build-library python)
  --python-prefix             Set the python prefix, this will be passed to CMake as Python3_ROOT_DIR, to make sure CMake is using the correct Python installation. (only valid when using --build-library python)
  --python-pip-install        Uses "pip install ." to compile and install the Python library (only valid when using --build-library python)

  --platform                  Set the platform, for when auto-detection doesn't work (one of: windows, macos, linux)

  --macos-architecture        Set the macOS architecture to compile for (one of: arm64, x86_64), useful for cross compiling.
  --docker-image              Set the docker image to forward this install command to, useful for cross compiling
  --docker-before-install     Run given commands in the docker container before running ./install.sh, (only valid when using --docker-image)
  --cmake-generator           Set the CMake Generator option
  --cmake-generator-platform  Set the CMake Generator Platform option, useful for cross compiling on Windows with the Visual Studio generator.
  --node-architecture         Set the node.js architecture to compile for, useful for cross compiling.

  --test                      Run a test script after compiling the library (only valid with --build-library)

  --help                      Shows this help page
EOF
    exit 1
}

original_args="$*"
while [[ "$#" -gt 0 ]]; do
    case $1 in
        --clean) OCL_CLEAN="1"; ;;
        --build-library) OCL_BUILD_LIBRARY="$2"; shift ;;
        --build-type) OCL_BUILD_TYPE="$2"; shift ;;
        --platform) OCL_PLATFORM="$2"; shift ;;
        --install-system-deps) OCL_INSTALL_SYSTEM_DEPS="1"; ;;
        --install-ci-deps) OCL_INSTALL_CI_DEPS="1"; ;;
        --disable-openmp) OCL_DISABLE_OPENMP="1"; ;;
        --install) OCL_INSTALL="1"; ;;
        --sudo-install) OCL_SUDO_INSTALL="1"; ;;
        --install-prefix) OCL_INSTALL_PREFIX="$2"; shift ;;
        --install-boost) OCL_INSTALL_BOOST="1"; ;;
        --install-boost-from-repo) OCL_INSTALL_BOOST_FROM_REPO="1"; ;;
        --boost-prefix) OCL_BOOST_PREFIX="$2"; shift ;;
        --boost-address-model) OCL_BOOST_ADDRESS_MODEL="$2"; shift ;;
        --boost-architecture) OCL_BOOST_ARCHITECTURE="$2"; shift ;;
        --boost-with-python) OCL_BOOST_WITH_PYTHON="1"; ;;
        --boost-python-version) OCL_BOOST_PYTHON_VERSION="$2"; shift ;;
        --macos-architecture) OCL_MACOS_ARCHITECTURE="$2"; shift ;;
        --docker-image) OCL_DOCKER_IMAGE="$2"; shift ;;
        --docker-before-install) OCL_DOCKER_IMAGE_BEFORE_INSTALL="$2"; shift ;;
        --cmake-generator) OCL_GENERATOR="$2"; shift ;;
        --cmake-generator-platform) OCL_GENERATOR_PLATFORM="$2"; shift ;;
        --python-executable) OCL_PYTHON_EXECUTABLE="$2"; shift ;;
        --python-prefix) OCL_PYTHON_PREFIX="$2"; shift ;;
        --python-pip-install) OCL_PYTHON_PIP_INSTALL="1"; ;;
        --node-architecture) OCL_NODE_ARCH="$2"; shift ;;
        --test) OCL_TEST="1"; ;;
        --help|--*)
            echo $1
            print_help ;;
        *)
    esac
    shift
done

verify_args() {
    if [ -n "${OCL_CLEAN}" ] && [ -z "${OCL_BUILD_LIBRARY}" ]; then
        echo "Cannot set --clean without building a library. add the --build-library [lib] option or remove the --clean option"
        exit 1
    elif [ -n "${OCL_BUILD_TYPE}" ] && [ -z "${OCL_BUILD_LIBRARY}" ]; then
        echo "Cannot set --build-type without building a library. add the --build-library [lib] option or remove the --build-type option"
        exit 1
    elif [ -n "${OCL_TEST}" ] && [ -z "${OCL_BUILD_LIBRARY}" ]; then
        echo "Cannot set --test without building a library. add the --build-library [lib] option or remove the --test option"
        exit 1
    elif [ -n "${OCL_DISABLE_OPENMP}" ] && [ -z "${OCL_BUILD_LIBRARY}" ]; then
        echo "Cannot set --disable-openmp without building a library. add the --build-library [lib] option or remove the --disable-openmp option"
        exit 1
    elif [ -n "${OCL_INSTALL_PREFIX}" ] && [ -z "${OCL_INSTALL}" ] && [ -z "${OCL_SUDO_INSTALL}" ]; then
        echo "WARN: Settings --install-prefix without setting --install or --sudo-install. add --install or --sudo-install option or remove the --install-prefix option"
    elif [ -n "${OCL_BOOST_WITH_PYTHON}" ] && [ -z "${OCL_INSTALL_BOOST}" ]; then
        echo "WARN: Setting --boost-with-python without setting --install-boost. add --install-boost or remove the --boost-with-python option"
    elif [ -n "${OCL_BOOST_WITH_PYTHON}" ] && [ -z "${OCL_BOOST_ARCHITECTURE}" ]; then
        echo "WARN: Setting --boost-with-python without setting --boost-architecture. add --boost-architecture or remove the --boost-with-python option"
    elif [ -n "${OCL_BOOST_ADDRESS_MODEL}" ] && [ -z "${OCL_INSTALL_BOOST}" ]; then
        echo "WARN: Setting --boost-address-model without setting --install-boost. add --install-boost or remove the --boost-address-model option"
    elif [ -n "${OCL_BOOST_ARCHITECTURE}" ] && [ -z "${OCL_INSTALL_BOOST}" ]; then
        echo "WARN: Setting --boost-architecture without setting --install-boost. add --install-boost or remove the --boost-address-model option"
    elif [ -n "${OCL_BOOST_PYTHON_VERSION}" ] && [ -z "${OCL_INSTALL_BOOST}" ]; then
        echo "WARN: Setting --boost-python-version without setting --install-boost. add --install-boost or remove the --boost-python-version option"
    fi
}
verify_args

if [ "${OCL_BUILD_TYPE}" = "debug" ]; then
    build_type="Debug"
    build_type_lower="debug"
else
    build_type="RelWithDebInfo"
    build_type_lower="release"
fi
build_dir="${project_dir}/build/${OCL_BUILD_LIBRARY}/${build_type_lower}"
primary='\033[1;34m'
secondary='\033[1;35m'
nc='\033[0m'
prettyprint() {
    printf "${primary}${1}${nc}${secondary}${2}${nc}\n"
}

prettyprint "./install.sh " "$original_args"

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
prettyprint "Determined OS: " "${determined_os}"

num_procs() {
    if [ "${determined_os}" = "macos" ]; then
        sysctl -n hw.logicalcpu
    elif [ "${determined_os}" = "linux" ]; then
        nproc
    else
        echo "${NUMBER_OF_PROCESSORS:-"2"}"
    fi
}
num_procs=$(num_procs)
prettyprint "Processor Cores: " "${num_procs}"

if [ -n "${OCL_DOCKER_IMAGE_BEFORE_INSTALL}" ] && [ -z "${OCL_DOCKER_IMAGE}" ]; then
    prettyprint "Found before install, running: " "${OCL_DOCKER_IMAGE_BEFORE_INSTALL}"
    eval "${OCL_DOCKER_IMAGE_BEFORE_INSTALL}"
fi

install_system_dependencies() {
    if [ "${determined_os}" = "linux" ]; then
        sudo apt update
        sudo apt install -y --no-install-recommends git cmake curl build-essential
        if [ -n "${OCL_INSTALL_BOOST_FROM_REPO}" ]; then
            sudo apt install -y --no-install-recommends libboost-dev
        fi
        if [ "${OCL_BUILD_LIBRARY}" = "python" ]; then
            if [ -z "${OCL_PYTHON_EXECUTABLE}" ]; then
                sudo apt install -y --no-install-recommends python3
            fi
            if [ -n "${OCL_INSTALL_BOOST_FROM_REPO}" ]; then
                sudo apt install -y --no-install-recommends libboost-python-dev
            fi
        fi
        if [ "${OCL_BUILD_LIBRARY}" = "nodejs" ]; then
            sudo apt install -y --no-install-recommends nodejs npm
        fi
    elif [ "${determined_os}" = "macos" ]; then
        brew install libomp
        if [ -n "${OCL_INSTALL_BOOST_FROM_REPO}" ]; then
            brew install boost
        fi
        if [ "${OCL_BUILD_LIBRARY}" = "python" ]; then
            if [ -z "${OCL_PYTHON_EXECUTABLE}" ]; then
                brew install python@3.11
            fi
            if [ -n "${OCL_INSTALL_BOOST_FROM_REPO}" ]; then
                brew install boost-python3
            fi
        fi
        if [ "${OCL_BUILD_LIBRARY}" = "nodejs" ]; then
            brew install node
        fi
    else
        # @todo installing git and cmake fails in github's CI
        choco install curl --no-progress
        if [ -n "${OCL_INSTALL_BOOST_FROM_REPO}" ]; then
            choco install boost-msvc-14.3 --no-progress
        fi
        if [ "${OCL_BUILD_LIBRARY}" = "nodejs" ]; then
            choco install nodejs
        fi
    fi
}

command_exists() {
    command -v "${1}" >/dev/null 2>&1;
}

is_root() {
    [ "${EUID:-$(id -u)}" -eq 0 ];
}

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

if [ -n "${OCL_DOCKER_IMAGE}" ]; then
    prettyprint "Running the docker image with the following environment variables"
    # collect all the options to check which ones are set and should be forwarded to the container
    ALL_OPTIONS=($(compgen -A variable | grep '^OCL_'))
    # create temporary env file
    TMPENV="/tmp/.env${RANDOM}"
    # empty the env file
    echo "" > "${TMPENV}"
    # escape the pwd to be used in find and replace later
    ESCAPED_PWD=$(pwd | sed 's/\//\\\//g')
    # look for options that are set
    for OPTION in "${ALL_OPTIONS[@]}"; do
        # exclude some that cause recursion or do not work in the container
        if [ -n "${!OPTION}" ] && [ "${OPTION}" != "OCL_DOCKER_IMAGE" ] && [ "${OPTION}" != "OCL_INSTALL_SYSTEM_DEPS" ]; then
            # save to options to the env file, replacing PWD with the container's /work mount path
            FIXED_OPTION="${!OPTION//${ESCAPED_PWD}//work}"
            echo "${OPTION}=${FIXED_OPTION}" >> "${TMPENV}"
            prettyprint "${OPTION}: " "${FIXED_OPTION}"
        fi
    done
    docker run --env-file "${TMPENV}" --user root -w /work -v "$(pwd):/work:rw" --rm "${OCL_DOCKER_IMAGE}" ./install.sh
    exit 0
fi

# when the --macos-architecture flag is set, export the CMAKE_OSX_ARCHITECTURES variable for cross compiling and download libomp.
if [ -n "${OCL_MACOS_ARCHITECTURE}" ]; then
    export CMAKE_OSX_ARCHITECTURES="${OCL_MACOS_ARCHITECTURE}"
fi

dependencies_nodejslib () {
    cd "${project_dir}/src/nodejslib"
    npm install --silent
}

dependencies_emscriptenlib () {
    cd "${project_dir}"
    if [ ! -d ../emsdk ]; then
        (
            cd ..
            git clone https://github.com/emscripten-core/emsdk.git
        )
    fi
    cd ../emsdk
    ./emsdk install latest
    ./emsdk activate latest
}

build_clean() {
    rm -rf "${build_dir}" || true
}

cmake_build() {
    if [ "${determined_os}" = "windows" ]; then
        ${OCL_SUDO_INSTALL:+"sudo"} cmake \
            --build . \
            --config "${build_type}" \
            -j "${num_procs}"
    else
        ${OCL_SUDO_INSTALL:+"sudo"} cmake \
            --build . \
            -j "${num_procs}"
    fi
}

cmake_install() {
    prettyprint "Installing"
    if [ "${determined_os}" = "windows" ]; then
        ${OCL_SUDO_INSTALL:+"sudo"} cmake \
            --install . \
            --config "${build_type}"
    else
        ${OCL_SUDO_INSTALL:+"sudo"} cmake \
            --install .
    fi
}

build_cxxlib() {
    mkdir -p "${build_dir}"
    cd "${build_dir}"
    set -x
    cmake \
        ${OCL_GENERATOR:+"-G ${OCL_GENERATOR}"} \
        ${OCL_GENERATOR_PLATFORM:+"-A ${OCL_GENERATOR_PLATFORM}"} \
        -D CMAKE_BUILD_TYPE="${build_type}" \
        -D BUILD_CXX_LIB="ON" \
        -D Boost_ADDITIONAL_VERSIONS="${boost_additional_versions}" \
        ${OCL_DISABLE_OPENMP:+"-DUSE_OPENMP=OFF"} \
        ${OCL_INSTALL_PREFIX:+"-DCMAKE_INSTALL_PREFIX=${OCL_INSTALL_PREFIX}"} \
        ${OCL_BOOST_PREFIX:+"-DBOOST_ROOT=${OCL_BOOST_PREFIX}"} \
        ../../..
    set +x
    cmake_build
    if [ -n "${OCL_INSTALL}" ] || [ -n "${OCL_SUDO_INSTALL}" ]; then
        cmake_install
    fi
}

test_cxxlib() {
    cd "${project_dir}/examples/cpp/test"
    if [ -n "${OCL_CLEAN}" ]; then
        prettyprint "Cleaning build directory..."
        rm -rf build || true
    fi
    mkdir build || true
    cd build
    set -x
    cmake \
        ${OCL_GENERATOR:+"-G ${OCL_GENERATOR}"} \
        ${OCL_GENERATOR_PLATFORM:+"-A ${OCL_GENERATOR_PLATFORM}"} \
        -D Boost_ADDITIONAL_VERSIONS="${boost_additional_versions}" \
        ${OCL_BOOST_PREFIX:+"-DBOOST_ROOT=${OCL_BOOST_PREFIX}"} \
        ..
    set +x
    cmake_build
    if [ "${determined_os}" = "windows" ]; then
        ./test_example.exe
    else
        ./test_example
    fi
}

build_nodejslib() {
    mkdir -p "${build_dir}"
    cd "${build_dir}"
    set -x
    determined_arch=$(node -p 'process.arch')
    install_prefix_fallback="${project_dir}/src/npmpackage/build/Release/${determined_os}-nodejs-${determined_arch}"
    ../../../src/nodejslib/node_modules/.bin/cmake-js \
        build \
        --directory "../../.." \
        --out "." \
        --parallel "${num_procs}" \
        ${OCL_NODE_ARCH:+"--arch=${OCL_NODE_ARCH}"} \
        ${OCL_GENERATOR:+"--generator=${OCL_GENERATOR}"} \
        ${OCL_GENERATOR_PLATFORM:+"--platform=${OCL_GENERATOR_PLATFORM}"} \
        --CDBUILD_NODEJS_LIB="ON" \
        --CDBoost_ADDITIONAL_VERSIONS="${boost_additional_versions}" \
        --CDCMAKE_INSTALL_PREFIX="${OCL_INSTALL_PREFIX:-"${install_prefix_fallback}"}" \
        ${OCL_DISABLE_OPENMP:+"--CDUSE_OPENMP=OFF"} \
        ${OCL_BOOST_PREFIX:+"--CDBOOST_ROOT=${OCL_BOOST_PREFIX}"} \
        --config "${build_type}"
    set +x
    if [ -n "${OCL_INSTALL}" ] || [ -n "${OCL_SUDO_INSTALL}" ]; then
        cmake_install
    fi
}

test_nodejslib() {
    cd "${project_dir}/src/npmpackage"
    npm install
    npm run build-node
    cd ../../examples/nodejs
    npm link ../../src/npmpackage
    if [ "${build_type}" = "debug" ]; then
        export DEBUG="1"
    fi
    node test.js
}

get_python_executable() {
    if command_exists "python3"; then
        python_executable_fallback="python3"
    else
        python_executable_fallback="python"
    fi
    echo "${OCL_PYTHON_EXECUTABLE:-"${python_executable_fallback}"}"
}

build_pythonlib() {
    python_executable=$(get_python_executable)
    if [ -n "${OCL_PYTHON_PIP_INSTALL}" ]; then
        ${python_executable} -m pip install scikit-build-core distlib pyproject_metadata
        # ${python_executable} -m venv env
        # if [ "${determined_os}" = "windows" ]; then
        #     source env/Scripts/activate
        # else
        #     source env/bin/activate
        # fi
        # forward cmake args
        export CMAKE_ARGS="${OCL_GENERATOR:+"-G ${OCL_GENERATOR} "}\
${OCL_GENERATOR_PLATFORM:+"-A ${OCL_GENERATOR_PLATFORM} "}\
-D CMAKE_BUILD_TYPE=${build_type} \
-D Boost_ADDITIONAL_VERSIONS=${boost_additional_versions} \
${OCL_BOOST_PREFIX:+"-D BOOST_ROOT=${OCL_BOOST_PREFIX} "}"
        cd "${project_dir}"
        ${python_executable} -m pip install --verbose .
    else
        mkdir -p "${build_dir}"
        cd "${build_dir}"
        set -x
        install_prefix_fallback=$(${python_executable} -c 'import sysconfig; print(sysconfig.get_paths()["purelib"])')
        cmake \
            ${OCL_GENERATOR:+"-G ${OCL_GENERATOR}"} \
            ${OCL_GENERATOR_PLATFORM:+"-A ${OCL_GENERATOR_PLATFORM}"} \
            -D CMAKE_BUILD_TYPE="${build_type}" \
            -D BUILD_PY_LIB="ON" \
            -D Boost_ADDITIONAL_VERSIONS="${boost_additional_versions}" \
            -D CMAKE_INSTALL_PREFIX="${OCL_INSTALL_PREFIX:-"${install_prefix_fallback}"}" \
            ${OCL_DISABLE_OPENMP:+"-DUSE_OPENMP=OFF"} \
            ${OCL_PYTHON_PREFIX:+"-DPython3_ROOT_DIR=${OCL_PYTHON_PREFIX}"} \
            ${OCL_BOOST_PREFIX:+"-DBOOST_ROOT=${OCL_BOOST_PREFIX}"} \
            ../../..
        set +x
        cmake_build
        if [ -n "${OCL_INSTALL}" ] || [ -n "${OCL_SUDO_INSTALL}" ]; then
            cmake_install
        fi
    fi
}

test_pythonlib() {
    python_executable=$(get_python_executable)
    cd "${project_dir}/examples/python"
    ${python_executable} test.py
}

build_emscriptenlib() {
    mkdir -p "${build_dir}"
    cd "${build_dir}"
    source "${project_dir}/../emsdk/emsdk_env.sh"
    set -x
    install_prefix_fallback="${project_dir}/src/npmpackage/build"
    emcmake cmake \
        -G "Unix Makefiles" \
        -D CMAKE_BUILD_TYPE="${build_type}" \
        -D BUILD_EMSCRIPTEN_LIB="ON" \
        -D USE_OPENMP="OFF" \
        -D Boost_ADDITIONAL_VERSIONS="${boost_additional_versions}" \
        -D CMAKE_INSTALL_PREFIX="${OCL_INSTALL_PREFIX:-"${install_prefix_fallback}"}" \
        ${OCL_BOOST_PREFIX:+"-DBOOST_ROOT=${OCL_BOOST_PREFIX}"} \
        ../../..
    if [ "${determined_os}" = "windows" ]; then
        emmake make \
            --config "${build_type}" \
            -j "${num_procs}"
    else
        emmake make \
            -j "${num_procs}"
    fi
    if [ -n "${OCL_INSTALL}" ] || [ -n "${OCL_SUDO_INSTALL}" ]; then
        prettyprint "Installing"
        ${OCL_SUDO_INSTALL:+"sudo"} emmake make install
    fi
    set +x
}

test_emscriptenlib() {
    cd "${project_dir}/src/npmpackage"
    npm install
    npm run build-emscripten
    cd ../../examples/emscripten
    npm link ../../src/npmpackage
    npm start &
    server_pid=$!
    sleep 3
    node test.js
    kill ${server_pid}
}

if [ -n "${OCL_INSTALL_SYSTEM_DEPS}" ]; then
    prettyprint "Installing system dependencies..."
    install_system_dependencies
fi

if [ -n "${OCL_INSTALL_CI_DEPS}" ]; then
    prettyprint "Installing ci dependencies..."
    install_ci_dependencies
fi

if [ -n "${OCL_INSTALL_BOOST}" ]; then
    prettyprint "Installing Boost..."
    install_boost
fi

if [ -n "${OCL_CLEAN}" ]; then
    prettyprint "Cleaning build directory..."
    build_clean
fi

if [ "${OCL_BUILD_LIBRARY}" = "cxx" ]; then
    prettyprint "Building C++ library"
    build_cxxlib
    if [ -n "${OCL_TEST}" ]; then
        prettyprint "Testing C++ library"
        test_cxxlib
    fi
fi

if [ "${OCL_BUILD_LIBRARY}" = "nodejs" ]; then
    prettyprint "Installing node.js dependencies"
    dependencies_nodejslib
    prettyprint "Building node.js library"
    build_nodejslib
    if [ -n "${OCL_TEST}" ]; then
        prettyprint "Testing node.js library"
        test_nodejslib
    fi
fi

if [ "${OCL_BUILD_LIBRARY}" = "python" ]; then
    prettyprint "Building Python library " "${OCL_BOOST_PYTHON_VERSION}"
    build_pythonlib
    if [ -n "${OCL_TEST}" ]; then
        prettyprint "Testing Python library"
        test_pythonlib
    fi
fi

if [ "${OCL_BUILD_LIBRARY}" = "emscripten" ]; then
    prettyprint "Installing emscripten dependencies"
    dependencies_emscriptenlib
    prettyprint "Building emscripten library"
    build_emscriptenlib
    if [ -n "${OCL_TEST}" ]; then
        prettyprint "Testing emscripten library"
        test_emscriptenlib
    fi
fi
