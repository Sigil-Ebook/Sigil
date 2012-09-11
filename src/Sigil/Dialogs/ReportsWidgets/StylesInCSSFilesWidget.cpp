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

#include <QtCore/QFile>
#include <QtCore/QHashIterator>
#include <QtGui/QFont>

#include "Misc/NumericItem.h"
#include "Misc/CSSInfo.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/CSSResource.h"
#include "Misc/SettingsStore.h"
#include "StylesInCSSFilesWidget.h"

static QString SETTINGS_GROUP = "reports_styles_in_css_files";

StylesInCSSFilesWidget::StylesInCSSFilesWidget(QList<Resource *>html_resources, QList<Resource *>css_resources, QSharedPointer< Book > book)
    :
    m_HTMLResources(html_resources),
    m_CSSResources(css_resources),
    m_Book(book),
    m_ItemModel(new QStandardItemModel)
{
    ui.setupUi(this);
    connectSignalsSlots();

    SetupTable();
    QHash< QString, QList<StylesInCSSFilesWidget::Selector *> > css_selectors = CheckHTMLFiles();
    CheckCSSFiles(css_selectors);

    for (int i = 0; i < ui.fileTree->header()->count(); i++) {
        ui.fileTree->resizeColumnToContents(i);
    }

}

void StylesInCSSFilesWidget::SetupTable()
{
    m_ItemModel->clear();

    QStringList header;

    header.append(tr("CSS File"));
    header.append(tr("Class Selector"));
    header.append(tr("Used In HTML File"));

    m_ItemModel->setHorizontalHeaderLabels(header);

    ui.fileTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.fileTree->setModel(m_ItemModel);
    ui.fileTree->header()->setSortIndicatorShown(true);

    ui.fileTree->header()->setToolTip(
        tr("<p>This is a list of the class based selectors in all CSS files and whether or not the selector was matched from a style in an HTML file.<p>") %
        tr("<p>NOTE:</p>") %
        tr("<p>Due to the complexities of CSS you must check your code manually to be certain if a style is used or not.</p>")
        );
}

QHash< QString, QList<StylesInCSSFilesWidget::Selector *> > StylesInCSSFilesWidget::CheckHTMLFiles()
{
    QHash< QString, QList<StylesInCSSFilesWidget::Selector *> > css_selectors;

    // Get classes found in all HTML files
    QHash<QString, QStringList> class_names_hash = m_Book->GetClassesInHTMLFiles();
    QHashIterator<QString, QStringList> class_name_iterator(class_names_hash);

    // Save the text for each stylesheet in the EPUB
    QList<Resource *> css_named_resources;
    QHash<QString, QString> css_names_to_text;
    foreach (Resource *resource, m_CSSResources) {
        QString filename = "../" + resource->GetRelativePathToOEBPS();
        if (!css_names_to_text.contains(filename)) {
            CSSResource *css_resource = dynamic_cast<CSSResource *>( resource );
            css_names_to_text[filename] = css_resource->GetText();
        }
    }

    // Check each file for classes to look for in inline and linked stylesheets
    foreach (Resource *html_resource, m_HTMLResources) {
        QString html_filename = html_resource->Filename();

        // Get the unique list of classes in this file
        QStringList classes_in_file = m_Book->GetClassesInHTMLFile(html_filename);    
        classes_in_file.removeDuplicates();

        // Get the text and linked stylesheets for this file
        HTMLResource *html_type_resource = dynamic_cast<HTMLResource *>( html_resource );
        QString html_text = html_type_resource->GetText();
        QStringList linked_stylesheets = m_Book->GetStylesheetsInHTMLFile(html_type_resource);

        // For each class check only the linked stylesheets
        foreach (QString class_name, classes_in_file) {
            QString found_location;
            QString selector_text;
            QString element_part = class_name.split(".").at(0);
            QString class_part = class_name.split(".").at(1);

            foreach (QString stylesheet_filename, linked_stylesheets) {
                if (css_names_to_text.contains(stylesheet_filename)) {
                    CSSInfo css_info(css_names_to_text[stylesheet_filename], true);
                    CSSInfo::CSSSelector* selector = css_info.getCSSSelectorForElementClass( element_part, class_part); 
                    if (selector && (selector->classNames.count() > 0)) {
                        found_location = stylesheet_filename;
                        // Record the matched information to use later when checking CSS file
                        StylesInCSSFilesWidget::Selector *selector_info = new StylesInCSSFilesWidget::Selector();
                        selector_text = selector->originalText;
                        selector_info->css_selector_text = selector_text;
                        selector_info->css_position = selector->position;
                        selector_info->html_filename = "../" + html_resource->GetRelativePathToOEBPS();
                        selector_info->html_filename = html_resource->Filename();
                        css_selectors[stylesheet_filename].append(selector_info);
                        break;
                    }
                }
            }
        }
    }

    return css_selectors;
}

