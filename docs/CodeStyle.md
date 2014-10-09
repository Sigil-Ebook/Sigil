Code Formatting Guidelines
==========================

Sigil uses astyle to auto format all .h and .cpp files. The following is used
for formatting:

    $ astyle -A10 -s4 -xl -S -w -Y -m1 -H -k3 -W3 -j -z2 -xy -R "*.h"
    $ astyle -A10 -s4 -xl -S -w -Y -m1 -H -k3 -W3 -j -z2 -xy -R "*.cpp"


-A10
----

"One True Brace Style" formatting/indenting uses linux brackets and adds
brackets to unbracketed one line conditional statements. Opening brackets
are broken from namespaces, classes, and function definitions. Brackets
re attached to everything else including statements within a function,
arrays, structs, and enums.

    int Foo(bool isBar)
    {
        if (isFoo) {
            bar();
            return 1;
        } else {
            return 0;
        }
    }


-s4
---

Indent using 4 spaces per indent.


-xl
---

Attach brackets to class and struct inline function definitions.


-S
--

Indent 'switch' blocks so that the 'case X:' statements are indented in the
switch block. The entire case block is indented.


-w
--

Indent multi-line preprocessor definitions ending with a backslash


-Y
--

Indent C++ comments beginning in column one.


-m1
---

Set the minimal indent that is added when a header is built of multiple lines.


-H
--

Insert space padding after paren headers only (e.g. 'if', 'for', 'while'...).


-k3
---

Attach a pointer or reference operator (*, &, or ^) to the variable name.


-W3
---

This option will align references separate from pointers.


-j
--

Add brackets to unbracketed one line conditional statements
(e.g. 'if', 'for', 'while'...).


-z2
---

Force use of the Linux (\n) line end style.


-xy
---

Closes whitespace in the angle brackets of template definitions. Closing the
ending angle brackets is now allowed by the C++11 standard.
