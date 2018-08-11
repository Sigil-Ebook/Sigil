About
=====

Sigil is a free, open source, multi-platform ebook editor.
It is designed to edit books in ePub format (both ePub 2 and ePub 3).


Links
=====

* Its website is located at http://sigil-ebook.com

* Its current code repository is located at https://github.com/Sigil-Ebook/Sigil

* Translations are located at https://www.transifex.com/projects/p/sigil/

* Support forums are located at http://www.mobileread.com/forums
    Select Sigil from the list of main forums

* Sigil Plugin Index (hosted by www.mobileread.com) at 
    http://www.mobileread.com/forums/showthread.php?t=247431


Issue Tracker
=============

Please do not use the issue tracker to ask questions.  Both of the main developers
of Sigil monitor the Sigil Forum at https://www.mobileread.com/forums.
All questions should be directed there.

Issue tracking is intended for discussion around issues with the code. 
It is also intended for actual bug tracking and for tracking feature requests.

Feature requests opened on the issue tracker will be closed if there isn't
anyone willing to implement the requested feature. Only items being worked
will be left open.


Linux Build and Install
=======================

For newer Linux systems like Ubuntu 16.04 (and its derivitives: like Mint 18), or Arch Linux, or Debian Unstable, you should be able to compile Sigil using repo-provided dependencies. Instructions for doing so can be found in:

> [docs/Building_on_Linux.md](./docs/Building_on_Linux.md) ([or .html](./docs/Building_on_Linux.html))

For older Linux systems whose software repositories do not provide Qt5.4.0 (or higher), the
detailed instructions for building/installing Sigil can be found in:

> [docs/Building_on_older_Linux.md](./docs/Building_on_older_Linux.md) ([or .html](./docs/Building_on_older_Linux.html))

For Building on Mac OS X
========================

Building using purely XCode is no longer supported on Mac OS X.  The easiest 
way to build Sigil on Mac OS X is to use cmake 3.0 and the command line.   

Also because Sigil now embeds Python 3.5.X, see  

> [docs/Building_A_Relocatable_Python_Framework_on_MacOSX.txt](./docs/Building_A_Relocatable_Python_Framework_on_MacOSX.txt)

for detailed instructions on how to build a fully relocatable Python 3.5.X framework before
building Sigil.  

Sigil requires at least Qt 5.4.2, or Qt 5.5.1.  If you use a newer Qt version you must
add back QtWebkit to the Qt sources and build all of Qt yourself from source, see

> [docs/Building_Qt5_From_Source_with_QtWebkit_Added_Back_on_MacOSX.txt](./docs/Building_Qt5_From_Source_with_QtWebkit_Added_Back_on_MacOSX.txt)


And finally to build Sigil itself see:

> [docs/Building_Sigil_On_MacOSX.txt](./docs/Building_Sigil_On_MacOSX.txt)


For Building on Windows
========================

To build Sigil on Windows, see:

> [docs/Building_Sigil_on_Windows.md](./docs/Building_Sigil_on_Windows.md) ([or .html](./docs/Building_Sigil_on_Windows.html))



License
=======

Sigil is licensed under the GPLv3. The complete license is located in
COPYING.txt.

Note that libraries and components Sigil used and bundles may use a different
license (that is compatible with the GPLv3) from Sigil. See the specific
component for their respective license.  The source code from these
projects can be found under Sigil/3rdparty unless otherwise indicated.  
Please see their respective folders for complete license information.

Currently these projects include:

* Hunspell - http://hunspell.sourceforge.net
* MiniZip version 1.1
* Perl-compatible Regular Expression Library (pcre)
* ZLib Data Compression Library (zlib 1.2.8)
* jQuery-2.2.4 (src/Resource_Files/javascript/jquery-2.2.4.min.js)
* jQuery.ScrollTo-2.1.2 (src/Resource_Files/javascript/jquery.scrollTo-2.1.2.min.js)
* MathJax.js single file version: (src/Resource_Files/polyfills)

In addtion, Sigil uses the following other packages that have been specifically
modified for use inside Sigil:

* Beautiful Soup 4 (src/Resource_Files/plugin_launchers/sigil_bs4)
* Google's Gumbo Parser (internal/gumbo)

