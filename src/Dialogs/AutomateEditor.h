/****************************************************************************
**
** Copyright (C) 2021 Kevin B. Hendricks, Stratford, ON Canada
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

#ifndef AUTOMATEEDITOR_H
#define AUTOMATEEDITOR_H

#include <QString>
#include <QDialog>
#include <QModelIndex>
#include <QHash>
#include "Misc/DescriptiveInfo.h"

#include "ui_AutomateEditor.h"

class QShortcut;
class MainWindow;


class AutomateEditor : public QDialog, private Ui::AutomateEditor
{
    Q_OBJECT

public:
    AutomateEditor(const QString& automate_path, QWidget *parent = 0);
    ~AutomateEditor();

public slots:
    void updateActions();

protected slots:
    void reject();

private slots:
    void insertRow(const QString& code, const QString& tip, const QString& contents="", const QString& vtip="");
    void removeRow();
    void moveRowUp();
    void moveRowDown();
    void WriteSettings();
    void saveData();

    void selectTool();
    void selectPlugin();

 private:
    void loadToolElements();
    void loadPluginElements();
    
    void ReadSettings();

    QString GetAutomateList();
    QString SetNewAutomateList(QString& data);

    QHash<QString, DescriptiveInfo> m_ToolInfo;
    QHash<QString, QString> m_ToolCode;

    QHash<QString, DescriptiveInfo> m_PluginInfo;
    QHash<QString, QString> m_PluginCode;

    MainWindow * m_mainWindow;
    QShortcut * m_RemoveRow;
    QString m_automate_path;
    
};

#endif // AUTOMATEEDITOR_H
