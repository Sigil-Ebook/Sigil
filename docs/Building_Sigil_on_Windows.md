#<center>Building Sigil on Windows</center>

##General Overview

To build Sigil on Windows, you need to get/do the following things:

1. [Visual Studio 2015](#vs2015) (The free Community Edition will work fine)
2. [CMake](#cmake) (3.0 or higher)
3. [Inno Setup](#inno) (the latest Unicode version available recommended)
4. [Qt5.6.2](#qt5) (**NOTE**: The standard precompiled binaries are no longer sufficient!)
5. [Python 3.5.x](#python)
6. [The Sigil source code](#sigil) (downloaded zipfile or a git clone)
7. [Building Sigil](#build)

## <a name="vs2015"/>Visual Studio 2015

VS2015 is now a firm Windows requirement starting with Sigil 0.9.7. Python 3.5 is built using VS2015, Qt5.6 needs to be built with VS2015 (with QtWebKit) and Sigil needs to be built using VS2015. So VS2015 it is for the forseeable future. So begin with making sure you have a working version of [Visual Studio 2015](https://beta.visualstudio.com/vs/community/) (the free Community edition will work fine).

The instructions given here will focus on using the command-line cmake and nmake tools. But if you're more comfortable in an IDE, you should find sufficient instructions to get you going. I simply don't use the IDE. Too many fiddly bits (sign-ins and expiring licenses for free software) for my taste. But it did work the last time I tried it.

From the Start button (you're on your own if you don't have one), go to "All Programs->Visual Studio 2015->Visual Studio Tools->Windows Desktop Command Prompts" and find the command prompt you'll need for your platform. Probably "VS2015 x64 Native Tools Command Prompt" for building a 64-bit package. If you're going to be building 32-bit packages, then use the "VS2015 x86 Native Tools Command Prompt". Create a shortcut to the applicable command-prompt on your Desktop. That's what you'll be using to configure and build Sigil.

If you're going to use the Visual Studio IDE and cmake-gui, you won't need to use these command-prompts.

##<a name="cmake"/>Getting CMake
CMake 3.0 or better is required. I'm using 3.6.x Download it from [cmake.org](http://www.cmake.org) and install it. **Make sure CMake's "bin" directory (the one containing "cmake.exe") is added to your PATH**.

##<a name="inno"/>Inno Setup
Get the unicode version (5.5.9 at the time of this writing) from [jrsoftware.org](http://www.jrsoftware.org/isdl.php) make sure you say yes to the Preprocessor option when installing. **Also make sure the Inno Setup directory (the one containing "ISCC.exe") is added to your PATH**. There is no 64-bit version of Inno Setup, but you can still use it to create 64-bit program installers.

##<a name="qt5"/>Qt5.6.2
Bit of a catch-22 here, unfortunately. Sigil for Windows requires VS2015, so Sigil requires a version of Qt5 built with VS2015. Sigil also needs a version of Qt5 that includes QtWebkit. But there are no precompiled versions of Qt5 with QtWebKit built with VS2015. Which means you need to build a special version of Qt5.6.2 with QtWebKit added back in yourself (or get someone else to do it for you and provide you a zip of the binary Qt5 SDK). You'll find a "Building_Qt_for_Sigil_on_Windows" document in the docs folder which outlines the general procedure for building Qt5.6.2 with QtWebKit enabled yourself.

For those who are looking for a shortcut, I'm also going to post links to my precompiled and stripped down versions of Qt5.6.2 that are tailored for building the release versions of Sigil that get distributed with the official Sigil installer packages. These archives are compressed using 7-Zip, so you'll need to [install that](http://www.7-zip.org/) to extract them. I recommend extracting them to the root of the C: drive (C:\MyQt64 or C:\MyQt32). They're links to my my Google Drive, so be prepared for slowness and/or unavailability at times.

**NOTE: My precompiled Qt5.6.2 binaries are built and distributed in agreement with Qt's GPL license requirements. They're tailored specifically for Sigil's needs, and they should only be used to build Sigil or other Open Source, GPL compatible software.**

[64-bit Precompiled (GPL) Qt5.6.2 with QtWebKit](https://goo.gl/mPpnQa) ~46-48Mb

[32-bit Precompiled (GPL) Qt5.6.2 with QtWebKit](https://goo.gl/dtVGOz) ~40-42Mb

Once you have a version of Qt5.6.2 (with QtWebKit enabled) built with VS2015 and installed, **make sure its "bin" directory (the one containing "windeployqt.exe) is added to your PATH** ("C:\MyQt64|32\Qt5.6.2\bin" if using my precompiled Qt5.6.2 package, and it was extracted to the root of the C: drive).


##<a name="python"/>Getting Python 3.5
**This is important**. If you're going to be building the 64-bit version of Sigil, you need to install the 64-bit version of Python 3.5. If you're building a 32-bit version of Sigil then you need to install a 32-bit version of Python 3.5.

The official Windows Sigil installer uses Python 3.5 from [Python.org](http://www.python.org) (3.5.2 at the time of this writing). Other flavors of Python may work, but you're on your own if they don't. Download it and install it. If you install somewhere that requires special privileges to add/remove files, you may need to use an administator command prompt to install Sigil's extra Python module dependencies. **I recommend installing Python to the default location ($USER/appdata) to avoid that problem. I also recommend allowing the Python installer to add Python to your PATH**. This will make it easier for Sigil to locate the necessary Python pieces it needs, and will make it easy to install the extra Python modules using Pythons "pip" tool. I'm going to assume you've done so for the rest of these instructions.

###Getting the extra Python module dependencies
After installing Python 3.5, I recommend making sure Python's pip/setuptools is updated to the latest version. The easiest way to do this is to open a command prompt (the shortcut to the VS2015 command prompt you made on your desktop [in step 1](#vs2015) will work fine) and type:

>`python -m pip install -U pip`

Once finished, you can begin to install the extra modules needed by Sigil. Seven of the modules are dead-simple to install (provided you have an internet connection), the eighth is a tad more involved. The first seven to install are:

+ six
+ html5lib
+ regex
+ Pillow
+ cssselect
+ cssutils
+ chardet

From the same command prompt you updated pip with, install the "six" module with the following command:

>`pip install six`

Repeat for the next six modules

>`pip install html5lib`

etc...


The next Python module to install is lxml. It's apparently too cool to be installed easily with pip on Windows, so follow lxml's own advice and download one of [Christoph Gohlke's precompiled Windows binaries](http://www.lfd.uci.edu/~gohlke/pythonlibs/#lxml) instead.

Getting the right one can be fiddly, so be careful and choose the correct one. The downloads are in the form of "wheel" files (.whl)--installable with pip after downloading.

At the time of this writing, the latest version of lxml was 3.7.0. So we're looking for the file that starts with "lxml-3.7.0". We need the one for Python 3.5, so the next portion of the file name will be cp35-cp35m (the "cp" is for CPython). The last portion of the filename is for 32- or 64-bit. This needs to match the version of Python you installed. So for 64-bit it's "win_amd64" and for 32-bit, it's win32.

Thus for lxml-3.7.0, the file we want is "lxml-3.7.0-cp35-cp35m-win_amd64.whl". Download it somewhere, **but DO NOT rename it**. The filename is relevant for the install.

In the same command-prompt, "cd" to the directory where you downloaded the wheel (.whl) and install with pip:

>`pip install lxml-3.7.0-cp35-cp35m-win_amd64.whl`

or

>`pip install lxml-3.7.0-cp35-cp35m-win32.whl`

If you're building a 32-bit version of Sigil and thus have the 32-bit version of Python 3.5 installed.

The last Python module to install is PyQt5. You can't install this one with pip either. The PyPi version is based on Qt5.7 instead of the version of Qt Sigil uses so you'll need to build PyQt5 manually.
(**You can install a [binary version of PyQt5-5.6](https://sourceforge.net/projects/pyqt/files/PyQt5/PyQt-5.6/) if you like, but it won't have the python bindings to QtWebKit. If you can live with that, have at it. Just make sure you install the correct one for your Sigil build -- x32 or x64**).


PyQt5 depends on sip which you can install with pip

>`pip install sip`

Brush up on the PyQt build instructions from the [PyQt website](http://pyqt.sourceforge.net/Docs/PyQt5/installation.html).

Download [the source](https://sourceforge.net/projects/pyqt/files/PyQt5/PyQt-5.6/PyQt5_gpl-5.6.zip/download) for PyQt5 v5.6 and extract it somewhere on your hard drive.

Using the shortcut to the proper VS2015 command-prompt created in [step 1](#vs2015), cd to where you extracted the PyQt5 source and configure the source with a command like:

>`python configure.py --no-designer-plugin --no-qml-plugin --no-qsci-api --no-sip-files --no-stubs --no-tools --disable QtNfc --disable QtBluetooth --disable QtWinExtras --disable QtLocation --disable QtXml --disable QtXmlPatterns --disable QtWebSockets --disable QtHelp --disable QtTest --disable QtDBus --disable QtDesigner --no-docstrings --confirm-license --destdir C:\Users\<username>\AppData\Local\Programs\Python\Python35\Lib\site-packages`

If you're building a 32-bit version of Sigil, use:

>`python configure.py --no-designer-plugin --no-qml-plugin --no-qsci-api --no-sip-files --no-stubs --no-tools --disable QtNfc --disable QtBluetooth --disable QtWinExtras --disable QtLocation --disable QtXml --disable QtXmlPatterns --disable QtWebSockets --disable QtHelp --disable QtTest --disable QtDBus --disable QtDesigner --no-docstrings --confirm-license --destdir C:\Users\<username>\AppData\Local\Programs\Python\Python35-32\Lib\site-packages`

Change the --destdir path if to match where your Python was installed. If everything goes according to plan, you can build PyQt with the command:

>`nmake`

If something breaks while building QPicture then change line 649 in QtGui/sipQtGuiQPicture.cpp from:

>`return new QPicture[sipNrElem];`

to:

>`return NULL;`

and issue nmake again.

Once it completes building, install with:

>`nmake install`


##<a name="sigil"/>Getting Sigil's Source Code

You can clone the Sigil Github repository (Requires a Windows git client - I use the [portable version from here](https://github.com/git-for-windows/git/releases/latest)):

>`git clone https://github.com/Sigil-Ebook/Sigil.git`

Or you can download a specific release's zipfile from Sigil's [releases page](https://github.com/Sigil-Ebook/Sigil/releases/latest) on Github (0.9.7 at the time of this writing). You'll find the source code zipfile at the bottom of the page, in the Downloads section. Named Sigil-X.X.X-Code.zip

I recommend the latter method, as the github repository version might not always be stable at any given moment (even though we try hard not to leave it broken).

Unzip the source code. Rename the uppermost directory ("Sigil-0.X.X" if you've download the Sigil-0.X.X-Code.zip file ) to something useful like "sigil-src". Unless you like typing extra-long directory names in command-prompts--in which case, don't rename it. Remember this location, you'll need it when generating the nmake makefiles with cmake

###Preparing Sigil's Source Code

To build the Sigil installer package, you'll need to copy the Visual Studio 2015 redistributable runtime installer to the `<sigil-src>\installer` folder (the one that contains the Sigil.iss file). These redistributable files can usually be found somewhere in VS2015's folder structure:

`C:\Program Files (x86)\Microsoft Visual Studio 14\VC\redist\1033\`

vcredist_x64.exe for 64-bit builds, and vcredist_x86.exe for 32-bit builds.

**The file names are important so don't rename them**. Just copy the appropriate one to the "installer" folder in Sigil's source mentioned above.

##<a name="build"/>Configuring and building Sigil (and the Sigil installer package)

###Configuring Sigil with cmake

With all the pre-requisites met and all the necessary additions to your PATH, the only thing left to do is generate the Sigil makefiles with cmake.

Using the shortcut to the proper VS2015 command-prompt created in [step 1](#vs2015), cd to a suitable empty directory for building Sigil (I recommend "sigil-build", or some such similar name), and issue the following command:

> `cmake -G "NMake Makefiles" WIN_INSTALLER_USE_64BIT_CRT=1 -DCMAKE_PREFIX_PATH="C:\path\to\qt5.6.2\lib\cmake" -DCMAKE_BUILD_TYPE=Release "C:\path\to\sigil-src"`

Leave out the WIN_INSTALLER_USE_64BIT_CRT=1 part if you're building a 32-bit version of Sigil with the "VS2015 x86 Native Tools Command Prompt" shortcut.

Obviously change the paths to match where you've actually placed the Qt5.6 files and the Sigil source code.

**NOTE**: The -DCMAKE_PREFIX_PATH will be "C:\MyQt(64|32)\Qt5.6.2\lib\cmake" if using my precompiled Qt5.6.2 package, and it was extracted to the root of the C: drive

If this completes successfully, then you're ready to compile Sigil (leave the command prompt open).

You can also generate Visual Studio Project/Solution Files with cmake by using:

> `cmake -G "Visual Studio 14 2015 Win64" WIN_INSTALLER_USE_64BIT_CRT=1 -DCMAKE_PREFIX_PATH="C:\path\to\qt5.6.2\lib\cmake" -DCMAKE_BUILD_TYPE=Release "C:\path\to\sigil-src"`

Leave off "Win64" and WIN_INSTALLER_USE_64BIT_CRT=1 if you're building the 32-bit version of Sigil with the "VS2015 x86 Native Tools Command Prompt" shortcut.

You can also use cmake-gui (double-click on cmake-gui in the cmake/bin directory) and avoid using the command-prompt altogether if you wish (although you're on your own in figuring out how to enter all the cmake configuration options in the gui).

###Compiling Sigil

If you generated NMake Makefiles with cmake (like I do), then compile Sigil by typing `nmake` (at the same command-prompt you just configured with) to begin building Sigil. If it completes without error, you're ready to build the installer package (leave the command prompt open).

If you generated Visual Studio 2015 projects/solutions, then open the Sigil.sln file in the build directory; make sure the solution configuration is set to "Release"; select the ALL_BUILD project in the Solution Explorer and build the ALL_BUILD project from the Build menu (Build->Build ALL_BUILD). **Note: don't build the solution**. If it completes without error, you're ready to build the installer package.

###Building the Sigil installer package

If you generated NMake Makefiles and have successfully compiled Sigil, then type `nmake makeinstaller` (at the same command prompt you just compiled Sigil with) to build the Sigil installer package. If it completes succesfully, the Sigil installer will be placed in the sigil-build directory's "installer" folder (NOTE: that's the *build* directory and not the *source* directory). If it doesn't complete succesfully, you may have to delete the "temp_folder" in the build directory before proceeding.

If you generated Visual Studio 2015 project/solutions and have built the ALL_BUILD project successfully, then select the "makeinstaller" project in the Solution Explorer and build the makeinstaller project from the Build menu (Build->Build makeinstaller). If it completes succesfully, the Sigil installer will be placed in the sigil-build directory's "installer" folder (NOTE: that's the *build* directory and not the *source* directory). If it doesn't complete succesfully, you may have to delete the "temp_folder" in the build directory before proceeding.
