/************************************************************************
 **
 **  Copyright (C) 2025 Kevin B. Hendricks, Stratford Ontario Canada
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

#ifndef PYTHONFUNCTIONEDITOR_H
#define PYTHONFUNCTIONEDITOR_H

#include <QSize>
#include <QString>
#include <QStringList>
#include <QList>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialog>
#include "Widgets/SourceEditor.h"


class PythonFunctionEditor : public QDialog

{
    Q_OBJECT

public:

    PythonFunctionEditor(QMap<QString, QVariant>& func, QWidget *parent);
    ~PythonFunctionEditor();

    QSize sizeHint();

    void LoadEditor();

public slots:
    int exec();
    void reject();
    void ReloadEditor();
    void createFunction();
    void deleteFunction();
    void useFunction();
    void saveFunction();
    void loadFunctionToEdit(const QString& data);

signals:
    void UseFunctionRequest(const QString& fn);

private:
    void ReadSettings();
    void WriteSettings();
    void connectSignalsToSlots();

    QMap<QString, QVariant>& m_funcmap;
    QStringList   m_blockmap;
    QString       m_filepath;
    QLabel*       m_lbl;
    QComboBox*    m_cb;
    SourceEditor* m_editor;
    QPushButton*  m_butnew;
    QPushButton*  m_butdel;
    QPushButton*  m_butuse;
    QVBoxLayout*  m_layout;
    SourceEditor::HighlighterType m_hightype;
    bool          m_safe_to_save = true;
};
#endif
