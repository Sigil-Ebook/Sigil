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

#ifndef JSONHIGHLIGHTER_H
#define JSONHIGHLIGHTER_H

#include <QtGui/QSyntaxHighlighter>

#include "Misc/SettingsStore.h"

class JSONHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit JSONHighlighter(QObject *parent);

    void do_rehighlight();

protected:
    void highlightBlock(const QString &text);

private:
    bool IsKeyString(const QString &text, int end_quote) const;
    int HighlightNumber(const QString &text, int start);
    int HighlightLiteral(const QString &text, int start);

    SettingsStore::CodeViewAppearance m_codeViewAppearance;
};

#endif // JSONHIGHLIGHTER_H
