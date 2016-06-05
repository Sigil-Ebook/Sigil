#<center>Building Sigil on Linux</center>

##General Overview

To build Sigil on Linux, you need to get/do the following things:

1. [A Linux build-toolchain](#gcc) with a C++11 capable compiler (gcc4.8.x-ish or higher recommended)
2. [CMake](#cmake) (3.0 or higher)
3. [Qt5.4.0 - Qt5.5.1](#qt5) (Qt5.4.2 is currently the "official" Sigil Qt5)
4. [Python 3.4](#python) (or higher)
5. [The Sigil source code](#sigil) (downloaded tarball/zipfile or a git clone)
6. [Build/Install Sigil](#build)
7. [Test Sigil's Plugin Framework](#testing)
8. [Advanced Stuff](#advanced)

Since I'm basically an Ubuntu/Debian guy at heart, I'll be mentioning stuff like:
 
>`sudo apt-get install`

from here on out. You'll have to forgive me for not knowing all the yum/pacman/emerge equivalents. It's not a slight--I can assure you.

## <a name="gcc"/>Linux Build Environment
On Debian/Ubuntu systems you can use:

>`sudo apt-get install build-essential`

to get pretty-much everything you need to configure/compile/install C++ projects. On other flavors of Linux you need to basically make sure that you have gcc/g++ and "make" installed. If your software repositories don't provide you with gcc/g++ 4.8 or higher, you may need to look at manually installing a newer version. [You're own your own, there.](https://gcc.gnu.org/install/index.html) Sorry. Try typing: 

>`gcc --version`

at a command prompt to see if your version is sufficient. I've seen some later versions of 4.7.x that worked, but it's fringe at best.

##<a name="cmake"/>Getting CMake
Once again: `sudo apt-get install cmake` will get you what you need on Debian type systems. If your favorite software repositories can't supply CMake 3.0 or better, you'll need to download the source from [cmake.org](http://www.cmake.org) and build it it yourself. I've done it myself and their instructions are pretty good. You can either build it with an older version of CMake, or there's a boot-strap method if all you have is gcc/make.

##<a name="qt5"/>Getting Qt5
 <center>***NOTE: No help/support/bug-fixes will be forthcoming for those attempting to build Sigil with anything higher than Qt5.5.1 at this time***</center>

You can waste a lot of time trying to figure out if you have all the individual Qt5 packages installed that are necessary to build Sigil (which your software repos provide) ... or you can just download the binary installer from the [official Qt website](http://download.qt.io/archive/qt/). Sigil requires Qt5.4.0 - Qt5.5.1, but the "official" Sigil releases are built with Qt5.4.2. Note: the official binary releases of Qt5 are incompatible with Sigil starting with Qt5.6 (a result of QtWebkit being dropped from their installers). Look for the version that's appropriate for your architecture (qt-opensource-linux-***x86***-5.4.x.run or qt-opensource-linux-***x64***-5.4.x.run). Make sure its executable bit is set and launch it with administrative privileges to install it in its default location of /opt/Qt5.4.x (which is what I recommend). Or install it wherever you like--but just note that my command line examples later are going to assume the location of /opt/Qt5.4.x. Adjust accordingly if you choose different location.


##<a name="python"/>Getting Python 3.4
If your software repos provide Python 3.4.0 or higher, by all means use them to get the correct pieces installed. On Ubuntu/Debian I recommend (at a minimum) to `sudo apt-get install` the following packages (might need to be `python3.4-<module name>` on some systems):

+ python3
+ python3-dev
+ libpython3
+ libpython3-dev
+ python3-pip
+ python3-tk
+ python3-lxml
+ python3-six

If your repos don't include Python 3.4.x or higher, truck on over to [Python.org](http://www.python.org) and start reading how to build/install it from source. Whatever else you do, make sure you configure it with the `--enable-shared` option. You'll need the libpython3.4m.so library to build Sigil.

That's all the Python 3.4 stuff you will need to get Sigil "up and running", but if you want to make use of Sigil plugins that people are developing, you will also want to install the "standard" modules that ship with the binary version of Sigil on Windows and OS X. These can all be installed from python.org's [Python Package Index](https://pypi.python.org) using pip3 from the command-line. The entire current list (which I *highly* recommend installing) is:

+ six (already installed if you installed python3-six with apt-get)
+ lxml (already installed if you installed python3-lxml with apt-get)
+ html5lib
+ regex
+ Pillow
+ cssselect
+ cssutils
+ chardet

To install the 'html5lib' module, for example, you would use the command:

>`sudo pip3 install html5lib`

Note that to install the Pillow module, you'll need the libjpeg and zlib libraries and development headers to be available on your system. If you see error messages referring to libjpeg and or zlib when installing Pillow, you'll need to install them first (and then run `sudo pip3 install Pillow` again). Those missing dependencies can be installed from the command line with apt-get (or your equivalent package manager) using the commands:

>`sudo apt-get install libjpeg8-dev`

>`sudo apt-get install zlib1g-dev`



##<a name="sigil"/>Getting Sigil's Source Code

You can clone the Sigil Github repository:

>`git clone https://github.com/Sigil-Ebook/Sigil.git`

Or you can download a specific release tarball/zipfile from Sigil's [releases page](https://github.com/Sigil-Ebook/Sigil/releases) on Github.

I recommend the latter method, as the github repository version might not always be stable at any given moment (even though we try hard not to leave it broken). 

Unzip/untar the source code. Rename the uppermost directory ("Sigil-0.X.X" if you've download the Sigil-0.X.X-Code.zip file ) to something useful like "sigil-src". Unless you like typing mixed-case stuff in a terminal.

##<a name="build"/>Building Sigil

First off ... you don't build IN the Sigil source directory. You do all the building in a separate "build" directory. So at the same directory level as the Sigil source code directory, create a new directory called "sigil-build". The rest of the instructions will assume that both your Sigil source directory (I renamed it "sigil-src" in the previous step; adjust accordingly if you didn't) and your Sigil build directory ("sigil-build) are at the root of your user's home (~) directory. I'm also assuming that you installed Qt5 into /opt/Qt5.4.2 (adjust accordingly for different versions and/or different locations)

So first off, open a terminal and cd into your sigil-build directory

>`cd ~/sigil-build`

Then issue the following command to configure Sigil for building on a 64-bit linux machine:

> `cmake -G "Unix Makefiles" -DCMAKE_PREFIX_PATH=/opt/Qt5.4.2/5.4/gcc_64/lib/cmake -DCMAKE_BUILD_TYPE=Release ../sigil-src`

For a 32-bit machine it would be:

> `cmake -G "Unix Makefiles" -DCMAKE_PREFIX_PATH=/opt/Qt5.4.2/5.4/gcc/lib/cmake -DCMAKE_BUILD_TYPE=Release ../sigil-src`

If there are no errors, you're ready to build.

The default install prefix is /usr/local. If you wish to change the install location, you can do so by adding a `-DCMAKE_INSTALL_PREFIX` option to the above cmake configure command like so:

> `cmake -G "Unix Makefiles" -DCMAKE_PREFIX_PATH=/opt/Qt5.4.2/5.4/gcc_64/lib/cmake -DCMAKE_INSTALL_PREFIX=/a/different/install/prefix -DCMAKE_BUILD_TYPE=Release ../sigil-src`

You can also customize/override where the Sigil support files get installed (`<CMAKE_INSTALL_PREFIX>/share` by default) with the `-DSHARE_INSTALL_PREFIX` option (not recommended for beginners).

If cmake couldn't automatically find the necessary Python 3.4 stuff it needs (like if you installed manually in an unusual location, or you want to use a different Python version) you may need to tell cmake *specifically* where things can be found. Do so with:

>`-DPYTHON_LIBRARY=<the full path to the python3.4 shared library> (usually something similar to /usr/lib/libpython34.so)`

>`-DPYTHON_INCLUDE_DIR=<the full path to the directory where python3.4's header files can be found> (ex: /usr/include/python3.4)`

>`-DPYTHON_EXECUTABLE=<the full path to the python3.4 interpreter> (ex: /usr/lib/python3)`

Once the cmake configure command finishes with no errors, build Sigil with:

>`make (or make -j4 if you have plenty of processor cores)`

If you get an error that mentions qopengl.h, or you get a "`fatal error: GL/gl.h: No such file or directory
 #  include <GL/gl.h>`" error message, this usually implies that the OpenGL development headers for your system's video driver are missing or outdated. This can usually be remedied by installing the mesa-common-dev meta-package:
 
> `sudo apt-get install mesa-common-dev`

> Note: this has nothing to do with actually updating or changing your system's video driver, it simply installs some development headers that can be used when compiling other programs.

###Installing Sigil
If all goes well, install Sigil with:

>`sudo make install`


If installing to a non-default and unprivileged prefix, simply:

>`make install`

will suffice.


If you configured with the default install prefix, you can launch by entering "sigil" (no quotes) at a terminal. If you configured to install somewhere else, you may need to create a link to the sigil launch script (`<CMAKE_INSTALL_PREFIX>/bin/sigil`) in a directory that is on your path. There's also a .desktop file in `<SIGIL_SHARE_PREFIX>/share/applications' that you can create a link to on your desktop.

##<a name="testing"/>Testing Sigil's Python plugin framework

To test if Sigil's Python 3.4 plugin framework is fully functional, you can do the following:

1. download testplugin_v013.zip from [https://github.com/Sigil-Ebook/Sigil/raw/master/docs/testplugin_v013.zip](https://github.com/Sigil-Ebook/Sigil/raw/master/docs/testplugin_v012.zip)
2. open Sigil to the normal nearly blank template epub it generates when opened
3. use Plugins->Manage Plugins menu and make sure you have a Python 3.4 interpreter configured 
4. use the "Add Plugin" button to navigate to and add testplugin_vXXX.zip and then hit "Okay" to exit the Manage Plugins Dialog
5. use Plugins->Edit->testplugin to launch the plugin and hit the "Start" button to run it
6. check the plugin output window for your missing or broken plugin test results

Install any missing Python modules with your system's package management system or Python's pip3.

##<a name="advanced"/>Advanced Stuff

There are several configuration and environment variable options that can tailor how Sigil is built and/or run. I've talked about a few of the cmake options already, but I'll mention them here again along with the rest--with a brief explanation of their purposes.

-DCMAKE_INSTALL_PREFIX=`<path>` Configures the prefix where Sigil will be installed to (default is /usr/local)

-DSHARE_INSTALL_PREFIX=`<path>` Configures the prefix where Sigil's support files will be installed to (default is /usr/local meaning the support files will be installed in /usr/local/share/sigil)

-DUSE_SYSTEM_LIBS=(0|1) Tells cmake to try and use the system libraries when building Sigil instead of the ones bundled with Sigil in the 3rdParty directory. If a system version of a 3rd-party can't be found, Sigil falls back on the bundled version -- unless -DSYSTEM_LIBS_REQUIRED=1 is also specified (default is 0).

-DSYSTEM_LIBS_REQUIRED=(0|1) When used in conjunction with -DUSE_SYSTEM_LIBS=1, the Sigil build process will fail if all the necessary libraries can't be located on the system, instead of falling back on the bundled versions (default is 0).

-DINSTALL_BUNDLED_DICTS=(0|1) Can be used to enable/disable the installation of the bundled Hunspell dictionaries used for spellchecking. If this is disabled (-DINSTALL_BUNDLED_DICTS=0) then the user will need to specify the location of the hunspell dictionaries they want Sigil to use by setting an environment variable named SIGIL_DICTIONARIES.

The following three cmake options are used to manually specify which Python3 you want to use when building Sigil instead of relying on the included cmake utilities to try and automatically find a suitable version.

-DPYTHON_LIBRARY=`<the path to the python3.4 shared library>`

-DPYTHON_INCLUDE_DIR=`<the path to the directory where python3.4's header files can be found>`

-DPYTHON_EXECUTABLE=`<the path to the python3.4 interpreter>`

The following are environment variables that can be set at runtime to affect how Sigil is run after building/installing. They are commonly set by manually editing Sigil's launch script (`<CMAKE_INSTALL_PREFIX>`/bin/sigil).

SIGIL_PREFS_DIR - Changes where sigil looks for and updates its user preference data. Needs to specify a full path in a directory where the user has write privileges.

SIGIL_EXTRA_ROOT - Handy for relocating the Sigil support files. For instance you can move the `<CMAKE_SHARE_PREFIX>/share/sigil` directory anywhere you like. You just have to set SIGIL_EXTRA_ROOT to the path where you moved `<CMAKE_SHARE_PREFIX>/share/sigil` to.

SIGIL_DICTIONARIES - Used to tell Sigil what directories are to be searched for Hunspell dictionary files. Multiple directories can be specified by separating the paths with a colon. i.e. SIGIL_DICTIONARIES="/usr/share/hunspell" or SIGIL_DICTIONARIES="/usr/share/hunspell:/usr/share/hunspellextra"

The Sigil launch script also sets a SIGIL_SHARE_PREFIX environment variable, but it is automatically set to be the same as the cmake SHARE_INSTALL_PREFIX build-time option. It would be unwise to change this environment variable. Use the SIGIL_EXTRA_ROOT environment variable instead, if you need to alter the location of Sigil's support files after building Sigil.
