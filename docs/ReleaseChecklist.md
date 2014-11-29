Introduction
============

All items on this list must be completed for a successful release.


Details
=======

* Ensure Sigil builds and runs.
* Write release announcement.
* Update translations.
* Ensure Qt is at the latest version on all OSs packages will be built for.
* Bump version in CMakeLists.txt.
* Bump version in version.xml.
* Set release date in Changelog.txt.
* Commit version changes but do not push them.
* Tag version.
* Build source package $ git archive --prefix Sigil-x.y.z/ -o ../Sigil-x.y.z-Code.zip HEAD
* Build packages.
  * OS X
  * Windows
  * Sign packages
    * OS X
* Generate Checksums file.
* Make Release on GitHub
  * Upload packages
    * Code
    * OS X
    * Windows
    * SHA256 sums
* Post release announcement on Blog(s).
* Post release announcement on MobileRead.
* Push git changes.
