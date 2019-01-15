======
README
======
.. -*- restructuredtext -*-

-------------------------------------------------------
cssutils: CSS Cascading Style Sheets library for Python
-------------------------------------------------------
:Copyright: 2004-2013 Christof Hoeke

Overview
========
A Python package to parse and build CSS Cascading Style Sheets. DOM only, not any rendering facilities!

Based upon and partly implementing the following specifications :

`CSS 2.1rev1 <http://www.w3.org/TR/CSS2/>`__
    General CSS rules and properties are defined here
`CSS3 Module: Syntax <http://www.w3.org/TR/css3-syntax/>`__
    Used in parts since cssutils 0.9.4. cssutils tries to use the features from CSS 2.1 and CSS 3 with preference to CSS3 but as this is not final yet some parts are from CSS 2.1
`CSS Fonts Module Level 3 <http://www.w3.org/TR/css3-fonts/>`__
    Added changes and additional stuff (since cssutils v0.9.6)
`MediaQueries <http://www.w3.org/TR/css3-mediaqueries/>`__
    MediaQueries are part of ``stylesheets.MediaList`` since v0.9.4, used in @import and @media rules.
`Namespaces <http://dev.w3.org/csswg/css3-namespace/>`__
    Added in v0.9.1, updated to definition in CSSOM in v0.9.4, updated in 0.9.5 for dev version
`CSS3 Module: Pages Media <http://www.w3.org/TR/css3-page/>`__
    Most properties of this spec are implemented including MarginRules
`Selectors <http://www.w3.org/TR/css3-selectors/>`__
    The selector syntax defined here (and not in CSS 2.1) should be parsable with cssutils (*should* mind though ;) )
`CSS Backgrounds and Borders Module Level 3 <http://www.w3.org/TR/css3-background/>`__, `CSS3 Basic User Interface Module <http://www.w3.org/TR/css3-ui/#resize>`__, `CSS Text Level 3 <http://www.w3.org/TR/css3-text/>`__
    Some validation for properties included, mainly  `cursor`, `outline`, `resize`, `box-shadow`, `text-shadow`
`Variables <http://disruptive-innovations.com/zoo/cssvariables/>`__ / `CSS Custom Properties <http://dev.w3.org/csswg/css-variables/>`__
    Experimental specification of CSS Variables which cssutils implements partly. The vars defined in the newer CSS Custom Properties spec should in main parts be at least parsable with cssutils.

`DOM Level 2 Style CSS <http://www.w3.org/TR/DOM-Level-2-Style/css.html>`__
    DOM for package css. 0.9.8 removes support for CSSValue and related API, see PropertyValue and Value API for now
`DOM Level 2 Style Stylesheets <http://www.w3.org/TR/DOM-Level-2-Style/stylesheets.html>`__
    DOM for package stylesheets
`CSSOM <http://dev.w3.org/csswg/cssom/>`__
    A few details (mainly the NamespaceRule DOM) are taken from here. Plan is to move implementation to the stuff defined here which is newer but still no REC so might change anytime...

The cssutils tokenizer is a customized implementation of `CSS3 Module: Syntax (W3C Working Draft 13 August 2003) <http://www.w3.org/TR/css3-syntax/>`_ which itself is based on the CSS 2.1 tokenizer. It tries to be as compliant as possible but uses some (helpful) parts of the CSS 2.1 tokenizer.

I guess cssutils is neither CSS 2.1 nor CSS 3 compliant but tries to at least be able to parse both grammars including some more real world cases (some CSS hacks are actually parsed and serialized). Both official grammars are not final nor bugfree but still feasible. cssutils aim is not to be fully compliant to any CSS specification (the specifications seem to be in a constant flow anyway) but cssutils *should* be able to read and write as many as possible CSS stylesheets "in the wild" while at the same time implement the official APIs which are well documented. Some minor extensions are provided as well.

Please visit http://cthedot.de/cssutils/ or https://bitbucket.org/cthedot/cssutils/ for more details.

There is also a low-traffic `cssutils discussion group <http://groups.google.com/group/cssutils>`_.


Compatibility
=============
cssutils is developed on standard Python but works under Python 2.x (from 2.5, 2.7.6 tested), 3.x (v3.3.3 tested) and Jython (from 2.5.1). IronPython has not been tested yet but might work? Python 2.4 and older are not supported since cssutils 0.9.8 anymore.
cssutils is not thread safe, please beware!

License
=======
Copyright 2005 - 2013 Christof Hoeke

cssutils is published under the LGPL 3 or later

cssutils is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

cssutils is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with cssutils. If not, see http://www.gnu.org/licenses.


Installation
============
From 0.9.6 cssutils uses `Distribute <http://pypi.python.org/pypi/distribute>`_

After installing Distribute use::

    > easy_install cssutils

to install the latest version of cssutils.

Alternatively download the provided source distribution. Expand the file and from a command line install with::

    > python setup.py install

To uninstall remove any registrations of cssutils eggs with Distribute and remove the eggs which should be installed at PYTHONDIR/Lib/site-packages/cssutils too.


Example
=======
::

    # -*- coding: utf-8 -*-
    import cssutils

    css = u'''/* a comment with umlaut &auml; */
         @namespace html "http://www.w3.org/1999/xhtml";
         @variables { BG: #fff }
         html|a { color:red; background: var(BG) }'''
    sheet = cssutils.parseString(css)

    for rule in sheet:
        if rule.type == rule.STYLE_RULE:
            # find property
            for property in rule.style:
                if property.name == 'color':
                    property.value = 'green'
                    property.priority = 'IMPORTANT'
                    break
            # or simply:
            rule.style['margin'] = '01.0eM' # or: ('1em', 'important')

    sheet.encoding = 'ascii'
    sheet.namespaces['xhtml'] = 'http://www.w3.org/1999/xhtml'
    sheet.namespaces['atom'] = 'http://www.w3.org/2005/Atom'
    sheet.add('atom|title {color: #000000 !important}')
    sheet.add('@import "sheets/import.css";')

    # cssutils.ser.prefs.resolveVariables == True since 0.9.7b2
    print sheet.cssText

results in::

	@charset "ascii";
	@import "sheets/import.css";
	/* a comment with umlaut \E4  */
	@namespace xhtml "http://www.w3.org/1999/xhtml";
	@namespace atom "http://www.w3.org/2005/Atom";
	xhtml|a {
	    color: green !important;
	    background: #fff;
	    margin: 1em
	    }
	atom|title {
	    color: #000 !important
	    }


Documentation
=============
The current documenation can be found at http://packages.python.org/cssutils/


Kind Request
============
cssutils is far from being perfect or even complete. If you find any bugs (especially specification violations) or have problems or suggestions please put them in the `Issue Tracker <https://bitbucket.org/cthedot/cssutils/issues>`_ at Bitbucket.


Thanks
======
Thanks to Simon Sapin, Jason R. Coombs and Walter Doerwald for patches, help and discussion. Thanks to Kevin D. Smith for the value validating module. Thanks also to Cory Dodt, Tim Gerla, James Dobson and Amit Moscovich for helpful suggestions and code patches. Thanks to Fredrik Hedman for help on port of encutils to Python 3.

