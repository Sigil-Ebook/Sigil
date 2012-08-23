/************************************************************************
**
**  Copyright (C) 2012  John Schember <john@nachtimwald.com>
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

#include <QString>
#include <QtGui/QColorDialog>
#include <QtGui/QListWidget>
#include <QtWebKit/QWebSettings>

#include "AppearanceWidget.h"
#include "Misc/SettingsStore.h"
#include "QtGui/QMessageBox"

AppearanceWidget::AppearanceWidget()
    :
    m_currentColor( QColor() )
{
    ui.setupUi(this);

    // Font size comboboxes
    QList<int> fontSizes;
    fontSizes << 8 << 9 << 10 << 11 << 12 << 14 << 16 << 18 << 20 << 22 << 24;
    foreach( int font_size, fontSizes )
    {
        ui.cbBookViewFontSize->addItem(QString::number(font_size));
        ui.cbCodeViewFontSize->addItem(QString::number(font_size));
    }

    readSettings();

    loadCodeViewColorsList();
    connectSignalsToSlots();
}

void AppearanceWidget::saveSettings()
{
    SettingsStore settings;

    SettingsStore::BookViewAppearance bookViewAppearance;
    bookViewAppearance.font_family_standard = ui.cbBookViewFontStandard->currentText();
    bookViewAppearance.font_family_serif = ui.cbBookViewFontSerif->currentText();
    bookViewAppearance.font_family_sans_serif = ui.cbBookViewFontSansSerif->currentText();
    bookViewAppearance.font_size = ui.cbBookViewFontSize->currentText().toInt();
    settings.setBookViewAppearance( bookViewAppearance );

    SettingsStore::CodeViewAppearance codeViewAppearance;

    codeViewAppearance.font_family = ui.cbCodeViewFont->currentText();
    codeViewAppearance.font_size = ui.cbCodeViewFontSize->currentText().toInt();

    int i = 0;
    codeViewAppearance.background_color = getListItemColor(i++);
    codeViewAppearance.foreground_color = getListItemColor(i++);
    codeViewAppearance.css_comment_color = getListItemColor(i++);
    codeViewAppearance.css_property_color = getListItemColor(i++);
    codeViewAppearance.css_quote_color = getListItemColor(i++);
    codeViewAppearance.css_selector_color = getListItemColor(i++);
    codeViewAppearance.css_value_color = getListItemColor(i++);
    codeViewAppearance.line_highlight_color = getListItemColor(i++);
    codeViewAppearance.line_number_background_color = getListItemColor(i++);
    codeViewAppearance.line_number_foreground_color = getListItemColor(i++);
    codeViewAppearance.selection_background_color = getListItemColor(i++);
    codeViewAppearance.selection_foreground_color = getListItemColor(i++);
    codeViewAppearance.spelling_underline_color = getListItemColor(i++);
    codeViewAppearance.xhtml_attribute_name_color = getListItemColor(i++);
    codeViewAppearance.xhtml_attribute_value_color = getListItemColor(i++);
    codeViewAppearance.xhtml_css_color = getListItemColor(i++);
    codeViewAppearance.xhtml_css_comment_color = getListItemColor(i++);
    codeViewAppearance.xhtml_doctype_color = getListItemColor(i++);
    codeViewAppearance.xhtml_entity_color = getListItemColor(i++);
    codeViewAppearance.xhtml_html_color = getListItemColor(i++);
    codeViewAppearance.xhtml_html_comment_color = getListItemColor(i++);

    settings.setCodeViewAppearance( codeViewAppearance );

    // BV/PV settings can be globally changed and will take effect immediately
    QWebSettings *web_settings = QWebSettings::globalSettings();
    web_settings->setFontSize(QWebSettings::DefaultFontSize, bookViewAppearance.font_size);
    web_settings->setFontFamily(QWebSettings::StandardFont,  bookViewAppearance.font_family_standard);
    web_settings->setFontFamily(QWebSettings::SerifFont, bookViewAppearance.font_family_serif);
    web_settings->setFontFamily(QWebSettings::SansSerifFont, bookViewAppearance.font_family_sans_serif);

    // CV settings require the tab to be closed/reopened. It is easiest to
    // tell the user to restart at this point, until Preferences widget has
    // some other options available.
    if ( (m_codeViewAppearance.font_family != codeViewAppearance.font_family) ||
         (m_codeViewAppearance.font_size != codeViewAppearance.font_size) ||
         (m_codeViewAppearance.background_color != codeViewAppearance.background_color) ||
         (m_codeViewAppearance.foreground_color != codeViewAppearance.foreground_color) ||
         (m_codeViewAppearance.css_comment_color != codeViewAppearance.css_comment_color) ||
         (m_codeViewAppearance.css_property_color != codeViewAppearance.css_property_color) ||
         (m_codeViewAppearance.css_quote_color != codeViewAppearance.css_quote_color) ||
         (m_codeViewAppearance.css_selector_color != codeViewAppearance.css_selector_color) ||
         (m_codeViewAppearance.css_value_color != codeViewAppearance.css_value_color) ||
         (m_codeViewAppearance.line_highlight_color != codeViewAppearance.line_highlight_color) ||
         (m_codeViewAppearance.line_number_background_color != codeViewAppearance.line_number_background_color) ||
         (m_codeViewAppearance.line_number_foreground_color != codeViewAppearance.line_number_foreground_color) ||
         (m_codeViewAppearance.selection_background_color != codeViewAppearance.selection_background_color) ||
         (m_codeViewAppearance.selection_foreground_color != codeViewAppearance.selection_foreground_color) ||
         (m_codeViewAppearance.spelling_underline_color != codeViewAppearance.spelling_underline_color) ||
         (m_codeViewAppearance.xhtml_attribute_name_color != codeViewAppearance.xhtml_attribute_name_color) ||
         (m_codeViewAppearance.xhtml_attribute_value_color != codeViewAppearance.xhtml_attribute_value_color) ||
         (m_codeViewAppearance.xhtml_css_color != codeViewAppearance.xhtml_css_color) ||
         (m_codeViewAppearance.xhtml_css_comment_color != codeViewAppearance.xhtml_css_comment_color) ||
         (m_codeViewAppearance.xhtml_doctype_color != codeViewAppearance.xhtml_doctype_color) ||
         (m_codeViewAppearance.xhtml_entity_color != codeViewAppearance.xhtml_entity_color) ||
         (m_codeViewAppearance.xhtml_html_color != codeViewAppearance.xhtml_html_color) ||
         (m_codeViewAppearance.xhtml_html_comment_color != codeViewAppearance.xhtml_html_comment_color) )
    {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning( this, tr( "Reload Required" ), tr( "You must close all tabs or reload your book to see the new Code View/CSS settings." ) );
        QApplication::setOverrideCursor(Qt::WaitCursor);
    }
}

void AppearanceWidget::readSettings()
{
    SettingsStore settings;

    // Book View font/size
    SettingsStore::BookViewAppearance bookViewAppearance;
    bookViewAppearance = settings.bookViewAppearance();
    int index = ui.cbBookViewFontStandard->findText( bookViewAppearance.font_family_standard );
    if ( index == -1 ) {
        index = ui.cbBookViewFontStandard->findText( "Arial" );
        if (index == -1) {
            index = 0;
        }
    }
    ui.cbBookViewFontStandard->setCurrentIndex( index );

    index = ui.cbBookViewFontSize->findText( QString::number(bookViewAppearance.font_size) );
    ui.cbBookViewFontSize->setCurrentIndex( index );

    index = ui.cbBookViewFontSerif->findText( bookViewAppearance.font_family_serif );
    if ( index == -1 ) {
        index = ui.cbBookViewFontSerif->findText( "Times New Roman" );
        if (index == -1) {
            index = 0;
        }
    }
    ui.cbBookViewFontSerif->setCurrentIndex( index );

    index = ui.cbBookViewFontSansSerif->findText( bookViewAppearance.font_family_sans_serif );
    if ( index == -1 ) {
        index = ui.cbBookViewFontSansSerif->findText( "Arial" );
        if (index == -1) {
            index = 0;
        }
    }
    ui.cbBookViewFontSansSerif->setCurrentIndex( index );

    // Code View appearance
    m_codeViewAppearance = settings.codeViewAppearance();
    index = ui.cbCodeViewFont->findText( m_codeViewAppearance.font_family );
    if ( index == -1 ) {
        index = ui.cbCodeViewFont->findText( "Consolas" );
        if (index == -1) {
            index = 0;
        }
    }
    ui.cbCodeViewFont->setCurrentIndex( index );
 
    index = ui.cbCodeViewFontSize->findText( QString::number(m_codeViewAppearance.font_size) );
    ui.cbCodeViewFontSize->setCurrentIndex( index );
}

void AppearanceWidget::loadCodeViewColorsList()
{
    ui.codeViewColorsList->clear();

    addColorItem("Background", m_codeViewAppearance.background_color);
    addColorItem("Foreground", m_codeViewAppearance.foreground_color);
    addColorItem("CSS Comment", m_codeViewAppearance.css_comment_color);
    addColorItem("CSS Property", m_codeViewAppearance.css_property_color);
    addColorItem("CSS Quote", m_codeViewAppearance.css_quote_color);
    addColorItem("CSS Selector", m_codeViewAppearance.css_selector_color);
    addColorItem("CSS Value", m_codeViewAppearance.css_value_color);
    addColorItem("Line Highlight", m_codeViewAppearance.line_highlight_color);
    addColorItem("Line# Background", m_codeViewAppearance.line_number_background_color);
    addColorItem("Line# Foreground", m_codeViewAppearance.line_number_foreground_color);
    addColorItem("Selection Background", m_codeViewAppearance.selection_background_color);
    addColorItem("Selection Foreground", m_codeViewAppearance.selection_foreground_color);
    addColorItem("Spelling Underline", m_codeViewAppearance.spelling_underline_color);
    addColorItem("XHTML Attribute Name", m_codeViewAppearance.xhtml_attribute_name_color);
    addColorItem("XHTML Attribute Value", m_codeViewAppearance.xhtml_attribute_value_color);
    addColorItem("XHTML CSS", m_codeViewAppearance.xhtml_css_color);
    addColorItem("XHTML CSS Comment", m_codeViewAppearance.xhtml_css_comment_color);
    addColorItem("XHTML DocType", m_codeViewAppearance.xhtml_doctype_color);
    addColorItem("XHTML Entity", m_codeViewAppearance.xhtml_entity_color);
    addColorItem("XHTML HTML", m_codeViewAppearance.xhtml_html_color);
    addColorItem("XHTML HTML Comment", m_codeViewAppearance.xhtml_html_comment_color);

    ui.codeViewColorsList->setCurrentRow(0);
    displaySelectedColor();
}

void AppearanceWidget::addColorItem(const QString &text, const QColor &color)
{
    QListWidgetItem *listItem = new QListWidgetItem(text);
    listItem->setData(Qt::UserRole, color);
    ui.codeViewColorsList->addItem(listItem); 
}

QColor AppearanceWidget::getListItemColor(const int &row)
{
    QListWidgetItem *listItem;
    if (row < 0) {
        listItem = ui.codeViewColorsList->currentItem();
    }
    else {
        listItem = ui.codeViewColorsList->item(row);
    }
    return listItem->data(Qt::UserRole).value<QColor>();
}

void AppearanceWidget::displaySelectedColor()
{
    m_currentColor = getListItemColor();

    if (!m_currentColor.isValid()) {
        // Special cases for colors we want to come from the system
        QString itemName = ui.codeViewColorsList->currentItem()->text();
        QPalette defaultPalette;
        if (itemName == QString("Background")) {
            m_currentColor = defaultPalette.base().color();
        }
        else if (itemName == QString("Foreground")) {
            m_currentColor = defaultPalette.windowText().color();
        }
        else if (itemName == QString("Selection Background")) {
            m_currentColor = defaultPalette.highlight().color();
        }
        else if (itemName == QString("Selection Foreground")) {
            m_currentColor = defaultPalette.highlightedText().color();
        }
    }
    QPalette palette = ui.swatchLabel->palette();
    palette.setColor(ui.swatchLabel->backgroundRole(), m_currentColor);
    ui.swatchLabel->setPalette(palette);
}

void AppearanceWidget::listWidgetItemChangedSlot(QListWidgetItem *current, QListWidgetItem *previous)
{
    displaySelectedColor();
}

void AppearanceWidget::customColorButtonClicked()
{
    QColorDialog *colorDlg = new QColorDialog(getListItemColor(), this);
    if (colorDlg->exec() == QDialog::Accepted) {
        ui.codeViewColorsList->currentItem()->setData(Qt::UserRole, colorDlg->selectedColor());
        displaySelectedColor();
    }
}

void AppearanceWidget::resetAllButtonClicked()
{
    SettingsStore settings;
    settings.clearAppearanceSettings();

    readSettings();

    ui.codeViewColorsList->blockSignals(true);
    loadCodeViewColorsList();
    ui.codeViewColorsList->blockSignals(false);
}

void AppearanceWidget::connectSignalsToSlots()
{
    connect(ui.codeViewColorsList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(listWidgetItemChangedSlot(QListWidgetItem*,QListWidgetItem*)));
    connect(ui.customColorButton, SIGNAL(clicked()), this, SLOT(customColorButtonClicked()));
    connect(ui.resetAllButton, SIGNAL(clicked()), this, SLOT(resetAllButtonClicked()));
}
