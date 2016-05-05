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
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QApplication>
#include <QtWidgets/QProgressDialog>

#include "BookManipulation/Book.h"
#include "BookManipulation/BookReports.h"
#include "BookManipulation/FolderKeeper.h"
#include "Misc/CSSInfo.h"
#include "Misc/SettingsStore.h"

QList<BookReports::StyleData *> BookReports::GetHTMLClassUsage(QSharedPointer<Book> book, bool show_progress)
{
    QList<HTMLResource *> html_resources = book->GetFolderKeeper()->GetResourceTypeList<HTMLResource>(false);
    QList<CSSResource *> css_resources = book->GetFolderKeeper()->GetResourceTypeList<CSSResource>(false);
    QList<BookReports::StyleData *> html_classes_usage;
    // Save each CSS file's text so we don't have to reload it when checking each HTML file
    QHash<QString, QString> css_text;
    foreach(CSSResource * css_resource, css_resources) {
        QString css_filename = "../" + css_resource->GetRelativePathToOEBPS();

        if (!css_text.contains(css_filename)) {
            css_text[css_filename] = css_resource->GetText();
        }
    }

    // Display progress dialog
    QProgressDialog progress(QObject::tr("Collecting classes..."), 0, 0, html_resources.count(), QApplication::activeWindow());
    int progress_value = 0;
    if (show_progress) {
        progress.setMinimumDuration(0);
        progress.setValue(progress_value);
        qApp->processEvents();
    }

    // Check each file for classes to look for in inline and linked stylesheets
    foreach(HTMLResource * html_resource, html_resources) {
        if (show_progress) {
            progress.setValue(progress_value++);
            qApp->processEvents();
        }

        QString html_filename = html_resource->Filename();
        // Get the unique list of classes in this file
        QStringList classes_in_file = book->GetClassesInHTMLFile(html_filename);
        classes_in_file.removeDuplicates();
        // Get the linked stylesheets for this file
        QStringList linked_stylesheets = book->GetStylesheetsInHTMLFile(html_resource);
        // Look at each class from the HTML file
        foreach(QString class_name, classes_in_file) {
            QString found_location;
            QString selector_text;
            QString element_part = class_name.split(".").at(0);
            QString class_part = class_name.split(".").at(1);
            // Save the details for found or not found classes
            BookReports::StyleData *class_usage = new BookReports::StyleData();
            class_usage->html_filename = html_resource->Filename();
            class_usage->html_element_name = element_part;
            class_usage->html_class_name = class_part;
            // Look in each stylesheet
            foreach(QString css_filename, linked_stylesheets) {
                if (css_text.contains(css_filename)) {
                    CSSInfo css_info(css_text[css_filename], true);
                    CSSInfo::CSSSelector *selector = css_info.getCSSSelectorForElementClass(element_part, class_part);

                    // If class matched a selector in a linked stylesheet, we're done
                    if (selector && (selector->classNames.count() > 0)) {
                        class_usage->css_filename = css_filename;
                        class_usage->css_selector_text = selector->groupText;
                        class_usage->css_selector_position = selector->position;
                        class_usage->css_selector_line = selector->line;
                        break;
                    }
                }
            }
            html_classes_usage.append(class_usage);
        }
    }
    return html_classes_usage;
}

