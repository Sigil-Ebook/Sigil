
Sigil is a free, open source, multi-platform WYSIWYG ebook editor.
It is designed to edit books in ePub format. The version of the package
can be found in the ChangeLog.txt file.

It's website is located here: http://code.google.com/p/sigil/

The src directory contains all of the source code. It has
a few subdirectories:
    src/BoostParts - source code of various parts of the Boost C++ 
                     Libraries that Sigil uses.
    src/FlightCrew - source code for the FlightCrew EPUB validation
                     Library.
    src/Sigil - source code for the Sigil application.
    src/tidyLib - source code for HTML Tidy, an HTML cleaner.
    src/Xerces - source code for Xerces-C++, XML manipulation Library.
    src/XercesExtensions - source code for helper classes that shuffle
                     data between into and out of formats supported and
                     required by Xerces.
    src/minizip - source code for minizip, a zipping library.
    src/zlib - source code for the zlib compression library.

The installer directory contains the files needed to produce
a binary installer for Windows and Linux.

The manual directory contains the manual in reStructuredText format,
designed to be preprocessed with Sphinx.

The INSTALL.txt file contains information on building Sigil from
source code (and installing it on Linux machines).

The GPLv3 license under which Sigil is released is located in
the file called COPYING.txt.
