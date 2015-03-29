3rdparty
========

Bundled copies of 3rd party libraries.

To prevent using bundled copies in favor of system libraries use the build flag:

USE_SYSTEM_LIBS

To require the use of System libraries and not allow falling back to bundled
copies if a system library is not found use the build flag:

SYSTEM_LIBS_REQUIRED

Both of the above libraries should be constructed like so:

-D...=1
