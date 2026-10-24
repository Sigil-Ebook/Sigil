#!/bin/bash -e

# WARNING!!  Do not attempt to run this script outside of a docker container. Doing so can damage your system!!

# This script is for building the sigil appimage
# Please run this script in docker image: ubuntu:24.04
# E.g: docker run --rm -v `git rev-parse --show-toplevel`:/reporoot ubuntu:24.04 /reporoot/.github/workflows/build_sigil_on_distro.sh
# Artifacts will copy to the same directory.

set -o pipefail

#export QT6_VER="6.4"
#export QT6_VER_FULL=6.4.2
#export QT6_FN="642"
export LC_ALL="C.UTF-8"
export DEBIAN_FRONTEND=noninteractive
export PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig
SELF_DIR="$(readlink -f "$(dirname "$0")")"

retry() {
  # max retry 5 times
  try=5
  # sleep 30 sec every retry
  sleep_time=15
  for i in $(seq ${try}); do
    echo "executing with retry: $@" >&2
    if eval "$@"; then
      return 0
    else
      echo "execute '$@' failed, tries: ${i}" >&2
      sleep ${sleep_time}
    fi
  done
  echo "execute '$@' failed" >&2
  return 1
}

prepare_baseenv() {
  rm -f /etc/apt/sources.list.d/*.list*

  # keep debs in container for store cache in docker volume
  rm -f /etc/apt/apt.conf.d/*
  echo 'Binary::apt::APT::Keep-Downloaded-Packages "true";' > /etc/apt/apt.conf.d/01keep-debs
  echo -e 'Acquire::https::Verify-Peer "false";\nAcquire::https::Verify-Host "false";' > /etc/apt/apt.conf.d/99-trust-https

  # Since cmake 3.23.0 CMAKE_INSTALL_LIBDIR will force set to lib/<multiarch-tuple> on Debian
  #echo '/usr/local/lib/x86_64-linux-gnu' > /etc/ld.so.conf.d/x86_64-linux-gnu-local.conf
  #echo '/usr/local/lib64' > /etc/ld.so.conf.d/lib64-local.conf
  retry apt-get update

  retry apt-get install -y --allow-downgrades software-properties-common apt-transport-https
  retry apt-get update

  retry apt-get install -y \
    make \
    build-essential \
    cmake \
    cmake-data \
    pkg-config \
    liblzma-dev \
    libglu1-mesa-dev \
    libhunspell-dev \
    libpcre2-dev \
    libminizip-dev\
    qt6-webengine-dev \
    qt6-webengine-dev-tools \
    qt6-base-dev-tools \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    qt6-l10n-tools \
    libqt6svg6-dev \
    libqt6webenginecore6-bin \
    python3-dev \
    python3-pip \
    python3-lxml \
    python3-six \
    python3-css-parser \
    python3-dulwich


  apt-get autoremove --purge -y
  # strip all compiled files by default
  export CFLAGS='-s'
  export CXXFLAGS='-s'
  # Force refresh ld.so.cache
  ldconfig
}


setup_qt6() {
  python3 -m pip install aqtinstall
  python3 -m aqt install-qt --outputdir /opt/sigiltools/Qt linux desktop "${QT6_VER_FULL}" gcc_64 -m qtpositioning qtpdf qtwebchannel qtserialport qtimageformats qtwebengine
  export PATH=/opt/sigiltools/Qt/$QT6_VER_FULL/gcc_64/bin:$PATH
  export LD_LIBRARY_PATH=/opt/sigiltools/Qt/${QT6_VER_FULL}/gcc_64/lib:$LD_LIBRARY_PATH
  echo "Qt version $(qmake -v)"
}


build_sigil() {
  mkdir -p /build
  cd /build
  cmake /reporoot \
    -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_SYSTEM_LIBS=1 \
    -DSYSTEM_LIBS_REQUIRED=1
  make -j$(getconf _NPROCESSORS_ONLN)
  make install
}


time {
  prepare_baseenv
  #setup_qt6
  build_sigil
}
