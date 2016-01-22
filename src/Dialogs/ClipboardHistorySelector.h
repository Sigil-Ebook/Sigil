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
#ifndef CLIPBOARDHISTORYSELECTOR_H
#define CLIPBOARDHISTORYSELECTOR_H

#include <QClipboard>
#include <QtWidgets/QDialog>

#include "Misc/SettingsStore.h"

#include "ui_ClipboardHistorySelector.h"

/**
 * The clipboard history window used for selecing entries to paste.
 */
class ClipboardHistorySelector : public QDialog
{
    Q_OBJECT

public:
    ClipboardHistorySelector(QWidget *parent);

    void LoadClipboardHistory(const QStringList &clipboardHistory);
    QStringList GetClipboardHistory() const;

public slots:
    void SaveClipboardState();
    void RestoreClipboardState();

signals:
    void PasteRequest(const QString &);

protected:
    bool eventFilter(QObject *obj, QEvent *ev);

protected slots:
    void showEvent(QShowEvent *event);
    void accept();
    void reject();

private slots:
    void ApplicationActivated();
    void ApplicationDeactivated();
    void ClipboardItemDoubleClicked(QTableWidgetItem *item);
    void ClipboardChanged();

private:
    void SetupClipboardHistoryTable();

    void ReadSettings();
    void WriteSettings();

    void ExtendUI();
    void ConnectSignalsToSlots();

    QStringList *m_ClipboardHistoryItems;
    QStringList *m_PreviousClipboardHistoryItems;

    Ui::ClipboardHistorySelector ui;
};

#endif // CLIPBOARDHISTORYSELECTOR_H
