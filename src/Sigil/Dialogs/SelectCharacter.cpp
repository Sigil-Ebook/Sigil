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


    << QString::fromUtf8("\u00A0") << "nbsp"       << "&nbsp;"     << "non-breaking space"
    << QString::fromUtf8("\u2003") << "emsp"       << "&emsp;"     << "em space"
    << QString::fromUtf8("\u2002") << "ensp"       << "&ensp;"     << "en space"
    << QString::fromUtf8("\u2009") << "thinsp"     << "&thinsp;"   << "thin space"
    << QString::fromUtf8("\u00AD") << "soft-hypen"        << "&shy;"      << "soft hyphen"

    << "" << "" << "" << ""

    << QString::fromUtf8("\u2018") << ""            << "&lsquo;"      << "left single quote"
    << QString::fromUtf8("\u2019") << ""            << "&rsquo;"      << "right single quote"
    << QString::fromUtf8("\u201C") << ""            << "&ldquo;"      << "left double quote"
    << QString::fromUtf8("\u201D") << ""            << "&rdaquo;"      << "right double quote"
    << QString::fromUtf8("\u2039") << ""            << "&lsaquo;"      << "left-pointing angle quote"
    << QString::fromUtf8("\u203A") << ""            << "&rsaquo;"      << "right-pointing anglequote"
    << QString::fromUtf8("\u00AB") << ""            << "&ldaquo;"      << "left-pointing double angle quote"
    << QString::fromUtf8("\u00BB") << ""            << "&rdaquo;"      << "right-pointing double anglequote"
    << QString::fromUtf8("\u0027") << ""            << "&apos;"      << "apos"
    << QString::fromUtf8("\u0022") << ""            << "&quot;"      << "double quote"
    << QString::fromUtf8("\u201A") << ""            << "&sbquo;"      << "single low-9 quote"
    << QString::fromUtf8("\u201E") << ""            << "&bdquo;"      << "double low-9 quote"

    << "" << "" << "" << ""

    << QString::fromUtf8("\u2014") << ""       << "&mdash;"     << "mdash"
    << QString::fromUtf8("\u2013") << ""       << "&ndash;"     << "ndash"
    << "&amp;"  << "&&"     << "&amp;"      << "ampersand"
    << "&lt;" << QString::fromUtf8("\u003C")       << "&lt;"       << "less-than sign"
    << "&gt;" << QString::fromUtf8("\u003E")       << "&gt;"       << "greater-than sign"
    << QString::fromUtf8("\u00A9") << ""            << "&copy;"      << "copyright"
    << QString::fromUtf8("\u00AE") << ""            << "&reg;"      << "registered sign"
    << QString::fromUtf8("\u2122") << ""            << "&trade;"      << "trademark symbol"
    << QString::fromUtf8("\u00A7") << ""            << "&sect;"      << "section sign"
    << QString::fromUtf8("\u00B6") << ""            << "&para;"      << "pilcrow - paragraph sign"
    << QString::fromUtf8("\u2020") << ""            << "&dagger;"      << "dagger"
    << QString::fromUtf8("\u2021") << ""            << "&Dagger;"      << "double dagger"
    << QString::fromUtf8("\u2192") << ""            << "&rarr;"      << "right arrow"
    << QString::fromUtf8("\u21D2") << ""            << "&rArr;"      << "double right arrow"
    << QString::fromUtf8("\u2026") << ""            << "&hellip;"      << "horizontal ellipsis - 3 dots"

    << "" << "" << "" << ""

    ;

    QHBoxLayout *layout;
    QSpacerItem *horizontal_spacer = new QSpacerItem(10, 10, QSizePolicy::Expanding);

    QToolButton *button;
    QFont font = *new QFont();
    font.setPointSize(font.pointSize() + 2);
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
//                layout->addItem(horizontal_spacer);
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
