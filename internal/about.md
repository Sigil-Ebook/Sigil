internal
========

Internal libraries. These are _not_ bundled copies but divergent from upstream projects.
Libraries in this directory should _never_ be removed and system libraries should _never_
be using in place of libraries in this directory.

The divergence is necessary for Sigil and either upstream will not merge our changes or
upstream cannot merge our changes (no longer exists).

For example Gumbo has two competing upstreams (Google and GitHub (vmg)) and neither 
has accepted our xhtml/epub specific parsing changes including the use of a larger 
set of known tags, and changes to make editing the gumbo tree possible, etc.  

The reasons for not accepting our changes are of course quite valid for those 
projects as gumbo is an html parser not an xhtml parser and the larger tag set means 
a bigger memory footprint and probably a bit slower performance, etc.

Our changes are needed to allow xhtml parsing and to allow for editing the gumbo tree
(only recognized tags can be edited given no underlying source is available), etc. 

For the full rambling discussions see these posts:
  https://github.com/google/gumbo-parser/pull/295
  https://github.com/google/gumbo-parser/issues/311
  https://github.com/vmg/gumbo-parser/pull/2