void StylesInCSSFilesWidget::CheckCSSFiles(QHash< QString, QList<StylesInCSSFilesWidget::Selector *> > css_selectors)
{
    // Now check the CSS files to see if their classes appear in an HTML file
    foreach (Resource *resource, m_CSSResources) {
        CSSResource *css_resource = dynamic_cast<CSSResource *>( resource );
        QString text = css_resource->GetText();
        CSSInfo css_info(text, true);
        QList<CSSInfo::CSSSelector*> selectors = css_info.getClassSelectors();

        foreach (CSSInfo::CSSSelector *selector, selectors) {
            QString filename = "../" + resource->GetRelativePathToOEBPS();

            QString selector_text = selector->originalText;
            QString found_location;

            if (css_selectors.contains(filename)) {
                foreach (StylesInCSSFilesWidget::Selector *selector_info, css_selectors[filename]) {
                    if (selector_info->css_position == selector->position) {
                        found_location = selector_info->html_filename;
                        break;
                    }
                }
            }

            // Write the table entries

            QList<QStandardItem *> rowItems;

            // File name
            QStandardItem *filename_item = new QStandardItem();
            filename_item->setText(css_resource->Filename());
            rowItems << filename_item;

            // Selector
            QStandardItem *selector_text_item = new QStandardItem();
            selector_text_item->setText(selector_text);
            rowItems << selector_text_item;

            // Found in
            QStandardItem *found_in_item = new QStandardItem();
            found_in_item->setText(found_location);
            rowItems << found_in_item;

            for (int i = 0; i < rowItems.count(); i++) {
                rowItems[i]->setEditable(false);
            }

            m_ItemModel->appendRow(rowItems);
        }
    }
}


void StylesInCSSFilesWidget::FilterEditTextChangedSlot(const QString &text)
{
    const QString lowercaseText = text.toLower();

    QStandardItem *root_item = m_ItemModel->invisibleRootItem();
    QModelIndex parent_index;

    // Hide rows that don't contain the filter text
    int first_visible_row = -1;
    for (int row = 0; row < root_item->rowCount(); row++) {
        if (text.isEmpty() || root_item->child(row, 0)->text().toLower().contains(lowercaseText) ||
                              root_item->child(row, 1)->text().toLower().contains(lowercaseText) ||
                              root_item->child(row, 2)->text().toLower().contains(lowercaseText) ||
                              root_item->child(row, 3)->text().toLower().contains(lowercaseText)) {
            ui.fileTree->setRowHidden(row, parent_index, false);
            if (first_visible_row == -1) {
                first_visible_row = row;
            }
        }
        else {
            ui.fileTree->setRowHidden(row, parent_index, true);
        }
    }

    if (!text.isEmpty() && first_visible_row != -1) {
        // Select the first non-hidden row
        ui.fileTree->setCurrentIndex(root_item->child(first_visible_row, 0)->index());
    }
    else {
        // Clear current and selection, which clears preview image
        ui.fileTree->setCurrentIndex(QModelIndex());
    }
}

QString StylesInCSSFilesWidget::saveSettings()
{
    QString selected_file;

    if (ui.fileTree->selectionModel()->hasSelection()) {
        QModelIndex index = ui.fileTree->selectionModel()->selectedRows(0).first();
        if (index.row() != m_ItemModel->rowCount() - 1) {
            selected_file = m_ItemModel->itemFromIndex(index)->text();
        }
    }

    return selected_file;
}

void StylesInCSSFilesWidget::connectSignalsSlots()
{
    connect(ui.Filter,    SIGNAL(textChanged(QString)), 
            this,         SLOT(FilterEditTextChangedSlot(QString)));
}
