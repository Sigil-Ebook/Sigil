= Introduction =

Starting with version 0.5.0, the user interface of Sigil supports being
displayed in multiple languages.

= Web Tools =

Sigil can be translated using the web based translation system Transifex. See
https://www.transifex.net/projects/p/sigil/ . This is the preferred method for
translating Sigil.


= Desktop Tools =

Sigil uses Qt's translation system. If you have Qt installed you can use Qt's
linguist tool to edit translations.


== How to add translations ==

Updating and creating new translation files is the same process.

* Open a terminal and change to the src/Sigil/Resource_Files/ts directory. This
  holds all of the raw translation files.
* Run `lupdate ../../* -ts out.ts` to either generate a new translation file or
  to update an existing on with any new strings that have been added.
* Use Linguist (Qt's translation editor to write translations for the strings
  in a given ts file.


= Naming convention =

All translations files should have the form sigil_lang.ts. Where lang is the
two letter language code. For example the Polish translation will have the
filename sigil_pl.ts and the German translation will have the filename
sigil_de.ts.
