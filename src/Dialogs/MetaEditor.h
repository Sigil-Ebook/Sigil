/****************************************************************************
**
** Copyright (C) 2016-2024 Kevin B. Hendricks, Stratford, ON Canada
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
#include <QMenu>
#include <QPointer>
#include <QModelIndex>
#include <QHash>
#include "Misc/DescriptiveInfo.h"

#include "ui_MetaEditor.h"

class QShortcut;
class QPoint;
class MainWindow;
class Book;
class MetaEditorItemDelegate;


class MetaEditor : public QDialog, private Ui::MetaEditor
{
    Q_OBJECT

public:
    MetaEditor(QWidget *parent = 0);
    ~MetaEditor();

    //Quick Utility Conversion from Code to Name
    const QString EName  (const QString& code); // meta elements
    const QString PName  (const QString& code); // element properties
    const QString LName  (const QString& code); // languages
    const QString RName  (const QString& code); // marc relator roles

    //Quick Utility Conversion from Name to Code
    const QString ECode  (const QString& name);
    const QString PCode  (const QString& name);
    const QString LCode  (const QString& name);
    const QString RCode  (const QString& name);

public slots:
    void updateActions();

protected slots:
    void reject();

private slots:
    void insertChild(const QString& code, const QString& tip, const QString& contents="", const QString& vtip="");
    void insertRow(const QString& code, const QString& tip, const QString& contents="", const QString& vtip="");
    void removeRow();
    void moveRowUp();
    void moveRowDown();
    void WriteSettings();
    void saveData();

    void selectElement();
    void selectProperty();

    void selectE2Element();
    void selectE2Property();
    void MakeDefaultFirstSelection();

    void OpenContextMenu(const QPoint &point);

 private:
    void loadMetadataElements();
    void loadMetadataProperties();
    void loadMetadataXProperties();
    void loadChoices();
    
    void loadE2MetadataElements();
    void loadE2MetadataProperties();
    void loadE2MetadataXProperties();
    void loadE2Choices();
    
    QStringList buildChoices(const QStringList& opts);

    QString getInput(const QString& title, const QString& prompt, const QString& initvalue);
    
    void ReadSettings();

    QString GetOPFMetadata();
    QString SetNewOPFMetadata(QString& data);

    void CreateContextMenuActions();
    void SetupContextMenu(const QPoint &point);

    QHash<QString, DescriptiveInfo> m_ElementInfo;
    QHash<QString, QString> m_ElementCode;

    QHash<QString, DescriptiveInfo> m_PropertyInfo;
    QHash<QString, QString> m_PropertyCode;

    QHash<QString, DescriptiveInfo> m_XPropertyInfo;
    QHash<QString, QString> m_XPropertyCode;

    QHash<QString, QStringList> m_Choices;
    QAction * m_AddMetadata = nullptr;
    QAction * m_AddProperty = nullptr;
    QAction * m_Remove = nullptr;
    QAction * m_MoveUp = nullptr;
    QAction * m_MoveDown = nullptr;
    
    MainWindow * m_mainWindow = nullptr;;
    QShortcut * m_RemoveRow = nullptr;;
    MetaEditorItemDelegate * m_cbDelegate = nullptr;;
    QSharedPointer<Book> m_book;
    QString m_version;
    QString m_opfdata;
    QString m_otherxml;
    QString m_metatag;
    QStringList m_idlist;
    QPointer<QMenu> m_ContextMenu;
    
};

#endif // METAEDITOR_H
