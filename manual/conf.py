# -*- coding: utf-8 -*-
#
# Sphinx documentation build configuration file

import re
import sphinx

master_doc = 'contents'
templates_path = ['_templates']
exclude_patterns = ['_build']

project = 'Sigil'
copyright = u'2009-2011, Strahinja Marković'
version = '0.4.1'
release = version

show_authors = True

html_theme = 'sphinxdoc'
modindex_common_prefix = ['sigil.']
html_static_path = ['_static']
html_logo = '_static/small_logo.png'
html_style = 'custom.css'
html_favicon = 'app_16.ico'

htmlhelp_basename = 'Sigildoc'

latex_documents = [('contents', 'sigil.tex', 'Sigil Manual',
                    u'Strahinja Marković', 'manual', 1)]
latex_logo = '_static/sigil.png'
latex_elements = {
    'fontpkg': '\\usepackage{palatino}',
    'papersize': 'a4paper',
    'pointsize': '12pt',
    'preamble': '\\linespread{1.05}' # palatino always needs more leading
}

