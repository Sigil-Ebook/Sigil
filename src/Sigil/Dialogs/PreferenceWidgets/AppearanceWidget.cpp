/************************************************************************
**
**  Copyright (C) 2012  John Schember <john@nachtimwald.com>
**  Copyright (C) 2012  Grant Drake
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
#include <QtGui/QPainter>
#include <QtGui/QStyledItemDelegate>
#include <QtWebKit/QWebSettings>

#include "AppearanceWidget.h"
#include "Misc/SettingsStore.h"
#include "QtGui/QMessageBox"

class ColorSwatchDelegate : public QStyledItemDelegate  
{
public:
    ColorSwatchDelegate( QObject *parent = 0 ) 
        : QStyledItemDelegate (parent) 
    {
    }

    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const 
    {
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.color(QPalette::Highlight));
            painter->setPen(option.palette.color(QPalette::HighlightedText));
        }
        
        QString text = index.data(Qt::DisplayRole).toString();
        QRect r = option.rect.adjusted(20, 0, 0, 0);
        painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignVCenter|Qt::AlignLeft, text, &r);

        QColor swatch_color = index.data(Qt::UserRole).value<QColor>();
        if (!swatch_color.isValid()) {
            // Special cases for colors we want to come from the system
            QPalette defaultPalette;
            if (text == QString( tr( "Background" ) )) {
                swatch_color = defaultPalette.base().color();
            }
            else if (text == QString( tr( "Foreground" ) )) {
                swatch_color = defaultPalette.windowText().color();
            }
            else if (text == QString( tr( "Selection Background" ) )) {
                swatch_color = defaultPalette.highlight().color();
            }
            else if (text == QString( tr( "Selection Foreground" ) )) {
                swatch_color = defaultPalette.highlightedText().color();
            }
        }

        r = option.rect.adjusted(1, 1, option.rect.height() - option.rect.width(), -2);
        painter->setPen(Qt::black);
        painter->setBrush(swatch_color);
        painter->drawRect(r);
    }
};

AppearanceWidget::AppearanceWidget()
    :
    m_currentColor( QColor() )
{
    ui.setupUi(this);

    // Font size comboboxes
    QList<int> fontSizes;
    fontSizes << 6 << 7 << 8 << 9 << 10 << 11 << 12 << 13 << 14 << 15 << 16 << 
                17 << 18 << 19 << 20 << 22 << 24 << 28 << 32 << 36 << 40;
    foreach( int font_size, fontSizes )
    {
        ui.cbBookViewFontSize->addItem(QString::number(font_size));
        ui.cbCodeViewFontSize->addItem(QString::number(font_size));
    }
    // Custom delegate for painting the color swatches
    ui.codeViewColorsList->setItemDelegate(new ColorSwatchDelegate(ui.codeViewColorsList));

    readSettings();

    loadCodeViewColorsList();
    connectSignalsToSlots();
}

void AppearanceWidget::saveSettings()
{
    SettingsStore settings;

    SettingsStore::BookViewAppearance bookViewAppearance;
    bookViewAppearance.font_family_standard     = ui.cbBookViewFontStandard->currentText();
    bookViewAppearance.font_family_serif        = ui.cbBookViewFontSerif->currentText();
    bookViewAppearance.font_family_sans_serif   = ui.cbBookViewFontSansSerif->currentText();
    bookViewAppearance.font_size                = ui.cbBookViewFontSize->currentText().toInt();
    settings.setBookViewAppearance( bookViewAppearance );

    SettingsStore::CodeViewAppearance codeViewAppearance;

    codeViewAppearance.font_family = ui.cbCodeViewFont->currentText();
    codeViewAppearance.font_size = ui.cbCodeViewFontSize->currentText().toInt();

    int i = 0;
    codeViewAppearance.background_color             = getListItemColor(i++);
    codeViewAppearance.foreground_color             = getListItemColor(i++);
    codeViewAppearance.css_comment_color            = getListItemColor(i++);
    codeViewAppearance.css_property_color           = getListItemColor(i++);
    codeViewAppearance.css_quote_color              = getListItemColor(i++);
    codeViewAppearance.css_selector_color           = getListItemColor(i++);
    codeViewAppearance.css_value_color              = getListItemColor(i++);
    codeViewAppearance.line_highlight_color         = getListItemColor(i++);
    codeViewAppearance.line_number_background_color = getListItemColor(i++);
    codeViewAppearance.line_number_foreground_color = getListItemColor(i++);
    codeViewAppearance.selection_background_color   = getListItemColor(i++);
    codeViewAppearance.selection_foreground_color   = getListItemColor(i++);
    codeViewAppearance.spelling_underline_color     = getListItemColor(i++);
    codeViewAppearance.xhtml_attribute_name_color   = getListItemColor(i++);
    codeViewAppearance.xhtml_attribute_value_color  = getListItemColor(i++);
    codeViewAppearance.xhtml_css_color              = getListItemColor(i++);
    codeViewAppearance.xhtml_css_comment_color      = getListItemColor(i++);
    codeViewAppearance.xhtml_doctype_color          = getListItemColor(i++);
    codeViewAppearance.xhtml_entity_color           = getListItemColor(i++);
    codeViewAppearance.xhtml_html_color             = getListItemColor(i++);
    codeViewAppearance.xhtml_html_comment_color     = getListItemColor(i++);

    settings.setCodeViewAppearance( codeViewAppearance );

    // BV/PV settings can be globally changed and will take effect immediately
    QWebSettings *web_settings = QWebSettings::globalSettings();
    web_settings->setFontSize(   QWebSettings::DefaultFontSize, bookViewAppearance.font_size );
    web_settings->setFontFamily( QWebSettings::StandardFont,    bookViewAppearance.font_family_standard );
    web_settings->setFontFamily( QWebSettings::SerifFont,       bookViewAppearance.font_family_serif );
    web_settings->setFontFamily( QWebSettings::SansSerifFont,   bookViewAppearance.font_family_sans_serif );

    // CV settings require the tab to be closed/reopened. It is easiest to tell the user
    // to reopen tabs or reload, perhaps in future the Preferences widget may have a signal
    // to the MainWindow requesting a reload of all open tabs.
    if ( (m_codeViewAppearance.font_family                  != codeViewAppearance.font_family) ||
         (m_codeViewAppearance.font_size                    != codeViewAppearance.font_size) ||
         (m_codeViewAppearance.background_color             != codeViewAppearance.background_color) ||
         (m_codeViewAppearance.foreground_color             != codeViewAppearance.foreground_color) ||
         (m_codeViewAppearance.css_comment_color            != codeViewAppearance.css_comment_color) ||
         (m_codeViewAppearance.css_property_color           != codeViewAppearance.css_property_color) ||
         (m_codeViewAppearance.css_quote_color              != codeViewAppearance.css_quote_color) ||
         (m_codeViewAppearance.css_selector_color           != codeViewAppearance.css_selector_color) ||
         (m_codeViewAppearance.css_value_color              != codeViewAppearance.css_value_color) ||
         (m_codeViewAppearance.line_highlight_color         != codeViewAppearance.line_highlight_color) ||
         (m_codeViewAppearance.line_number_background_color != codeViewAppearance.line_number_background_color) ||
         (m_codeViewAppearance.line_number_foreground_color != codeViewAppearance.line_number_foreground_color) ||
         (m_codeViewAppearance.selection_background_color   != codeViewAppearance.selection_background_color) ||
         (m_codeViewAppearance.selection_foreground_color   != codeViewAppearance.selection_foreground_color) ||
         (m_codeViewAppearance.spelling_underline_color     != codeViewAppearance.spelling_underline_color) ||
         (m_codeViewAppearance.xhtml_attribute_name_color   != codeViewAppearance.xhtml_attribute_name_color) ||
         (m_codeViewAppearance.xhtml_attribute_value_color  != codeViewAppearance.xhtml_attribute_value_color) ||
         (m_codeViewAppearance.xhtml_css_color              != codeViewAppearance.xhtml_css_color) ||
         (m_codeViewAppearance.xhtml_css_comment_color      != codeViewAppearance.xhtml_css_comment_color) ||
         (m_codeViewAppearance.xhtml_doctype_color          != codeViewAppearance.xhtml_doctype_color) ||
         (m_codeViewAppearance.xhtml_entity_color           != codeViewAppearance.xhtml_entity_color) ||
         (m_codeViewAppearance.xhtml_html_color             != codeViewAppearance.xhtml_html_color) ||
         (m_codeViewAppearance.xhtml_html_comment_color     != codeViewAppearance.xhtml_html_comment_color) )
    {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning( this, tr( "Sigil" ), tr( "Changes will take effect for any new tabs you open." ) );
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

    addColorItem( tr( "Background" ),            m_codeViewAppearance.background_color );
    addColorItem( tr( "Foreground" ),            m_codeViewAppearance.foreground_color );
    addColorItem( tr( "CSS Comment" ),           m_codeViewAppearance.css_comment_color );
    addColorItem( tr( "CSS Property" ),          m_codeViewAppearance.css_property_color );
    addColorItem( tr( "CSS Quote" ),             m_codeViewAppearance.css_quote_color );
    addColorItem( tr( "CSS Selector" ),          m_codeViewAppearance.css_selector_color );
    addColorItem( tr( "CSS Value" ),             m_codeViewAppearance.css_value_color );
    addColorItem( tr( "Line Highlight" ),        m_codeViewAppearance.line_highlight_color );
    addColorItem( tr( "Line# Background" ),      m_codeViewAppearance.line_number_background_color );
    addColorItem( tr( "Line# Foreground" ),      m_codeViewAppearance.line_number_foreground_color );
    addColorItem( tr( "Selection Background" ),  m_codeViewAppearance.selection_background_color );
    addColorItem( tr( "Selection Foreground" ),  m_codeViewAppearance.selection_foreground_color );
    addColorItem( tr( "Spelling Underline" ),    m_codeViewAppearance.spelling_underline_color );
    addColorItem( tr( "XHTML Attribute Name" ),  m_codeViewAppearance.xhtml_attribute_name_color );
    addColorItem( tr( "XHTML Attribute Value" ), m_codeViewAppearance.xhtml_attribute_value_color );
    addColorItem( tr( "XHTML CSS" ),             m_codeViewAppearance.xhtml_css_color );
    addColorItem( tr( "XHTML CSS Comment" ),     m_codeViewAppearance.xhtml_css_comment_color );
    addColorItem( tr( "XHTML DocType" ),         m_codeViewAppearance.xhtml_doctype_color );
    addColorItem( tr( "XHTML Entity" ),          m_codeViewAppearance.xhtml_entity_color );
    addColorItem( tr( "XHTML HTML" ),            m_codeViewAppearance.xhtml_html_color );
    addColorItem( tr( "XHTML HTML Comment" ),    m_codeViewAppearance.xhtml_html_comment_color );

    ui.codeViewColorsList->setCurrentRow(0);
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

void AppearanceWidget::customColorButtonClicked()
{
    QColorDialog *colorDlg = new QColorDialog(getListItemColor(), this);
    if (colorDlg->exec() == QDialog::Accepted) {
        ui.codeViewColorsList->currentItem()->setData(Qt::UserRole, colorDlg->selectedColor());
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
    connect(ui.customColorButton, SIGNAL(clicked()), this, SLOT(customColorButtonClicked()));
    connect(ui.resetAllButton, SIGNAL(clicked()), this, SLOT(resetAllButtonClicked()));
}
