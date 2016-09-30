#<center>Building Qt 5.6.1-1 (suitable for Sigil) on Windows</center>

Sigil needs a special version of Qt5.6.1-1 built with QtWebKit (which has been removed from official binary distributions) using Visual Studio 2015. If this sounds like too much work, feel free to download one of my pre-compiled versions of Qt5.6.1 that will work for building Sigil. Links can be found in the "Building_Sigil_on_Windows" document (always check the online docs folder for the latest links). If you want to build Qt yourself, read on for some tips.

First off ... this is not going to be the definitive, step-by-step instructions for building Qt on Windows. There are plenty of online resources which do that. I'm going to focus on the specific needs for building a version of Qt5.6.1-1 (with QtWebKit added back in) with VS2015 that is suitable for using to build Sigil.

Here are a some resources for building Qt on Windows that I suggest reading up on to familiarize yourself with the process:

[This document describes](http://doc.qt.io/qt-5/windows-building.html) downloading/extracting the source. Setting up some environment variables and a build environment with a cmd script and creating a desktop shortcut. I highly recommend that method. It's much easier to add the necessary things to your PATH without messing up your system PATH. Obviously, you'll need to adjust a few things for Visual Studio 2015. Download JOM and use it if you like (you'll need to add it to your PATH), but I just use nmake, myself. I'll include the contents of my cmd file at the end of these instructions as an example.

[This next document](http://doc.qt.io/qt-5/windows-requirements.html) discusses some external requirements necessary for building Qt. I **do** recommend building Qt5.6.1 with ICU support enabled, so pay close attention to those details. In order to do so, you'll either need to build ICU yourself (with VS2015) or find some suitable precompiled binaries. I highly recommend the latter. [The Sigmoid site](http://www.npcglib.org/~stathis/blog/precompiled-icu/) is fantastic for this. Download a version (I used [ICU v55.1](http://www.npcglib.org/~stathis/blog/precompiled-icu-past/) at the time of this writing) built with VS2015 (make sure you get the proper 32/64-bit version to match whatever version of Qt/Sigil you're going to build). The documents tells you what needs to be added to the build environment's LIB and INCLUDE environment variables so ICU support will be compiled into Qt.

The main things to take away from the requirements document are:

* ICU: download it from sigmoid (for VS2015) and add its lib and include dirs to the build environment's LIB and INCLUDE variables.
* Perl: (I recommend [Strawberry Perl for Windows](http://strawberryperl.com/), myself). Install it and add it to your PATH.
* [Python](https://www.python.org/): make sure it can be found on your PATH
* Add the gnuwin32/bin folder of the downloaded/extracted Qt5.6.1-1 source to your PATH.

Not mentioned in that document is that you'll also need [Ruby for Windows](http://rubyinstaller.org/) to build QtWebKit when we plug it back into the source. Install it and add it to your PATH.

[The next document](http://doc.qt.io/qt-5/configure-options.html) describes some configuration options for Qt. I highly suggest doing a "shadow build" as described in this document. It keeps the source clean in case you need to build multiple versions (or you want to start all over when you mess up!). I also recommend setting a prefix to install to after Qt compiles. This makes it easy to zip it all up and move it around to any machine you may want to use to build Sigil. The document also describes how to skip certain modules that may help you speed up the compilation time.

Now that you've downloaded and extracted the source code for Qt5.6.1-1, and you've installed all the dependencies (and added/prepended all of their directories to your PATH), we need to plug QtWebKit's source back into Qt5.6.1-1's source tree. You'll need a Windows git client for this. I recommend one with a command-line, bash-type interface. I use the [portable version from here](https://github.com/git-for-windows/git/releases/latest).

Using git bash: clone the QtWebKit source with the following command:

> `git clone git://code.qt.io/qt/qtwebkit.git

Once that completes, copy the resulting "qtwebkit" folder into the root of the Qt5.6.1-1 source (the same place you see the qtbase, qtdeclarative, etc ... folders). I recommend keeping the QtWebKit source in a safe place for future use. They're probably not going to keep it around forever.

Then edit the qtwebkit/.qmake.conf file so that the MODULE_VERSION matches the version of Qt you're building (5.6.1).

That's it. You're ready to build Qt5.6.1 with QtWebKit. Launch your command prompt shortcut (created using the instructions in the first online resource above), cd to your empty build directory (if the shortcut didn't do it for you) and issue the configure command with all of the options you've decided upon. The configure command I use for building the Qt that gets distributed with the official Sigil installer package is this:

> `path\to\configure.bat -opensource -confirm-license -mp -opengl dynamic -shared -prefix C:\MyQt64\Qt5.6.1 -nomake examples -nomake tests -release -icu -skip qtwebengine -no-openssl -qt-zlib -qt-libpng -qt-libjpeg -qt-freetype -qt-harfbuzz'

(obviously change the path to where the configure.bat path *really* is in the Qt source, and the prefix to something that works for you)

Once that's done, type "nmake" (or jom, if you're using it) and wait a long, long time for it to complete. Once you have a successful build, type "nmake install" to install Qt to the path indicated.

At this point, I recommend copying the ICU dlls to that same install prefix (the bin folder). The three you need are: icudt5x.dll, icuin5x.dll, and icuuc5x.dll. I'd also copy the correct d3dcompiler_47.dll file to that same bin folder in the install directory. The 64-bit version can be found in:

C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\amd64\d3dcompiler_47.dll.

If you need the 32-bit version, it is:

C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\d3dcompiler_47.dll.

Copying these files will ensure that they will be found (without adding them to your PATH) when building Sigil's installer package.

That's it. When building Sigil, point the CMAKE_PREFIX_PATH to C:\MyQt64\Qt5.6.1\lib\cmake (or whatever your prefix was set to when configuring Qt).

---

The info from this point on is the contents of my cmd script (mentioned in the first online document above). Feel free to use it to create your own. Mine is for building a 64-bit Qt5.6.1

REM Set up \Microsoft Visual Studio 2015 for an x64 build.<br>
CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64<br>
SET QTSRC=C:\CustomQt\qt-5.6.1-src<br>
SET QMAKESPEC=win32-msvc2015

SET INCLUDE=C:\icu55_x64\include;%INCLUDE%<br>
SET PATH=%QTSRC%\gnuwin32\bin;C:\jom;C:\icu55_x64\lib;%PATH%<br>
SET LIB=C:\icu55_x64\lib;%LIB%<br>
SET CL=/MP

A desktop link can then be created by specifying the command %SystemRoot%\system32\cmd.exe /E:ON /V:ON /k c:\qt5vars.cmd as application and C:\CustomQt\qt-build as working directory.

