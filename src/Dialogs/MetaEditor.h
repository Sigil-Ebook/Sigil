/****************************************************************************
**
** Copyright (C) 2016 Kevin B. Hendricks, Stratford, ON Canada
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

#ifndef METAEDITOR_H
#define METAEDITOR_H

#include <QString>
#include <QDialog>
#include <QModelIndex>
#include <QHash>
#include "Misc/DescriptiveInfo.h"
#include "Misc/Language.h"
#include "Misc/MarcRelators.h"
#include "Misc/PythonRoutines.h"

#include "ui_MetaEditor.h"

class MainWindow;
class Book;

class MetaEditor : public QDialog, private Ui::MetaEditor
{
    Q_OBJECT

public:
    MetaEditor(QWidget *parent = 0);

public slots:
    void updateActions();

protected slots:
    void reject();

private slots:
    void insertChild(QString code, QString contents="");
    void insertRow(QString code, QString contents="");
    void removeRow();
    void moveRowUp();
    void moveRowDown();
    void WriteSettings();
    void saveData();

    void selectElement();
    void selectProperty();

    void selectE2Element();
    void selectE2Property();

 private:
    void loadMetadataElements();
    void loadMetadataProperties();

    void loadE2MetadataElements();
    void loadE2MetadataProperties();

    void ReadSettings();

    QString GetOPFMetadata();
    QString SetNewOPFMetadata(QString& data);

    const QHash<QString, DescriptiveInfo> & GetElementMap();
    const QHash<QString, DescriptiveInfo> & GetPropertyMap();

    QHash<QString, DescriptiveInfo> m_ElementInfo;
    QHash<QString, QString> m_ElementCode;

    QHash<QString, DescriptiveInfo> m_PropertyInfo;
    QHash<QString, QString> m_PropertyCode;
    
    QHash<QString, DescriptiveInfo> m_E2ElementInfo;
    QHash<QString, QString> m_E2ElementCode;

    QHash<QString, DescriptiveInfo> m_E2PropertyInfo;
    QHash<QString, QString> m_E2PropertyCode;
    
    MarcRelators * m_Relator;

    MainWindow * m_mainWindow;
    QSharedPointer<Book> m_book;
    QString m_version;
    QString m_opfdata;
    MetadataPieces m_mdp;
};

#endif // METAEDITOR_H
