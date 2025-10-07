#!/bin/bash -e

# WARNING!!  Do not attempt to run this script outside of a docker container. Doing so can damage your system!!

# This script is for building qtwebengine for sigil appimage
# Please run this script in docker image: ubuntu:22.04
# E.g: docker run --rm -v `git rev-parse --show-toplevel`:/reporoot ubuntu:22.04 /reporoot/.github/workflows/build_sigilwebengine.sh
# Artifacts will copy to the same directory.

set -o pipefail

export PYTHON_VER="3.13.2"
export QT6_VER="6.9"
export QT6_VER_FULL="6.9.3"
export QT6_FN="693"
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

  retry apt-get install -y --allow-downgrades software-properties-common apt-transport-https
  retry apt-get update

  retry apt-get install -y \
    make \
    build-essential \
    curl \
    gperf \
    bison \
    flex \
    libfontconfig1-dev \
    libdbus-glib-1-dev \
    libgl1-mesa-dev \
    libgbm-dev \
    libnss3-dev \
    libasound2-dev \
    libpulse-dev \
    libdrm-dev \
    libxshmfence-dev \
    libxkbfile-dev \
    libxcomposite-dev \
    libxcursor-dev \
    libxrandr-dev \
    libxi-dev \
    x11proto-dev \
    libxtst-dev \
    libxkbcommon-dev \
    libxcb-dri3-dev \
    libxdamage-dev \
    zip \
    zlib1g-dev


  apt-get autoremove --purge -y
  # strip all compiled files by default
  export CFLAGS='-s'
  export CXXFLAGS='-s'
  # Force refresh ld.so.cache
  ldconfig
}

prepare_buildenv() {
  # Install Cmake
  if ! which cmake &>/dev/null; then
    cmake_latest_ver="$(retry curl -ksSL --compressed https://cmake.org/download/ \| grep "'Latest Release'" \| sed -r "'s/.*Latest Release\s*\((.+)\).*/\1/'" \| head -1)"
    cmake_binary_url="https://github.com/Kitware/CMake/releases/download/v${cmake_latest_ver}/cmake-${cmake_latest_ver}-linux-x86_64.tar.gz"
    cmake_sha256_url="https://github.com/Kitware/CMake/releases/download/v${cmake_latest_ver}/cmake-${cmake_latest_ver}-SHA-256.txt"
    if [ -f "/usr/src/cmake-${cmake_latest_ver}-linux-x86_64.tar.gz" ]; then
      cd /usr/src
      cmake_sha256="$(retry curl -ksSL --compressed "${cmake_sha256_url}" \| grep "cmake-${cmake_latest_ver}-linux-x86_64.tar.gz")"
      if ! echo "${cmake_sha256}" | sha256sum -c; then
        rm -f "/usr/src/cmake-${cmake_latest_ver}-linux-x86_64.tar.gz"
      fi
    fi
    if [ ! -f "/usr/src/cmake-${cmake_latest_ver}-linux-x86_64.tar.gz" ]; then
      retry curl -kLo "/usr/src/cmake-${cmake_latest_ver}-linux-x86_64.tar.gz" "${cmake_binary_url}"
    fi
    tar -zxf "/usr/src/cmake-${cmake_latest_ver}-linux-x86_64.tar.gz" -C /usr/local --strip-components 1
  fi
  cmake --version

  # Install Ninja
  if ! which ninja &>/dev/null; then
    ninja_ver="$(retry curl -ksSL --compressed https://ninja-build.org/ \| grep "'The last Ninja release is'" \| sed -r "'s@.*<b>(.+)</b>.*@\1@'" \| head -1)"
    ninja_binary_url="https://github.com/ninja-build/ninja/releases/download/${ninja_ver}/ninja-linux.zip"
    if [ ! -f "/usr/src/ninja-${ninja_ver}-linux.zip.download_ok" ]; then
      rm -f "/usr/src/ninja-${ninja_ver}-linux.zip"
      retry curl -kLC- -o "/usr/src/ninja-${ninja_ver}-linux.zip" "${ninja_binary_url}"
      touch "/usr/src/ninja-${ninja_ver}-linux.zip.download_ok"
    fi
    unzip -d /usr/local/bin "/usr/src/ninja-${ninja_ver}-linux.zip"
  fi
  echo "Ninja version $(ninja --version)"
}

