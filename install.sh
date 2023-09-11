#!/usr/bin/env bash

set -e

boost_url="https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.gz"
boost_additional_versions="1.81.0;1.80.0;1.79.0;1.78.0;1.77.0;1.76.0;1.75.0;1.74.0;1.73.0;1.72.0;1.71.0;1.70.0"
project_dir=$(pwd)

print_help() {
cat << EOF
Usage: ./install.sh [OPTIONS] [LIBRARY]

LIBRARY: one of: cxx, nodejs, python, emscripten

OPTIONS:
  --clean                     Clean the build folder before compiling a library
  --build-type                Choose the build type (one of: debug, release)
  --disable-openmp            Disable OpenMP in the build.
  --standalone                Static linking and copying dependencies (OpenMP) into install destination
  --install-system-deps       Install dependencies for compiling libraries (only aware of apt, brew and choco at the moment)

  --install                   Install the CMake install targets to the prefix (see: --install-prefix)
  --sudo-install              Install the CMake install targets to the prefix with root privileges (see: --install-prefix)
  --install-prefix            Set the install prefix location for CMake installs (only valid when using --install)

  --boost-prefix              Set a custom path where to look for Boost

  --python-executable         Set a custom path (or name of) the Python executable
  --python-prefix             Set the python prefix, this will be passed to CMake as Python3_ROOT_DIR, to make sure CMake is using the correct Python installation.
  
  --platform                  Set the platform, for when auto-detection doesn't work (one of: windows, macos, linux)

  --macos-architecture        Set the macOS architecture to compile for (one of: arm64, x86_64), useful for cross compiling.
  --docker-image              Set the docker image to forward this install command to, useful for cross compiling
  --docker-before-install     Run given commands in the docker container before running ./install.sh, (only valid when using --docker-image)
  --cmake-generator           Set the CMake Generator option
  --cmake-generator-platform  Set the CMake Generator Platform option, useful for cross compiling on Windows with the Visual Studio generator.
  --node-architecture         Set the node.js architecture to compile for, useful for cross compiling.

  --test                      Run a test script after compiling the library

  --help                      Shows this help page
EOF
    exit 1
}

primary='\033[1;34m'
secondary='\033[1;35m'
nc='\033[0m'
prettyprint() {
    printf "${primary}${1}${nc}${secondary}${2}${nc}\n"
}

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

# defaults
OCL_CLEAN="1"
OCL_BUILD_TYPE="release"
OCL_PLATFORM="${determined_os}"
OCL_INSTALL="1"

original_args="$*"
positional_args=()
while [[ "$#" -gt 0 ]]; do
    case $1 in
        --clean) OCL_CLEAN="1"; ;;
        --build-type) OCL_BUILD_TYPE="$2"; shift ;;
        --platform) OCL_PLATFORM="$2"; shift ;;
        --standalone) OCL_STANDALONE="1"; ;;
        --install-system-deps) OCL_INSTALL_SYSTEM_DEPS="1"; ;;
        --disable-openmp) OCL_DISABLE_OPENMP="1"; ;;
        --install) OCL_INSTALL="1"; ;;
        --sudo-install) OCL_SUDO_INSTALL="1"; ;;
        --install-prefix) OCL_INSTALL_PREFIX="$2"; shift ;;
        --boost-prefix) OCL_BOOST_PREFIX="$2"; shift ;;
        --macos-architecture) OCL_MACOS_ARCHITECTURE="$2"; shift ;;
        --docker-image) OCL_DOCKER_IMAGE="$2"; shift ;;
        --docker-before-install) OCL_DOCKER_IMAGE_BEFORE_INSTALL="$2"; shift ;;
        --cmake-generator) OCL_GENERATOR="$2"; shift ;;
        --cmake-generator-platform) OCL_GENERATOR_PLATFORM="$2"; shift ;;
        --python-executable) OCL_PYTHON_EXECUTABLE="$2"; shift ;;
        --node-architecture) OCL_NODE_ARCH="$2"; shift ;;
        --test) OCL_TEST="1"; ;;
        --help|--*)
            echo $1
            print_help ;;
        *)
            # Append unmatched arguments to the positional_args array
            positional_args+=("$1")
    esac
    shift
done

