.. include:: substitutions.txt

.. _toc-editor:

Table Of Contents Editor
========================

Every book needs a Table of Contents (|toc|). It makes navigation simpler, and also provides a quick overview of the book's structure. With Sigil, one can create a hierarchical |toc| with ease. The user interface is illustrated in :ref:`fig_toc`. 

You can open the |toc| Editor by selecting :menuselection:`Tools --> TOC Editor`.

.. _fig_toc:

.. figure:: _static/toc_editor.* 
   
   |toc| editor   
   
You should provide your books with a |toc|, since clicking on an item in it in a :term:`Reading System` "jumps" the user to that point in the book. This can be very useful.

.. _tocbuild:

Building the TOC
----------------

Sigil follows standard word-processor practice by creating a |toc| from the headings used in the text. The :ref:`book-view` provides a drop-down box for changing normal text into a heading level, from one to six. Headings of a lower level ("bigger" headings) become the parents of headings of a higher level ("smaller" headings) that succeed them.

Let's look at an example. If the headings that are used in your book are in the level order of ``1 2 2 3 3 1 2 4 3``, they would be ordered in the |toc| like this::

    Heading 1
        Heading 2
        Heading 2
            Heading 3
            Heading 3
    Heading 1
        Heading 2
            Heading 4
            Heading 3
            
Usually, the "natural" |toc| that Sigil creates using this technique will be enough for 90% of the books you'll want to make. But you can always make changes directly.

.. hint:: For the technical details regarding the storage of the Table of Contents, see :ref:`ncx`.
    
The editor interface
--------------------

The editor shows you what the |toc| will look like on a *Reading System*. Double-clicking on the text of a heading makes the text editable. Note that any changes here will also change the heading in the text of the book.

Every entry also has a checkbox next to it. Unchecking the box removes that heading from the |toc|, but the heading itself remains in the text. Checking the box includes the heading back in.

Notice the :guilabel:`TOC items only` checkbox at the bottom of the dialog. If the option is checked, the list shows *only* the items that will be included in the final |toc|: the headings that will be ignored (those that were unchecked) are not shown. Checking this *TOC items only* option shows them again.

.. note:: Unchecking a heading while the *TOC items only* option is selected will make it "disappear" from the list. You can see it again by deselecting the *TOC items only* option.
    
Advanced uses
-------------

Advanced users may want to show a different value in the |toc| than what the actual heading text is. This can be easily achieved by locating the heading element in the :ref:`code-view` and adding a ``title`` attribute to it. 

.. code-block:: html

   <h1 title="Alternate text">Normal text</h1>

The attribute should hold the alternate text that should be used for the |toc|.

You can also use this functionality to have images as the targets of the |toc| entries:

.. code-block:: html

   <h1 title="Text in TOC"><img src="../Images/some_image.png" /></h1>   
   
   
   
 

 




