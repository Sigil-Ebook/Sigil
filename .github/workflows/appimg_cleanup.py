#!/usr/bin/env python3
# vim:fileencoding=UTF-8:ts=4:sw=4:sta:et:sts=4:ai

import os, sys, glob

exclude_libs = [
    "libatk-1.0.so.0",
    "libatk-bridge-2.0.so.0",
    "libatspi.so.0",
    "libblkid.so.1",
    "libbsd.so.0",
    "libcairo-gobject.so.2",
    "libcairo.so.2",
    "libcapnp-0.5.3.so",
    "libcapnp-0.6.1.so",
    "libdatrie.so.1",
    "libdbus-1.so.3",
    "libepoxy.so.0",
    "libffi.so.6",
    "libgcrypt.so.20",
    "libgdk-3.so.0",
    #"libgdk_pixbuf-2.0.so.0",
    "libgdk-x11-2.0.so.0",
    "libgio-2.0.so.0",
    "libglib-2.0.so.0",
    "libgmodule-2.0.so.0",
    "libgobject-2.0.so.0",
    "libgraphite2.so.3",
    "libgtk-3.so.0",
    "libgtk-x11-2.0.so.0",
    "libkj-0.5.3.so",
    "libkj-0.6.1.so",
    "liblz4.so.1",
    "liblzma.so.5",
    "libmirclient.so.9",
    "libmircommon.so.7",
    "libmircore.so.1",
    "libmirprotobuf.so.3",
    "libmount.so.1",
    "libpango-1.0.so.0",
    "libpangocairo-1.0.so.0",
    "libpangoft2-1.0.so.0",
    "libpcre2-8.so.0",
    "libpcre.so.3",
    "libpixman-1.so.0",
    "libprotobuf-lite.so.9",
    "libreadline.so.8",
    "libselinux.so.1",
    "libsystemd.so.0",
    "libthai.so.0",
    "libwayland-client.so.0",
    "libwayland-cursor.so.0",
    "libwayland-egl.so.1",
    "libwayland-server.so.0",
    "libX11-xcb.so.1",
    "libXau.so.6",
    #"libxcb-cursor.so.0",
    #"libxcb-glx.so.0",
    #"libxcb-icccm.so.4",
    #"libxcb-image.so.0",
    #"libxcb-keysyms.so.1",
    #"libxcb-randr.so.0",
    #"libxcb-render.so.0",
    #"libxcb-render-util.so.0",
    #"libxcb-shape.so.0",
    #"libxcb-shm.so.0",
    #"libxcb-sync.so.1",
    #"libxcb-util.so.1",
    #"libxcb-xfixes.so.0",
    #"libxcb-xkb.so.1",
    "libXcomposite.so.1",
    "libXcursor.so.1",
    "libXdamage.so.1",
    "libXdmcp.so.6",
    "libXext.so.6",
    "libXfixes.so.3",
    "libXinerama.so.1",
    "libXi.so.6",
    #"libxkbcommon.so.0",
    #"libxkbcommon-x11.so.0",
    "libXrandr.so.2",
    "libXrender.so.1",
]

def remove_libs(pth):
    for f in exclude_libs:
        libpth = os.path.join(pth, f)
        if os.path.isfile(libpth):
            os.remove(libpth)
        else:
            # If it fails, inform the user.
            print(f"{f} not found")

    dupe_pyside = glob.glob('{}/libpyside6*'.format(pth))
    if len(dupe_pyside):
        print('pyside libs {}'.format(dupe_pyside))
        for f in dupe_pyside:
            if os.path.isfile(f):
                os.remove(f)

    dupe_shiboken = glob.glob('{}/libshiboken6*'.format(pth))
    if len(dupe_shiboken):
        print('shiboken libs {}'.format(dupe_shiboken))
        for f in dupe_shiboken:
            if os.path.isfile(f):
                os.remove(f)

def clean_pillow_libs(srcd, destd):
    print('src {}'.format(srcd))
    print('dest {}'.format(destd))
    pillow_libs = next(os.walk(srcd), (None, None, []))[2]  # [] if no file
    #pillow_libs = glob.glob('{}/*'.format(srcd))
    print('pillow.libs {}'.format(pillow_libs))
    for entry in pillow_libs:
        f = os.path.join(destd, entry)
        print('File to remove {}'.format(f))
        if os.path.isfile(f):
            os.remove(f)

if __name__ == '__main__':
    appimagelib_path = sys.argv[1]
    py_ver = sys.argv[2]
    pil_src = os.path.join(appimagelib_path, 'python{}'.format(py_ver), 'site-packages', 'pillow.libs')
    remove_libs(appimagelib_path)
    clean_pillow_libs(pil_src, appimagelib_path)
