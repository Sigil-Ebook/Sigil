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
#include <QtGui/QApplication>
#include <QtCore/QSignalMapper>

#include "Dialogs/SelectCharacter.h"
#include "ResourceObjects/HTMLResource.h"

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

void SelectCharacter::show()
{
    SettingsStore settings;
    SettingsStore::SpecialCharacterAppearance appearance = settings.specialCharacterAppearance();

    if ( (m_SpecialCharacterAppearance.font_family != appearance.font_family) ||
         (m_SpecialCharacterAppearance.font_size != appearance.font_size) ) {
        // We need to update the grid to reflect the new appearance
        m_SpecialCharacterAppearance = appearance;

        QFont font(m_SpecialCharacterAppearance.font_family, m_SpecialCharacterAppearance.font_size);
        // Find all the buttons initialised on the grid and set their new font.
        QList<QToolButton*> buttons = findChildren<QToolButton*>();
        foreach (QToolButton *button, buttons) {
            button->setFont(font);
        }
    }
    QDialog::show();
}

void SelectCharacter::SetList()
{
    // chars to insert, chars to display, tooltip entity, tooltip desc
    // http://www.utf8-chartable.de/unicode-utf8-table.pl?utf8=0x&htmlent=1

    QStringList spaces = QStringList()
    << "&nbsp;"                             << "nbsp"       << "&nbsp;"     << "NON-BREAKING SPACE"
    << "&ensp;"                             << "ensp"       << "&ensp;"     << "EN SPACE"
    << "&emsp;"                             << "emsp"       << "&emsp;"     << "EM SPACE"
    << "&thinsp;"                           << "thinsp"     << "&thinsp;"   << "THIN SPACE"
    << "&shy;"                              << "shy"        << "&shy;"      << "SOFT HYPHEN"
    ;

    QStringList characters = QStringList()
    << QString::fromUtf8("\xe2\x80\x98")    << ""           << "&lsquo;"    << "LEFT SINGLE QUOTE"
    << QString::fromUtf8("\xe2\x80\x99")    << ""           << "&rsquo;"    << "RIGHT SINGLE QUOTE"
    << QString::fromUtf8("\xe2\x80\x9C")    << ""           << "&ldquo;"    << "LEFT DOUBLE QUOTE"
    << QString::fromUtf8("\xe2\x80\x9D")    << ""           << "&rdaquo;"   << "RIGHT DOUBLE QUOTE"
    << QString::fromUtf8("\xe2\x80\xB9")    << ""           << "&lsaquo;"   << "LEFT-POINTING SINGLE ANGLE QUOTE"
    << QString::fromUtf8("\xe2\x80\xBA")    << ""           << "&rsaquo;"   << "RIGHT-POINTING SINGLE ANGLE QUOTE"
    << QString::fromUtf8("\xc2\xab")        << ""           << "&ldaquo;"   << "LEFT-POINTING DOUBLE ANGLE QUOTE"
    << QString::fromUtf8("\xc2\xbb")        << ""           << "&rdaquo;"   << "RIGHT-POINTING DOUBLE ANGLE QUOTE"

    << "'"                                  << ""           << "&apos;"     << "APOSTROPHE"
    << "\""                                 << ""           << "&quot;"     << "DOUBLE QUOTE"
    << QString::fromUtf8("\xe2\x80\x9A")    << ""           << "&sbquo;"    << "SINGLE LOW-9 QUOTE"
    << QString::fromUtf8("\xe2\x80\x9E")    << ""           << "&bdquo;"    << "DOUBLE LOW-9 QUOTE"

    << QString::fromUtf8("\xe2\x80\x94")    << ""           << "&mdash;"    << "EM DASH"
    << QString::fromUtf8("\xe2\x80\x93")    << ""           << "&ndash;"    << "EN DASH"
    << QString::fromUtf8("\xc2\xa7")        << ""           << "&sect;"     << "SECTION SIGN"
    << QString::fromUtf8("\xc2\xb6")        << ""           << "&para;"     << "PILCROW - PARAGRAPH SIGN"
    << QString::fromUtf8("\xe2\x80\xa0")    << ""           << "&dagger;"   << "DAGGER"
    << QString::fromUtf8("\xe2\x80\xa1")    << ""           << "&Dagger;"   << "DOUBLE DAGGER"

    << "&amp;"                              << "&&"         << "&amp;"      << "AMPERSAND"
    << "&lt;"                               << "<"          << "& lt;"      << "LESS-THAN SIGN"
    << "&gt;"                               << ">"          << "&gt;"       << "GREATER-THAN SIGN"
    << QString::fromUtf8("\xc2\xa9")        << ""           << "&copy;"     << "COPYRIGHT"
    << QString::fromUtf8("\xc2\xae")        << ""           << "&reg;"      << "REGISTERED SIGN"
    << QString::fromUtf8("\xe2\x84\xa2")    << ""           << "&trade;"    << "TRADEMARK SYMBOL"

    << QString::fromUtf8("\xe2\x86\x92")    << ""           << "&rarr;"     << "RIGHT ARROW"
    << QString::fromUtf8("\xe2\x87\x92")    << ""           << "&rArr;"     << "DOUBLE RIGHT ARROW"

    << QString::fromUtf8("\xe2\x80\xa2")    << ""           << "&bull;"     << "BULLET"
    << QString::fromUtf8("\xc2\xb7")        << ""           << "&middot;"   << "MIDDLE DOT"
    << QString::fromUtf8("\xc2\xb0")        << ""           << "&deg;"      << "DEGREE SIGN"

    << QString::fromUtf8("\xc2\xb1")        << ""           << "&plusmn;"   << "PLUS MINUS SIGN"
    << QString::fromUtf8("\xe2\x88\x92")    << ""           << "&minus;"    << "MINUS SIGN"
    << QString::fromUtf8("\xc3\x97")        << ""           << "&times;"    << "MULTIPLICATION SIGN"
    << QString::fromUtf8("\xc3\xb7")        << ""           << "&divide;"   << "DIVISION SIGN"
    << QString::fromUtf8("\xc2\xbc")        << ""           << "&frac14;"   << "FRACTION 1/4"
    << QString::fromUtf8("\xc2\xbd")        << ""           << "&frac12;"   << "FRACTION 1/2"
    << QString::fromUtf8("\xc2\xbe")        << ""           << "&frac34;"   << "FRACTION 3/4"
    << QString::fromUtf8("\xe2\x80\xa6")    << ""           << "&hellip;"   << "HORIZONTAL ELLIPSIS"
    << QString::fromUtf8("\xc2\xb5")        << ""           << "&micro;"    << "MICRON"

    << QString::fromUtf8("\xc2\xa2")        << ""           << "&cent;"     << "CENT SIGN"
    << QString::fromUtf8("\xc2\xa3")        << ""           << "&pound;"    << "POUND SIGN"
    << QString::fromUtf8("\xe2\x82\xac")    << ""           << "&euro;"     << "EURO SIGN"


    << QString::fromUtf8("\xc2\xbf")        << ""           << "&iquest;"   << "INVERTED QUESTION MARK"
    << QString::fromUtf8("\xc2\xa1")        << ""           << "&iexcl;"    << "INVERTED EXCLAMATION MARK"

    << QString::fromUtf8("\xc2\xa8")        << ""           << "&uml;"      << "DIAERESIS"
    << QString::fromUtf8("\xc2\xb4")        << ""           << "&acute;"    << "ACUTE ACCENT"
    << QString::fromUtf8("\xc2\xb8")        << ""           << "&cedil;"    << "CEDILLA"
    << QString::fromUtf8("\xcb\x86")        << ""           << "&circ;"     << "CIRCUMFLEX ACCENT"
    << QString::fromUtf8("\xcb\x9c")        << ""           << "&tilde;"    << "SMALL TILDE"

    << QString::fromUtf8("\xc3\x80")        << ""           << "&Agrave;"   << "CAPITAL A WITH GRAVE"
    << QString::fromUtf8("\xc3\x81")        << ""           << "&Aacute;"   << "CAPITAL A WITH ACUTE"
    << QString::fromUtf8("\xc3\x82")        << ""           << "&Acirc;"    << "CAPITAL A WITH CIRCUMFLEX"
    << QString::fromUtf8("\xc3\x83")        << ""           << "&Atilde;"   << "CAPITAL A WITH TILDE"
    << QString::fromUtf8("\xc3\x84")        << ""           << "&Auml;"     << "CAPITAL A WITH DIAERESIS"
    << QString::fromUtf8("\xc3\x85")        << ""           << "&Aring;"    << "CAPITAL A WITH RING ABOVE"
    << QString::fromUtf8("\xc3\x86")        << ""           << "&Aelig;"    << "CAPITAL AE"
    << QString::fromUtf8("\xc3\x87")        << ""           << "&Ccedil;"   << "CAPITAL C WITH CEDILLA"
    << QString::fromUtf8("\xc3\x88")        << ""           << "&Egrave;"   << "CAPITAL E WITH GRAVE"
    << QString::fromUtf8("\xc3\x89")        << ""           << "&Eacute;"   << "CAPITAL E WITH ACUTE"
    << QString::fromUtf8("\xc3\x8a")        << ""           << "&Ecirc;"    << "CAPITAL E WITH CIRCUMFLEX"
    << QString::fromUtf8("\xc3\x8b")        << ""           << "&Euml;"     << "CAPITAL E WITH DIAERESIS"
    << QString::fromUtf8("\xc3\x8c")        << ""           << "&Igrave;"   << "CAPITAL I WITH GRAVE"
    << QString::fromUtf8("\xc3\x8d")        << ""           << "&Iacute;"   << "CAPITAL I WITH ACUTE"
    << QString::fromUtf8("\xc3\x8e")        << ""           << "&Icirc;"    << "CAPITAL I WITH CIRCUMFLEX"
    << QString::fromUtf8("\xc3\x8f")        << ""           << "&Iuml;"     << "CAPITAL I WITH DIAERESIS"
    << QString::fromUtf8("\xc3\x90")        << ""           << "&ETH;"      << "CAPITAL ETH"
    << QString::fromUtf8("\xc3\x91")        << ""           << "&Ntilde;"   << "CAPITAL N WITH TILDE"
    << QString::fromUtf8("\xc3\x92")        << ""           << "&Ograve;"   << "CAPITAL O WITH GRAVE"
    << QString::fromUtf8("\xc3\x93")        << ""           << "&Oacute;"   << "CAPITAL O WITH ACUTE"
    << QString::fromUtf8("\xc3\x94")        << ""           << "&Ocirc;"    << "CAPITAL O WITH CIRCUMFLEX"
    << QString::fromUtf8("\xc3\x95")        << ""           << "&Otilde;"   << "CAPITAL O WITH TILDE"
    << QString::fromUtf8("\xc3\x96")        << ""           << "&Ouml;"     << "CAPITAL O WITH DIAERESIS"
    << QString::fromUtf8("\xc3\x98")        << ""           << "&Oslash;"   << "CAPITAL O WITH STROKE"
    << QString::fromUtf8("\xc5\x92")        << ""           << "&OElig;"    << "CAPITAL LIGATURE OE"
    << QString::fromUtf8("\xc5\xa0")        << ""           << "&Scaron;"   << "CAPITAL S WITH CARON"
    << QString::fromUtf8("\xc3\x99")        << ""           << "&Ugrave;"   << "CAPITAL U WITH GRAVE"
    << QString::fromUtf8("\xc3\x9a")        << ""           << "&Uacute;"   << "CAPITAL U WITH ACUTE"
    << QString::fromUtf8("\xc3\x9b")        << ""           << "&Ucirc;"    << "CAPITAL U WITH CIRCUMFLEX"
    << QString::fromUtf8("\xc3\x9c")        << ""           << "&Uuml;"     << "CAPITAL U WITH DIAERESIS"
    << QString::fromUtf8("\xc3\x9d")        << ""           << "&Yacute;"   << "CAPITAL Y WITH ACUTE"
    << QString::fromUtf8("\xc5\xb8")        << ""           << "&Yuml;"     << "CAPITAL Y WITH DIAERESIS"
    << QString::fromUtf8("\xc3\x9e")        << ""           << "&THORN;"    << "CAPITAL THORN"

    << QString::fromUtf8("\xc3\x9f")        << ""           << "&szlig;"    << "SMALL SHARP S"


    << QString::fromUtf8("\xc3\xa0")        << ""           << "&agrave;"   << "SMALL A WITH GRAVE"
    << QString::fromUtf8("\xc3\xa1")        << ""           << "&aacute;"   << "SMALL A WITH ACUTE"
    << QString::fromUtf8("\xc3\xa2")        << ""           << "&acirc;"    << "SMALL A WITH CIRCUMFLEX"
    << QString::fromUtf8("\xc3\xa3")        << ""           << "&atilde;"   << "SMALL A WITH TILDE"
    << QString::fromUtf8("\xc3\xa4")        << ""           << "&auml;"     << "SMALL A WITH DIARESIS"
    << QString::fromUtf8("\xc3\xa5")        << ""           << "&aring;"    << "SMALL A WITH RING ABOVE"
    << QString::fromUtf8("\xc3\xa6")        << ""           << "&aelig;"    << "SMALL AE"
    << QString::fromUtf8("\xc3\xa7")        << ""           << "&ccedil;"   << "SMALL C WITH CEDILIA"
    << QString::fromUtf8("\xc3\xa8")        << ""           << "&egrave;"   << "SMALL E WITH GRAVE"
    << QString::fromUtf8("\xc3\xa9")        << ""           << "&eacute;"   << "SMALL E WITH ACUTE"
    << QString::fromUtf8("\xc3\xaa")        << ""           << "&ecirc;"    << "SMALL E WITH CIRCUMFLEX"
    << QString::fromUtf8("\xc3\xab")        << ""           << "&euml;"     << "SMALL E WITH DIAERESIS"
    << QString::fromUtf8("\xc3\xac")        << ""           << "&igrave;"   << "SMALL I WITH GRAVE"
    << QString::fromUtf8("\xc3\xad")        << ""           << "&iacute;"   << "SMALL I WITH ACUTE"
    << QString::fromUtf8("\xc3\xae")        << ""           << "&icirc;"    << "SMALL I WITH CIRCUMFLEX"
    << QString::fromUtf8("\xc3\xaf")        << ""           << "&iuml;"     << "SMALL I WITH DIAERESIS"
    << QString::fromUtf8("\xc3\xb0")        << ""           << "&eth;"      << "SMALL ETH"
    << QString::fromUtf8("\xc3\xb1")        << ""           << "&ntilde;"   << "SMALL N WITH TILDE"
    << QString::fromUtf8("\xc3\xb2")        << ""           << "&ograve;"   << "SMALL O WITH GRAVE"
    << QString::fromUtf8("\xc3\xb3")        << ""           << "&oacute;"   << "SMALL O WITH ACUTE"
    << QString::fromUtf8("\xc3\xb4")        << ""           << "&ocirc;"    << "SMALL O WITH CIRCUMFLEX"
    << QString::fromUtf8("\xc3\xb5")        << ""           << "&otilde;"   << "SMALL O WITH TILDE"
    << QString::fromUtf8("\xc3\xb6")        << ""           << "&ouml;"     << "SMALL O WITH DIAERESIS"
    << QString::fromUtf8("\xc3\xb8")        << ""           << "&oslash;"   << "SMALL O WITH STROKE"
    << QString::fromUtf8("\xc5\x93")        << ""           << "&oelig;"    << "SMALL LIGATURE OE"
    << QString::fromUtf8("\xc5\xa1")        << ""           << "&scaron;"   << "SMALL S WITH CARON"
    << QString::fromUtf8("\xc3\xb9")        << ""           << "&ugrave;"   << "SMALL U WITH GRAVE"
    << QString::fromUtf8("\xc3\xba")        << ""           << "&uacute;"   << "SMALL U WITH ACUTE"
    << QString::fromUtf8("\xc3\xbb")        << ""           << "&ucirc;"    << "SMALL U WITH CIRCUMFLEX"
    << QString::fromUtf8("\xc3\xbc")        << ""           << "&uuml;"     << "SMALL U WITH DIAERESIS"
    << QString::fromUtf8("\xc3\xbd")        << ""           << "&yacute;"   << "SMALL Y WITH ACUTE"
    << QString::fromUtf8("\xc3\xbf")        << ""           << "&yuml;"     << "SMALL Y WITH DIAERESIS"
    << QString::fromUtf8("\xc3\xbe")        << ""           << "&thorn;"    << "SMALL THORN"

    << QString::fromUtf8("\xc2\xaa")        << ""           << "&ordf;"     << "FEMININE ORDINAL INDICATOR"
    << QString::fromUtf8("\xc2\xba")        << ""           << "&ordm;"     << "MASCULINE ORDINAL INDICATOR"

    << QString::fromUtf8("\xce\xb1")        << ""           << "&alpha;"     << "SMALL ALPHA"
    << QString::fromUtf8("\xce\xa9")        << ""           << "&Omega;"     << "CAPITAL OMEGA"
    << QString::fromUtf8("\xe2\x88\x9e")    << ""           << "&infin;"     << "INFINITY"
    ;

    AddGrid(spaces, spaces.count());
    AddGrid(characters, 12);
}

