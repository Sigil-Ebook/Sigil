/************************************************************************
**
**  Copyright (C) 2026  Sigil Contributors
**
**  This file is part of Sigil.
**
**  Sigil is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
*************************************************************************/

#include <QApplication>

#include "Misc/JSONHighlighter.h"
#include "Misc/Utility.h"

enum JSONBlockState {
    JSONNormalState = 0,
    JSONStringState = 1
};

JSONHighlighter::JSONHighlighter(QObject *parent)
    : QSyntaxHighlighter(parent)
{
    SettingsStore settings;
    if (Utility::IsDarkMode()) {
        m_codeViewAppearance = settings.codeViewDarkAppearance();
    } else {
        m_codeViewAppearance = settings.codeViewAppearance();
    }
}

void JSONHighlighter::do_rehighlight()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    rehighlight();
    QApplication::restoreOverrideCursor();
}

void JSONHighlighter::highlightBlock(const QString &text)
{
    bool in_string = previousBlockState() == JSONStringState;
    int string_start = in_string ? 0 : -1;
    bool escaped = false;

    setCurrentBlockState(JSONNormalState);

    for (int i = 0; i < text.length(); ++i) {
        const QChar ch = text.at(i);

        if (in_string) {
            if (escaped) {
                escaped = false;
                continue;
            }
            if (ch == QLatin1Char('\\')) {
                escaped = true;
                continue;
            }
            if (ch == QLatin1Char('"')) {
                int length = i - string_start + 1;
                if (IsKeyString(text, i)) {
                    setFormat(string_start, length, m_codeViewAppearance.css_property_color);
                } else {
                    setFormat(string_start, length, m_codeViewAppearance.css_quote_color);
                }
                in_string = false;
                string_start = -1;
            }
            continue;
        }

        if (ch == QLatin1Char('"')) {
            in_string = true;
            string_start = i;
        } else if (ch.isDigit() || ch == QLatin1Char('-')) {
            i = HighlightNumber(text, i) - 1;
        } else if (ch.isLetter()) {
            i = HighlightLiteral(text, i) - 1;
        } else if (QStringLiteral("{}[],:").contains(ch)) {
            setFormat(i, 1, m_codeViewAppearance.css_selector_color);
        }
    }

    if (in_string) {
        setCurrentBlockState(JSONStringState);
        setFormat(string_start, text.length() - string_start, m_codeViewAppearance.css_quote_color);
    }
}

bool JSONHighlighter::IsKeyString(const QString &text, int end_quote) const
{
    int pos = end_quote + 1;
    while (pos < text.length() && text.at(pos).isSpace()) {
        ++pos;
    }
    return pos < text.length() && text.at(pos) == QLatin1Char(':');
}

int JSONHighlighter::HighlightNumber(const QString &text, int start)
{
    int pos = start;

    if (pos < text.length() && text.at(pos) == QLatin1Char('-')) {
        ++pos;
    }
    while (pos < text.length() && text.at(pos).isDigit()) {
        ++pos;
    }
    if (pos < text.length() && text.at(pos) == QLatin1Char('.')) {
        ++pos;
        while (pos < text.length() && text.at(pos).isDigit()) {
            ++pos;
        }
    }
    if (pos < text.length() && (text.at(pos) == QLatin1Char('e') || text.at(pos) == QLatin1Char('E'))) {
        ++pos;
        if (pos < text.length() && (text.at(pos) == QLatin1Char('+') || text.at(pos) == QLatin1Char('-'))) {
            ++pos;
        }
        while (pos < text.length() && text.at(pos).isDigit()) {
            ++pos;
        }
    }

    if (pos > start && (pos - start > 1 || text.at(start) != QLatin1Char('-'))) {
        setFormat(start, pos - start, m_codeViewAppearance.css_value_color);
    }
    return qMax(pos, start + 1);
}

int JSONHighlighter::HighlightLiteral(const QString &text, int start)
{
    static const QStringList literals = QStringList() << QStringLiteral("true")
                                                      << QStringLiteral("false")
                                                      << QStringLiteral("null");

    foreach(const QString &literal, literals) {
        if (text.mid(start, literal.length()) == literal) {
            int end = start + literal.length();
            if (end == text.length() || !text.at(end).isLetterOrNumber()) {
                setFormat(start, literal.length(), m_codeViewAppearance.css_value_color);
                return end;
            }
        }
    }
    return start + 1;
}
