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
[here](http://www.cmake.org/cmake/resources/software.html).

CMake is a cross-platform system for build automation. It can generate Visual
Studio project files on Windows, Xcode project files on Mac OS X and Makefiles
on Unix systems. After installing CMake, you can see a list of generators
provided on your system by typing `cmake` in your terminal and pressing enter.
The supported generators should be listed near the bottom of the printout.

Qt 5.3.0 is also required on all platforms. It can be downloaded
[here](http://qt-project.org/downloads).

A compiler that supports C++11 is also required.
Compilers that are known to work are:
* GCC 4.8+
* Apple's compiler that ships with 10.9+
* Visual Studio 2013+


Build Options
=============

* FORCE_BUNDLED_COPIES=1
  * Only use bundled copies and don't use system versions of dependencies.
* WIN_INSTALLER_USE_64BIT_CRT=1
  * Windows only.
  * Must be specified if building a 64 bit installable package.


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

1. Start the "Cross Tools Command Prompt" or "vcshell".
2. Set the path.
3. Create the build dir.
4. Generate nmake build files.
5. build Sigil.

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

Create the build directories and get the Sigil source code.

    $ mkdir -p ~/sigil-x.y.z/src ~/sigil-x.y.z/build ~/sigil-x.y.z/run
    $ cd ~/sigil-x.y.z/src
    $ wget https://sigil.googlecode.com/files/Sigil-x.y.z-Code.zip
    $ unzip Sigil-x.y.z-Code.zip
    $ cd ~/sigil-x.y.z/build
    $ cmake -G "Unix Makefiles" -DCMAKE_PREFIX_PATH=/opt/Qt5.y.z/5.y.z/gcc/lib/cmake -DCMAKE_INSTALL_PREFIX=~/sigil-x.y.z/run -DCMAKE_BUILD_TYPE=Release -DFORCE_BUNDLED_COPIES=1 ~/sigil-x.y.z/src
    $ make install

Create a script to run Sigil:

    $ echo "
    $ export LD_LIBRARY_PATH=/opt/Qt5.y.z/5.y.z/gcc/lib
    $ ~/sigil-x.y.z/run/bin/sigil" > ~/sigil.sh
    $ chmod +x ~/sigil.sh

### Run Sigil

    $ ~/sigil.sh


Building from source in the git repository for release is *NOT* recommended, since code in
the git repository is not stable. Do not open a bug report against a non-release version.
