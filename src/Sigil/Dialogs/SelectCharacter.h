/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
**
**  This file is part of Sigil.
**
**  Sigil is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  Sigil is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Sigil.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#pragma once
#ifndef SELECTCHARACTER_H
#define SELECTCHARACTER_H

#include <QtGui/QDialog>

#include "BookManipulation/Book.h"
#include "Misc/SettingsStore.h"
#include "ResourceObjects/Resource.h"
#include "ui_SelectCharacter.h"

class QGridLayout;
class QSignalMapper;

class SelectCharacter: public QDialog
{
    Q_OBJECT

public:
    SelectCharacter(QWidget *parent = 0);
    ~SelectCharacter();

    void SetList();

    QString Selection();

public slots:
    void show();

signals:
    void SelectedCharacter(const QString &text);

private slots:
    void WriteSettings();
    void SetSelectedCharacter(const QString &text);

private:

    void AddGrid(const QStringList &characters, int width);
    void ReadSettings();
    void connectSignalsSlots();

    QString m_SelectedText;

    QSignalMapper *m_buttonMapper;
    SettingsStore::SpecialCharacterAppearance m_SpecialCharacterAppearance;

    Ui::SelectCharacter ui;
};

#endif // SELECTCHARACTER_H
