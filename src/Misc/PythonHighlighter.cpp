// MIT License
// 
// Copyright (c) 2019 Edwin Christian Yllanes Cucho
// Edits and Changes to work with Sigil: Copyright (c) Kevin B. Hendricks
//                                       Stratford, Ontario Canada
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "Misc/Utility.h"
#include "Misc/PythonHighlighter.h"

HighlightingRule::HighlightingRule(const QString &patternStr, int n,
                                   const QTextCharFormat &matchingFormat) {
  originalRuleStr = patternStr;
  pattern = QRegularExpression(patternStr);
  nth = n;
  format = matchingFormat;
}

PythonHighlighter::PythonHighlighter(QObject *parent)
    : QSyntaxHighlighter(parent) {
  keywords = QStringList() << "and"
                           << "assert"
                           << "break"
                           << "class"
                           << "continue"
                           << "def"
                           << "del"
                           << "elif"
                           << "else"
                           << "except"
                           << "exec"
                           << "finally"
                           << "for"
                           << "from"
                           << "global"
                           << "if"
                           << "import"
                           << "in"
                           << "is"
                           << "lambda"
                           << "not"
                           << "or"
                           << "pass"
                           << "print"
                           << "raise"
                           << "return"
                           << "try"
                           << "while"
                           << "yield"
                           << "None"
                           << "True"
                           << "False";

  operators = QStringList() << "=" <<
              // Comparison
              "=="
                            << "!="
                            << "<"
                            << "<="
                            << ">"
                            << ">=" <<
              // Arithmetic
              "\\+"
                            << "-"
                            << "\\*"
                            << "/"
                            << "//"
                            << "%"
                            << "\\*\\*" <<
              // In-place
              "\\+="
                            << "-="
                            << "\\*="
                            << "/="
                            << "%=" <<
              // Bitwise
              "\\^"
                            << "\\|"
                            << "&"
                            << "~"
                            << ">>"
                            << "<<";

  braces = QStringList() << "{"
                         << "}"
                         << "\\("
                         << "\\)"
                         << "\\["
                         << "]";

  triSingleQuote.setPattern("'''");
  triDoubleQuote.setPattern("\"\"\"");

  initializeRules();
}

void PythonHighlighter::do_rehighlight()
{
    initializeRules();
    rehighlight();
}

