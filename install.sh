#!/usr/bin/env bash

set -e

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

  --install                   Install the CMake install targets to the prefix (see: --install-prefix)
  --sudo-install              Install the CMake install targets to the prefix with root privileges (see: --install-prefix)
  --install-prefix            Set the install prefix location for CMake installs (only valid when using --install)

  --python-executable         Set a custom path (or name of) the Python executable (only valid when using --build-library python)
  --python-prefix             Set the python prefix, this will be passed to CMake as Python3_ROOT_DIR, to make sure CMake is using the correct Python installation. (only valid when using --build-library python)
  --python-pip-install        Uses the current Python environment's "pip install ." to compile and install the Python library (only valid when using --build-library python)

  --platform                  Set the platform, for when auto-detection doesn't work (one of: windows, macos, linux)

  --macos-architecture        Set the macOS architecture to compile for (one of: arm64, x86_64), useful for cross compiling. Defaults to the host architecture.
  --docker-image              Set the docker image to forward this install command to, useful for cross compiling
  --docker-before-install     Run given commands in the docker container before running ./install.sh, (only valid when using --docker-image)
  --cmake-generator           Set the CMake Generator option
  --cmake-generator-platform  Set the CMake Generator Platform option, useful for cross compiling on Windows with the Visual Studio generator.
  --node-architecture         Set the node.js architecture to compile for, useful for cross compiling.

  --test                      Run a test script after compiling the library (only valid with --build-library)

  --help                      Shows this help page
EOF
}

invalid_argument() {
    echo "$1" >&2
    exit 1
}

require_option_value() {
    if [ "$#" -lt 2 ] || [ -z "$2" ] || [[ "$2" = --* ]]; then
        invalid_argument "Missing value for $1"
    fi
}

original_args="$*"
while [[ "$#" -gt 0 ]]; do
    case $1 in
        --clean) OCL_CLEAN="1"; ;;
        --build-library) require_option_value "$@"; OCL_BUILD_LIBRARY="$2"; shift ;;
        --build-type) require_option_value "$@"; OCL_BUILD_TYPE="$2"; shift ;;
        --platform) require_option_value "$@"; OCL_PLATFORM="$2"; shift ;;
        --install-system-deps) OCL_INSTALL_SYSTEM_DEPS="1"; ;;
        --install-ci-deps) OCL_INSTALL_CI_DEPS="1"; ;;
        --disable-openmp) OCL_DISABLE_OPENMP="1"; ;;
        --install) OCL_INSTALL="1"; ;;
        --sudo-install) OCL_SUDO_INSTALL="1"; ;;
        --install-prefix) require_option_value "$@"; OCL_INSTALL_PREFIX="$2"; shift ;;
        --macos-architecture) require_option_value "$@"; OCL_MACOS_ARCHITECTURE="$2"; shift ;;
        --docker-image) require_option_value "$@"; OCL_DOCKER_IMAGE="$2"; shift ;;
        --docker-before-install) require_option_value "$@"; OCL_DOCKER_IMAGE_BEFORE_INSTALL="$2"; docker_before_install_argument="1"; shift ;;
        --cmake-generator) require_option_value "$@"; OCL_GENERATOR="$2"; shift ;;
        --cmake-generator-platform) require_option_value "$@"; OCL_GENERATOR_PLATFORM="$2"; shift ;;
        --python-executable) require_option_value "$@"; OCL_PYTHON_EXECUTABLE="$2"; shift ;;
        --python-prefix) require_option_value "$@"; OCL_PYTHON_PREFIX="$2"; shift ;;
        --python-pip-install) OCL_PYTHON_PIP_INSTALL="1"; ;;
        --node-architecture) require_option_value "$@"; OCL_NODE_ARCH="$2"; shift ;;
        --test) OCL_TEST="1"; ;;
        --help) print_help; exit 0 ;;
        --*) invalid_argument "Unknown option: $1" ;;
        *) invalid_argument "Unexpected argument: $1" ;;
    esac
    shift
done

