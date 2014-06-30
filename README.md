Sigil is a free, open source, multi-platform ebook editor.
It is designed to edit books in ePub format.

* It's website is located here: http://code.google.com/p/sigil/
* It's current code repository is located here: https://github.com/user-none/Sigil
* Translations are located here: https://www.transifex.com/projects/p/sigil/
* Support forums are located here: http://www.mobileread.com/

Issue tracking is disabled because it has become unhelpful. If you want a
feature implemented or a bug fixed, implement it yourself and submit a pull
request on GitHub.

The src directory contains all of the source code. It has
a few subdirectories:
    src/BoostParts - source code of various parts of the Boost C++ 
                     Libraries that Sigil uses.
    src/FlightCrew - source code for the FlightCrew EPUB validation
                     Library.
    src/Sigil - source code for the Sigil application.
    src/tidyLib - source code for HTML Tidy, an HTML cleaner. This is
                  a heavily modified version of tidy because tidy
                  upstream is dead so patches from us don't have a
                  chance of being integarted.
    src/Xerces - source code for Xerces-C++, XML manipulation Library.
    src/XercesExtensions - source code for helper classes that shuffle
                     data between into and out of formats supported and
                     required by Xerces. This is part of Sigil and not
                     a third party library.
    src/minizip - source code for minizip, a zipping library.
    src/zlib - source code for the zlib compression library.
    src/hunspell - spell checking library.
    src/pcre - regular epression library.
    src/utf8-cpp - UTF8 manipulation and validation library.

The INSTALL.txt file contains information on building Sigil from
source code (and installing it on Linux machines).

The GPLv3 license under which Sigil is released is located in
the file called COPYING.txt.

