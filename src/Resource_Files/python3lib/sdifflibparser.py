"""

Modified for internal use in Sigil
Modifications Copyright (C) 2020 Kevin B. Hendricks, Stratford Ontario Canada

MIT License

Copyright (c) 2016 Yasser Elsayed

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

# Convert all info to strings so that they can be passed in a single string list
# [code, line, newline, leftchanges, rightchanges] 

import difflib

class DiffCode:
    SIMILAR   = '0'       # starts with '  '
    RIGHTONLY = '1'       # starts with '+ '
    LEFTONLY  = '2'       # starts with '- '
    CHANGED   = '3'       # either three or four lines with the prefixes ('-', '+', '?'), ('-', '?', '+') or ('-', '?', '+', '?') respectively

class DifflibParser:
    def __init__(self, text1, text2):
        self.__text1 = text1
        self.__text2 = text2
        self.__diff = list(difflib.ndiff(text1, text2, linejunk=None, charjunk=None))
        self.__currentLineno = 0

    def __iter__(self):
        return self

    def __next__(self):  # python3
        if self.__currentLineno >= len(self.__diff):
            raise StopIteration
        currentLine = self.__diff[self.__currentLineno]
        code = currentLine[:2]
        line = currentLine[2:]
        dcode = ''
        newline = ''
        leftchanges = ''
        rightchanges = ''
        if code == '  ':
            dcode = DiffCode.SIMILAR
        elif code == '- ':
            incrementalChange = self.__tryGetIncrementalChange(self.__currentLineno)
            if not incrementalChange:
                dcode = DiffCode.LEFTONLY
            else:
                dcode = DiffCode.CHANGED
                leftchanges = incrementalChange['left'] if 'left' in incrementalChange else ''
                rightchanges = incrementalChange['right'] if 'right' in incrementalChange else ''
                newline = incrementalChange['newline']
                self.__currentLineno += incrementalChange['skiplines']
        elif code == '+ ':
            dcode = DiffCode.RIGHTONLY
        self.__currentLineno += 1
        return (dcode, line, newline, leftchanges, rightchanges) 

    next = __next__  # for Python 2
    
    def __tryGetIncrementalChange(self, lineno):
        lineOne = self.__diff[lineno] if lineno < len(self.__diff) else None
        lineTwo = self.__diff[lineno + 1] if lineno + 1 < len(self.__diff) else None
        lineThree = self.__diff[lineno + 2] if lineno + 2 < len(self.__diff) else None
        lineFour = self.__diff[lineno + 3] if lineno + 3 < len(self.__diff) else None

        changes = {}
        # ('-', '?', '+', '?') case
        if lineOne and lineOne[:2] == '- ' and \
           lineTwo and lineTwo[:2] == '? ' and \
           lineThree and lineThree[:2] == '+ ' and \
           lineFour and lineFour[:2] == '? ':
            changes['left'] = lineTwo[2:]
            changes['right'] = lineFour[2:]
            changes['newline'] = lineThree[2:]
            changes['skiplines'] = 3
            return changes
        # ('-', '+', '?')
        elif lineOne and lineOne[:2] == '- ' and \
           lineTwo and lineTwo[:2] == '+ ' and \
           lineThree and lineThree[:2] == '? ':
            changes['right'] = lineThree[2:]
            changes['left'] = ""
            changes['newline'] = lineTwo[2:]
            changes['skiplines'] = 2
            return changes
        # ('-', '?', '+')
        elif lineOne and lineOne[:2] == '- ' and \
           lineTwo and lineTwo[:2] == '? ' and \
           lineThree and lineThree[:2] == '+ ':
            changes['right'] = ""
            changes['left'] = lineTwo[2:]
            changes['newline'] = lineThree[2:]
            changes['skiplines'] = 2
            return changes
        # no incremental change
        else:
            return None