verify_args() {
    case "${OCL_BUILD_LIBRARY:-}" in
        ""|cxx|nodejs|python|emscripten) ;;
        *) invalid_argument "Invalid library type: ${OCL_BUILD_LIBRARY}" ;;
    esac

    case "${OCL_BUILD_TYPE:-}" in
        ""|debug|release) ;;
        *) invalid_argument "Invalid build type: ${OCL_BUILD_TYPE}" ;;
    esac

    case "${OCL_PLATFORM:-}" in
        ""|windows|macos|linux) ;;
        *) invalid_argument "Invalid platform: ${OCL_PLATFORM}" ;;
    esac

    case "${OCL_MACOS_ARCHITECTURE:-}" in
        ""|arm64|x86_64) ;;
        *) invalid_argument "Invalid macOS architecture: ${OCL_MACOS_ARCHITECTURE}" ;;
    esac

    if [ -n "${OCL_CLEAN:-}" ] && [ -z "${OCL_BUILD_LIBRARY:-}" ]; then
        echo "Cannot set --clean without building a library. add the --build-library [lib] option or remove the --clean option"
        exit 1
    elif [ -n "${OCL_BUILD_TYPE:-}" ] && [ -z "${OCL_BUILD_LIBRARY:-}" ]; then
        echo "Cannot set --build-type without building a library. add the --build-library [lib] option or remove the --build-type option"
        exit 1
    elif [ -n "${OCL_TEST:-}" ] && [ -z "${OCL_BUILD_LIBRARY:-}" ]; then
        echo "Cannot set --test without building a library. add the --build-library [lib] option or remove the --test option"
        exit 1
    elif [ -n "${OCL_DISABLE_OPENMP:-}" ] && [ -z "${OCL_BUILD_LIBRARY:-}" ]; then
        echo "Cannot set --disable-openmp without building a library. add the --build-library [lib] option or remove the --disable-openmp option"
        exit 1
    elif { [ -n "${OCL_PYTHON_EXECUTABLE:-}" ] || [ -n "${OCL_PYTHON_PREFIX:-}" ] || [ -n "${OCL_PYTHON_PIP_INSTALL:-}" ]; } && [ "${OCL_BUILD_LIBRARY:-}" != "python" ]; then
        invalid_argument "Python options require --build-library python"
    elif [ -n "${OCL_NODE_ARCH:-}" ] && [ "${OCL_BUILD_LIBRARY:-}" != "nodejs" ]; then
        invalid_argument "--node-architecture requires --build-library nodejs"
    elif [ -n "${docker_before_install_argument:-}" ] && [ -z "${OCL_DOCKER_IMAGE:-}" ]; then
        invalid_argument "--docker-before-install requires --docker-image"
    elif [ -n "${OCL_INSTALL_PREFIX:-}" ] && [ -z "${OCL_INSTALL:-}" ] && [ -z "${OCL_SUDO_INSTALL:-}" ]; then
        echo "WARN: Settings --install-prefix without setting --install or --sudo-install. add --install or --sudo-install option or remove the --install-prefix option"
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
        if [ "${OCL_BUILD_LIBRARY}" = "python" ]; then
            if [ -z "${OCL_PYTHON_PIP_INSTALL}" ]; then
                sudo apt install -y --no-install-recommends pybind11-dev
            fi
            if [ -z "${OCL_PYTHON_EXECUTABLE}" ]; then
                sudo apt install -y --no-install-recommends python3 python3-dev
            fi
        fi
        if [ "${OCL_BUILD_LIBRARY}" = "nodejs" ]; then
            sudo apt install -y --no-install-recommends nodejs npm
        fi
    elif [ "${determined_os}" = "macos" ]; then
        brew install libomp
        if [ "${OCL_BUILD_LIBRARY}" = "python" ]; then
            if [ -z "${OCL_PYTHON_PIP_INSTALL}" ]; then
                brew install pybind11
            fi
            if [ -z "${OCL_PYTHON_EXECUTABLE}" ]; then
                brew install python
            fi
        fi
        if [ "${OCL_BUILD_LIBRARY}" = "nodejs" ]; then
            brew install node
        fi
    else
        # @todo installing git and cmake fails in github's CI
        choco install curl --no-progress
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
        OCL_MACOS_ARCHITECTURE="${OCL_MACOS_ARCHITECTURE:-$(uname -m)}"
        if [ "${OCL_MACOS_ARCHITECTURE}" = "$(uname -m)" ]; then
            prettyprint "Installing libomp for: " "${OCL_MACOS_ARCHITECTURE}"
            brew install libomp
            libomp_prefix=$(brew --prefix libomp)
        else
            prettyprint "Downloading libomp for cross compilation: " "${OCL_MACOS_ARCHITECTURE}"
            brew_cache=$(brew --cache)
            if [ "${OCL_MACOS_ARCHITECTURE}" = "arm64" ]; then
                brew fetch --bottle-tag=arm64_sequoia libomp >/dev/null
                libomp_tar_loc=$(find "${brew_cache}/downloads" -maxdepth 1 -name "*--libomp--*arm64_sequoia.bottle.tar.gz" -print | tail -n1)
            else
                arch -x86_64 brew fetch --bottle-tag=sonoma libomp >/dev/null
                libomp_tar_loc=$(find "${brew_cache}/downloads" -maxdepth 1 -name "*--libomp--*sonoma.bottle.tar.gz" -print | tail -n1)
            fi
            [ -n "${libomp_tar_loc}" ] || invalid_argument "Failed to locate ${OCL_MACOS_ARCHITECTURE} libomp bottle in Homebrew cache"
            cp "${libomp_tar_loc}" /tmp/libomp.tar.gz
            mkdir -p /tmp/libomp
            tar -xzf /tmp/libomp.tar.gz -C /tmp/libomp
            libomp_prefix=$(find /tmp/libomp/libomp -mindepth 1 -maxdepth 1 -type d ! -name fixed -print | sort | tail -n1)
            [ -n "${libomp_prefix}" ] || invalid_argument "Failed to locate extracted libomp bottle"
            # Raw bottles retain Homebrew's placeholder install name. Relocate it
            # before linking so executables can load this runtime through CMake's rpath.
            install_name_tool -id "@rpath/libomp.dylib" "${libomp_prefix}/lib/libomp.dylib"
            codesign --force --sign - "${libomp_prefix}/lib/libomp.dylib"
        fi
        # Keep the same dependency path for CMake and cibuildwheel subprocesses.
        mkdir -p /tmp/libomp/libomp
        ln -sfn "${libomp_prefix}" /tmp/libomp/libomp/fixed
        export OPENMP_PREFIX_MACOS="/tmp/libomp/libomp/fixed"
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
        ${OCL_DISABLE_OPENMP:+"-DUSE_OPENMP=OFF"} \
        ${OCL_INSTALL_PREFIX:+"-DCMAKE_INSTALL_PREFIX=${OCL_INSTALL_PREFIX}"} \
        ../../..
    set +x
    cmake_build
    if [ -n "${OCL_INSTALL}" ] || [ -n "${OCL_SUDO_INSTALL}" ]; then
        cmake_install
    fi
}

