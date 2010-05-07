.. include:: substitutions.txt

Introduction
============

`Sigil <http://code.google.com/p/sigil/>`_ is a free and open source editor for the |epub| format. It is designed for easy, :term:`WYSIWYG` editing of |epub| files and for converting other formats to |epub|. It also provides features for advanced users, like direct :term:`XHTML`, :term:`CSS` and :term:`XPGT` editing. You can use it to add any of the metadata entries supported by the |epub| specification and create a hierarchical Table of Contents.

The editor works on all three major platforms: Windows, Linux and Mac OS X.

Sigil is a work in progress, and as such is still missing many features. If you have the programming skills, you can directly contribute to its development. Even if you don't, just using Sigil and reporting any bugs or missing features on the `issue tracker <http://code.google.com/p/sigil/issues/list>`_ will go a long way towards making it a better application for everyone.

Installation
------------

Installing Sigil is fairly easy on all platforms. The files can be downloaded from the `download section <http://code.google.com/p/sigil/downloads/list>`_ of the project site.

Windows
~~~~~~~

On Windows, you merely run the the appropriate installer. If you have a 64 bit version of Windows, you should use the Windows installer labeled with ``x86_64``. Otherwise, use the vanilla one.

Then just run it. Everything should take care of itself. If you get an "application configuration is incorrect" message when starting Sigil, see `this item <http://code.google.com/p/sigil/wiki/FAQ#I_am_running_Windows_and_I_get_an_%22application_configuratio>`_ in the |faq|.

Sigil is tested on XP, Vista x64 and Windows 7 x64.

Linux
~~~~~

Run the installer for your |cpu| architecture. Two are available: ``x86`` and ``x86_64``. If in doubt, use the ``x86`` one. The `Qt Framework <http://qt.nokia.com/products>`_ is included in the installer.

You have to do this from the console. First, make the installer executable and then run it:

.. code-block:: bash

    chmod +x installer.bin
    sudo ./installer.bin

After the installer completes, there should be an icon on your desktop and an entry in your "start menu".

Sigil can be uninstalled by running the ``uninstall`` executable located in the folder in which Sigil was installed. By default, this is ``/opt/sigil``. So with default options, uninstalling Sigil would look like this:

.. code-block:: bash

    cd /opt/sigil
    sudo ./uninstall

Sigil is tested on Ubuntu 9.10 (Karmic Koala).

Mac OS X
~~~~~~~~

Unpack the DMG file and drag the ``Sigil.app`` file to your ``Applications`` folder.

Sigil is built as a universal binary, and is tested on Mac OS X 10.5 and 10.6. Sigil should also work on Tiger (10.4), but is untested on that version of Mac OS X.