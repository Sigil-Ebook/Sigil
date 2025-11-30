#!/bin/bash -e

# WARNING!!  Do not attempt to run this script outside of a docker container. Doing so can damage your system!!

# This script is for building the sigil appimage
# Please run this script in docker image: ubuntu:22.04
# E.g: docker run --rm -v `git rev-parse --show-toplevel`:/reporoot ubuntu:22.04 /reporoot/.github/workflows/build_sigil_appimage.sh
# Artifacts will copy to the same directory.

set -o pipefail

export PY_SHORT_VER="3.13"
export PYTHON_VER="3.13.2"
export QT6_VER="6.9"
export QT6_VER_FULL="6.9.3"
export QT6_FN="693"
export LC_ALL="C.UTF-8"
export DEBIAN_FRONTEND=noninteractive
export PKG_CONFIG_PATH=/usr/local/lib64/pkgconfig
SELF_DIR="$(readlink -f "$(dirname "$0")")"
COMMITISH="${1}"

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
    wget \
    file \
    liblzma-dev \
    libnss3-dev \
    libasound2-dev \
    libxkbfile-dev \
    libnspr4-dev \
    libncurses-dev \
    libxkbcommon-dev \
    libxkbcommon-x11-dev \
    libwayland-dev \
    libwayland-egl-backend-dev \
    libxcb1-dev \
    libxcb1-dev \
    libxcb-cursor-dev \
    libxcb-glx0-dev \
    libxcb-icccm4-dev \
    libxcb-image0-dev \
    libxcb-keysyms1-dev \
    libxcb-randr0-dev \
    libxcb-render-util0-dev \
    libxcb-shape0-dev \
    libxcb-shm0-dev \
    libxcb-sync-dev \
    libxcb-util-dev \
    libxcb-xfixes0-dev \
    libxcb-xinerama0-dev \
    libxcb-xkb-dev \
    mesa-common-dev \
    libglu1-mesa-dev \
    libxcb-cursor-dev \
    libgtk-3-dev \
    libtiff-dev \
    libwebp-dev \
    zlib1g-dev \
    tk-dev \
    zip \
    zstd \
    zsync \


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
  echo "Python version $(python3 --version)"
}

setup_qt6() {
  python3 -m pip install --root-user-action ignore aqtinstall
  python3 -m aqt install-qt --outputdir /opt/sigiltools/Qt linux desktop "${QT6_VER_FULL}" linux_gcc_64 -m qtpositioning qtpdf qtwebchannel qtserialport qtimageformats
  export PATH=/opt/sigiltools/Qt/$QT6_VER_FULL/gcc_64/bin:$PATH
  export LD_LIBRARY_PATH=/opt/sigiltools/Qt/${QT6_VER_FULL}/gcc_64/lib:$LD_LIBRARY_PATH
  echo "Qt version $(qmake -v)"
}

setup_webengine() {
  we_url="https://github.com/dougmassay/win-qtwebkit-5.212/releases/download/v5.212-1/AppImageWebEngine${QT6_FN}.tar.xz"
  if [ -f "/reporoot/AppImageWebEngine${QT6_FN}.tar.xz" ]; then
    echo "Using local QtWebEngine archive"
    tar -xJf "/reporoot/AppImageWebEngine${QT6_FN}.tar.xz" -C /opt/sigiltools
  else
    if [ ! -f "/usr/src/AppImageWebEngine${QT6_FN}.tar.xz.download_ok" ]; then
      echo "Downloading remote QtWebEngine archive"
      rm -f "/usr/src/AppImageWebEngine${QT6_FN}.tar.xz"
      retry curl -kLC- -o "/usr/src/AppImageWebEngine${QT6_FN}.tar.xz" "${we_url}"
      touch "/usr/src/AppImageWebEngine${QT6_FN}.tar.xz.download_ok"
    fi
    tar -xJf "/usr/src/AppImageWebEngine${QT6_FN}.tar.xz" -C /opt/sigiltools
  fi
  ls -l "/opt/sigiltools/Qt/${QT6_VER_FULL}/gcc_64/libexec"
  export Qt6_Dir="/opt/sigiltools/Qt/${QT6_VER_FULL}/gcc_64/lib/cmake/Qt6"
  export Qt6_DIR="/opt/sigiltools/Qt/${QT6_VER_FULL}/gcc_64/lib/cmake/Qt6"
  export QT_PLUGIN_PATH="/opt/sigiltools/Qt/${QT6_VER_FULL}/gcc_64/plugins"
}

