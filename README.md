About
=====

Sigil is a free, open source, multi-platform ebook editor.
It is designed to edit books in ePub format.


Links
=====

* Its website is located at http://sigil-ebook.com
* Its current code repository is located at https://github.com/Sigil-Ebook/Sigil
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


Linux Build and Install
=======================

The INSTALL.md file in the docs directory contains information on building
Sigil from source code (and installing it on Linux machines).


For Building on Mac OS X
========================

Building using purely XCode is no longer supported on Mac OS X.  The easiest 
way to build Sigil on Mac OS X is to use cmake 3.0 and the command line.   

Also because Sigil now embeds Python 3.4, see  

     docs/Building_A_Relocatable_Python_Framework_on_MacOSX.txt

for detailed instructions on how to build a fully relocatable Python 3.4 framework before
building Sigil.  

To build Sigil with your newly built relocateable Python 3.4 framework see:
   docs/Bundling_Python3_With_Sigil_on_MacOSX.txt


License
=======

Sigil is licensed under the GPLv3. The complete license is located in
COPYING.txt.

Note that libraries and components Sigil used and bundles may use a different
license (that is compatible with the GPLv3) from Sigil. See the specific
component for their respective license.
