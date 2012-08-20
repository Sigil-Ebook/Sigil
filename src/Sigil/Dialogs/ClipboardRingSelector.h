/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
**  Copyright (C) 2012 Grant Drake
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
#ifndef CLIPBOARDRINGSELECTOR_H
#define CLIPBOARDRINGSELECTOR_H

#include <QClipboard>
#include <QtGui/QDialog>

#include "Misc/SettingsStore.h"

#include "ui_ClipboardRingSelector.h"

/**
 * The editor used to create and modify saved clip entries
 */
class ClipboardRingSelector : public QDialog
{
    Q_OBJECT

public:
    ClipboardRingSelector(QWidget *parent);

    void LoadClipboardHistory(const QStringList &clipboardHistory);
    QStringList GetClipboardHistory() const;

signals:
    void PasteFromClipboardRequest();

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

protected slots:
    void showEvent(QShowEvent *event);
    void accept();
    void reject();

private slots:
    void ClipboardItemDoubleClicked(QTableWidgetItem *item);
    void ClipboardChanged( QClipboard::Mode mode );

private:
    void SetupClipboardRingTable();

    void ReadSettings();
    void WriteSettings();

    void ExtendUI();
    void ConnectSignalsToSlots();

    QStringList *m_ClipboardRingHistory;

    Ui::ClipboardRingSelector ui;
};

#endif // CLIPBOARDRINGSELECTOR_H