clone_sigil_src() {
  cd /
  git config --global --add safe.directory /
  git clone https://github.com/Sigil-Ebook/Sigil.git
}

pkg_python() {
  mkdir -p /build
  python3 -m pip install --root-user-action ignore patchelf
  python3 -m pip install --root-user-action ignore -r "${SELF_DIR}/requirements.txt"
  python3 "${SELF_DIR}/appimg_python3_gather.py" /build/sigil.AppDir "${PY_SHORT_VER}"
}

get_appimage_tools() {
  cd /build
  wget -nv https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
  chmod +x linuxdeploy-x86_64.AppImage
  wget -nv https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
  chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
  wget -nv https://github.com/linuxdeploy/linuxdeploy-plugin-appimage/releases/download/continuous/linuxdeploy-plugin-appimage-x86_64.AppImage
  chmod +x linuxdeploy-plugin-appimage-x86_64.AppImage
  cp /lib/x86_64-linux-gnu/libtiff.so.5 /build/sigil.AppDir/usr/lib/
  cp /lib/x86_64-linux-gnu/libwebp.so.7 /build/sigil.AppDir/usr/lib/
}

build_sigil() {
  cd /build
  cmake /reporoot \
    -G "Ninja" \
    -DCMAKE_PREFIX_PATH="/opt/sigiltools/Qt/${QT6_VER_FULL}/gcc_64/lib/cmake" \
    -DPKG_SYSTEM_PYTHON=1 \
    -DAPPIMAGE_BUILD=1 \
    -DCMAKE_BUILD_TYPE=Release \
    -DINSTALL_HICOLOR_ICONS=1 \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_INSTALL_LIBDIR=lib \
    -DCMAKE_SKIP_RPATH=ON
  ninja -j$(getconf _NPROCESSORS_ONLN)
  DESTDIR=sigil.AppDir ninja install
}

build_appimage() {
  cd /build
  # If COMMITISH not provided as a parameter, try to pick
  # up the Sigil version from cmake-created version file.
  if [ -z "$COMMITISH" ]; then
    if [ -f appimage_version.txt ]; then
      COMMITISH=$(cat appimage_version.txt | tr -d '\n')
      echo "Using COMMITISH of ${COMMITISH} for OUTPUT_APP_NAME"
    fi
  fi
  export LINUXDEPLOY_OUTPUT_APP_NAME="Sigil-${COMMITISH}"
  export APPIMAGE_EXTRACT_AND_RUN=1
  DEPLOY_PLATFORM_THEMES=1 \
  DISABLE_COPYRIGHT_FILES_DEPLOYMENT=1 \
  LD_LIBRARY_PATH=lib:sigil.AppDir/usr/lib/python$PY_SHORT_VER/site-packages/pillow.libs:$LD_LIBRARY_PATH \
  EXTRA_PLATFORM_PLUGINS=libqwayland-generic.so \
  EXTRA_QT_MODULES="waylandcompositor" \
  ./linuxdeploy-x86_64.AppImage --appdir sigil.AppDir --custom-apprun=${SELF_DIR}/AppRun --plugin qt
  python3 "${SELF_DIR}/appimg_cleanup.py" /build/sigil.AppDir/usr/lib $PY_SHORT_VER
  #cp -fv "${SELF_DIR}/AppRun" /build/sigil.AppDir/
  LDAI_UPDATE_INFORMATION="gh-releases-zsync|Sigil-Ebook|Sigil|latest|Sigil-*x86_64.AppImage.zsync" \
  LDAI_VERBOSE=1 \
  ./linuxdeploy-plugin-appimage-x86_64.AppImage --appdir=sigil.AppDir
  cp -fv Sigil-*-x86_64.AppImage* /reporoot/
}

time {
  prepare_baseenv
  prepare_buildenv
  setup_python
  setup_qt6
  setup_webengine
  #clone_sigil_src
  pkg_python
  get_appimage_tools
  build_sigil
  build_appimage
}
