.. include:: substitutions.txt

.. _main-ui:

The main user interface
=======================

Sigil provides an advanced editing environment for |epub| books. The main user interface (UI) is built around a tabbed central editing view-space and a :ref:`book-browser` to the left. You double-click files in the Book Browser with your mouse, and an editing (or display) tab is added to the main pane. These tabs can then be closed, reordered or opened again.

:ref:`fig_mainui` presents a typical example of the UI at work.

.. _fig_mainui:

.. figure:: _static/book_view.* 
   
   Main user interface

   
What are all these files?
-------------------------

You may be wondering what are all these files in the Book Browser. Why isn't the book displayed as a set of pages? Well for that you first need to understand how the |epub| format works. A more in-depth description can be found in the :ref:`epub-overview` section, but what follows is a short summation (but do read that section too).

An |epub| file is just a container for various resources that go into an |epub| book. The text of the book is in |xhtml| files and usually, content producers use one |xhtml| file for one chapter of the book. You can also commonly find several |css| styling files, images and fonts files. Now, along with all these resources come several other special files that describe the Table of Contents, the book's :term:`metadata`, the reading order of the |xhtml| files, list all the resources present in the container and more.

A :term:`Reading System` (|rs|) takes an |epub| file and then displays to the user the first |xhtml| file in the specified reading order. It doesn't present it like a web browser would—as one large page with a scrollbar—but "paginates" the content into pages the size of the user's screen. When the user reaches the end of the |xhtml| file, the next page will be the start of the succeeding |xhtml| file in the reading order.

What the |rs| provides is merely a paged rendition of those resources. *Every* |rs| will provide a slightly different rendition that adapts to the screen size and technical limitations of the device it's running on; that's the whole point of |epub|.

Now, that is how an |rs| works—everything is displayed in pages. But this is not the ideal way to work with a book when you're creating it. Books can well go into thousands of pages.

So Sigil presents a file-focused editing environment. You can open every |xhtml| file and edit it in a |wysiwyg| manner in the :ref:`book-view`, or edit the code directly in the :ref:`code-view`. You can edit the |css|, |xpgt| and other files, and view the images one by one.

To add and edit that special information like the Table of Contents and the metadata, Sigil presents you with purpose-built editors. You can access them through the :guilabel:`Tools` menu. Everything you enter in them is saved and exported when the resource files are bundled together and the |epub| book is built. When you open the resulting |epub| file in some |rs|, it will be displayed as a paged book.

The Views
---------

.. _book-view:

Book View
~~~~~~~~~

The Book View displays the |xhtml| files "rendered", that is as they would look like in a *Reading System* when presented to the user.

Most of the |wysiwyg| buttons and actions only work in this view. The :guilabel:`Headings` combo-box for instance changes the selected paragraph into a heading from which a Table of Contents can be built (see the :ref:`toc-editor` section). You can also insert images, chapter breaks and apply all the standard formatting operations like bold, italic, paragraph alignment and more.

This view is only available for |xhtml| files.

.. _code-view:

Code View
~~~~~~~~~

The Code View provides a way to edit the underlying code of |xhtml|, |css| and |xpgt| files while displaying it with advanced syntax highlighting. Line numbers are displayed in a gutter on the left side of the pane. 

An example of what an |xhtml| file looks like in this view can bee seen in the :ref:`fig_codeview`. 

.. _fig_codeview:

.. figure:: _static/code_view.* 
   
   Code View
   
Split View
~~~~~~~~~~

The Split View is a sort of combination between the previous two views: content is shown rendered in the top half, and the corresponding code is shown in the bottom half. Switching between one half to the other synchronizes their positions. The dividing bar between the bars can be dragged and adjusted with the mouse.  

This view is also only available for |xhtml| files. An example is shown in the :ref:`fig_splitview`.

.. _fig_splitview:

.. figure:: _static/split_view.* 
   
   Split View

 