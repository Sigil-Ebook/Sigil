.. include:: substitutions.txt

.. _toc-editor:

Table of Contents Editor
========================

A Table of Contents (|toc|) makes navigation simpler and provides a quick overview of a book's structure. Clicking on a |toc| item in a :term:`Reading System` "jumps" the user to that point in the book. With Sigil, you can easily create a hierarchical |toc|.

.. _tocbuild:

Building the TOC
----------------

Sigil follows standard word-processor practice by creating a |toc| from the headings used in the text. The hierarchy of your |toc| is determined by the "level" of each heading. :guilabel:`Heading 1` produces the largest text and :guilabel:`Heading 6` the smallest. In terms of hierarchy, a higher number is the child of a lower number. 

For example if the headings in your book are in the level order of ``1 2 2 3 3 1 2 3 4``, they will appear like this in the |toc|::

  Heading 1
    Heading 2
    Heading 2
      Heading 3
      Heading 3
  Heading 1
    Heading 2
      Heading 3
        Heading 4

Creating Headings
~~~~~~~~~~~~~~~~~

While in :ref:`book-view`, select the text you want to use in your |toc|. Use the heading drop-down to apply a heading level.

.. _fig_heading_dropdown:

.. figure:: _static/heading_dropdown.* 
   
   Heading Drop-Down

You can also define a heading in :ref:`code-view` by placing text in heading tags. For example:

.. code-block:: html

  <h1>HEADING 1</h1>

.. hint:: For the technical details regarding the storage of the Table of Contents, see :ref:`ncx`.

Generating the |toc|
~~~~~~~~~~~~~~~~~~~~

At any time you can click the :guilabel:`Generate |toc| from headings` button in the Table of Contents pane (:menuselection:`View --> Table of Contents`) to update your |toc|.

.. _fig_generate_toc:

.. figure:: _static/generate_toc.* 
   
   The Generate |toc| from headings button

When you do this the :guilabel:`Heading Selector` window will open. This window allows you to uncheck any headings you want to exclude from your |toc|. As you uncheck headings they will be removed from the list, but not from the text.

Notice the :guilabel:`TOC items only` checkbox at the bottom of the dialog. If this option is checked, the list will show *only* the items that will be included in the final |toc|. The headings that will be excluded (those that were unchecked) are not shown. Checking :guilabel:`TOC items only` shows them again.

Click :guilabel:`OK` to generate your |toc|.

.. _fig_heading_selector:

.. figure:: _static/heading_selector.* 
   
   Heading Selector Dialog

Previewing and Testing the |toc|
--------------------------------

By default a preview of the |toc| is displayed in a pane on the right-hand side of the Sigil window. You can toggle this pane on and off by navigating to :menuselection:`View --> Table of Contents` or with Alt + F3 on your keyboard.

.. _fig_toc_preview:

.. figure:: _static/toc_preview.* 
   
   |toc| Preview

This pane allows you to see how your |toc| is structured. If you double-click on any entry you will jump to that location in your text.

  
Displaying Different Text
-------------------------

You may want to display a different value in the |toc| than the actual heading text. For example, your heading may be "HEADING 1" but in the |toc| you want it to display as "Heading 1".

You can do this by locating the heading element in the :ref:`code-view` and adding a ``title`` attribute to it. 

.. code-block:: html

  <h1 title="Chapter I">CHAPTER I</h1>

The attribute ``title`` should contain the text as you want it displayed in the |toc|.

You can use the same functionality if you want an image to be the target of a |toc| entry:

.. code-block:: html

  <h1 title="Text in TOC"><img src="../Images/some_image.png" /></h1>