void SelectCharacter::AddGrid(const QStringList &characters, int width)
{
    QToolButton *button;
    QFont font(m_SpecialCharacterAppearance.font_family, m_SpecialCharacterAppearance.font_size);

    QGridLayout *grid = new QGridLayout();
    grid->setHorizontalSpacing(0);
    grid->setVerticalSpacing(0);

    int col = 0;
    int row = 0;
    int i = 0;

    while (i < characters.count()) {
    
        const QString &insert_text = characters.at(i);
        QString display_text = characters.at(i + 1);
        if (display_text.isEmpty()) {
            display_text = insert_text;
        }
        const QString &entity = characters.at(i + 2);
        const QString &description = characters.at(i + 3);
        i += 4;

        if (!insert_text.isEmpty()) {
            button = new QToolButton(this);
            button->setAutoRaise(true);
            button->setToolTip(entity + " " + description);
            button->setText(display_text);
            button->setFont(font);
    
            connect(button, SIGNAL(clicked()), m_buttonMapper, SLOT(map()));
            m_buttonMapper->setMapping(button, insert_text);

            grid->addWidget(button, row, col);
        }

        col++;
        if (col == width) {
            col = 0;
            row++;
        }
    }

    ui.character_box->addLayout(grid);
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

    // Return focus to last window to allow typing to continue
    QApplication::setActiveWindow(parentWidget());
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
    // Load our initial appearance settings
    m_SpecialCharacterAppearance = settings.specialCharacterAppearance();
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
