Introduction
============

Starting with version 0.5.0, the user interface of Sigil supports being
displayed in multiple languages.


Web Tools
=========

Sigil can be translated using the web based translation system Transifex. See
https://www.transifex.net/projects/p/sigil/ . This is the preferred method for
translating Sigil.


Updating Base Translation File
==============================

1. Open a terminal and change to the src/Sigil/Resource_Files/ts directory.
2. Run `lupdate ../../* -ts base.ts`


Naming convention
=================

All translations files should have the form sigil_lang.ts. Where lang is the
two letter language code. For example the Polish translation will have the
filename sigil_pl.ts and the German translation will have the filename
sigil_de.ts.
