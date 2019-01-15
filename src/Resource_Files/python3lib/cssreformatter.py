import sys
import os
import css_parser
from css_parser.serialize import Preferences as serialize_Preferences

# Portions Copyright (c) 2016 Francesco Martini
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Part of the code in this file is derived from the package cssutils.
# cssutils is published under the GNU Lesser General Public License version 3,
# copyright 2005 - 2013 Christof Hoeke

class MyCSSSerializer(css_parser.CSSSerializer):

    def __init__(self, prefs):
        super().__init__(prefs)
        # self.prefs=prefs

    def do_CSSStyleSheet(self, stylesheet):
        """serializes a complete CSSStyleSheet"""
        useduris = stylesheet._getUsedURIs()
        out = []
        for rule in stylesheet.cssRules:
            if self.prefs.keepUsedNamespaceRulesOnly and\
               rule.NAMESPACE_RULE == rule.type and\
               rule.namespaceURI not in useduris and (
                    rule.prefix or None not in useduris):
                continue

            cssText = rule.cssText
            if cssText:
                out.append(cssText+self.prefs.linesAfterRules)
        text = self._linenumbers(self.prefs.lineSeparator.join(out))

        # get encoding of sheet, defaults to UTF-8
        try:
            encoding = stylesheet.cssRules[0].encoding
        except (IndexError, AttributeError):
            encoding = 'UTF-8'

        # TODO: py3 return b str but tests use unicode?
        return text.encode(encoding, 'escapecss')

    def do_CSSFontFaceRule(self, rule):
        """
        serializes CSSFontFaceRule

        style
            CSSStyleDeclaration

        + CSSComments
        """
        styleText = self.do_css_CSSStyleDeclaration(rule.style)

        if styleText and rule.wellformed:
            out = cssparser.serialize.Out(self)
            out.append(self._atkeyword(rule))
            for item in rule.seq:
                # assume comments {
                out.append(item.value, item.type)
            out.append('{')
            out.append('%s' % (styleText),
                       indent=1)
            out.append('%s}' % (self.prefs.lineSeparator))
            return out.value()
        else:
            return ""

    def do_CSSPageRule(self, rule):
        """
        serializes CSSPageRule

        selectorText
            string
        style
            CSSStyleDeclaration
        cssRules
            CSSRuleList of MarginRule objects

        + CSSComments
        """
        # rules
        rules = ""
        rulesout = []
        for r in rule.cssRules:
            rtext = r.cssText
            if rtext:
                rulesout.append(rtext)
                rulesout.append(self.prefs.lineSeparator)

        rulesText = "".join(rulesout)#.strip()

        # omit semicolon only if no MarginRules
        styleText = self.do_css_CSSStyleDeclaration(rule.style,
                                                    omit=not rulesText)

        if (styleText or rulesText) and rule.wellformed:
            out = css_parser.serialize.Out(self)
            out.append(self._atkeyword(rule))
            out.append(rule.selectorText)
            out.append('{')

            if styleText:
                if not rulesText:
                    out.append('%s' % styleText, indent=1)
                    out.append('%s' % self.prefs.lineSeparator)
                else:
                    out.append(styleText, type_='styletext', indent=1, space=False)

            if rulesText:
                out.append(rulesText, indent=1)
            #?
            self._level -= 1
            out.append('}')
            self._level += 1

            return out.value()
        else:
            return ""

    def do_CSSUnknownRule(self, rule):
        if not self.prefs.formatUnknownRules:
            if rule.wellformed and self.prefs.keepUnknownAtRules:
                return rule.atkeyword + "".join(x.value for x in rule.seq)
            else:
                return ""
        else:
            return super().do_CSSUnknownRule(rule)

def nofetch(url):
    return None, None

def get_prefs():
    prefs = serialize_Preferences()
    prefs.indent = 2 * ' '
    prefs.indentClosingBrace = False
    prefs.keepEmptyRules = True
    prefs.keepComments = True
    prefs.keepUnknownAtRules = True
    prefs.omitLastSemicolon = False
    prefs.omitLeadingZero = False
    prefs.linesAfterRules = 1 * '\n'
    prefs.formatUnknownRules = True
    prefs.lineSeparator = '\n'
    prefs.keepUsedNamespaceRulesOnly = False
    prefs.lineNumbers = False
    return prefs

def reformat_css(css_string, useoneline):
    new_css_string = ""
    css_errors = ""
    css_warnings = ""
    prefs = get_prefs()
    if useoneline:
        prefs.indent = ''
        prefs.lineSeparator = ''
        prefs.listItemSpacer = ' '
        prefs.paranthesisSpacer = ''
        prefs.propertyNameSpacer = ''
        prefs.selectorCombinatorSpacer = ''
        prefs.spacer = ' '
        prefs.validOnly = False
        prefs.linesAfterRules = 1 * '\n'
    css_parser.setSerializer(MyCSSSerializer(prefs))
    aparser = css_parser.CSSParser(raiseExceptions=True, validate=False, fetcher=nofetch)
    try:
    	parsed_css = aparser.parseString(css_string)
    except Exception as E: # css_parser.xml.dom.HierarchyRequestErr as E:
        # parsing error - make no changes
        css_errors = str(E)
        new_css_string = css_string    
    else:
        # 0 means UNKNOWN_RULE, as from cssparser.css.cssrule.CSSRule
        for unknown_rule in parsed_css.cssRules.rulesOfType(0):
            line = css_string[:css_string.find(unknown_rule.atkeyword)].count('\n')+1
            css_warnings += "Unknown rule: " + unknown_rule.atkeyword + " at line: " + line + '\n'
            
        # we want unicode not a  byte string here
        new_css_string = parsed_css.cssText.decode('utf-8', errors='replace')
    	
    return (new_css_string, css_errors, css_warnings)


def main():
    argv = sys.argv
    if len(argv) < 2:
        return 0
    if not os.path.exists(argv[1]):
        return 1
    useoneline = False
    if len(argv) > 2 and argv[2] in ("True", "TRUE", "true"):
        useoneline = True
    css_string = ""
    with open(argv[1], 'rb') as f:
        css_string = f.read().decode('utf-8', errors='replace')
    new_css_string, css_errors, css_warnings = reformat_css(css_string, useoneline)
    if css_errors != "":
        print(css_errors)
        return -1
    if css_warnings != "":
        print(css_warnings)
    print(new_css_string)
    return 0


if __name__ == '__main__':
    sys.exit(main())
