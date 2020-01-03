/************************************************************************
**
**  Copyright (C) 2019  Kevin B. Hendricks, Stratford Ontario Canada
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
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QListWidget>
#include <QtGui/QPainter>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWebEngineWidgets/QWebEngineSettings>

#include "AppearanceWidget.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"

class ColorSwatchDelegate : public QStyledItemDelegate
{
public:
    ColorSwatchDelegate(QObject *parent = 0)
        : QStyledItemDelegate(parent) {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.color(QPalette::Highlight));
            painter->setPen(option.palette.color(QPalette::HighlightedText));
        }

        QString text = index.data(Qt::DisplayRole).toString();
        QRect r = option.rect.adjusted(option.rect.height() + 4, 0, -4 - option.rect.height(), 0);
        painter->drawText(r, Qt::AlignVCenter | Qt::AlignLeft, text, &r);
        QColor swatch_color = index.data(Qt::UserRole).value<QColor>();

        if (!swatch_color.isValid()) {
            // Special cases for colors we want to come from the system
            QPalette defaultPalette;

            if (text == QString(tr("Background"))) {
                swatch_color = defaultPalette.base().color();
            } else if (text == QString(tr("Foreground"))) {
                swatch_color = defaultPalette.windowText().color();
            } else if (text == QString(tr("Selection Background"))) {
                swatch_color = defaultPalette.highlight().color();
            } else if (text == QString(tr("Selection Foreground"))) {
                swatch_color = defaultPalette.highlightedText().color();
            }
        }

        r = option.rect.adjusted(1, 1, option.rect.height() - option.rect.width(), -2);
        painter->setPen(QPalette().text().color());
        painter->setBrush(swatch_color);
        painter->drawRect(r);
    }
};

AppearanceWidget::AppearanceWidget()
    :
    m_currentColor(QColor())
{
    ui.setupUi(this);
    // Custom delegate for painting the color swatches
    ui.codeViewColorsList->setItemDelegate(new ColorSwatchDelegate(ui.codeViewColorsList));
    m_codeViewAppearance = readSettings();
    loadCodeViewColorsList(m_codeViewAppearance);
    connectSignalsToSlots();
}

PreferencesWidget::ResultAction AppearanceWidget::saveSettings()
{
    SettingsStore settings;
    settings.setAppearancePrefsTabIndex(ui.tabAppearance->currentIndex());
    settings.setShowFullPathOn(ui.ShowFullPath->isChecked() ? 1 : 0);
    SettingsStore::PreviewAppearance PVAppearance;
    PVAppearance.font_family_standard     = ui.cbPreviewFontStandard->currentText();
    PVAppearance.font_family_serif        = ui.cbPreviewFontSerif->currentText();
    PVAppearance.font_family_sans_serif   = ui.cbPreviewFontSansSerif->currentText();
    PVAppearance.font_size                = ui.previewFontSizeSpin->value();
    settings.setPreviewAppearance(PVAppearance);
    SettingsStore::CodeViewAppearance codeViewAppearance;
    codeViewAppearance.font_family = ui.cbCodeViewFont->currentText();
    codeViewAppearance.font_size = ui.codeViewFontSizeSpin->value();
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
    settings.setCodeViewAppearance(codeViewAppearance);
    SettingsStore::SpecialCharacterAppearance specialCharacterAppearance;
    specialCharacterAppearance.font_family = ui.cbSpecialCharacterFont->currentText();
    specialCharacterAppearance.font_size   = ui.specialCharacterFontSizeSpin->value();
    settings.setSpecialCharacterAppearance(specialCharacterAppearance);
    settings.setMainMenuIconSize(double(ui.iconSizeSlider->value())/10);
    // PV settings can be globally changed and will take effect immediately
    QWebEngineSettings *web_settings = QWebEngineSettings::defaultSettings();
    web_settings->setFontSize(QWebEngineSettings::DefaultFontSize,   PVAppearance.font_size);
    web_settings->setFontFamily(QWebEngineSettings::StandardFont,    PVAppearance.font_family_standard);
    web_settings->setFontFamily(QWebEngineSettings::SerifFont,       PVAppearance.font_family_serif);
    web_settings->setFontFamily(QWebEngineSettings::SansSerifFont,   PVAppearance.font_family_sans_serif);

    // CV settings require the tab to be closed/reopened. It is easiest to tell the user
    // to reopen tabs or reload, perhaps in future the Preferences widget may have a signal
    // to the MainWindow requesting a reload of all open tabs.
    if ((m_codeViewAppearance.font_family                  != codeViewAppearance.font_family) ||
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
        (m_codeViewAppearance.xhtml_html_comment_color     != codeViewAppearance.xhtml_html_comment_color)) {
        return PreferencesWidget::ResultAction_ReloadTabs;
    }
    if (m_ShowFullPathOn != (ui.ShowFullPath->isChecked() ? 1 : 0)) {
        return PreferencesWidget::ResultAction_RefreshBookBrowser;
    }
    return PreferencesWidget::ResultAction_None;
}

SettingsStore::CodeViewAppearance AppearanceWidget::readSettings()
{
    SettingsStore settings;
    ui.tabAppearance->setCurrentIndex(settings.appearancePrefsTabIndex());
    m_ShowFullPathOn = settings.showFullPathOn();
    ui.ShowFullPath->setChecked(settings.showFullPathOn());
    SettingsStore::PreviewAppearance PVAppearance = settings.previewAppearance();
    SettingsStore::CodeViewAppearance codeViewAppearance = settings.codeViewAppearance();
    SettingsStore::SpecialCharacterAppearance specialCharacterAppearance = settings.specialCharacterAppearance();
    loadComboValueOrDefault(ui.cbPreviewFontStandard,  PVAppearance.font_family_standard,    "Arial");
    loadComboValueOrDefault(ui.cbPreviewFontSerif,     PVAppearance.font_family_serif,       "Times New Roman");
    loadComboValueOrDefault(ui.cbPreviewFontSansSerif, PVAppearance.font_family_sans_serif,  "Arial");
    loadComboValueOrDefault(ui.cbCodeViewFont,          codeViewAppearance.font_family,             "Courier New");
    loadComboValueOrDefault(ui.cbSpecialCharacterFont,  specialCharacterAppearance.font_family,     "Helvetica");
    ui.previewFontSizeSpin->setValue(PVAppearance.font_size);
    ui.codeViewFontSizeSpin->setValue(codeViewAppearance.font_size);
    ui.specialCharacterFontSizeSpin->setValue(specialCharacterAppearance.font_size);
    codeViewAppearance.font_family = ui.cbCodeViewFont->currentText();
    ui.iconSizeSlider->setValue(int(settings.mainMenuIconSize()*10));
    return codeViewAppearance;
}

void AppearanceWidget::loadComboValueOrDefault(QFontComboBox *fontComboBox, const QString &value, const QString &defaultValue)
{
    int index = fontComboBox->findText(value);

    if (index == -1) {
        index = fontComboBox->findText(defaultValue);

        if (index == -1) {
            index = 0;
        }
    }

    fontComboBox->setCurrentIndex(index);
}

void AppearanceWidget::loadCodeViewColorsList(SettingsStore::CodeViewAppearance codeViewAppearance)
{
    ui.codeViewColorsList->clear();
    addColorItem(tr("Background"),            codeViewAppearance.background_color);
    addColorItem(tr("Foreground"),            codeViewAppearance.foreground_color);
    addColorItem(tr("CSS Comment"),           codeViewAppearance.css_comment_color);
    addColorItem(tr("CSS Property"),          codeViewAppearance.css_property_color);
    addColorItem(tr("CSS Quote"),             codeViewAppearance.css_quote_color);
    addColorItem(tr("CSS Selector"),          codeViewAppearance.css_selector_color);
    addColorItem(tr("CSS Value"),             codeViewAppearance.css_value_color);
    addColorItem(tr("Line Highlight"),        codeViewAppearance.line_highlight_color);
    addColorItem(tr("Line# Background"),      codeViewAppearance.line_number_background_color);
    addColorItem(tr("Line# Foreground"),      codeViewAppearance.line_number_foreground_color);
    addColorItem(tr("Selection Background"),  codeViewAppearance.selection_background_color);
    addColorItem(tr("Selection Foreground"),  codeViewAppearance.selection_foreground_color);
    addColorItem(tr("Spelling Underline"),    codeViewAppearance.spelling_underline_color);
    addColorItem(tr("XHTML Attribute Name"),  codeViewAppearance.xhtml_attribute_name_color);
    addColorItem(tr("XHTML Attribute Value"), codeViewAppearance.xhtml_attribute_value_color);
    addColorItem(tr("XHTML CSS"),             codeViewAppearance.xhtml_css_color);
    addColorItem(tr("XHTML CSS Comment"),     codeViewAppearance.xhtml_css_comment_color);
    addColorItem(tr("XHTML DocType"),         codeViewAppearance.xhtml_doctype_color);
    addColorItem(tr("XHTML Entity"),          codeViewAppearance.xhtml_entity_color);
    addColorItem(tr("XHTML HTML Tag"),        codeViewAppearance.xhtml_html_color);
    addColorItem(tr("XHTML HTML Comment"),    codeViewAppearance.xhtml_html_comment_color);
    ui.codeViewColorsList->setCurrentRow(0);
}

void AppearanceWidget::addColorItem(const QString &text, const QColor &color)
{
    QListWidgetItem *listItem = new QListWidgetItem(text, ui.codeViewColorsList);
    listItem->setData(Qt::UserRole, color);
    ui.codeViewColorsList->addItem(listItem);
}

QColor AppearanceWidget::getListItemColor(const int &row)
{
    QListWidgetItem *listItem;

    if (row < 0) {
        listItem = ui.codeViewColorsList->currentItem();
    } else {
        listItem = ui.codeViewColorsList->item(row);
    }

    return listItem->data(Qt::UserRole).value<QColor>();
}

void AppearanceWidget::customColorButtonClicked()
{
    QColorDialog colorDlg(getListItemColor(), this);

    if (colorDlg.exec() == QDialog::Accepted) {
        ui.codeViewColorsList->currentItem()->setData(Qt::UserRole, colorDlg.selectedColor());
    }
}

void AppearanceWidget::resetAllButtonClicked()
{
    SettingsStore settings;
    settings.clearAppearanceSettings();
    // Read and apply the settings without changing our m_codeViewAppearance
    // instance holding the last persisted values.
    SettingsStore::CodeViewAppearance codeViewAppearance = readSettings();
    ui.codeViewColorsList->blockSignals(true);
    loadCodeViewColorsList(codeViewAppearance);
    ui.codeViewColorsList->blockSignals(false);
}

void AppearanceWidget::newSliderValue(int value) {
    SettingsStore settings;
    settings.setMainMenuIconSize(double(ui.iconSizeSlider->value())/10);

    QWidget *mainWindow_w = Utility::GetMainWindow();
    MainWindow *mainWindow = qobject_cast<MainWindow *>(mainWindow_w);
    mainWindow->sizeMenuIcons();
}

void AppearanceWidget::connectSignalsToSlots()
{
    connect(ui.customColorButton, SIGNAL(clicked()), this, SLOT(customColorButtonClicked()));
    connect(ui.resetAllButton,    SIGNAL(clicked()), this, SLOT(resetAllButtonClicked()));
    connect(ui.iconSizeSlider,    SIGNAL(valueChanged(int)), this, SLOT(newSliderValue(int)));
}