// needed because one use of a class in html can actually match more than one selector with same specificity
QList<BookReports::StyleData *> BookReports::GetAllHTMLClassUsage(QSharedPointer<Book> book, bool show_progress)
{
    QList<HTMLResource *> html_resources = book->GetFolderKeeper()->GetResourceTypeList<HTMLResource>(false);
    QList<CSSResource *> css_resources = book->GetFolderKeeper()->GetResourceTypeList<CSSResource>(false);
    QList<BookReports::StyleData *> html_classes_usage;
    // Save each CSS file's text so we don't have to reload it when checking each HTML file
    QHash<QString, QString> css_text;
    foreach(CSSResource * css_resource, css_resources) {
        QString css_filename = "../" + css_resource->GetRelativePathToOEBPS();

        if (!css_text.contains(css_filename)) {
            css_text[css_filename] = css_resource->GetText();
        }
    }

    // Display progress dialog
    QProgressDialog progress(QObject::tr("Collecting classes..."), 0, 0, html_resources.count(), QApplication::activeWindow());
    int progress_value = 0;
    if (show_progress) {
        progress.setMinimumDuration(0);
        progress.setValue(progress_value);
        qApp->processEvents();
    }

    // Check each file for classes to look for in inline and linked stylesheets
    foreach(HTMLResource * html_resource, html_resources) {
        if (show_progress) {
            progress.setValue(progress_value++);
            qApp->processEvents();
        }

        QString html_filename = html_resource->Filename();
        // Get the unique list of classes in this file
        QStringList classes_in_file = book->GetClassesInHTMLFile(html_filename);
        classes_in_file.removeDuplicates();
        // Get the linked stylesheets for this file
        QStringList linked_stylesheets = book->GetStylesheetsInHTMLFile(html_resource);
        // Look at each class from the HTML file
        foreach(QString class_name, classes_in_file) {
            QString found_location;
            QString selector_text;
            QString element_part = class_name.split(".").at(0);
            QString class_part = class_name.split(".").at(1);
            // Look in each stylesheet
            foreach(QString css_filename, linked_stylesheets) {
                if (css_text.contains(css_filename)) {
                    CSSInfo css_info(css_text[css_filename], true);
                    QList<CSSInfo::CSSSelector *> selectors = css_info.getAllCSSSelectorsForElementClass(element_part, class_part);
                    foreach(CSSInfo::CSSSelector * selector, selectors) {
                        // If class matched a selector in a linked stylesheet, we're done
                        if (selector && (selector->classNames.count() > 0)) {
                            // Save the details for found or not found classes
                            BookReports::StyleData *class_usage = new BookReports::StyleData();
                            class_usage->html_filename = html_resource->Filename();
                            class_usage->html_element_name = element_part;
                            class_usage->html_class_name = class_part;
                            class_usage->css_filename = css_filename;
                            class_usage->css_selector_text = selector->groupText;
                            class_usage->css_selector_position = selector->position;
                            class_usage->css_selector_line = selector->line;
                            html_classes_usage.append(class_usage);
                        }
                    }
                }
            }
        }
    }
    return html_classes_usage;
}



QList<BookReports::StyleData *> BookReports::GetCSSSelectorUsage(QSharedPointer<Book> book, const QList<BookReports::StyleData *> html_classes_usage)
{
    QList<CSSResource *> css_resources = book->GetFolderKeeper()->GetResourceTypeList<CSSResource>(false);
    QList<BookReports::StyleData *> css_selectors_usage;
    // Now check the CSS files to see if their classes appear in an HTML file
    foreach(CSSResource *css_resource, css_resources) {
        QString text = css_resource->GetText();
        CSSInfo css_info(text, true);
        QList<CSSInfo::CSSSelector *> selectors = css_info.getClassSelectors();
        foreach(CSSInfo::CSSSelector * selector, selectors) {
            QString css_filename = "../" + css_resource->GetRelativePathToOEBPS();
            // Save the details for found or not found classes
            BookReports::StyleData *selector_usage = new BookReports::StyleData();
            selector_usage->css_filename = css_filename;
            selector_usage->css_selector_text = selector->groupText;
            selector_usage->css_selector_position = selector->position;
            selector_usage->css_selector_line = selector->line;
            foreach(BookReports::StyleData *html_class, html_classes_usage) {
                if (css_filename == html_class->css_filename && 
                    selector->position == html_class->css_selector_position &&
                    selector->groupText == html_class->css_selector_text) {
                    selector_usage->html_filename = html_class->html_filename;
                    break;
                }
            }
            css_selectors_usage.append(selector_usage);
        }
    }
    return css_selectors_usage;
}
