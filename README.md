About
=====

Sigil is a free, open source, multi-platform ebook editor.
It is designed to edit books in ePub format.


Links
=====

* Its website is located at http://code.google.com/p/sigil/
* Its current code repository is located at https://github.com/user-none/Sigil
* Translations are located at https://www.transifex.com/projects/p/sigil/
* Support forums are located at http://www.mobileread.com/


Issue Tracker
=============

Issue tracking is intended for discussion around issues with the code. It
is not intended for developers to discuss ideas before taking the time to
implement them. It is also intended for actual bug tracking. It is not
intended for feature requests.

At one point in time issue tracking was disabled because it had become
unhelpful. The majority of issues where requests or abusive comments related
to Sigil not running on older OS's whose own manufacturer had declared
end of life.

Feature requests opened on the issue tracker will be closed if there isn't
anyone willing to implement the requested feature. Only items being worked
will be left open.


Source Components
=================

The src directory contains all of the source code. It has
a few subdirectories:

BoostParts
----------

Various parts of the Boost C++ Libraries that Sigil uses.  This is not a
complete copy of Boost, BCP was used to only include the parts of Boost that
are used.

FlightCrew
----------

EPUB validation Library. FlightCrew at its Google Code project page is largely
abandoned. All work on FlightCrew is happening within Sigil and changes are not
being backported to the standalone project.


Sigil
-----

Source code for Sigil.


tidyLib
-------

HTML Tidy, an HTML cleaner. This is a heavily modified
version of tidy because tidy upstream is dead so patches from us
don't have a chance of being integrated.


Xerces
------

Xerces-C++, XML manipulation Library.

XercesExtensions
----------------

Helper classes that shuffle data between into and out of formats supported and
required by Xerces. This is part of Sigil and not a third party library.

minizip
-------

A zipping library.

zlib
----

Compression library. Epub uses the zip container with deflate compression.
This is used by minizip for the compression part of the zip format.

hunspell
--------

Spell checking library.

pcre
----

Regular expression library.

utf8-cpp
--------

UTF8 manipulation and validation library.


Build and Install
=================

The INSTALL.md file in the docs directory contains information on building
Sigil from source code (and installing it on Linux machines).


License
=======

Sigil is licensed under the GPLv3. The complete license is located in
COPYING.txt.

Note that libraries and components Sigil used and bundles may use a different
license (that is compatible with the GPLv3) from Sigil. See the specific
component for their respective license.
