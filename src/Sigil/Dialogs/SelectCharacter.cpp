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

#include <QtGui/QCompleter>
#include <QtGui/QLineEdit>
#include <QtGui/QToolButton>
#include <QtCore/QSignalMapper>

#include "Dialogs/SelectCharacter.h"
#include "ResourceObjects/HTMLResource.h"
#include "Misc/SettingsStore.h"

static QString SETTINGS_GROUP = "select_character";

SelectCharacter::SelectCharacter(QWidget *parent)
    :
    QDialog(parent),
    m_buttonMapper(new QSignalMapper(this))

{
    ui.setupUi(this);
    connectSignalsSlots();

    ReadSettings();

    SetList();
}

SelectCharacter::~SelectCharacter()
{
    WriteSettings();
}

void SelectCharacter::SetList()
{
    QStringList characters = QStringList()
    // chars to insert, chars to display, tooltip entity, tooltip desc

    << "" << "" << "" << ""

    << QString::fromUtf8("\xc2\xA0")        << "nbsp"       << "&nbsp;"     << "non-breaking space"
    << QString::fromUtf8("\xc2\x80\x82")    << "ensp"       << "&ensp;"     << "en space"
    << QString::fromUtf8("\xc2\x80\x83")    << "emsp"       << "&emsp;"     << "em space"
    << QString::fromUtf8("\xc2\x80\x89")    << "thinsp"     << "&thinsp;"   << "thin space"
    << QString::fromUtf8("\xc2\xad")        << "soft-hypen" << "&shy;"      << "soft hyphen"

    << "" << "" << "" << ""

    << QString::fromUtf8("\xe2\x80\x98")    << ""           << "&lsquo;"    << "left single quote"
    << QString::fromUtf8("\xe2\x80\x99")    << ""           << "&rsquo;"    << "right single quote"
    << QString::fromUtf8("\xe2\x80\x9C")    << ""           << "&ldquo;"    << "left double quote"
    << QString::fromUtf8("\xe2\x80\x9D")    << ""           << "&rdaquo;"   << "right double quote"
    << QString::fromUtf8("\xe2\x80\xB9")    << ""           << "&lsaquo;"   << "left-pointing single angle quote"
    << QString::fromUtf8("\xe2\x80\xBA")    << ""           << "&rsaquo;"   << "right-pointing single angle quote"
    << QString::fromUtf8("\xc2\xab")        << ""           << "&ldaquo;"   << "left-pointing double angle quote"
    << QString::fromUtf8("\xc2\xbb")        << ""           << "&rdaquo;"   << "right-pointing double anglequote"
    << QString::fromUtf8("'")               << ""           << "&apos;"     << "apos"
    << QString::fromUtf8("\"")              << ""           << "&quot;"     << "double quote"
    << QString::fromUtf8("\xe2\x80\x9A")    << ""           << "&sbquo;"    << "single low-9 quote"
    << QString::fromUtf8("\xe2\x80\x9E")    << ""           << "&bdquo;"    << "double low-9 quote"

    << "" << "" << "" << ""

    << QString::fromUtf8("\xe2\x80\x94")    << ""           << "&mdash;"    << "emdash"
    << QString::fromUtf8("\xe2\x80\x93")    << ""           << "&ndash;"    << "endash"
    << "&amp;"                              << "&&"         << "&amp;"      << "ampersand"
    << "&lt;"                               << "<"          << "&lt;"       << "less-than sign"
    << "&gt;"                               << ">"          << "&gt;"       << "greater-than sign"
    << QString::fromUtf8("\xc2\xa9")        << ""           << "&copy;"     << "copyright"
    << QString::fromUtf8("\xc2\xae")        << ""           << "&reg;"      << "registered sign"
    << QString::fromUtf8("\xe2\x84\xa2")    << ""           << "&trade;"    << "trademark symbol"
    << QString::fromUtf8("\xc2\xa7")        << ""           << "&sect;"     << "section sign"
    << QString::fromUtf8("\xc2\xb6")        << ""           << "&para;"     << "pilcrow - paragraph sign"
    << QString::fromUtf8("\xe2\x80\xa0")    << ""           << "&dagger;"   << "dagger"
    << QString::fromUtf8("\xe2\x80\xa1")    << ""           << "&Dagger;"   << "double dagger"
    << QString::fromUtf8("\xe2\x86\x92")    << ""           << "&rarr;"     << "right arrow"
    << QString::fromUtf8("\xe2\x87\x92")    << ""           << "&rArr;"     << "double right arrow"
    << QString::fromUtf8("\xe2\x80\xa6")    << ""           << "&hellip;"   << "horizontal ellipsis - 3 dots"

    << "" << "" << "" << ""

    ;

    QHBoxLayout *layout;

    QToolButton *button;
    QFont font = *new QFont();
    font.setPointSize(font.pointSize() + 2);
    font.setFamily("helvetica");
    font.setFixedPitch(true);

    bool first_row = true;
    int i = 0;
    while (i < characters.count()) {
    
        QString insert_text = characters.at(i);
        QString display_text = characters.at(i + 1);
        if (display_text.isEmpty()) {
            display_text = insert_text;
        }
        QString entity = characters.at(i + 2);
        QString description = characters.at(i + 3);
        i += 4;

        if (insert_text.isEmpty()) {
            if (!first_row) {
                ui.character_box->addLayout(layout);
            }
            layout = new QHBoxLayout();
            first_row = false;
            continue;
        }

        button = new QToolButton(this);
        button->setAutoRaise(true);
        button->setToolTip(entity + " " + description);
        button->setText(display_text);
        button->setFont(font);

        layout->addWidget(button);

        connect(button, SIGNAL(clicked()), m_buttonMapper, SLOT(map()));
        m_buttonMapper->setMapping(button, insert_text);
    }
}

QString SelectCharacter::Selection()
{
    return m_SelectedText;
}

void SelectCharacter::SetSelectedCharacter(const QString& text)
{
    bool isCtrl = QApplication::keyboardModifiers() & Qt::ControlModifier;

    emit SelectedCharacter(text);
    if (isCtrl) {
        accept();
    }
}

void SelectCharacter::ReadSettings()
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

void SelectCharacter::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);

    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());

    settings.endGroup();
}

void SelectCharacter::connectSignalsSlots()
{
    connect(m_buttonMapper, SIGNAL(mapped(const QString&)), this, SLOT(SetSelectedCharacter(const QString&)));
}
