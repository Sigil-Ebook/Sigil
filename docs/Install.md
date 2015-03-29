Introduction
============

Instructions for building Sigil from source, on all platforms.

Experience with compiling on your chosen platform is assumed. This is not a step
by step guide but general information to help fill in gaps.

Current releases of Sigil (as of 0.5) bundle the majority of dependencies. Qt,
CMake and general build tools are required to be installed separately. Sigil's
build setup will try to use system installed versions of any bundled
dependencies and fall back to the bundled copies. This is provided for
connivance for Linux packagers but it is not supported.


General notes
=============

You will need CMake 3.0 or later on all platforms. You can download it
[here](http://www.cmake.org/download/).

CMake is a cross-platform system for build automation. It can generate Visual
Studio project files on Windows, Xcode project files on Mac OS X and Makefiles
on Unix systems. After installing CMake, you can see a list of generators
provided on your system by typing `cmake` in your terminal and pressing enter.
The supported generators should be listed near the bottom of the printout.

Qt 5.4.0 is also required on all platforms. It can be downloaded
[here](http://qt-project.org/downloads).

A compiler that supports C++11 is also required.
Compilers that are known to work are:
* GCC 4.8+
* Apple's compiler that ships with 10.9+
* Visual Studio 2013+

Primary development happens on OS X. Limited testing happens on Windows.
Linux is not considered a platform for user submitted bug reports. Only
patches to fix issues specific to Linux will be evaluated. But only if
they do not interfere with other platforms.


Build Options
=============

* USE_SYSTEM_LIBS=1
  * Use system libraries for dependencies if they are available.
* SYSTEM_LIBS_REQUIRED=1
  * System libraries are required in place of all 3rdparty bundled copies.
  * Build will fail if any system libraries are not found. If CMake failes
    check which libraries are marked as bundled to know what's missing.
* WIN_INSTALLER_USE_64BIT_CRT=1
  * Windows only.
  * Must be specified if building a 64 bit installable package.
* CODE_SIGN_ID=XYZ
  * OS X Only.
  * Sigil can be automatically signed.
  * Where XYZ is the identity to sign with. This could be common name or the
    SHA1 hash of the certificate to use.
  * You can check if the application was signed successfully using
    `spctl --assess --type execute bin/Sigil.app`
    No output means it was. Any output means it was not signed successfully.
* LINUX_PACKAGE_TYPE
  * Can be "deb" or "rpm"

Other useful variables are:
* CMAKE_PREFIX_PATH
  * E.g. /usr/local/opt/qt5/lib/cmake/
  * Which allows you to specifiy where cmake's Qt5 files are located if they are not part of the system path.
* Qt5_DIR
  * E.g. "C:\Qt\x64\5.4.0\5.4\msvc2013_64\lib\cmake\Qt5"
  * Location of Qt5 CMake .cmake files for finding cmake. It's more proper to set this than the prefix path above.
* CMAKE_OSX_SYSROOT
  * E.g. /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk/
  * Which allows you to specify the SDK to build against. This allows you to build on 10.10 but support
    running on 10.9 for example.


Compiling on Windows
--------------------

Only Microsoft's Visual C++ compiler is supported.
You'll need Visual Studio to build Sigil. The express edition can be used.

The installer directory contains the files needed to produce a binary installer
for Windows. You need to provide the vcredist for your compiler. You need to
put it in the installer directory and name it vcredist_x86.exe or
vcredist_x64.exe.

On Windows a `makeinstaller` target is provided which will build a binary
installer. For that to work, you need to have
[Inno Setup](http://www.jrsoftware.org/isinfo.php) on Windows. The installer
builder needs to be installed and on the system PATH.

### Method 1 (nmake)

*Note*: This assumes building an x64 build of Sigil. A x86 build is very similar.

1. Start the "Cross Tools Command Prompt".
2. Set the path.
3. Create the build dir.
4. Generate nmake build files.
5. build Sigil.


An example of building using VS2013 x64 Cross Tools Command Prompt:

    > PATH=%PATH%;"C:\Program Files (x86)\CMake";"C:\Program Files (x86)\Inno Setup 5";"C:\Qt\x64\Qt5.3.1\5.3\msvc2013_64\bin
    > mkdir build
    > cd build
    > cmake -G "NMake Makefiles" WIN_INSTALLER_USE_64BIT_CRT=1 -DFORCE_BUNDLED_COPIES=1 -DCMAKE_BUILD_TYPE=Release ..
    > nmake
    > nmake makeinstaller


### Method 2 (Visual Studio)

You can generate Visual Studio project files by creating a new folder *outside*
of the source distribution.

Now navigate to that folder with a terminal like cmd.exe or PowerShell.  Then
type in and run the following:

    > cmake -G "Visual Studio 10" /path/to/extracted/folder

This should create sln and vcproj files for Visual Studio in that directory.
You can also generate project files for some other VS version. You can get a
list of all supported generators by typing in and running `cmake`.

The default build procedure will build "Sigil.exe"; if you want to package that
with the required DLL's into an installer, build the `makeinstaller` project.

There is also an [Add-in](http://qt-project.org/downloads) for VS on Qt's
website. It will make it easier to develop Qt applications like Sigil, but is
not strictly necessary.


Compiling on OS X
-----------------

On OS X a `makedmg` target is provided which will build a redistributable dmg.
There is also an `addframeworks` target which will add all necessary
dependencies to the .app for distribution. The `makedmg` will not invoke
`addframeworks`.


### Method 1 (make)

This is the preferred method and the method used by all Sigil developers.

    $ mkdir build
    $ cd build
    $ cmake -DFORCE_BUNDLED_COPIES=1 -DCMAKE_BUILD_TYPE=Release ..
    $ make
    $ make addframeworks
    $ make makedmg


### Method 2 (XCode)

You can generate Xcode project files by creating a new folder *outside* of the
source distribution.

Now navigate to that folder with the Terminal. Then type in and run the following:

    $ cmake -G Xcode /path/to/extracted/folder

This should create Xcode project files in that directory. The default build
procedure will build "Sigil.app"; if you want to package that into a DMG file,
invoke the `makedmg` build target.


Compiling on Linux
------------------

Here is an example of installing Sigil on in your Home directory.
NOTE: While Linux instructions are provided Linux is not considered
      a supported platform. Bug reports against Linux build will not
      be accepted or evaluated.

Create the build directories and get the Sigil source code.

    $ mkdir -p ~/sigil-x.y.z/src ~/sigil-x.y.z/build ~/sigil-x.y.z/run
    $ cd ~/sigil-x.y.z/src
    $ wget https://sigil.googlecode.com/files/Sigil-x.y.z-Code.zip
    $ unzip Sigil-x.y.z-Code.zip
    $ cd ~/sigil-x.y.z/build
    $ cmake -G "Unix Makefiles" -DCMAKE_PREFIX_PATH=/opt/Qt5.y.z/5.y.z/gcc/lib/cmake -DCMAKE_INSTALL_PREFIX=~/sigil-x.y.z/run -DCMAKE_BUILD_TYPE=Release -DFORCE_BUNDLED_COPIES=1 ~/sigil-x.y.z/src
    $ make install

On Linux a 'linuxbinpkg' target is provided which will attempt to build a standalone binary installer. For that to work,
you need to have dpkg-deb installed on Debian/Ubuntu systems and rpm-build installed on systems that use rpm based packages.
Instead of "make install", issue 'sudo make linuxbinpkg' to build the deb or rpm binary package. (note: sudo or su should
not be used to build rpm package). The *.deb or *.rpm file will be created in the 'installer' subdirectory
of the build directory. Install/remove using your system's dpkg -i/-r (Debian/Ubuntu) or rpm -i/-e.

### Running Sigil on Linux

If your INSTALL_PREFIX was either /usr/bin or /usr/local/bin), all you need to do is execute 'sigil' from a terminal.
You can also create a shortcut to the sigil.desktop file in INSTALL_PREFIX/share/applications/ to launch Sigil.
Sigil should also be available in your desktop menu system. It's typically found under the "Office" applications submenu.

If you've installed Sigil to your home directory or another prefix not on your path, add INSTALL_PREFIX/bin to your path
(via .profile or .bash_profile or .bashrc or similar) and execute 'sigil' from a terminal.

If you relocate your Qt5 libraries after building Sigil, you'll need to edit the Sigil launch script (INSTALL_PREFIX/bin/sigil)
and supply the new location of the Qt5 libraries (only necessary if you've installed via the typical "make, make install" method).

Sigil by default (when packaging) will install sigil-bin into /usr/lib/sigil and a launcher script into /usr/bin. Helper files
such as plugins launchers, translations, dictionaries... will be installed into /usr/share/sigil. The base location (/usr)
can be controlled by the environment variable SIGIL_EXTRA_ROOT. Meaning  the location of the helper files will become
SIGIL_EXTRA_ROOT/share/sigil/.

The directory structure needs to always be:

* install_base/bin/sigil (wrapper file)
* install_base/share/sigil (helper files)
* install_base/lib/sigil/sigil (binary)


Git Builds
==========

Building from source in the git repository for release is *NOT* recommended, since code in
the git repository is not stable. Do not open a bug report against a non-release version.
