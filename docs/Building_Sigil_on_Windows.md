# <center>Building Sigil on Windows</center>

## General Overview

To build Sigil on Windows, you need to get/do the following things:

1. [Visual Studio 2017](#vsstudio) The free Community Edition will work fine
   (VS2015 will work as well -- it will just require the msvc2015 version of Qt5)
2. [CMake](#cmake) (3.0 or higher)
3. [Inno Setup](#inno) (the latest Unicode version available recommended)
4. [Qt5.12.3](#qt5) (**NOTE**: The standard precompiled binaries will work but Sigil patches a few things)
5. [QtWebKit](#qtwebkit)
5. [Python 3.7.x](#python)
6. [The Sigil source code](#sigil) (downloaded zipfile or a git clone)
7. [Building Sigil](#build)

## <a name="vsstudio"/>Visual Studio

VS2015 is now a firm Windows minimum starting with Sigil 0.9.7, but Sigil is built with VS2017 in the latest versions of Sigil. Begin with making sure you have a working version of [Visual Studio](https://visualstudio.microsoft.com/vs/community/), (the free Community edition will work fine). Note that VS2017's 2015 developer prompts might need some tweaking if you're going that route.

The instructions given here will focus on using the command-line cmake and nmake tools. But if you're more comfortable in an IDE, you should find sufficient instructions to get you going. I simply don't use the IDE. Too many fiddly bits (sign-ins and expiring licenses for free software) for my taste. But it did work the last time I tried it.

From the Start button (you're on your own if you don't have one), go to "All Programs->Visual Studio 2017" and find the command prompt you'll need for your platform. Probably "VS2017 x64 Native Tools Command Prompt" for building a 64-bit package. If you're going to be building 32-bit packages, then use the "VS2017 x86 Native Tools Command Prompt". Create a shortcut to the applicable command-prompt on your Desktop. That's what you'll be using to configure and build Sigil.

If you're going to use the Visual Studio IDE and cmake-gui, you won't need to use these command-prompts.

## <a name="cmake"/>Getting CMake
CMake 3.0 or better is required. I'm using 3.12.x Download it from [cmake.org](http://www.cmake.org) and install it. **Make sure CMake's "bin" directory (the one containing "cmake.exe") is added to your PATH**.

## <a name="inno"/>Inno Setup
Get the unicode version (5.5.9 at the time of this writing) from [jrsoftware.org](http://www.jrsoftware.org/isdl.php) make sure you say yes to the Preprocessor option when installing. **Also make sure the Inno Setup directory (the one containing "ISCC.exe") is added to your PATH**. There is no 64-bit version of Inno Setup, but you can still use it to create 64-bit program installers.

## <a name="qt5"/>Qt5.12.3
Download qt-opensource-windows-x86-5.12.3.exe from [Qt's Website](http://download.qt.io/archive/qt/5.12/5.12.3) and install the msvc2017 (or msvc2015) component for the architecture you will be building Sigil for (you can install both msvc2017 and msvc2017_64 if you like).

Once you have Qt5.12.3 for Visual Studio installed, **make sure its "bin" directory (the one containing "windeployqt.exe) is added to your PATH** (see the QtWebKit section below if you want to download a 64-bit patched version of Qt5.12.3 for VS2017)

## <a name="qtwebkit"/>QtWebKit
You need to beg, borrow or build a version of QtWebKit from [QtWebKit Reloaded's Github repository](https://github.com/annulen/webkit). You can use one of the binary releases if you like, but the latest at the time of this writing (5.212.0-alpha2) includes a buggy version of libxml2 that will result in html entity doubling (Linux Sigil users have already encountered this bug). Perhaps there will be a new release by the time you're reading this and you can just use it as is. That is certainly my hope in the long run. In the meantime, there's some fairly good documentation in their [wiki](https://github.com/annulen/webkit/wiki) for building QtWebKit on Windows (I'm using the "Conan" instructions). I'm building the release versions of Sigil using a version of Qt5.12.3/QtWebKit that I've compiled myself with VS2017; which can be found [here](https://github.com/dougmassay/win-qtwebkit-5.212/releases/tag/v5.212-1). So if you want to use the exact, patched versions of Qt5.12.3 and QtWebKit that I've used to release Sigil, feel free to download the whole shebang [from the same place](https://github.com/dougmassay/win-qtwebkit-5.212/releases/tag/v5.212-1). It's the archive named: MyQtx64-5.12.3_WE-WK_VS2017.7z (NOTE: You can't use this if you're building Sigil with Visual Studio 2015 -- you're also on your own with a 32-bit version).

## <a name="python"/>Getting Python 3.7
**This is important**. If you're going to be building the 64-bit version of Sigil, you need to install the 64-bit version of Python 3.7. If you're building a 32-bit version of Sigil then you need to install a 32-bit version of Python 3.7.

The official Windows Sigil installer uses Python 3.7 from [Python.org](http://www.python.org) (3.7.2 at the time of this writing). Other flavors of Python may work, but you're on your own if they don't. Download it and install it. If you install somewhere that requires special privileges to add/remove files, you may need to use an administator command prompt to install Sigil's extra Python module dependencies. **I recommend installing Python to the default location ($USER/appdata) to avoid that problem. I also recommend allowing the Python installer to add Python to your PATH**. This will make it easier for Sigil to locate the necessary Python pieces it needs, and will make it easy to install the extra Python modules using Pythons "pip" tool. I'm going to assume you've done so for the rest of these instructions.

### Getting the extra Python module dependencies
After installing Python 3.7, I recommend making sure Python's pip/setuptools is updated to the latest version. The easiest way to do this is to open a command prompt (the shortcut to the VS2015 command prompt you made on your desktop [in step 1](#vs2015) will work fine) and type:

>`python -m pip install -U pip`

Once finished, you can begin to install the extra modules needed by Sigil.

+ six
+ html5lib
+ regex (2019.3.12 minimum required starting with Sigil 0.9.14)
+ cssselect
+ cssutils
+ chardet
+ Pillow (v5.4.1 recommended/verified)
+ lxml (v4.3.2 recommended/verified)
+ PyQt5 (5.12.2 recommended/verified)

From the same command prompt you updated pip with, install the "six" module with the following command:

>`pip install six`

Repeat for the next five modules:

>`pip install html5lib`

etc...

### Installing Pillow

Other versions of Pillow may work fine, but Sigil's installer build is predicated on v5.4.1, To install that specific version, use the following pip command.

>`pip install Pillow==5.4.1`

### Installing lxml.

Version 4.3.2 comes with precompiled binary wheels for Windows. Not all versions do. So if you want to install a different version, you'll need to find out if there's precompiled binaries for Windows or not. Install a specific version with pip using the following command

>`pip install lxml==4.3.2`

### Installing PyQt5.

Like lxml, not all versions of PyQt5 will have compatible binaries that will work with Sigil's Qt5 and Python. Stick to version 5.12.1 and everything should work with Python 3.7 and Qt5.12.1 (the trick is to always select a version of PyQt that will work with Sigil's version of Qt and Python)

>`pip install PyQt5==5.12.2`


## <a name="sigil"/>Getting Sigil's Source Code

You can clone the Sigil Github repository (Requires a Windows git client - I use the [portable version from here](https://github.com/git-for-windows/git/releases/latest)):

>`git clone https://github.com/Sigil-Ebook/Sigil.git`

Or you can download a specific release's zipfile from Sigil's [releases page](https://github.com/Sigil-Ebook/Sigil/releases/latest) on Github (0.9.10 at the time of this writing).

I recommend the latter method, as the github repository version might not always be stable at any given moment (even though we try hard not to leave it broken).

Unzip the source code. Rename the uppermost directory to something useful like "sigil-src". Unless you like typing extra-long directory names in command-prompts--in which case, don't rename it. Remember this location, you'll need it when generating the nmake makefiles with cmake

### Preparing Sigil's Source Code

To build the Sigil installer package, you'll need to copy the Visual Studio 2017 redistributable runtime installer to the `<sigil-src>\installer` folder (the one that contains the Sigil.iss file). These redistributable files can usually be found somewhere in Visual Studio's folder structure:

`C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Redist\MSVC\14.XX.XXXXX\`

`C:\Program Files (x86)\Microsoft Visual Studio 14\VC\redist\1033\`

vcredist_x64.exe for 64-bit builds, and vcredist_x86.exe for 32-bit builds.

**The file names are important so don't rename them**. Just copy the appropriate one to the "installer" folder in Sigil's source mentioned above.

## <a name="build"/>Configuring and building Sigil (and the Sigil installer package)

### Configuring Sigil with cmake

With all the pre-requisites met and all the necessary additions to your PATH, the only thing left to do is generate the Sigil makefiles with cmake.

Using the shortcut to the proper VSStudio command-prompt created in [step 1](#vsstudio), cd to a suitable empty directory for building Sigil (I recommend "sigil-build", or some such similar name), and issue the following command:

> `cmake -G "NMake Makefiles" -DWIN_INSTALLER_USE_64BIT_CRT=1 -DQt5_DIR="C:\Qt\Qt5.12.3\5.12.3\mscv2017_64\lib\cmake\Qt5" -DCMAKE_BUILD_TYPE=Release "C:\path\to\sigil-src"`

Leave out the -DWIN_INSTALLER_USE_64BIT_CRT=1 part if you're building a 32-bit version of Sigil with the "VS2017 x86 Native Tools Command Prompt" shortcut.

Obviously change the paths to match where you've actually installed Qt5.12.3 and the Sigil source code. For instance: using my Specially compiled version of Qt5/WebKit, it would look like:

`cmake -G "NMake Makefiles" -DWIN_INSTALLER_USE_64BIT_CRT=1 -DQt5_DIR="C:\MyQtx64_WE\Qt5.12.3\lib\cmake\Qt5" -DCMAKE_BUILD_TYPE=Release "C:\path\to\sigil-src"`

**NOTE**: The -DQt5_DIR will be "C:\Qt\Qt5.12.3\5.12.3\mscv2017(_64)\lib\cmake\Qt5" if you installed the standard Qt5.12.3 to its default location

If this completes successfully, then you're ready to compile Sigil (leave the command prompt open).

You can also generate Visual Studio Project/Solution Files with cmake by using:

> `cmake -G "Visual Studio 14 2017 Win64" WIN_INSTALLER_USE_64BIT_CRT=1 -DQt5_DIR="C:\Qt\Qt5.12.3\5.12.3\mscv2017(_64)\lib\cmake\Qt5" -DCMAKE_BUILD_TYPE=Release "C:\path\to\sigil-src"`

Leave off "Win64" and -DWIN_INSTALLER_USE_64BIT_CRT=1 if you're building the 32-bit version of Sigil.

You can also use cmake-gui (double-click on cmake-gui in the cmake/bin directory) and avoid using the command-prompt altogether if you wish (although you're on your own in figuring out how to enter all the cmake configuration options in the gui).

The following three cmake options are used to manually specify which Python3 you want to use when building Sigil instead of relying on the included cmake utilities to try and automatically find a suitable version. They can come in handy it you have multiple versions of Python 3 installed on your computer.

-DPYTHON_LIBRARY=`<the full path to the python3.x library (ex. python37.lib)>`

-DPYTHON_INCLUDE_DIR=`<the path to the directory where python3.x's header files (python.h) can be found>`

-DPYTHON_EXECUTABLE=`<the full path to the python3.x binary (python.exe)>`

If you don't want to build/include the bundled Python environment in the Sigil installer, use the -DPKG_SYSTEM_PYTHON=0 in the CMake configure command to disable it. **NOTE**: you'll have to configure an external Python interpeter for running Sigil plugins. The "Use Bundled Python" feature will be unavailable.

### Compiling Sigil

If you generated NMake Makefiles with cmake (like I do), then compile Sigil by typing `nmake` (at the same command-prompt you just configured with) to begin building Sigil. If it completes without error, you're ready to build the installer package (leave the command prompt open).

If you generated Visual Studio 2017 projects/solutions, then open the Sigil.sln file in the build directory; make sure the solution configuration is set to "Release"; select the ALL_BUILD project in the Solution Explorer and build the ALL_BUILD project from the Build menu (Build->Build ALL_BUILD). **Note: don't build the solution**. If it completes without error, you're ready to build the installer package. It is not possible at this time build a Debug version of Sigil with Visual Studio. So make sure the solution configuration is changed to build "Release" only.

### Building the Sigil installer package

If you generated NMake Makefiles and have successfully compiled Sigil, then type `nmake makeinstaller` (at the same command prompt you just compiled Sigil with) to build the Sigil installer package. If it completes succesfully, the Sigil installer will be placed in the sigil-build directory's "installer" folder (NOTE: that's the *build* directory and not the *source* directory). If it doesn't complete succesfully, you may have to delete the "temp_folder" in the build directory before proceeding.

If you generated Visual Studio 2017 project/solutions and have built the ALL_BUILD project successfully, then select the "makeinstaller" project in the Solution Explorer and build the makeinstaller project from the Build menu (Build->Build makeinstaller). If it completes succesfully, the Sigil installer will be placed in the sigil-build directory's "installer" folder (NOTE: that's the *build* directory and not the *source* directory). If it doesn't complete succesfully, you may have to delete the "temp_folder" in the build directory before proceeding.
