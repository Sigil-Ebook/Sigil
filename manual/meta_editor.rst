.. include:: substitutions.txt

.. _meta-editor:

Meta Editor
===========

The Meta Editor is what you use when you want to add or edit metadata for your |epub| book. As the :ref:`epub-overview` explains, the standard allows for hundreds of different metadata entries to be added to any book.

You can open the Meta Editor by selecting :menuselection:`Tools --> Meta Editor`.

Collapsed UI
------------

When you first open the Meta Editor, it will be in its collapsed state, as shown in :ref:`fig_metacol`. This allows you to add the minimum metadata elements that *all* |epub| books need to have: the :guilabel:`Title` of the work, the :guilabel:`Author` and the :guilabel:`Language` in which it is written. If you don't include these elements, then tools like :term:`epubcheck` will complain that your book has errors (rightfully so).

This information is the bare minimum you should provide. You can always go the extra mile though. The expanded version of the dialog enables you to do just that.

.. _fig_metacol:

.. figure:: _static/meta_small.* 
   
   Meta Editor, collapsed
   
.. hint:: You can list several authors of a book by separating the names with a semicolon. Like this: ``Smith, John; Doe, Jane``.
   
Expanded UI
-----------
   
If you click on the :guilabel:`More` button, the dialog will expand; see :ref:`fig_metaex`. You now have a table listing all the other types of metadata present in your |epub| book. You can remove an item by selecting it and clicking the :guilabel:`Remove` button, or add new items with the :guilabel:`Add Basic` and :guilabel:`Add Adv[anced]` buttons.
   
.. _fig_metaex:

.. figure:: _static/meta_large.* 
   
   Meta Editor, expanded
   
The basic metadata items (see :ref:`fig_basicmeta`) are things like the dates of publication, creation and modification; description; content coverage; intellectual property rights; the |isbn|, |issn| and |doi| numbers etc. You don't need to use any of these, the only ones that are required are the aforementioned title, author and language. But if you do know any of this information, do add it to your books.

.. _fig_basicmeta:

.. figure:: _static/add_basic_meta.* 
   
   Basic metadata list   
   
While there are only a few basic metadata items, there's more than two hundred advanced entries (see :ref:`fig_advmeta`), ranging from *Illustrator* to *Patent holder* to *Performer*. These all describe the various contributors to a work, and are terms defined and used by the Library of Congress.  
   
.. _fig_advmeta:

.. figure:: _static/add_adv_meta.* 
   
   Advanced metadata list

.. hint:: Do you see the line that divides the list of metadata entries and the description text? Put your mouse cursor over it. Yes, it can be dragged to increase/decrease the amount of space for the description area.

In any field where you are expected to enter the name of a person (like *Author* or *Performer*), try to write the name in a "normalized" form. For instance, instead of ``John Smith``, write ``Smith, John``. Why? Because the |epub| standard provides alternate ways of storing the metadata information for people involved with the work. If you use ``John Smith``, Sigil will store this information as just ``John Smith`` and nothing else. But if you use ``Smith, John`` (notice the comma), then the name will be stored in *two* ways. People who read your book will then see ``John Smith``, but the :term:`Reading System` will categorize the book under ``Smith, John``.

This makes machine processing easier, and also makes more sense for anyone searching through their book collection.

.. hint:: For the technical details regarding the storage of metadata, see :ref:`opf`.





   
   