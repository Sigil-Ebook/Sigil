.. include:: substitutions.txt

.. _epub-overview:

EPUB Overview
=============

The |epub| format is a standard created by the *International Digital Publishing Forum* (|idpf|). It consists of three separate specifications:

* *Open Publication Structure* (|ops|), which describes how the content is to be presented;
* *Open Packaging Format* (|opf|), which describes how the content files and resources are connected into a *logical* whole, a *publication*;
* *OEBPS Container Format* (|ops|), which describes how the publication is encapsulated in a |zip| archive.

This chapter will provide a brief overview of the format aimed at beginners.

Open Publication Structure
--------------------------

This specification describes what the different content files of the book should look like. The basis of the content files is :term:`XHTML` 1.1. An example document follows:

.. code-block:: html

    <?xml version="1.0" encoding="UTF-8" ?>
    <!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" 
        "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
    <html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
      <head>
        <link rel="stylesheet" type="text/css" 
            href="../Styles/stylesheet.css" />
      </head>
      <body>
        <p>This is the first paragraph. 
           This is the first paragraph. 
           This is the first paragraph.
        </p>
        <p>This is the second paragraph.
           This is the second paragraph. 
           This is the second paragraph.
        </p>
        <img src="../Images/example_image.jpg">
      </body>
    </html>
    
You would see something very similar to this if you switched to :ref:`code-view` in the main Sigil :term:`UI`. Notice the pattern: content is *marked up* between the start and end *tags* of an *element*. This is how :term:`XML` (on which |xhtml| is based) works. The ``<p>`` element is a good example; it delimits the words and sentences that should be regarded as making up a single paragraph. There are many more elements in |xhtml|.

With |xhtml|, the user semantically marks up the content of the document and specifies its structure. Usually, a :term:`CSS` stylesheet is "applied" to the document. In this example, it is linked with the ``<link>`` element; this tells any |xhtml| renderer to use this stylesheet when rendering the document.

What does the renderer do? It looks at the marked-up document and the linked stylesheets and determines how this content should be displayed on the user's screen. The |xhtml| document describes *what* is the content, and the |css| stylesheet describes *how* it should be laid out and formatted. |xhtml| answers questions like: "What is a paragraph? What is a list of items? What is a table?", and |css| answers questions like: "How much should the paragraphs be indented? What color is the text? What fonts should be used? How big is the text?". 

