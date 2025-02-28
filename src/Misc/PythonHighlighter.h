// MIT License
// 
// Copyright (c) 2019 Edwin Christian Yllanes Cucho
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

#ifndef PYTHONHIGHLIGHTER_H
#define PYTHONHIGHLIGHTER_H

#include <QRegularExpression>
#include <QSyntaxHighlighter>


class HighlightingRule {
public:
  HighlightingRule(const QString &patternStr, int n,
                   const QTextCharFormat &matchingFormat);
  QString originalRuleStr;
  QRegularExpression pattern;
  int nth;
  QTextCharFormat format;
};

class PythonHighlighter : public QSyntaxHighlighter {
  Q_OBJECT
public:
  PythonHighlighter(QObject *parent);
  const QTextCharFormat getTextCharFormat(const QString &colorName,
                                          const QString &style = QString());
  void initializeRules();
  bool matchMultiline(const QString &text, const QRegularExpression &delimiter,
                      const int inState, const QTextCharFormat &style);
  void do_rehighlight();

protected:
  void highlightBlock(const QString &text);

private:
  QStringList keywords;
  QStringList operators;
  QStringList braces;
  QHash<QString, QTextCharFormat> basicStyles;
  QList<HighlightingRule> rules;
  QRegularExpression triSingleQuote;
  QRegularExpression triDoubleQuote;
};

#endif // PYTHONHIGHLIGHTER_H
