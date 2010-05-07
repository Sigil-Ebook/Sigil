.. include:: substitutions.txt

Find & Replace
==============

Sigil includes a powerful Find & Replace (F&R) dialog. By default, it opens in its collapsed state; see :ref:`fig_search_small`. To see and modify the available options, press the :guilabel:`More` button. The dialog would then expand (see :ref:`fig_search_large`) and show you the following options::

Match whole word only
    Available only for the *Normal* search mode. Using ``car`` with this option selected would match ``car``, but not ``carpet``.
    
Match case
    All matching is by default case-insensitive. Using ``John`` with this option selected would match ``John``, but not ``john``.
    
Minimal matching
    Available only for the *Wildcard* and *Regular expression* search modes. Makes the search string match as little as possible.
    
Direction Up
    Searches from the :term:`caret` upwards.

Direction Down
    Searches from the :term:`caret` downwards.
    
Direction All
    Searches from the :term:`caret` downwards, but loops back and starts from the beginning when the end is reached.
    
The :guilabel:`Look in` combo-box offers various places where Sigil should search. Currently, it can only search in the currently open file or all the |html| files [#]_. In the future it will be able to search across all the |css| files and all the files of any type in the |epub| book.

.. [#] And only in the :ref:`code-view`, not in the :ref:`book-view`. This limitation exists because of technical restrictions, and will be removed in time. 
    
This leads us to the various search modes, which represent the true power of the Find & Replace dialog; the *Normal* search mode just matches every character you typed exactly as it is, but the other modes are more involved.

.. _fig_search_small:

.. figure:: _static/search_small.* 
   
   Find & Replace, collapsed  
   
   
.. _fig_search_large:

.. figure:: _static/search_large.* 
   
   Find & Replace, expanded


Wildcard mode
-------------

One of the available advanced matching modes is the *Wildcard* mode. It should be familiar to anyone who's used file globbing in ``bash`` or Windows' ``cmd``. It is fairly straightforward:

.. tabularcolumns:: |c|l|

===========  =========
 Character    Matches 
===========  =========
``a``         Any character represents itself by default. ``a`` would match ``a``.
``?``         Matches *any* single character.
``*``         Matches *zero or more* of *any* characters.
``[...]``     Any character in the set; ``[abc]`` would match ``a`` *or* ``b`` *or* ``c``.
===========  =========

So if for example you used the search string ``[123]abc?dd*`` with the *Wildcard* mode selected, it would match any of the following: ``1abceddertert``, ``3abc9dd--.!``, ``2abc_dd!#$79840sd  12adad ad`` and many more. 

Regular expression mode
-------------------------
The regular expression (*regex*) engine used in Sigil is very "Perl-like" [#]_. A general introduction to regular expressions is beyond the scope of this manual. There are many good overviews on the internet; one of which is available `here <http://www.regular-expressions.info/tutorialcnt.html>`_. That site will help you get started with regexes.

Here are *some* of the differences from Perl:

* In Sigil ``^`` always signifies the start of the string (apart from within character classes), so carets should be escaped unless used in this way. The same applies to ``$`` which in Sigil always signifies the end of the string.

* Non-greedy matching cannot be applied to individual quantifiers. Expressions like :regexp:`abc*?dd` are not allowed. Use the *Minimal matching* option to set minimal matching on the whole expression.

* Back-reference syntax is ``sed``-like, that is in the form of ``\#``, e.g. ``\1``, ``\2``, ``\3`` etc. (``\0`` is the whole matched string). 

* While zero-width positive and zero-width negative lookahead assertions (in the form of ``(?=pattern)`` and ``(?!pattern)``) are supported, Perl's look-behind assertions, "independent" subexpressions and conditional expressions are **not** supported.

.. [#] The current regex engine used is actually Qt's QRegExp. It will eventually be replaced with `PCRE <http://en.wikipedia.org/wiki/Perl_Compatible_Regular_Expressions>`_ because of the latter's advanced features and performance.


 