void PythonHighlighter::initializeRules()
{
  basicStyles.clear();
  if (Utility::IsDarkMode()) {
      basicStyles.insert("keyword", getTextCharFormat("lightBlue", "bold"));
      basicStyles.insert("operator", getTextCharFormat("magenta"));
      basicStyles.insert("brace", getTextCharFormat("orangered"));
      basicStyles.insert("defclass", getTextCharFormat("lightGreen", "bold"));
      basicStyles.insert("string", getTextCharFormat("lightCyan"));
      basicStyles.insert("string2", getTextCharFormat("lightCyan"));
      basicStyles.insert("comment", getTextCharFormat("lightYellow", "italic"));
      basicStyles.insert("self", getTextCharFormat("blue", "italic"));
      basicStyles.insert("numbers", getTextCharFormat("yellow"));
  } else {
      basicStyles.insert("keyword", getTextCharFormat("blue", "bold"));
      basicStyles.insert("operator", getTextCharFormat("darkMagenta"));
      basicStyles.insert("brace", getTextCharFormat("darkRed"));
      basicStyles.insert("defclass", getTextCharFormat("green", "bold"));
      basicStyles.insert("string", getTextCharFormat("darkCyan"));
      basicStyles.insert("string2", getTextCharFormat("darkCyan"));
      basicStyles.insert("comment", getTextCharFormat("darkYellow", "italic"));
      basicStyles.insert("self", getTextCharFormat("darkBlue", "italic"));
      basicStyles.insert("numbers", getTextCharFormat("brown"));
  }
  
  for (const QString &currKeyword : keywords) {
    rules.append(HighlightingRule(QString("\\b%1\\b").arg(currKeyword), 0,
                                  basicStyles.value("keyword")));
  }
  for (const QString &currOperator : operators) {
    rules.append(HighlightingRule(QString("%1").arg(currOperator), 0,
                                  basicStyles.value("operator")));
  }
  for (const QString &currBrace : braces) {
    rules.append(HighlightingRule(QString("%1").arg(currBrace), 0,
                                  basicStyles.value("brace")));
  }
  // 'self'
  rules.append(HighlightingRule("\\bself\\b", 0, basicStyles.value("self")));

  // Double-quoted string, possibly containing escape sequences
  // FF: originally in python : r'"[^"\\]*(\\.[^"\\]*)*"'
  rules.append(HighlightingRule("\"[^\"\\\\]*(\\\\.[^\"\\\\]*)*\"", 0,
                                basicStyles.value("string")));
  // Single-quoted string, possibly containing escape sequences
  // FF: originally in python : r"'[^'\\]*(\\.[^'\\]*)*'"
  rules.append(HighlightingRule("'[^'\\\\]*(\\\\.[^'\\\\]*)*'", 0,
                                basicStyles.value("string")));

  // 'def' followed by an identifier
  // FF: originally: r'\bdef\b\s*(\w+)'
  rules.append(HighlightingRule("\\bdef\\b\\s*(\\w+)", 1,
                                basicStyles.value("defclass")));
  //  'class' followed by an identifier
  // FF: originally: r'\bclass\b\s*(\w+)'
  rules.append(HighlightingRule("\\bclass\\b\\s*(\\w+)", 1,
                                basicStyles.value("defclass")));

  // From '#' until a newline
  // FF: originally: r'#[^\\n]*'
  rules.append(HighlightingRule("#[^\\n]*", 0, basicStyles.value("comment")));

  // Numeric literals
  rules.append(HighlightingRule(
      "\\b[+-]?[0-9]+[lL]?\\b", 0,
      basicStyles.value("numbers"))); // r'\b[+-]?[0-9]+[lL]?\b'
  rules.append(HighlightingRule(
      "\\b[+-]?0[xX][0-9A-Fa-f]+[lL]?\\b", 0,
      basicStyles.value("numbers"))); // r'\b[+-]?0[xX][0-9A-Fa-f]+[lL]?\b'
  rules.append(HighlightingRule(
      "\\b[+-]?[0-9]+(?:\\.[0-9]+)?(?:[eE][+-]?[0-9]+)?\\b", 0,
      basicStyles.value(
          "numbers"))); // r'\b[+-]?[0-9]+(?:\.[0-9]+)?(?:[eE][+-]?[0-9]+)?\b'
}

void PythonHighlighter::highlightBlock(const QString &text) {
  for (const HighlightingRule &rule : rules) {
    QRegularExpressionMatchIterator matchIterator =
        rule.pattern.globalMatch(text);
    while (matchIterator.hasNext()) {
      QRegularExpressionMatch match = matchIterator.next();
      setFormat(match.capturedStart(), match.capturedLength(), rule.format);
    }
  }
  setCurrentBlockState(0);
  bool isInMultilne =
      matchMultiline(text, triSingleQuote, 1, basicStyles.value("string2"));
  if (!isInMultilne)
    isInMultilne =
        matchMultiline(text, triDoubleQuote, 2, basicStyles.value("string2"));
}

bool PythonHighlighter::matchMultiline(
    const QString &text, const QRegularExpression &delimiter, const int inState,
    const QTextCharFormat &style) {
  QRegularExpressionMatch match;
  int startIndex = 0;
  if (previousBlockState() != 1)
    startIndex = text.indexOf(delimiter);
  while (startIndex >= 0) {
    QRegularExpressionMatch match = delimiter.match(text, startIndex);
    int endIndex = match.capturedStart();
    int commentLength = 0;
    if (endIndex == -1) {
      setCurrentBlockState(1);
      commentLength = text.length() - startIndex;
    } else {
      commentLength = endIndex - startIndex + match.capturedLength();
    }
    setFormat(startIndex, commentLength, style);
    startIndex = text.indexOf(delimiter, startIndex + commentLength);
  }

  return currentBlockState() == inState;
}

const QTextCharFormat
PythonHighlighter::getTextCharFormat(const QString &colorName,
                                           const QString &style) {
  QTextCharFormat charFormat;
  QColor color(colorName);
  charFormat.setForeground(color);
  if (style.contains("bold", Qt::CaseInsensitive))
    charFormat.setFontWeight(QFont::Bold);
  if (style.contains("italic", Qt::CaseInsensitive))
    charFormat.setFontItalic(true);
  return charFormat;
}

