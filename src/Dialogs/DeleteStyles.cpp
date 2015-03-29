/************************************************************************
**
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

#include <QtWidgets/QPushButton>
#include "Misc/SettingsStore.h"
#include "Dialogs/DeleteStyles.h"

static const QString SETTINGS_GROUP      = "delete_styles";

DeleteStyles::DeleteStyles(QHash<QString, QList<CSSInfo::CSSSelector *>> css_styles_to_delete, QWidget *parent)
    :
    QDialog(parent),
    m_CSSStylesToDelete(css_styles_to_delete)
{
    ui.setupUi(this);
    ConnectSignals();
    SetUpTable();
    ReadSettings();
    // Get list of styles
    QHashIterator<QString, QList<CSSInfo::CSSSelector *>> stylesheets(m_CSSStylesToDelete);

    while (stylesheets.hasNext()) {
        stylesheets.next();
        QString css_short_filename = stylesheets.key();
        css_short_filename = css_short_filename.right(css_short_filename.length() - css_short_filename.lastIndexOf('/') - 1);
        foreach(CSSInfo::CSSSelector * s, stylesheets.value()) {
            QList<QStandardItem *> rowItems;
            // Checkbox
            QStandardItem *checkbox_item = new QStandardItem();
            checkbox_item->setCheckable(true);
            checkbox_item->setCheckState(Qt::Checked);
            rowItems << checkbox_item;
            // Filename
            QStandardItem *file_item = new QStandardItem();
            file_item->setText(css_short_filename);
            file_item->setData(QString::number(s->line));
            rowItems << file_item;
            // Class
            QStandardItem *class_item = new QStandardItem();
            class_item->setText(s->groupText);
            rowItems << class_item;

            for (int i = 0; i < rowItems.count(); i++) {
                rowItems[i]->setEditable(false);
            }

            m_Model.appendRow(rowItems);
        }
    }
}

DeleteStyles::~DeleteStyles()
{
    WriteSettings();
}

void DeleteStyles::SetUpTable()
{
    QStringList header;
    QPushButton *delete_button = ui.buttonBox->button(QDialogButtonBox::Ok);
    delete_button->setText(tr("Delete Marked Styles"));
    header.append(tr("Delete"));
    header.append(tr("File"));
    header.append(tr("Style"));
    m_Model.setHorizontalHeaderLabels(header);
    ui.Table->setModel(&m_Model);
    // Make the header fill all the available space
    ui.Table->horizontalHeader()->setStretchLastSection(true);
    ui.Table->verticalHeader()->setVisible(false);
    ui.Table->setSortingEnabled(true);
    ui.Table->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.Table->setSelectionMode(QAbstractItemView::SingleSelection);
    ui.Table->setAlternatingRowColors(true);
}

void DeleteStyles::SaveStylesToDelete()
{
    m_CSSStylesToDelete.clear();

    for (int row = 0; row < m_Model.rowCount(); row++) {
        bool checked = m_Model.itemFromIndex(m_Model.index(row, 0))->checkState() == Qt::Checked;

        if (checked) {
            QString filename  = m_Model.item(row, 1)->text();
            int style_line = m_Model.item(row, 1)->data().toInt();
            QString style_name = m_Model.item(row, 2)->text();
            CSSInfo::CSSSelector *selector = new CSSInfo::CSSSelector();
            selector->groupText = style_name;
            selector->line = style_line;
            m_CSSStylesToDelete[filename].append(selector);
        }
    }
}

QHash<QString, QList<CSSInfo::CSSSelector *>> DeleteStyles::GetStylesToDelete()
{
    return m_CSSStylesToDelete;
}

void DeleteStyles::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    QByteArray geometry = settings.value("geometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }

    settings.endGroup();
}


void DeleteStyles::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

void DeleteStyles::DoubleClick(const QModelIndex index)
{
    QString filename = m_Model.item(index.row(), 1)->text();
    int line = m_Model.item(index.row(), 1)->data().toInt();
    emit OpenFileRequest(filename, line);
}

void DeleteStyles::ConnectSignals()
{
    connect(this, SIGNAL(accepted()), this, SLOT(SaveStylesToDelete()));
    connect(ui.Table, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(DoubleClick(const QModelIndex &)));
}
