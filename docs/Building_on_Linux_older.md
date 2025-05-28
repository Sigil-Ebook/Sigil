# <center>Building Sigil on Linux</center>
## <center>Systems like Ubuntu 22.04 (and its derivatives) or older</center>

Starting with Sigil 2.0.2, Sigil defaults to building with Qt6. If your distro provides recent enough versions of Qt6 modules, It is recommended to [build Sigil that way](./Building_on_Linux.md). If not, the latest versions of Sigil will still be able to be built with Qt5 for a little while (but not indefinitely).

Starting with version 2.3.0, Sigil only supports being built with Qt6. This means that **Sigil 2.2.1 marks the very last version of Sigil that can be built with Qt5**.


## General Overview

The requirements for building Sigil on newer Linux systems like Ubuntu 20.04, Mint 20, Arch Linux, etc., should be able to be installed entirely from your system's software repositories.

To build Sigil on newer Linux systems, you need to get/do the following things:

1. [A Linux build-toolchain](#gcc) with a C++11 capable compiler (gcc 4.9.x or higher recommended)
2. [CMake](#cmake) (3.0 or higher)
3. [Qt5.10 or higher](#qt5) (with QtWebEngine) Qt5.12+ recommended
4. [3rd-party dependencies](#thirdparty) (an optional step)
5. [Python 3.5](#python) (or higher) 3.6+ recommended
6. [The Sigil source code](#sigil) (downloaded tarball/zipfile or a git clone)
7. [Build/Install Sigil](#build)
8. [Test Sigil's Plugin Framework](#testing)
9. [Advanced Stuff](#advanced)

Since I'm basically an Ubuntu/Debian guy at heart, I'll be mentioning stuff like:

>`sudo apt-get install`

from here on out. You'll have to forgive me for not knowing all the yum/pacman/emerge equivalents. It's not a slight--I can assure you.

## <a name="gcc"/>Linux Build Environment
On Ubuntu-type systems you can use:

>`sudo apt-get install build-essential git`

to get pretty-much everything you need to configure/compile/install C++ projects. On other flavors of Linux you need to basically make sure that you have gcc/g++ and "make" installed.

## <a name="cmake"/>Getting CMake
Once again: `sudo apt-get install cmake` will get you what you need on Ubuntu-type systems.

## <a name="qt5"/>Getting Qt5
<center>**If your repos don't provide at least Qt5.10, use the [Building_on_older_Linux](./Building_on_older_Linux.md) documentation**</center>
To get Sigil's Qt5 requirements, `sudo apt-get install` the following packages:

+ qtbase5-dev
+ qttools5-dev
+ qttools5-dev-tools
+ qtwebengine5-dev
+ libqt5svg5-dev (build requirement starting with Sigil v2.1.0)


The following command can be copied and pasted for convenience:

`sudo apt-get install qtbase5-dev qttools5-dev qttools5-dev-tools qtwebengine5-dev libqt5svg5-dev`

## <a name="thirdparty"/>3rd-Party Dependencies (optional step)
Sigil will provide the extra third-party libs if you do nothing, but most (if not all) of Sigil's third-party dependencies should be available in your software repos. If you want to make use of them, `sudo apt-get install` the following packages.

+ libhunspell-dev
+ libpcre2-dev
+ libminizip-dev

The following command can be copied and pasted for convenience:

`sudo apt-get install libhunspell-dev libpcre2-dev libminizip-dev`

If you do install them, remember to use use the -DUSE_SYSTEM_LIBS=1 option when configuring Sigil with cmake later on. Otherwise, the build process will ignore them and provide/build its own.

## <a name="python"/>Getting Python 3.5 (or higher -- 3.6+ recommended) 
On Ubuntu/Debian `sudo apt-get install` (at a minimum) the following packages:

+ python3-dev
+ python3-pip
+ python3-lxml
+ python3-six
+ python3-css-parser (may have to use `pip3 install css-parser` if your distro has no package for this
+ python3-dulwich (unless your distro has very recent version (0.19.x) in its repos, you'll probably need to use `pip3 install dulwich` to install a new enough version that will work with Sigil. dulwich requires that the urllib3 and certifi modules be installed as well)

The following command can be copied and pasted for convenience:

`sudo apt-get install python3-dev python3-pip python3-lxml python3-six python3-css-parser python3-dulwich`

That's all the Python 3.5 (or higher) stuff you will need to get Sigil "up and running", but if you want to make use of Sigil plugins that people are developing, you will also want to install the "standard" modules that ship with the binary version of Sigil on Windows and OS X. These should all be able to be installed with `sudo apt-get install`.

+ python3-tk
+ python3-pyqt5
+ python3-pyqt5.qtwebengine
+ python3-html5lib
+ python3-regex
+ python3-pil.imagetk
+ python3-cssselect
+ python3-chardet

The following command can be copied and pasted for convenience:

`sudo apt-get install python3-tk python3-pyqt5 python3-pyqt5.qtwebengine python3-html5lib python3-regex python3-pil.imagetk python3-cssselect python3-chardet`

If you run into any that won't install with `sudo apt-get install` you can still use pip3 to install them.

## <a name="sigil"/>Getting Sigil's Source Code

You can clone the Sigil Github repository:

>`git clone https://github.com/Sigil-Ebook/Sigil.git`

Or you can download a specific release tarball/zipfile from Sigil's [releases page](https://github.com/Sigil-Ebook/Sigil/releases/latest) on Github.

I recommend the latter method, as the github repository version might not always be stable at any given moment (even though we try hard not to leave it broken).

Unzip/untar the source code. Rename the uppermost directory ("Sigil-0.X.X" if you've download the Sigil-0.X.X-Code.zip file ) to something useful like "sigil-src". Unless you like typing mixed-case stuff in a terminal.

## <a name="build"/>Building Sigil

First off ... you don't build IN the Sigil source directory. You do all the building in a separate "build" directory. So at the same directory level as the Sigil source code directory, create a new directory called "sigil-build". The rest of the instructions will assume that both your Sigil source directory (I renamed it "sigil-src" in the previous step; adjust accordingly if you didn't) and your Sigil build directory ("sigil-build) are at the root of your user's home (~) directory.

So first off, open a terminal and cd into your sigil-build directory

>`cd ~/sigil-build`

Then issue the following command to configure Sigil for building (for Sigil 2.0.2 and higher):

> `cmake -G "Unix Makefiles" -DUSE_QT5=1 -DCMAKE_BUILD_TYPE=Release ../sigil-src`

For earlier than Sigil 2.0.2 you can leave out -DUSE_QT5=1 (but it will hurt nothing if left in).

If there are no errors, you're ready to build.

The default install prefix is /usr/local. If you wish to change the install location, you can do so by adding a `-DCMAKE_INSTALL_PREFIX` option to the above cmake configure command like so:

> `cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=/a/different/install/prefix -DCMAKE_BUILD_TYPE=Release ../sigil-src`

You can also customize/override where the Sigil support files get installed (`<CMAKE_INSTALL_PREFIX>/share` by default) with the `-DSHARE_INSTALL_PREFIX` option (not recommended for beginners).

If cmake couldn't automatically find the necessary Python 3.4 (or higher) stuff it needs (like if you installed manually in an unusual location, or you want to use a different Python version) you may need to tell cmake *specifically* where things can be found. Do so with:

>`-DPYTHON_LIBRARY=<the full path to the python3.4 (or higher) shared library> (usually something similar to /usr/lib/libpython34.so)`

>`-DPYTHON_INCLUDE_DIR=<the full path to the directory where python3.4's (or higher) header files can be found> (ex: /usr/include/python3.4)`

>`-DPYTHON_EXECUTABLE=<the full path to the python3.4 (or higher) interpreter> (ex: /usr/lib/python3)`

Once the cmake configure command finishes with no errors, build Sigil with:

>`make (or make -j4 if you have plenty of processor cores)`

### Common compilation failures/Errors.

To be determined.

### Installing Sigil
If all goes well, install Sigil with:

>`sudo make install`

If you configured with the default install prefix, you can launch by entering "sigil" (no quotes) at a terminal. If you configured to install somewhere else, you may need to create a link to the sigil launch script (`<CMAKE_INSTALL_PREFIX>/bin/sigil`) in a directory that is on your path. There's also a .desktop file in `<SIGIL_SHARE_PREFIX>/share/applications` that you can create a link to on your desktop. Sigil should also appear in your Desktop Environment's menu system (under Office and/or Accessories). You may need to logout and back in for the menu entries to be visible after installing (you can also force your menus to update if you have the xdg-utils package installed by issuing the "xdg-desktop-menu forceupdate" command from a terminal)

## <a name="testing"/>Testing Sigil's Python plugin framework

To test if Sigil's Python 3.5+ plugin framework is fully functional, you can do the following:

1. download testplugin_v019.zip from [https://github.com/Sigil-Ebook/Sigil/raw/master/docs/testplugin_v019.zip](https://github.com/Sigil-Ebook/Sigil/raw/master/docs/testplugin_v019.zip)
2. open Sigil to the normal nearly blank template epub it generates when opened
3. use Plugins->Manage Plugins menu and make sure you have a Python 3.4+ interpreter configured 
4. use the "Add Plugin" button to navigate to and add testplugin_vXXX.zip and then hit "Okay" to exit the Manage Plugins Dialog
5. use Plugins->Edit->testplugin to launch the plugin and hit the "Start" button to run it
6. check the plugin output window for your missing or broken plugin test results

Install any missing Python modules with your system's package management system or Python's pip3.

## <a name="advanced"/>Advanced Stuff

There are several configuration and environment variable options that can tailor how Sigil is built and/or run. I've talked about a few of the cmake options already, but I'll mention them here again along with the rest--with a brief explanation of their purposes.

### CMake options

-DQt5_DIR=`<path>` Configures cmake to use a Qt5 installation other than the normal system version of Qt5 (ex. /opt/Qt5.12.3/5.12/gcc_64/lib/cmake/Qt5 - the path should alays end in /lib/cmake/Qt5)

-DCMAKE_INSTALL_PREFIX=`<path>` Configures the prefix where Sigil will be installed to (default is /usr/local)

-DSHARE_INSTALL_PREFIX=`<path>` Configures the prefix where Sigil's support files will be installed to (default is /usr/local meaning the support files will be installed in /usr/local/share/sigil)

-DCMAKE_INSTALL_LIBDIR=(lib|lib64) Use to override GnuInstallDirs if it doesn't choose the correct lib directory for your distro.

-DUSE_SYSTEM_LIBS=(0|1) Tells cmake to try and use the system libraries when building Sigil instead of the ones bundled with Sigil in the 3rdParty directory. If a system version of a 3rd-party can't be found, Sigil falls back on the bundled version -- unless -DSYSTEM_LIBS_REQUIRED=1 is also specified (default is 0).

-DSYSTEM_LIBS_REQUIRED=(0|1) When used in conjunction with -DUSE_SYSTEM_LIBS=1, the Sigil build process will fail if all the necessary libraries can't be located on the system, instead of falling back on the bundled versions (default is 0).

-DDISABLE_UPDATE_CHECK=(0|1) Defaults to 0. Use -DDISABLE_UPDATE_CHECK=1 to disable the builtin online update check. Mainly for use by *nix distros whose Sigil packages can't make use of the new release downloads anyway.

-DINSTALL_BUNDLED_DICTS=(0|1) Default is 1. Can be used to enable/disable the installation of the bundled Hunspell dictionaries used for spellchecking. If this is disabled (-DINSTALL_BUNDLED_DICTS=0), then the standard system spell-check dictionary location of /usr/share/hunspell will be searched for eligible dictionaries. If additional system paths need to be searched for dictionaries, they can be added using the -DEXTRA_DICT_DIRS option. Setting this to 0 will require that you manually install the language-specific hunspell dictionaries (from your software repos) yourself (e.g. `sudo apt-get install hunspell-en-us`).

-DEXTRA_DICT_DIRS=`<path1>`:`<path2>` Path(s) that should be searched for eligible spellcheck dictionaries (in addition to /usr/share/hunspell). Multiple paths should be separated by colons. This option is only relevant if -DINSTALL_BUNDLED_DICTS=0 is also specified.

-DMATHJAX_DIR=`<path>` **Only for Sigil 1.9.10 and earlier!** If you would like use your system's MathJax implementation instead of the one that comes bundled with Sigil, use this cmake directive when first configuring. A minimum of MathJax v2.7.0 is required to work with Sigil. NOTE: if -DMATHJAX_DIR=`<path>` is used, Sigil will install a config script to `<path>`/config/local. This file is required for Sigil's Preview to be able properly render MathML. This feature was added between Sigil 0.9.12 and 0.9.13.

-DMATHJAX3_DIR=`<path>` **Only for Sigil 1.9.20 and Later!** If you would like use your system's MathJax implementation instead of the one that comes bundled with Sigil, use this cmake directive when first configuring. A minimum of MathJax v3.2.2 is required to work with Sigil. MathJax 3.2.2+ is required for Sigil's Preview to be able properly render MathML starting with Sigil v1.9.20.

-DINSTALL_HICOLOR_ICONS=(0|1) Install various-sized Sigil application icons to the typical hicolor theme directories (`<INSTALL_PREFIX>/share/icons/hicolor`). The default is 0, which installs a single icon to the `<INSTALL_PREFIX>/share/pixmap` folder.

-DUSE_ALT_ICONS=(0|1) Defaults to 0. Install/use alternative teal-colored Sigil application icon(s). You still need to specify -DINSTALL_HICOLOR_ICONS=1 if you want all the alternative teal-colored icons installed the hicolor theme.

The following three cmake options are used to manually specify which Python3 you want to use when building Sigil instead of relying on the included cmake utilities to try and automatically find a suitable version.

-DPYTHON_LIBRARY=`<the path to the python3.x shared library>`

-DPYTHON_INCLUDE_DIR=`<the path to the directory where python3.x's header files can be found>`

-DPYTHON_EXECUTABLE=`<the path to the python3.x interpreter>`

-DTRY_NEWER_FINDPYTHON3=(0|1) Defaults to 0. If you use cmake 3.18 or higher you may want to take advantage of its newer FindPython3 module.  If so, the above directives used to define specific versions of Python3 are as follows:

-DPython3_LIBRARIES=`<the path to the python3.x shared library>`

-DPython3_INCLUDE_DIRS=`<the path to the directory where python3.x's header files can be found>`

-DPython3_EXECUTABLE=`<the path to the python3.x interpreter>`

-DBUILD_PATCHED_LIBXML2=(0|1) Some newer versions of libxml2 have a bug that causes QtWebKit to render html entities twice. Adding -DBUILD_PATCHED_LIBXML2=1 to the cmake command will clone the libxml2 git repo, checkout a specific commit, patch the source, build it and install it alongside Sigil (does not affect the system version of libxml2). Requires git, libtool, autoconf and automake packages to be installed (as well as a working internet connection). Cmake should notify of any missing programs needed. The default is to NOT build the patched version of libxml2 (-DBUILD_PATCHED_LIBXML2=0).

### Environment Variables

The following are environment variables that can be set at runtime to affect how Sigil is run after building/installing. They are commonly set by manually editing Sigil's launch script (`<CMAKE_INSTALL_PREFIX>`/bin/sigil).

SIGIL_PREFS_DIR - Changes where sigil looks for and updates its user preference data. Needs to specify a full path in a directory where the user has write privileges.

SIGIL_EXTRA_ROOT - Handy for relocating the Sigil support files. For instance you can move the `<CMAKE_SHARE_PREFIX>/share/sigil` directory anywhere you like. You just have to set SIGIL_EXTRA_ROOT to the path where you moved `<CMAKE_SHARE_PREFIX>/share/sigil` to.

SIGIL_DICTIONARIES - Used to tell Sigil what directories are to be searched for Hunspell dictionary files. Multiple directories can be specified by separating the paths with a colon. i.e. SIGIL_DICTIONARIES="/usr/share/hunspell" or SIGIL_DICTIONARIES="/usr/share/hunspell:/usr/share/hunspellextra" Setting this variable at run time will override all compile-time dictionary search paths (except for any user-supplied dictionaries manually added to their preference directory's hunspell_dictionary location).

FORCE_SIGIL_DARKMODE_PALETTE - If this variable is set at runtime, it tells Sigil to ignore any defined platform themes/styles (QT_QPA_PLATFORMTHEME, or QT_STYLE_OVERRIDE) and to use the dark color palette provided by Sigil (starting with Sigil v1.1).

SIGIL_ICON_SCALE_FACTOR - Valid values: 1.0 to 3.0. The default value (with no variable set) is 1.8. Sigil scales its menu icons based on font-size. This can sometimes result in icons being a bit too large (or too small) depending on the system Qt theme. Use this variable to tweak the icon size if deemed necessary. **(Only works with Sigil v0.9.7 and earlier; v0.9.8 has a preference setting to adjust icons)**

SKIP_SIGIL_UPDATE_CHECK - Defining this variable (to any value) will cause Sigil to skip its online check for a newer version.

The Sigil launch script also sets a SIGIL_SHARE_PREFIX environment variable, but it is automatically set to be the same as the cmake SHARE_INSTALL_PREFIX build-time option. It would be unwise to change this environment variable. Use the SIGIL_EXTRA_ROOT environment variable instead, if you need to alter the location of Sigil's support files after building Sigil.
