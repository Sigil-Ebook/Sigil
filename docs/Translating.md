Introduction
============

Starting with version 0.5.0, the user interface of Sigil supports being
displayed in multiple languages.


Web Tools
=========

Sigil can be translated using the web based translation system Transifex. See
https://www.transifex.com/zdpo/sigil/ . This is the preferred method for
translating Sigil.


Updating Base Translation File
==============================

NOTE: You need to delete base.ts first or it will get rebuilt in a broken 
manner for some reason (a bug in Qt?)

1. Make sure the Qt project "bin" directory is in your path so that
the right version of "lconvert" can be found. For me this is:

   export MYQTHOME=~/Qt512
   export PATH=${PATH}:${MYQTHOME}/bin

2. Open a terminal and change to the Sigil/src/Resource_Files/ts directory.

3. rm base.ts

4. Run `lupdate ../../* -ts base.ts`

5. Now manually edit the resulting base.ts file and change all of the 
following single numerusform tags:

    <numerusform></numerusform>

to be double empty numerusform tags as follows:

    <numerusform></numerusform><numerusform></numerusform>

so that our base.ts file will work with the much older tools used
on the Transifex site.



Naming convention
=================

All translations files should have the form sigil_lang.ts. Where lang is the
two letter language code. For example the Polish translation will have the
filename sigil_pl.ts and the German translation will have the filename
sigil_de.ts.
