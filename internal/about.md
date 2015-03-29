internal
========

Internal libraries. These are _not_ bundled copies but divergent from upstream projects.
Libraries in this directory should _never_ be removed and system libraries should _never_
be using in place of libraries in this directory.

The divergence is necessary for Sigil and either upstream will not merge our changes or
upstream cannot merge our changes (no longer exists).

For example Gumbo has two competing upstreams (Google and GitHub) and neither is willing
to accept our epub specific parsing changes.