setup_python() {
  mkdir -p /opt/sigiltools
  python_url="https://github.com/dougmassay/win-qtwebkit-5.212/releases/download/v5.212-1/sigilpython${PYTHON_VER}.tar.xz"
  if [ -f "/reporoot/sigilpython${PYTHON_VER}.tar.xz" ]; then
    echo "Using local Python archive"
    tar -xJf "/reporoot/sigilpython${PYTHON_VER}.tar.xz" -C /opt/sigiltools
  else
    if [ ! -f "/usr/src/sigilpython${PYTHON_VER}.tar.xz.download_ok" ]; then
      rm -f "/usr/src/sigilpython${PYTHON_VER}.tar.xz"
      retry curl -kLC- -o "/usr/src/sigilpython${PYTHON_VER}.tar.xz" "${python_url}"
      touch "/usr/src/sigilpython${PYTHON_VER}.tar.xz.download_ok"
    fi
    tar -xJf "/usr/src/sigilpython${PYTHON_VER}.tar.xz" -C /opt/sigiltools
  fi
  export PATH=/opt/sigiltools/python/bin:$PATH
  export LD_LIBRARY_PATH=/opt/sigiltools/python/lib:$LD_LIBRARY_PATH
  export PYTHONHOME=/opt/sigiltools/python
  which python3
  echo "Upgrading pip..."
  export PIP_ROOT_USER_ACTION=ignore
  python3 -m ensurepip
  python3 -m pip install --upgrade --force-reinstall --root-user-action ignore pip --disable-pip-version-check --no-warn-script-location
  python3 -m pip install --root-user-action ignore html5lib 
  echo "Python version $(python3 --version)"
}

setup_nodejs() {
  curl -fsSL https://deb.nodesource.com/setup_23.x -o nodesource_setup.sh
  bash nodesource_setup.sh
  apt-get install -y nodejs
  node -v
}

setup_qt6() {
  python3 -m pip install --root-user-action ignore aqtinstall
  python3 -m aqt install-qt --outputdir /opt/sigiltools/qt linux desktop "${QT6_VER_FULL}" linux_gcc_64 -m qtpositioning qtpdf qtwebchannel qtserialport qtimageformats
  export PATH=/opt/sigiltools/qt/$QT6_VER_FULL/gcc_64/bin:$PATH
  echo "Qt version $(qmake -v)"
}

setup_webengine_src() {
  mkdir -p /opt/we_src
  webengine_url="https://download.qt.io/official_releases/qt/${QT6_VER}/${QT6_VER_FULL}/submodules/qtwebengine-everywhere-src-${QT6_VER_FULL}.tar.xz"
  webengine_sha256_url="https://download.qt.io/official_releases/qt/${QT6_VER}/${QT6_VER_FULL}/submodules/qtwebengine-everywhere-src-${QT6_VER_FULL}.tar.xz.sha256"
  if [ -f "/usr/src/qtwebengine-everywhere-src-${QT6_VER_FULL}.tar.xz" ]; then
    cd /usr/src
    webengine_sha256="$(retry curl -ksSL --compressed "${webengine_sha256_url}" \| grep "qtwebengine-everywhere-src-${QT6_VER_FULL}.tar.xz")"
    if ! echo "${webengine_sha256}" | sha256sum -c; then
      rm -f "/usr/src/qtwebengine-everywhere-src-${QT6_VER_FULL}.tar.xz"
    fi
  fi
  if [ ! -f "/usr/src/qtwebengine-everywhere-src-${QT6_VER_FULL}.tar.xz" ]; then
    retry curl -kLo "/usr/src/qtwebengine-everywhere-src-${QT6_VER_FULL}.tar.xz" "${webengine_url}"
  fi
  tar -xJf "/usr/src/qtwebengine-everywhere-src-${QT6_VER_FULL}.tar.xz" -C /opt/we_src --strip-components 1
  cd /opt/we_src
  patch -p1 < /reporoot/docs/Qt_Patches/qt672_fix_h6_insertParagraph.patch
  patch -p1 < /reporoot/docs/Qt_Patches/qt693_fix_mesa_issues.patch
  patch -p1 < /reporoot/docs/Qt_Patches/qt693_memory_leak_fix_886ff03.patch
  mkdir /opt/we_src/build
  cd /opt/we_src/build
  qt-configure-module .. -list-features
  qt-configure-module .. -feature-qtwebengine-build -feature-qtwebengine-widgets-build -no-feature-qtpdf-quick-build -no-feature-qtwebengine-quick-build -no-feature-webengine-system-libxml -feature-webengine-proprietary-codecs -webengine-ffmpeg -webengine-icu -- -D CMAKE_BUILD_TYPE=MinSizeRel
  cmake --build . --parallel --verbose
  cmake --install . --prefix /opt/Qt/${QT6_VER_FULL}/gcc_64
  cd /opt
  #XZ_OPT='-9' tar -cJf AppImageWebEngine${QT6_FN}.tar.xz --exclude='*.debug' Qt
  tar -cJf AppImageWebEngine${QT6_FN}.tar.xz --exclude='*.debug' Qt
  cp -fv AppImageWebEngine${QT6_FN}.tar.xz /reporoot/
}

prepare_baseenv
prepare_buildenv
setup_python
setup_nodejs
setup_qt6
time {
  setup_webengine_src
}
