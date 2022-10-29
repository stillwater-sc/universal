#!/bin/bash

runner_os=$1
inputs_compiler=$2

if [[ -z ${GITHUB_ACTION+z} ]]; then
    ECHO=echo
else
    ECHO=
fi

# $1 - runner os
# $2 - compiler
# $3 - version; optional
function install_compilers {
    if [[ -n $3 ]]; then
        _VER=$3
        P_VER='-'$_VER
    fi

    case $2 in
        gcc | g++)
            _CC=gcc
            _CXX=g++
            PKGS="$_CC$P_VER $_CXX$P_VER"
            WINPKGS="mingw --version=$_VER"
            MACPKGS="gcc@$_VER"
        ;;
        clang | clang++)
            _CC=clang
            _CXX=clang++
            PKGS="$_CC$P_VER"
            WINPKGS="llvm --version=$_VER"
            MACPKGS="llvm@$_VER"
        ;;
        *)
            echo "::error ::Unknown compiler '$2'"
            exit 1
        ;;
    esac

    case $1 in
        Linux)
            echo "::group::apt install"
            echo sudo apt update
            $ECHO sudo apt update
            echo sudo apt install $PKGS
            $ECHO sudo apt install $PKGS
            echo "::endgroup::"
            echo "::set-output name=cc::$_CC$P_VER"
            echo "::set-output name=cxx::$_CXX$P_VER"
        ;;
        Windows)
            echo "::group::choco install"
            echo choco upgrade $WINPKGS -y --no-progress --allow-downgrade
            $ECHO choco upgrade $WINPKGS -y --no-progress --allow-downgrade
            echo "::endgroup::"
            echo "::set-output name=cc::$_CC"
            echo "::set-output name=cxx::$_CXX"
        ;;
        macOS)
            case ${_CC}${P_VER} in
                gcc-*)
                    echo "::group::Brew install"
                    echo brew update
                    $ECHO brew update
                    echo brew install $MACPKGS
                    $ECHO brew install $MACPKGS
                    echo brew link $MACPKGS
                    $ECHO brew link $MACPKGS
                    echo "::endgroup::"
                    echo "::set-output name=cc::/usr/local/bin/$_CC$P_VER"
                    echo "::set-output name=cxx::/usr/local/bin/$_CXX$P_VER"
                ;;
                gcc)
                    echo "::warning ::For MacOS gcc must have specified version, fallback to clang"
                    echo "::set-output name=cc::clang"
                    echo "::set-output name=cxx::clang++"
                ;;
                clang*)
                    echo "::notice ::For MacOS compilers fallback to default system clang"
                    echo "::set-output name=cc::clang"
                    echo "::set-output name=cxx::clang++"
                ;;
            esac
        ;;
        *)
            echo "::error ::Unsupported runner '$1'"
            exit 1
        ;;
    esac
}

ARR=($(echo $inputs_compiler | tr '-' '\n'))

#echo "::notice::Input: ${inputs_compiler} Parsed: ${ARR[@]} Size: ${#ARR[@]}"

if [[ ${#ARR[@]} == 2 ]]; then
  case ${ARR[0]} in
    gcc | g++)
        COMPILER=gcc
    ;;
    clang | clang++)
        COMPILER=clang
    ;;
    *)
    ;;
  esac

  case ${ARR[1]} in
    latest)
        VERSION=
    ;;
    *)
        VERSION=${ARR[1]}
    ;;
  esac

else
  case ${ARR[0]} in
    latest | gcc | g++)
        COMPILER=gcc
    ;;
    clang | clang++)
        COMPILER=clang
    ;;
    *)
    ;;
  esac
  VERSION=

fi

#echo "::notice::Runner: ${runner_os} Compiler: ${COMPILER} Version: ${VERSION}"

install_compilers $runner_os $COMPILER $VERSION