if [[ ${#positional_args[@]} != 1 ]] && [[ -z "${OCL_BUILD_LIBRARY}" ]]; then
    print_help
    exit 0
fi

if [[ ${#positional_args[@]} == 1 ]]; then
    OCL_BUILD_LIBRARY="${positional_args[0]}"
fi

if [ -n "${OCL_INSTALL_PREFIX}" ] && [ -z "${OCL_INSTALL}" ] && [ -z "${OCL_SUDO_INSTALL}" ]; then
    echo "WARN: Settings --install-prefix without setting --install or --sudo-install. add --install or --sudo-install option or remove the --install-prefix option"
fi

if [ "${OCL_BUILD_TYPE}" = "debug" ]; then
    build_type="Debug"
    build_type_lower="debug"
else
    build_type="RelWithDebInfo"
    build_type_lower="release"
fi
build_dir="${project_dir}/build/${OCL_BUILD_LIBRARY}/${build_type_lower}"

prettyprint "./install.sh " "$original_args"

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
        sudo apt install -y --no-install-recommends git cmake curl build-essential libboost-dev python3 libboost-python-dev nodejs npm
    elif [ "${determined_os}" = "macos" ]; then
        brew install libomp boost python boost-python3 node
    else
        # @todo installing git and cmake fails in github's CI
        choco install curl --no-progress
        choco install boost-msvc-14.3 --no-progress
        choco install nodejs --no-progress
    fi
}

command_exists() {
    command -v "${1}" >/dev/null 2>&1;
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
    (cd "${project_dir}/src/nodejslib" && npm install --silent)
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
    cd "${project_dir}"
}

build_clean() {
    rm -rf "${build_dir}" || true
}

cmake_build() {
    if [ "${determined_os}" = "windows" ]; then
        cmake \
            --build . \
            --config "${build_type}" \
            -j "${num_procs}"
    else
        cmake \
            --build . \
            -j "${num_procs}"
    fi
}

cmake_install() {
    if [ -n "${OCL_INSTALL}" ] || [ -n "${OCL_SUDO_INSTALL}" ]; then
        prettyprint "Installing"
        if [ "${determined_os}" = "windows" ]; then
            ${OCL_SUDO_INSTALL:+"sudo"} cmake \
                --install . \
                --config "${build_type}"
        else
            ${OCL_SUDO_INSTALL:+"sudo"} cmake \
                --install .
        fi
    fi
}

get_cmake_args() {
    echo "-S . \
    -B "${build_dir}" \
    ${OCL_GENERATOR:+"-G ${OCL_GENERATOR}"} \
    ${OCL_GENERATOR_PLATFORM:+"-A ${OCL_GENERATOR_PLATFORM}"} \
    -D CMAKE_BUILD_TYPE="${build_type}" \
    ${OCL_STANDALONE:+"-D USE_STANDALONE=ON"} \
    -D Boost_ADDITIONAL_VERSIONS="${boost_additional_versions}" \
    -D VERSION_STRING="$(cat VERSION)" \
    ${OCL_DISABLE_OPENMP:+"-D USE_OPENMP=OFF"} \
    ${OCL_INSTALL_PREFIX:+"-D CMAKE_INSTALL_PREFIX=${OCL_INSTALL_PREFIX}"} \
    ${OCL_BOOST_PREFIX:+"-D BOOST_ROOT=${OCL_BOOST_PREFIX}"}"
}

get_cmakejs_args() {
    echo "$(get_cmake_args)" | sed -e 's/-D /--CD/g' -e 's/-S/--directory/g' -e 's/-B/--out/g'
}

build_cxxlib() {
    set -x
    cmake $(get_cmake_args) -D BUILD_CXX_LIB="ON"
    set +x
    (cd "${build_dir}" && cmake_build)
    if [ -n "${OCL_INSTALL}" ] || [ -n "${OCL_SUDO_INSTALL}" ]; then
        prettyprint "Installing"
        (cd "${build_dir}" && cmake_install)
    fi
}

test_cxxlib() {
    cd "${project_dir}/examples/cpp/test"
    if [ -n "${OCL_CLEAN}" ]; then
        prettyprint "Cleaning build directory..."
        rm -rf build || true
    fi
    set -x
    cmake \
        -S . \
        -B build \
        ${OCL_GENERATOR:+"-G ${OCL_GENERATOR}"} \
        ${OCL_GENERATOR_PLATFORM:+"-A ${OCL_GENERATOR_PLATFORM}"} \
        -D Boost_ADDITIONAL_VERSIONS="${boost_additional_versions}" \
        ${OCL_BOOST_PREFIX:+"-DBOOST_ROOT=${OCL_BOOST_PREFIX}"}
    set +x
    cd build
    cmake_build
    if [ "${determined_os}" = "windows" ]; then
        ./test_example.exe
    else
        ./test_example
    fi
}

build_nodejslib() {
    set -x
    determined_arch=$(node -p 'process.arch')
    install_prefix_fallback="${project_dir}/src/npmpackage/build/Release/${determined_os}-nodejs-${determined_arch}"
    OCL_INSTALL_PREFIX="${OCL_INSTALL_PREFIX:-"${install_prefix_fallback}"}"
    src/nodejslib/node_modules/.bin/cmake-js \
        build \
        $(get_cmakejs_args) \
        --parallel "${num_procs}" \
        ${OCL_NODE_ARCH:+"--arch=${OCL_NODE_ARCH}"} \
        --CDBUILD_NODEJS_LIB="ON" \
        --config "${build_type}"
    set +x
    if [ -n "${OCL_INSTALL}" ] || [ -n "${OCL_SUDO_INSTALL}" ]; then
        (cd "${build_dir}" && cmake_install)
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
    export CMAKE_ARGS="${OCL_GENERATOR:+"-G ${OCL_GENERATOR} "}\
${OCL_GENERATOR_PLATFORM:+"-A ${OCL_GENERATOR_PLATFORM} "}\
-D CMAKE_BUILD_TYPE=${build_type} \
${OCL_BOOST_PREFIX:+"-D BOOST_ROOT=${OCL_BOOST_PREFIX} "}"
    cd "${project_dir}"
    ${python_executable} -m pip install --verbose .
}

test_pythonlib() {
    python_executable=$(get_python_executable)
    cd "${project_dir}/examples/python"
    ${python_executable} test.py
}

build_emscriptenlib() {
    source "${project_dir}/../emsdk/emsdk_env.sh"
    set -x
    install_prefix_fallback="${project_dir}/src/npmpackage/build"
    OCL_INSTALL_PREFIX="${OCL_INSTALL_PREFIX:-"${install_prefix_fallback}"}"
    OCL_DISABLE_OPENMP="1"
    emcmake cmake $(get_cmake_args) \
        -D BUILD_EMSCRIPTEN_LIB="ON" \
        -D USE_STANDALONE="ON"
    cd "${build_dir}"
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

if [ -n "${OCL_CLEAN}" ]; then
    prettyprint "Cleaning build directory..."
    build_clean
fi

if [[ "${OCL_BUILD_LIBRARY}" == "cxx" ]]; then
    prettyprint "Building C++ library"
    build_cxxlib
    if [ -n "${OCL_TEST}" ]; then
        prettyprint "Testing C++ library"
        test_cxxlib
    fi
fi

if [[ "${OCL_BUILD_LIBRARY}" == "python" ]]; then
    prettyprint "Building Python library " "${OCL_BOOST_PYTHON_VERSION}"
    build_pythonlib
    if [ -n "${OCL_TEST}" ]; then
        prettyprint "Testing Python library"
        test_pythonlib
    fi
fi

if [[ "${OCL_BUILD_LIBRARY}" == "emscripten" ]]; then
    prettyprint "Installing emscripten dependencies"
    dependencies_emscriptenlib
    prettyprint "Building emscripten library"
    build_emscriptenlib
    if [ -n "${OCL_TEST}" ]; then
        prettyprint "Testing emscripten library"
        test_emscriptenlib
    fi
fi

if [[ "${OCL_BUILD_LIBRARY}" == "nodejs" ]]; then
    prettyprint "Installing node.js dependencies"
    dependencies_nodejslib
    prettyprint "Building node.js library"
    build_nodejslib
    if [ -n "${OCL_TEST}" ]; then
        prettyprint "Testing node.js library"
        test_nodejslib
    fi
fi
