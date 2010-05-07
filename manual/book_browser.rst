.. include:: substitutions.txt

.. _book-browser:

Book Browser
============

The Book Browser (|bb|) offers a view into the structure of your |epub| book. You can see all the different files that make up the archive, from :term:`XHTML` files to images.

.. hint:: If your not familiar with the internal structure of an |epub| file, see the :ref:`epub-overview` chapter. 

The Book Browser pane can be either docked, or undocked. A docked |bb| can be undocked by clicking on the little window icon in the top right corner, and docked by simply dragging the new window to the edge of the main Sigil window and then releasing the mouse button. An image of it undocked can be see in :ref:`fig_bb`. 

You can also open and close the Book Browser through the :guilabel:`View` menu.

.. _fig_bb:

.. figure:: _static/book_browser.* 
   
   Book Browser, undocked

Content folders
---------------

The view is organized into several folders which can be expanded or collapsed by clicking on the little arrow icon next to them:

* Text — all the |xhtml| files that make up the text of the work;
* Styles — the |css| and |xpgt| stylesheets;
* Images — |png|, |jpg|, |gif| and other image types you have in your book;
* Fonts — |ttf| and |otf| fonts;
* Misc — miscellaneous files.

In most folders, the files are listed alphabetically. The exception is the ``Text`` folder in which the files are listed in their reading order. Since the |epub| archive consists of several |xhtml| files (customarily one per book chapter), the order in which they are displayed to the user needs to be determined in advance. Thus, the ``Text`` folder allows you to click a file and drag it amongst the other files in that folder. The file at the "top" is the one which will be shown to the user first, and the one at the bottom will be shown last.

Context menu
------------
 
Right-clicking on an item in the Book Browser brings up a context menu, and depending on the item on which you invoked the menu, you'll be able to:

* Add existing items,
* Add new items,
* Remove the file,
* Rename the file,
* Add semantic information.

If you select :guilabel:`Add existing items`, then an :guilabel:`Open file` dialog will be shown. From here, you'll be able to add any type of file that can be embedded inside an |epub|, and they'll be automatically routed to the correct folder. If you pick an |htm|, |html| or |xhtml| file (or several), Sigil will also import all the resources that those files reference, like |css| stylesheets or images (but *not* other |xhtml| files). Using this technique, you can build up your |epub| file from several |html| files prepared in advance.

The :guilabel:`Add Semantics` sub-menu is special. It is only displayed for |xhtml| files and images, and the contents differ. When invoked on an |xhtml| file, it enables you to indicate that that file contains the book's dedication section, the glossary, the foreword, the preface and many more [#]_. Some (but few) Reading Systems will then use this information and display it to the user.

.. [#] For those interested in the technical details, this information is stored in :ref:`opf`'s ``<guide>`` element. 

For images, a different semantic action is offered. You can mark an image as a *cover*. This bit of semantic information is important for several *Reading Systems*, the most prominent of which is the *iBooks* application for the *iPad*. If an image is not marked as a cover, the book won't have a cover set in the iBooks "bookshelf".

.. hint:: 
    Sigil has heuristics that will mark the appropriate image as the cover if you don’t do it yourself. If the first file in the reading order is “very small” and has only one image in it [#]_, that image will be selected as the cover.

    So if you follow best practices, Sigil helps you out. Still, mark it by hand if you can. You will always know better than the machine.  

.. [#] Sigil looks for a normal ``<img>`` tag or an |svg| ``<image>`` one.


 
   

   

    