You create many |xhtml| documents [#]_ for one |epub| file. Customarily, you create one per book chapter, but you can do it differently if you want to (but do so only if you have a good reason). For novels, one stylesheet is usually enough and is used in all the |xhtml| files.

Now you may be wondering, why should readers of an |epub| have to jump from one |xhtml| file to the next? Why can't they just see the book as one text "flow"? The answer is that they do. The readers are *not* aware that an |epub| file is an archive of several different files since their :term:`Reading System` always displays everything in it as one text flow. Even when every chapter is actually a separate file, when the user pages through their book, they see one chapter ending, and on the next page, the next chapter starting. They are *never* aware that their *Reading System* has actually transitioned them from one file to the next.

You may be surprised, but Microsoft Word's new |docx| format is also several marked-up files inside a |zip| archive. And yet you see it as "one" document.

Open Packaging Format
---------------------

This specification deals with connecting the various |xhtml|, |css|, font and image files into a "publication". It brings in two new |xml| files: the |opf| and the |ncx|.

.. _opf:

The OPF file
~~~~~~~~~~~~

The |opf| file describes several major components:

* The metadata — the metadata for the publication;
* The manifest — lists all the files that make up the publication;
* The spine — provides a linear reading order of the |xhtml| files;
* The guide — provides a set of references to some of the basic structural elements of the publication, such as a table of contents, foreword, bibliography, etc. 

Here is an example |opf| file:

.. code-block:: xml

    <?xml version="1.0"?>
    <package version="2.0" 
             xmlns="http://www.idpf.org/2007/opf" 
             unique-identifier="BookId" >
     
      <metadata xmlns:dc="http://purl.org/dc/elements/1.1/"
                xmlns:opf="http://www.idpf.org/2007/opf">
        <dc:title>Tale of Two Cities</dc:title>
        <dc:creator opf:file-as="Dickens, Charles" 
                    opf:role="aut">Charles Dickens</dc:creator>
        <dc:language>en</dc:language>
        <dc:identifier id="BookId" opf:scheme="ISBN">
            123456789X
        </dc:identifier>
      </metadata>
     
      <manifest>
        <item id="ncx"        href="toc.ncx" 
            media-type="application/x-dtbncx+xml"/>
        <item id="chapter001" href="Text/chapter001.xhtml" 
            media-type="application/xhtml+xml"/>
        <item id="chapter002" href="Text/chapter002.xhtml" 
            media-type="application/xhtml+xml"/>
        <item id="loi"        href="Text/loi.xhtml" 
            media-type="application/xhtml+xml"/>
        <item id="stylesheet" href="Styles/stylesheet.css"
            media-type="text/css"/>
        <item id="cover"      href="Images/cover.png" 
            media-type="image/png"/>
        <item id="caecilia"   href="Fonts/caecilia.otf" 
            media-type="application/x-font-opentype"/>        
      </manifest>
     
      <spine toc="ncx">
        <itemref idref="chapter001" />
        <itemref idref="chapter002" />
      </spine>
     
      <guide>
        <reference type="loi" title="List Of Illustrations"
                   href="loi.xhtml" />
      </guide>
      
    </package>
    
You can clearly see the ``metatada``, ``manifest``, ``spine`` and ``guide`` elements and their children. There really is nothing complicated about this, lots of people end up writing all this information by hand. It's tedious, but doable. Sigil writes this file automatically for you, and in the future it will provide the user with the means of editing it directly if he wants the extra power that comes with this ability.

.. _ncx:

The NCX file
~~~~~~~~~~~~

The other file that every publication needs is the *Navigation Center eXtended* (|ncx|). This file describes the hierarchical Table of Contents (|toc|) for the publication. Here's an example:

.. code-block:: xml

    <?xml version="1.0" encoding="UTF-8"?>
    <!DOCTYPE ncx PUBLIC "-//NISO//DTD ncx 2005-1//EN"
    "http://www.daisy.org/z3986/2005/ncx-2005-1.dtd">
     
    <ncx version="2005-1" 
         xml:lang="en" 
         xmlns="http://www.daisy.org/z3986/2005/ncx/">
     
      <head>
        <meta name="dtb:uid" content="123456789X"/>
        <meta name="dtb:depth" content="1"/>
        <meta name="dtb:totalPageCount" content="0"/>
        <meta name="dtb:maxPageNumber" content="0"/>
      </head>
     
      <docTitle>
        <text>Tale of Two Cities</text>
      </docTitle>
     
      <docAuthor>
        <text>Dickens, Charles</text>
      </docAuthor>
     
      <navMap>
        <navPoint class="chapter" id="chapter001" playOrder="1">
          <navLabel><text>Chapter 1</text></navLabel>
          <content src="chapter00l.xhtml"/>
        </navPoint>
        
        <navPoint class="chapter" id="chapter002" playOrder="2">
          <navLabel><text>Chapter 2</text></navLabel>
          <content src="chapter002.xhtml"/>
        </navPoint>
      </navMap>
     
    </ncx>
    
The ``<navPoint>`` elements point either to whole documents, or to the specific elements within those documents. They can be nested to create a hierarchical |toc|.

Sigil creates this file from the headings present in your |xhtml| documents. Every heading is referenced by a ``navPoint``, and headings of different levels interact to create a hierarchy. More details of this behaviour can be found in the :ref:`tocbuild` section. As with the |opf| file, future versions of Sigil will enable direct editing of this file for those who want it.

.. note:: If your heading is "near" the beginning of your |xhtml| file, Sigil will link directly to that file and not to the heading element. This is because on some *Reading Systems*, linking directly to an element slows down the display of the |toc|.

OEBPS Container Format
----------------------

The |ocf| specification states how the publication should be packaged. Large parts of it deal with things like encryption and alternate renditions of a publication within a single archive, but the main idea to take away is that |epub| files are basically |zip| archives of the files comprising the publication. The `ZIP <http://en.wikipedia.org/wiki/ZIP_(file_format)>`_ format stores the files using the `DEFLATE <http://en.wikipedia.org/wiki/DEFLATE>`_ compression algorithm.

|epub| files also must have a ``META-INF`` folder with a ``container.xml`` file pointing to the |opf| file of the publication. Sigil takes care of all of this for you.

.. [#] In the context of |epub|, |xhtml| documents are often called |ops| documents. 

   





