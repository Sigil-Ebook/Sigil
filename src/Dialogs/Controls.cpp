/************************************************************************
**
**  Copyright (C) 2021 Kevin B. Hendricks, Stratford, Ontario, Canada
**  Copyright (C) 2021 Doug Massay
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

#include <QWidget>
#include <QDialog>
#include <QEvent>
#include <QCloseEvent>
#include <QShowEvent>

#include "Misc/SettingsStore.h"
#include "Dialogs/Controls.h"


static const QString SETTINGS_GROUP = "search_controls";

Controls::Controls(QWidget* parent)
    : QDialog(parent),
      m_RegexOptionDotAll(false),
      m_RegexOptionMinimalMatch(false),
      m_RegexOptionAutoTokenise(false),
      m_OptionWrap(false),
      m_ClearAll(false)
{
    ui.setupUi(this);
    ConnectSignalsToSlots();
    ExtendUI();
    ReadSettings();
}


Controls::~Controls()
{
}


void Controls::showEvent(QShowEvent *e)
{
    ReadSettings();
    QDialog::showEvent(e);
}


void Controls::closeEvent(QCloseEvent *e)
{
    WriteSettings();
    QDialog::closeEvent(e);
}


void Controls::show()
{
    QDialog::show();
}


void Controls::hide()
{
    QDialog::hide();
}


void Controls::UpdateSearchControls(const QString &text)
{
    if (text.isEmpty()) DoClearAll();

#if 0
    //default to normal down no wrap current file
    if (text.isEmpty()) {
        SetSearchMode("NL");
        SetLookWhere("CF");
        SetSearchDirection("DN");
        SetOptionWrap(true);
        SetRegexOptionDotAll(false);
        SetRegexOptionMinimalMatch(false);
        SetRegexOptionAutoTokenise(false);
        return;
    }
#endif

    // Search Mode
    if (text.contains("NL")) {
        SetSearchMode("NL");
    } else if (text.contains("RX")) {
        SetSearchMode("RX");
    } else if (text.contains("CS")) {
        SetSearchMode("CS");
    }

    // Search LookWhere
    if (text.contains("CF")) {
        SetLookWhere("CF");
    } else if (text.contains("AH")) {
        SetLookWhere("AH");
    } else if (text.contains("SH")) {
        SetLookWhere("SH");
    } else if (text.contains("TH")) {
        SetLookWhere("TH");
    } else if (text.contains("AC")) {
        SetLookWhere("AC");
    } else if (text.contains("SC")) {
        SetLookWhere("SC");
    } else if (text.contains("TC")) {
        SetLookWhere("TC");
    } else if (text.contains("OP")) {
        SetLookWhere("OP");
    } else if (text.contains("NX")) {
        SetLookWhere("NX");
    }

    // Search Direction
    if (text.contains("UP")) {
        SetSearchDirection("UP");
    } else if (text.contains("DN")) {
        SetSearchDirection("DN");
    }

    // Search Flags
    SetOptionWrap(text.contains("WR"));
    SetRegexOptionDotAll(text.contains("DA"));
    SetRegexOptionMinimalMatch(text.contains("MM"));
    SetRegexOptionAutoTokenise(text.contains("AT"));
}


QString Controls::GetControlsCode()
{
    // if any cb left with placeholder clear entire entry
    // to prevent impossible search controls from being generated
    if ((ui.cbSearchMode->currentIndex() == 0) ||
        (ui.cbSearchDirection->currentIndex() == 0) ||
        (ui.cbLookWhere->currentIndex() == 0)) {
        return "";
    }
    QStringList codes;
    codes.append(GetSearchMode());
    if (m_RegexOptionDotAll) codes.append("DA");
    if (m_RegexOptionMinimalMatch) codes.append("MM");
    if (m_RegexOptionAutoTokenise) codes.append("AT");
    if (m_OptionWrap) codes.append("WR");
    codes.append(GetSearchDirection());
    codes.append(GetLookWhere());
    return codes.join(" ");
}


QString Controls::GetSearchMode()
{
    return ui.cbSearchMode->itemData(ui.cbSearchMode->currentIndex()).toString();
}

QString Controls::GetLookWhere()
{
    return ui.cbLookWhere->itemData(ui.cbLookWhere->currentIndex()).toString();
}

QString Controls::GetSearchDirection()
{
    return ui.cbSearchDirection->itemData(ui.cbSearchDirection->currentIndex()).toString();
}


void Controls::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }
}

void Controls::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("geometry", saveGeometry());
}


void Controls::SetSearchMode(QString code)
{
    ui.cbSearchMode->setCurrentIndex(0);
    for (int i = 0; i < ui.cbSearchMode->count(); ++i) {
        if (ui.cbSearchMode->itemData(i).toString() == code) {
            ui.cbSearchMode->setCurrentIndex(i);
            break;
        }
    }
}


void Controls::SetLookWhere(QString code)
{
    ui.cbLookWhere->setCurrentIndex(0);
    for (int i = 0; i < ui.cbLookWhere->count(); ++i) {
        if (ui.cbLookWhere->itemData(i).toString()  == code) {
            ui.cbLookWhere->setCurrentIndex(i);
            break;
        }
    }
}


void Controls::SetSearchDirection(QString code)
{
    ui.cbSearchDirection->setCurrentIndex(0);

    for (int i = 0; i < ui.cbSearchDirection->count(); ++i) {
        if (ui.cbSearchDirection->itemData(i).toString() == code) {
            ui.cbSearchDirection->setCurrentIndex(i);
            break;
        }
    }
}


void Controls::SetRegexOptionDotAll(bool new_state)
{
    m_RegexOptionDotAll = new_state;
    ui.chkRegexOptionDotAll->setChecked(new_state);
}


void Controls::SetRegexOptionMinimalMatch(bool new_state)
{
    m_RegexOptionMinimalMatch = new_state;
    ui.chkRegexOptionMinimalMatch->setChecked(new_state);
}

void Controls::SetRegexOptionAutoTokenise(bool new_state)
{
    m_RegexOptionAutoTokenise = new_state;
    ui.chkRegexOptionAutoTokenise->setChecked(new_state);
}

void Controls::SetOptionWrap(bool new_state)
{
    m_OptionWrap = new_state;
    ui.chkOptionWrap->setChecked(new_state);
}

void Controls::DoClearAll()
{
    SetOptionWrap(false);
    SetRegexOptionAutoTokenise(false);
    SetRegexOptionMinimalMatch(false);
    SetRegexOptionDotAll(false);
    ui.cbSearchMode->setCurrentIndex(0);
    ui.cbLookWhere->setCurrentIndex(0);
    ui.cbSearchDirection->setCurrentIndex(0);
}

// The UI is setup based on the capabilities.
void Controls::ExtendUI()
{
    ui.btClearAll->setDefault(false);
    ui.btClearAll->setAutoDefault(false);

    SetOptionWrap(false);
    SetRegexOptionAutoTokenise(false);
    SetRegexOptionMinimalMatch(false);
    SetRegexOptionDotAll(false);
    
    // Clear these because we want to add their items based on the
    // capabilities.
    ui.cbSearchMode->clear();
    ui.cbLookWhere->clear();
    ui.cbSearchDirection->clear();

    QString mode_tooltip = "<p>" + tr("What to search for") + ":</p><dl>";
    ui.cbSearchMode->addItem("-- " + tr("Select Mode") + " --", "");
    ui.cbSearchMode->addItem(tr("Normal"), "NL");
    mode_tooltip += "<dt><b>" + tr("Normal") + "</b><dd>" + tr("Case in-sensitive search of exactly what you type.") + "</dd>";

    ui.cbSearchMode->addItem(tr("Case Sensitive"), "CS");
    mode_tooltip += "<dt><b>" + tr("Case Sensitive") + "</b><dd>" + tr("Case sensitive search of exactly what you type.") + "</dd>";

    ui.cbSearchMode->addItem(tr("Regex"), "RX");
    mode_tooltip += "<dt><b>" + tr("Regex") + "</b><dd>" + tr("Search for a pattern using Regular Expression syntax.") + "</dd>";

    ui.cbSearchMode->setToolTip(mode_tooltip);
    ui.cbSearchMode->setCurrentIndex(0);

    QString look_tooltip = "<p>" + tr("Where to search") + ":</p><dl>";

    ui.cbLookWhere->addItem("-- " + tr("Select Target") + " --", "");
    ui.cbLookWhere->addItem(tr("Current File"), "CF");
    look_tooltip += "<dt><b>" + tr("Current File") + "</b><dd>" + tr("Restrict the find or replace to the opened file.  Hold the Ctrl key down while clicking any search buttons to temporarily restrict the search to the Current File.") + "</dd>";

    ui.cbLookWhere->addItem(tr("All HTML Files"), "AH");
    look_tooltip += "<dt><b>" + tr("All HTML Files") + "</b><dd>" + tr("Find or replace in all HTML files in Code View.") + "</dd>";

    ui.cbLookWhere->addItem(tr("Selected HTML Files"), "SH");
    look_tooltip += "<dt><b>" + tr("Selected HTML Files") + "</b><dd>" + tr("Restrict the find or replace to the HTML files selected in the Book Browser in Code View.") + "</dd>";

    ui.cbLookWhere->addItem(tr("Tabbed HTML Files"), "TB");
    look_tooltip += "<dt><b>" + tr("Current File") + "</b><dd>" + tr("Restrict the find or replace to the HTML files open in Tabs.") + "</dd>";

    ui.cbLookWhere->addItem(tr("All CSS Files"), "AC");
    look_tooltip += "<dt><b>" + tr("All CSS Files") + "</b><dd>" + tr("Find or replace in all CSS files in Code View.") + "</dd>";

    ui.cbLookWhere->addItem(tr("Selected CSS Files"), "SC");
    look_tooltip += "<dt><b>" + tr("Selected CSS Files") + "</b><dd>" + tr("Restrict the find or replace to the CSS files selected in the Book Browser in Code View.") + "</dd>";

    ui.cbLookWhere->addItem(tr("Tabbed CSS Files"), "TC");
    look_tooltip += "<dt><b>" + tr("Tabbed CSS Files") + "</b><dd>" + tr("Restrict the find or replace to the CSS files open in Tabs.") + "</dd>";

    ui.cbLookWhere->addItem(tr("OPF File"), "OP");
    look_tooltip += "<dt><b>" + tr("OPF File") + "</b><dd>" + tr("Restrict the find or replace to the OPF file.") + "</dd>";

    ui.cbLookWhere->addItem(tr("NCX File"), "NX");
    look_tooltip += "<dt><b>" + tr("NCX File") + "</b><dd>" + tr("Restrict the find or replace to the NCX file.") + "</dd>";

    look_tooltip += "</dl>";
    ui.cbLookWhere->setToolTip(look_tooltip);
    ui.cbLookWhere->setCurrentIndex(0);

    ui.cbSearchDirection->addItem("-- " + tr("Select Direction") + " --", ""); 
    ui.cbSearchDirection->addItem(tr("Up"), "UP");
    ui.cbSearchDirection->addItem(tr("Down"), "DN");
    ui.cbSearchDirection->setToolTip("<p>" + tr("Direction to search") + ":</p>"
                                     "<dl>"
                                     "<dt><b>" + tr("Up") + "</b><dd>" + tr("Search for the previous match from your current position.") + "</dd>"
                                     "<dt><b>" + tr("Down") + "</b><dd>" + tr("Search for the next match from your current position.") + "</dd>"
                                     "</dl>");
    ui.cbSearchDirection->setCurrentIndex(0);
    // Needed to be movable/resizable by some versions of Qt on Linux
    // when launched as a custom delegate editor.
    setFocusProxy(ui.frame);
}

void Controls::ConnectSignalsToSlots()
{
    connect(ui.chkRegexOptionDotAll, SIGNAL(clicked(bool)), this, SLOT(SetRegexOptionDotAll(bool)));
    connect(ui.chkRegexOptionMinimalMatch, SIGNAL(clicked(bool)), this, SLOT(SetRegexOptionMinimalMatch(bool)));
    connect(ui.chkRegexOptionAutoTokenise, SIGNAL(clicked(bool)), this, SLOT(SetRegexOptionAutoTokenise(bool)));
    connect(ui.chkOptionWrap, SIGNAL(clicked(bool)), this, SLOT(SetOptionWrap(bool)));
    connect(ui.btClearAll, SIGNAL(clicked()), this, SLOT(DoClearAll()));
}