test_cxxlib() {
    (
        cd "${build_dir}"
        ctest --build-config "${build_type}" --output-on-failure
    )
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
        --CDCMAKE_INSTALL_PREFIX="${OCL_INSTALL_PREFIX:-"${install_prefix_fallback}"}" \
        ${OCL_DISABLE_OPENMP:+"--CDUSE_OPENMP=OFF"} \
        --config "${build_type}"
    set +x
    if [ -n "${OCL_INSTALL}" ] || [ -n "${OCL_SUDO_INSTALL}" ]; then
        cmake_install
    fi
}

test_nodejslib() {
    node_addon_path="${build_dir}/ocl.node"
    if [ ! -f "${node_addon_path}" ]; then
        invalid_argument "Cannot find compiled node.js library: ${node_addon_path}"
    fi
    export OCL_NODE_PATH="${node_addon_path}"
    cd "${project_dir}/src/npmpackage"
    npm install
    npm run build-node
    cd ../../examples/nodejs
    npm link ../../src/npmpackage
    if [ "${build_type_lower}" = "debug" ]; then
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
    if ! command_exists "${python_executable}"; then
        invalid_argument "Cannot find Python executable: ${python_executable}"
    fi
    python_path=$("${python_executable}" -c 'import sys; print(sys.executable)')

    if [ -n "${OCL_PYTHON_PIP_INSTALL}" ]; then
        # PEP 517 installs pyproject.toml build dependencies. Arrays preserve spaces.
        pip_options=(
            "--config-settings=cmake.build-type=${build_type}"
            "--config-settings=cmake.define.Python3_EXECUTABLE=${python_path}"
        )
        if [ -n "${OCL_GENERATOR}" ]; then
            pip_options+=("--config-settings=cmake.args=-G" "--config-settings=cmake.args=${OCL_GENERATOR}")
        fi
        if [ -n "${OCL_GENERATOR_PLATFORM}" ]; then
            pip_options+=("--config-settings=cmake.args=-A" "--config-settings=cmake.args=${OCL_GENERATOR_PLATFORM}")
        fi
        if [ -n "${OCL_DISABLE_OPENMP}" ]; then
            pip_options+=("--config-settings=cmake.define.USE_OPENMP=OFF")
        fi
        if [ -n "${OCL_PYTHON_PREFIX}" ]; then
            pip_options+=("--config-settings=cmake.define.Python3_ROOT_DIR=${OCL_PYTHON_PREFIX}")
        fi
        cd "${project_dir}"
        "${python_executable}" -m pip install --verbose "${pip_options[@]}" .
    else
        mkdir -p "${build_dir}"
        cd "${build_dir}"
        install_prefix_fallback=$("${python_executable}" -c 'import sysconfig; print(sysconfig.get_paths()["purelib"])')
        python_cmake_options=("-DPython3_EXECUTABLE=${python_path}")
        if pybind11_cmake_dir=$("${python_executable}" -m pybind11 --cmakedir); then
            python_cmake_options+=("-Dpybind11_DIR=${pybind11_cmake_dir}")
        fi
        if [ -n "${OCL_GENERATOR}" ]; then
            python_cmake_options+=("-G" "${OCL_GENERATOR}")
        fi
        if [ -n "${OCL_GENERATOR_PLATFORM}" ]; then
            python_cmake_options+=("-A" "${OCL_GENERATOR_PLATFORM}")
        fi
        set -x
        cmake \
            "${python_cmake_options[@]}" \
            -D CMAKE_BUILD_TYPE="${build_type}" \
            -D BUILD_PY_LIB="ON" \
            -D CMAKE_INSTALL_PREFIX="${OCL_INSTALL_PREFIX:-${install_prefix_fallback}}" \
            ${OCL_DISABLE_OPENMP:+"-DUSE_OPENMP=OFF"} \
            ${OCL_PYTHON_PREFIX:+"-DPython3_ROOT_DIR=${OCL_PYTHON_PREFIX}"} \
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
    "${python_executable}" test.py
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
        -D CMAKE_INSTALL_PREFIX="${OCL_INSTALL_PREFIX:-"${install_prefix_fallback}"}" \
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
    prettyprint "Building Python library"
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
