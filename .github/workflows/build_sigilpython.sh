#!/bin/bash -e

# WARNING!!  Do not attempt to run this script outside of a docker container. Doing so can damage your system!!

# This script is for building python for sigil appimage
# Please run this script in docker image: ubuntu:22.04
# E.g: docker run --rm -v `git rev-parse --show-toplevel`:/reporoot ubuntu:22.04 /reporoot/.github/workflows/build_sigilpython.sh
# Artifacts will copy to the same directory.

set -o pipefail

export PYTHON_VER="3.13.2"
export OPENSSL_VER="3.0.16"
export LC_ALL="C.UTF-8"
export DEBIAN_FRONTEND=noninteractive
export PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig
SELF_DIR="$(dirname "$(readlink -f "${0}")")"

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

# join array to string. E.g join_by ',' "${arr[@]}"
join_by() {
  local separator="$1"
  shift
  local first="$1"
  shift
  printf "%s" "$first" "${@/#/$separator}"
}

prepare_baseenv() {
  rm -f /etc/apt/sources.list.d/*.list*

  # keep debs in container for store cache in docker volume
  rm -f /etc/apt/apt.conf.d/*
  echo 'Binary::apt::APT::Keep-Downloaded-Packages "true";' > /etc/apt/apt.conf.d/01keep-debs
  echo -e 'Acquire::https::Verify-Peer "false";\nAcquire::https::Verify-Host "false";' > /etc/apt/apt.conf.d/99-trust-https

  # Since cmake 3.23.0 CMAKE_INSTALL_LIBDIR will force set to lib/<multiarch-tuple> on Debian
  echo '/usr/local/lib/x86_64-linux-gnu' > /etc/ld.so.conf.d/x86_64-linux-gnu-local.conf
  echo '/usr/local/lib64' > /etc/ld.so.conf.d/lib64-local.conf
  retry apt-get update

  retry apt-get install -y --allow-downgrades software-properties-common apt-transport-https python3-tk tk-dev 
  retry apt-get update

  retry apt-get install -y \
    make \
    build-essential \
    curl \
    libssl-dev \
    libgdbm-dev \
    libgdbm-compat-dev \
    liblzma-dev \
    libbz2-dev \
    libsqlite3-dev \
    libffi-dev \
    pkg-config \
    unzip \
    zip \
    zlib1g-dev

#libncursesw5-dev \
#libreadline-dev \

  apt-get autoremove --purge -y
  # strip all compiled files by default
  export CFLAGS='-s'
  export CXXFLAGS='-s'
  # Force refresh ld.so.cache
  ldconfig
}

prepare_ssl() {
  openssl_url="https://github.com/openssl/openssl/releases/download/openssl-${OPENSSL_VER}/openssl-${OPENSSL_VER}.tar.gz"
  echo "OpenSSL version: ${OPENSSL_VER}"
  mkdir -p "/usr/src/openssl-${OPENSSL_VER}/"
  if [ ! -f "/usr/src/openssl-${OPENSSL_VER}/.unpack_ok" ]; then
    retry curl -kSL "${openssl_url}" \| tar zxf - -C "/usr/src/openssl-${OPENSSL_VER}/" --strip-components 1
    touch "/usr/src/openssl-${OPENSSL_VER}/.unpack_ok"
  fi
  cd "/usr/src/openssl-${OPENSSL_VER}"
  ./Configure no-tests --openssldir=/etc/ssl
  make -j$(nproc)
  make install_sw
  ldconfig
}

prepare_python() {
  python_url="https://www.python.org/ftp/python/${PYTHON_VER}/Python-${PYTHON_VER}.tar.xz"
  echo "Python version: ${PYTHON_VER}"
  mkdir -p "/usr/src/python3-${PYTHON_VER}/"
  if [ ! -f "/usr/src/python3-${PYTHON_VER}/.unpack_ok" ]; then
    retry curl -kSL "${python_url}" \| tar Jxf - -C "/usr/src/python3-${PYTHON_VER}/" --strip-components 1
    touch "/usr/src/python3-${PYTHON_VER}/.unpack_ok"
  fi
  cd "/usr/src/python3-${PYTHON_VER}"
  ./configure --prefix=/opt/sigiltools/python --enable-shared --enable-optimizations --enable-loadable-sqlite-extensions --disable-test-modules
  make -j$(nproc)
  make install
  cd /opt/sigiltools
  tar -cvJf sigilpython${PYTHON_VER}.tar.xz --exclude='**/__pycache__/*' python
  cp -fv sigilpython${PYTHON_VER}.tar.xz /reporoot/
  ldconfig
}

time {
  prepare_baseenv
  #prepare_ssl
  prepare_python
}