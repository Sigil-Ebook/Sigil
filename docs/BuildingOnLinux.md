#<center>Building Sigil on Linux</center>

##General Overview

To build Sigil on Linux, you need to get/do the following things:

1. [A Linux build-toolchain](#gcc) with a C++11 capable compiler (gcc4.8.x-ish or higher recommended)
2. [CMake](#cmake) (3.0 or higher)
3. [Qt5.4.0](#qt5) (or higher)
4. [Python 3.4](#python) (or higher)
5. [The Sigil source code](#sigil) (downloaded tarball/zipfile or a git clone)
6. [Build Sigil](#build)
7. [Advanced Stuff](#advanced)

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
You can waste a lot of time trying to figure out if you have all the individual Qt5 packages installed that are necessary to build Sigil (which your software repos provide) ... or you can just download the binary installer from the [official Qt website](http://download.qt.io/archive/qt/). Sigil requires Qt5.4.0 or higher, but the "official" Sigil releases are built with Qt5.4.2. Look for the version that's appropriate for your architecture (qt-opensource-linux-***x86***-5.4.x.run or qt-opensource-linux-***x64***-5.4.x.run). Make sure its executable bit is set and launch it with administrative privileges to install it in its default location of /opt/Qt5.4.x (which is what I recommend). Or install it wherever you like--but just note that my command line examples later are going to assume the location of /opt/Qt5.4.x. Adjust accordingly if you choose different location.

##<a name="python"/>Getting Python 3.4
If your software repos provide Python 3.4.0 or higher, by all means use them to get the correct pieces installed. On Ubuntu/Debian I recommend (at a minimum) to `sudo apt-get install` the following packages:

+ python3.4
+ python3.4-dev
+ libpython3.4
+ libpython3.4-dev
+ python3.4-pip (might be python3-pip on some systems)
+ python3.4-tk

In addition you may find you need to `sudo apt-get install` the following development packages (if they're not already installed):

+ libxml2-dev
+ libxslt1-dev

If libxslt1-dev can't be found, try libxslt-dev.

Once you have those installed, use pip3 from a terminal to install the python modules that Sigil requires.

>`sudo pip3 install six`

>`sudo pip3 install lxml`

If pip3 barks about installing lxml, go back and make sure you installed the two development packages I mentioned.

That's all the Python 3.4 stuff you will need to get Sigil "up and running", but if you want to make use of Sigil plugins that people are developing, you will also want to install the "standard" modules that ship with the binary version of Sigil on Windows and OS X. The entire current list (which I *highly* recommend installing) is:

+ six
+ lxml
+ html5lib
+ regex
+ Pillow
+ cssselect
+ cssutils
+ chardet

If your repos don't include Python 3.4.x, truck on over to [Python.org](http://www.python.org) and start reading how to build/install it from source. Whatever else you do, make sure you configure it with the `--enable-shared' option. You'll need the libpython3.4m.so library to build Sigil.

##<a name="sigil"/>Getting Sigil's Source Code

You can clone the Sigil Github repository:

>`git clone https://github.com/Sigil-Ebook/Sigil.git`

Or you can download a specific release tarball/zipfile from Sigil's [releases page](https://github.com/Sigil-Ebook/Sigil/releases) on Github.

I recommend the latter method, as the github repository version might not always be stable at any given moment (even though we try hard not to leave it broken). 
ed
Unzip/untar the source code. Rename the uppermost directory ("Sigil-0.X.X" if you've download the Sigil-0.X.X-Code.zip file ) to something useful like "sigil-src". Unless you like typing mixed-case stuff in a terminal.

##<a name="build"/>Building Sigil

First off ... you don't build in the Sigil source directory. You do all the building in a "build" directory. So at the same directory level as the Sigil source code directory, create a new directory called "sigil-build". The rest of the instructions will assume that both your Sigil source directory (I renamed it "sigil-src" in the previous step. adjust accordingly if you didn't) and your Sigil build directory ("sigil-build) are at the root of your user's home (~) directory. I'm also assuming that you installed Qt5 into /opt/Qt5.4.2 (adjust accordingly for different versions and/or different locations)

So first off, open a terminal and cd into your sigil-build directory

>`cd ~/sigil-build`

Then issue the following command to configure Sigil for building on a 64-bit linux machine:

> `cmake -G "Unix Makefiles" -DCMAKE_PREFIX_PATH=/opt/Qt5.4.2/5.4/gcc_64/lib/cmake -DCMAKE_BUILD_TYPE=Release ../sigil-src`

For a 32-bit machine it would be:

> `cmake -G "Unix Makefiles" -DCMAKE_PREFIX_PATH=/opt/Qt5.4.2/5.4/gcc/lib/cmake -DCMAKE_BUILD_TYPE=Release ../sigil-src`

If there are no errors, you're ready to build.

The default install prefix is /usr/local. If you wish to change the install location, you can do so by adding a `-DCMAKE_INSTALL_PREFIX` option to the above cmake configure command like so:

> `cmake -G "Unix Makefiles" -DCMAKE_PREFIX_PATH=/opt/Qt5.4.2/5.4/gcc_64/lib/cmake -DCMAKE_INSTALL_PREFIX=/a/different/install/prefix -DCMAKE_BUILD_TYPE=Release ../sigil-src`

You can also customize/override where the Sigil support files get installed (`<CMAKE_INSTALL_PREFIX>/share` by default) with the `-DSHARE_INSTALL_PREFIX` option.

If cmake couldn't automatically find the necessary Python 3.4 stuff it needs (like if you installed manually in an unusual location) you may need to tell cmake *specifically* where things can be found. Do so with:

>`-DPYTHON_LIBRARY=<the path to the python3.4 shared library>`

>`-DPYTHON_INCLUDE_DIR=<the path to the directory where python3.4's header files can be found>`

>`-DPYTHON_EXECUTABLE=<the path to the python3.4 interpreter>`

Once the cmake configure command finishes with no errors, build Sigil with:

>`make`

If all goes well, install it with:

>`sudo make install`

If installing to a non-default and unprivileged prefix, simply:

>`make install`

will suffice.


If you configured with the default install prefix, you can launch by entering "sigil" (no quotes) at a terminal. If you configured to install somewhere else, you may need to create a link to the sigil launch script (`<CMAKE_INSTALL_PREFIX>/bin/sigil`) in a directory that is on your path. There's also a .desktop file in `<SIGIL_SHARE_PREFIX>/share/applications' that you can create a link to on your desktop.

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

SIGIL_EXTRA_ROOT - Handy for relocating the Sigil support files. For instance you can move the `<CMAKE_SHARE_PREFIX>/share/sigil` directory anywhere you like. You just have to set SIGIL_EXTRA_ROOT to the path where you moved `<CMAKE_SHARE_PREFIX>/share/sigil` to.

SIGIL_DICTIONARIES - Used to tell Sigil what directories are to be searched for Hunspell dictionary files. Multiple directories can be specified by separating the paths with a colon. i.e. SIGIL_DICTIONARIES="/usr/share/hunspell" or SIGIL_DICTIONARIES="/usr/share/hunspell:/usr/share/hunspellextra"

The Sigil launch script also sets a SIGIL_SHARE_PREFIX environment variable, but it is automatically set to be the same as the cmake SHARE_INSTALL_PREFIX build-time option. It would be unwise to change this environment variable. Use the SIGIL_EXTRA_ROOT environment variable instead, if you need to alter the location of Sigil's support files after building Sigil.
