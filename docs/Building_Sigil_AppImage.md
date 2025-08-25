# <center>Building the Sigil AppImage</center>

## General Overview

**Warning! Do not attempt to run any of these scripts outside of a Docker container. Doing so could damage your system. All scripts provided are intended to be used inside of a temporary docker container that will be removed after the process completes.**

The only requirement for building the Sigil Linux AppImage is having Docker installed/configured and having an internet connection. How to get Docker installed and configured on your system is beyond the scope of this document. Every distro is different, so you're on your own there. Everything after this assumes that Docker is ready to go. Typing `docker info` in a terminal should verify everything is ready.

There are two separate pieces that are needed for building the AppImage. These are a custom build of Python and a custom build of QtWebEngine. The AppImage build process will automatically download these pieces from my [personal github repository](https://github.com/dougmassay/win-qtwebkit-5.212/releases/tag/v5.212-1). The files used from there are sigilpython3.\*.\*.tar.gz and appimagewebengine6\*.tar.gz. These two pieces can also be fairly easily built if you wish. Should you desire to build everything from scratch, the instructions to do so will be at the end of this document.

The building of the Sigil AppImage takes place entirely within a docker container built from a stock Ubuntu 22.04 image. No system changes or updates will be made to your host system during this process. Once the Docker process completes, the Sigil AppImage (and its corresonding zsync file) will be found in in the same directory the build process was launched from.

Everything is done by entering commands in a terminal.

### Get Sigil's Source

You can either download Sigil's source code from [GitHub](https://github.com/Sigil-Ebook/Sigil), or you can clone Sigil's repository using git `git clone https://github.com/Sigil-Ebook/Sigil.git sigil`. Your choice. These built pieces will be ignored by git, so don't worry about contaminating the source tree.

### CD to the root of Sigil's source

`cd sigil` or `cd sigil-master` or whatever. You need to be in the same directory that Sigil's README.md, CHANGELOG.txt, and docker-compose.yml files are in.

### Build the AppImage

If you have docker-compose installed, you can build the AppImage with the following simple command:

`docker compose run --rm build_appimage`

If you don't have docker-compose installed, use the following command instead:

`docker run --rm -v $PWD:/reporoot ubuntu:22.04 /reporoot/.github/workflows/build_sigil_appimage.sh`

Once completed, the Sigil-\*-x86_64.AppImage file (as well as the Sigil-\*-x86_64.AppImage.zsync file - which can be safely ignored/removed) will be located in the current directory. Depending on how your docker permissions are configured you may need to take ownership of the file(s) with `sudo chown <user>:<user> Sigil-*AppImage*`. The file should already be executable, but if not just fix it up with `chmod a+x Sigil-*AppImage`.

## Optionally Building the Support Pieces First

If you want to build the custom Python and the custom QtWebEngine used to make the AppImage, you can do so by using the following instructions. The second step relies on the first, so do them in order!

### Building the custom Python piece

Just like building the AppImage, you need to be in the root of Sigil's source tree. Use `cd sigil` or `cd sigil-master` or whatever. You need to be in the same directory that Sigil's README.md, CHANGELOG.txt, and docker-compose.yml files are in.

After that issue the following docker command:

`docker run --rm -v $PWD:/reporoot ubuntu:22.04 /reporoot/.github/workflows/build_sigilpython.sh`

You should end up with a sigilpython3.\*.\*.tar.gz file in the same directory. You can fix up the permissions if you like with chown, but it's not necessary to do so for the build process. Leave the file where it is, so the next step can access it.

### Building the custom QtWebEngine piece

Just like all of the other steps, you need to be in the root of Sigil's source tree. Use `cd sigil` or `cd sigil-master` or whatever. You need to be in the same directory that Sigil's README.md, CHANGELOG.txt, and docker-compose.yml files are in.

Issue the following docker command:

`docker run --rm -v $PWD:/reporoot ubuntu:22.04 /reporoot/.github/workflows/build_sigilwebengine.sh`

This build can take an hour or so on the beefiest of machines, so be prepared to wait. You should end up with an appimagewebengine6\*.tar.gz file. You can fix up the permissions if you like with chown, but again, it's not necessary to do so for the AppImage build process.

With these two additional pieces in place, go back to the "Build the AppImage" step and repeat the process. The build process will automatically use the manually built pieces instead of downloading them from my Github repository (so long as you haven't moved or renamed them).
