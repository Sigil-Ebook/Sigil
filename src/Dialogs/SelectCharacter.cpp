/************************************************************************
**
**  Copyright (C) 2018-2021 Kevin B. Hendricks, Stratford, ON Canada
**  Copyright (C) 2012      John Schember <john@nachtimwald.com>
**  Copyright (C) 2012      Dave Heiland
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

#include <QtWidgets/QCompleter>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QApplication>
#include <QtCore/QSignalMapper>
#include <QScrollArea>
#include <QVBoxLayout>
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
    QWidget *viewport = new QWidget;
    viewport->setLayout(ui.character_box);
    QScrollArea * scrollArea = new QScrollArea;
    scrollArea->setWidget(viewport);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(scrollArea);
    setLayout(vbox);
}

SelectCharacter::~SelectCharacter()
{
    WriteSettings();
}

void SelectCharacter::show()
{
    SettingsStore settings;
    SettingsStore::SpecialCharacterAppearance appearance = settings.specialCharacterAppearance();

    if ((m_SpecialCharacterAppearance.font_family != appearance.font_family) ||
        (m_SpecialCharacterAppearance.font_size != appearance.font_size)) {
        // We need to update the grid to reflect the new appearance
        m_SpecialCharacterAppearance = appearance;
        QFont font(m_SpecialCharacterAppearance.font_family, m_SpecialCharacterAppearance.font_size);
        // Find all the buttons initialised on the grid and set their new font.
        QList<QToolButton *> buttons = findChildren<QToolButton *>();
        foreach(QToolButton * button, buttons) {
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
        << QString::fromUtf8("\xc2\xa0")        << "nbsp"       << "&nbsp;"   << tr("non-breaking space")
        << QString::fromUtf8("\xe2\x80\x82")    << "ensp"       << "&ensp;"   << tr("en space")
        << QString::fromUtf8("\xe2\x80\x83")    << "emsp"       << "&emsp;"   << tr("em space")
        << QString::fromUtf8("\xe2\x80\x89")    << "thinsp"     << "&thinsp;" << tr("thin space")
        << "&#173;"                             << "shy"        << "&shy;"    << tr("soft hyphen")
        << QString::fromUtf8("\xe2\x80\xaf")    << "nnbsp"      << "&#x202f;" << tr("narrow non-breaking space")
    ;

    QStringList characters = QStringList()
        << QString::fromUtf8("\xe2\x80\x98")    << ""           << "&lsquo;"    << tr("left single quote")
        << QString::fromUtf8("\xe2\x80\x99")    << ""           << "&rsquo;"    << tr("right single quote")
        << QString::fromUtf8("\xe2\x80\x9C")    << ""           << "&ldquo;"    << tr("left double quote")
        << QString::fromUtf8("\xe2\x80\x9D")    << ""           << "&rdquo;"    << tr("right double quote")
        << QString::fromUtf8("\xe2\x80\xB9")    << ""           << "&lsaquo;"   << tr("left-pointing single angle quote")
        << QString::fromUtf8("\xe2\x80\xBA")    << ""           << "&rsaquo;"   << tr("right-pointing single angle quote")
        << QString::fromUtf8("\xc2\xab")        << ""           << "&laquo;"    << tr("left-pointing double angle quote")
        << QString::fromUtf8("\xc2\xbb")        << ""           << "&raquo;"    << tr("right-pointing double angle quote")
        << "'"                                  << ""           << "&apos;"     << tr("apostrophe")
        << "\""                                 << ""           << "&quot;"     << tr("double quote")
        << QString::fromUtf8("\xe2\x80\x9A")    << ""           << "&sbquo;"    << tr("single low-9 quote")
        << QString::fromUtf8("\xe2\x80\x9E")    << ""           << "&bdquo;"    << tr("double low-9 quote")
        << QString::fromUtf8("\xe2\x80\x94")    << ""           << "&mdash;"    << tr("em dash")
        << QString::fromUtf8("\xe2\x80\x93")    << ""           << "&ndash;"    << tr("en dash")
        << QString::fromUtf8("\xc2\xa7")        << ""           << "&sect;"     << tr("section sign")
        << QString::fromUtf8("\xc2\xb6")        << ""           << "&para;"     << tr("pilcrow - paragraph sign")
        << QString::fromUtf8("\xe2\x80\xa0")    << ""           << "&dagger;"   << tr("dagger")
        << QString::fromUtf8("\xe2\x80\xa1")    << ""           << "&Dagger;"   << tr("double dagger")
        << "&amp;"                              << "&&"         << "&amp;"      << tr("ampersand")
        << "&lt;"                               << "<"          << "&lt;"       << tr("less-than sign")
        << "&gt;"                               << ">"          << "&gt;"       << tr("greater-than sign")
        << QString::fromUtf8("\xc2\xa9")        << ""           << "&copy;"     << tr("copyright")
        << QString::fromUtf8("\xc2\xae")        << ""           << "&reg;"      << tr("registered sign")
        << QString::fromUtf8("\xe2\x84\xa2")    << ""           << "&trade;"    << tr("trademark symbol")
        << QString::fromUtf8("\xe2\x86\x90")    << ""           << "&larr;"     << tr("left arrow")
        << QString::fromUtf8("\xe2\x86\x92")    << ""           << "&rarr;"     << tr("right arrow")
        << QString::fromUtf8("\xe2\x80\xa2")    << ""           << "&bull;"     << tr("bullet")
        << QString::fromUtf8("\xc2\xb7")        << ""           << "&middot;"   << tr("middle dot")
        << QString::fromUtf8("\xc2\xb0")        << ""           << "&deg;"      << tr("degree sign")
        << QString::fromUtf8("\xc2\xb1")        << ""           << "&plusmn;"   << tr("plus minus sign")
        << QString::fromUtf8("\xe2\x88\x92")    << ""           << "&minus;"    << tr("minus sign")
        << QString::fromUtf8("\xc3\x97")        << ""           << "&times;"    << tr("multiplication sign")
        << QString::fromUtf8("\xc3\xb7")        << ""           << "&divide;"   << tr("division sign")
        << QString::fromUtf8("\xc2\xbc")        << ""           << "&frac14;"   << tr("fraction 1/4")
        << QString::fromUtf8("\xc2\xbd")        << ""           << "&frac12;"   << tr("fraction 1/2")
        << QString::fromUtf8("\xc2\xbe")        << ""           << "&frac34;"   << tr("fraction 3/4")
        << QString::fromUtf8("\xe2\x85\x93")    << ""           << "&#8531;"    << tr("fraction 1/3")
        << QString::fromUtf8("\xe2\x85\x94")    << ""           << "&#8532;"    << tr("fraction 2/3")
        << QString::fromUtf8("\xe2\x85\x9b")    << ""           << "&#8539;"    << tr("fraction 1/8")
        << QString::fromUtf8("\xe2\x85\x9c")    << ""           << "&#8540;"    << tr("fraction 3/8")
        << QString::fromUtf8("\xe2\x85\x9d")    << ""           << "&#8541;"    << tr("fraction 5/8")
        << QString::fromUtf8("\xe2\x85\x9e")    << ""           << "&#8542;"    << tr("fraction 7/8")
        << QString::fromUtf8("\xe2\x80\xa6")    << ""           << "&hellip;"   << tr("horizontal ellipsis")
        << QString::fromUtf8("\xc2\xb5")        << ""           << "&micro;"    << tr("micron")
        << QString::fromUtf8("\xc2\xa2")        << ""           << "&cent;"     << tr("cent sign")
        << QString::fromUtf8("\xc2\xa3")        << ""           << "&pound;"    << tr("pound sign")
        << QString::fromUtf8("\xe2\x82\xac")    << ""           << "&euro;"     << tr("euro sign")
        << QString::fromUtf8("\xc2\xbf")        << ""           << "&iquest;"   << tr("inverted question mark")
        << QString::fromUtf8("\xc2\xa1")        << ""           << "&iexcl;"    << tr("inverted exclamation mark")
        << QString::fromUtf8("\xc2\xa8")        << ""           << "&uml;"      << tr("diaeresis")
        << QString::fromUtf8("\xc2\xb4")        << ""           << "&acute;"    << tr("acute accent")
        << QString::fromUtf8("\xc2\xb8")        << ""           << "&cedil;"    << tr("cedilla")
        << QString::fromUtf8("\xcb\x86")        << ""           << "&circ;"     << tr("circumflex accent")
        << QString::fromUtf8("\xcb\x9c")        << ""           << "&tilde;"    << tr("small tilde")
        << QString::fromUtf8("\xc3\x80")        << ""           << "&Agrave;"   << tr("capital A with grave")
        << QString::fromUtf8("\xc3\x81")        << ""           << "&Aacute;"   << tr("capital A with acute")
        << QString::fromUtf8("\xc3\x82")        << ""           << "&Acirc;"    << tr("capital A with circumflex")
        << QString::fromUtf8("\xc3\x83")        << ""           << "&Atilde;"   << tr("capital A with tilde")
        << QString::fromUtf8("\xc3\x84")        << ""           << "&Auml;"     << tr("capital A with diaeresis")
        << QString::fromUtf8("\xc3\x85")        << ""           << "&Aring;"    << tr("capital A with ring above")
        << QString::fromUtf8("\xc3\x86")        << ""           << "&AElig;"    << tr("capital AE")
        << QString::fromUtf8("\xc3\x87")        << ""           << "&Ccedil;"   << tr("capital C with cedilla")
        << QString::fromUtf8("\xc3\x88")        << ""           << "&Egrave;"   << tr("capital E with grave")
        << QString::fromUtf8("\xc3\x89")        << ""           << "&Eacute;"   << tr("capital E with acute")
        << QString::fromUtf8("\xc3\x8a")        << ""           << "&Ecirc;"    << tr("capital E with circumflex")
        << QString::fromUtf8("\xc3\x8b")        << ""           << "&Euml;"     << tr("capital E with diaeresis")
        << QString::fromUtf8("\xc3\x8c")        << ""           << "&Igrave;"   << tr("capital I with grave")
        << QString::fromUtf8("\xc3\x8d")        << ""           << "&Iacute;"   << tr("capital I with acute")
        << QString::fromUtf8("\xc3\x8e")        << ""           << "&Icirc;"    << tr("capital I with circumflex")
        << QString::fromUtf8("\xc3\x8f")        << ""           << "&Iuml;"     << tr("capital I with diaeresis")
        << QString::fromUtf8("\xc3\x90")        << ""           << "&ETH;"      << tr("capital eth")
        << QString::fromUtf8("\xc3\x91")        << ""           << "&Ntilde;"   << tr("capital N with tilde")
        << QString::fromUtf8("\xc3\x92")        << ""           << "&Ograve;"   << tr("capital O with grave")
        << QString::fromUtf8("\xc3\x93")        << ""           << "&Oacute;"   << tr("capital O with acute")
        << QString::fromUtf8("\xc3\x94")        << ""           << "&Ocirc;"    << tr("capital O with circumflex")
        << QString::fromUtf8("\xc3\x95")        << ""           << "&Otilde;"   << tr("capital O with tilde")
        << QString::fromUtf8("\xc3\x96")        << ""           << "&Ouml;"     << tr("capital O with diaeresis")
        << QString::fromUtf8("\xc3\x98")        << ""           << "&Oslash;"   << tr("capital O with stroke")
        << QString::fromUtf8("\xc5\x92")        << ""           << "&OElig;"    << tr("capital ligature OE")
        << QString::fromUtf8("\xc5\xa0")        << ""           << "&Scaron;"   << tr("capital S with caron")
        << QString::fromUtf8("\xc3\x99")        << ""           << "&Ugrave;"   << tr("capital U with grave")
        << QString::fromUtf8("\xc3\x9a")        << ""           << "&Uacute;"   << tr("capital U with acute")
        << QString::fromUtf8("\xc3\x9b")        << ""           << "&Ucirc;"    << tr("capital U with circumflex")
        << QString::fromUtf8("\xc3\x9c")        << ""           << "&Uuml;"     << tr("capital U with diaeresis")
        << QString::fromUtf8("\xc3\x9d")        << ""           << "&Yacute;"   << tr("capital Y with acute")
        << QString::fromUtf8("\xc5\xb8")        << ""           << "&Yuml;"     << tr("capital Y with diaeresis")
        << QString::fromUtf8("\xc3\x9e")        << ""           << "&THORN;"    << tr("capital THORN")
        << QString::fromUtf8("\xc3\x9f")        << ""           << "&szlig;"    << tr("small sharp s")
        << QString::fromUtf8("\xc3\xa0")        << ""           << "&agrave;"   << tr("small a with grave")
        << QString::fromUtf8("\xc3\xa1")        << ""           << "&aacute;"   << tr("small a with acute")
        << QString::fromUtf8("\xc3\xa2")        << ""           << "&acirc;"    << tr("small a with circumflex")
        << QString::fromUtf8("\xc3\xa3")        << ""           << "&atilde;"   << tr("small a with tilde")
        << QString::fromUtf8("\xc3\xa4")        << ""           << "&auml;"     << tr("small a with diaeresis")
        << QString::fromUtf8("\xc3\xa5")        << ""           << "&aring;"    << tr("small a with ring above")
        << QString::fromUtf8("\xc3\xa6")        << ""           << "&aelig;"    << tr("small ae")
        << QString::fromUtf8("\xc3\xa7")        << ""           << "&ccedil;"   << tr("small c with cedilia")
        << QString::fromUtf8("\xc3\xa8")        << ""           << "&egrave;"   << tr("small e with grave")
        << QString::fromUtf8("\xc3\xa9")        << ""           << "&eacute;"   << tr("small e with acute")
        << QString::fromUtf8("\xc3\xaa")        << ""           << "&ecirc;"    << tr("small e with circumflex")
        << QString::fromUtf8("\xc3\xab")        << ""           << "&euml;"     << tr("small e with diaeresis")
        << QString::fromUtf8("\xc3\xac")        << ""           << "&igrave;"   << tr("small i with grave")
        << QString::fromUtf8("\xc3\xad")        << ""           << "&iacute;"   << tr("small i with acute")
        << QString::fromUtf8("\xc3\xae")        << ""           << "&icirc;"    << tr("small i with circumflex")
        << QString::fromUtf8("\xc3\xaf")        << ""           << "&iuml;"     << tr("small i with diaeresis")
        << QString::fromUtf8("\xc3\xb0")        << ""           << "&eth;"      << tr("small eth")
        << QString::fromUtf8("\xc3\xb1")        << ""           << "&ntilde;"   << tr("small n with tilde")
        << QString::fromUtf8("\xc3\xb2")        << ""           << "&ograve;"   << tr("small o with grave")
        << QString::fromUtf8("\xc3\xb3")        << ""           << "&oacute;"   << tr("small o with acute")
        << QString::fromUtf8("\xc3\xb4")        << ""           << "&ocirc;"    << tr("small o with circumflex")
        << QString::fromUtf8("\xc3\xb5")        << ""           << "&otilde;"   << tr("small o with tilde")
        << QString::fromUtf8("\xc3\xb6")        << ""           << "&ouml;"     << tr("small o with diaeresis")
        << QString::fromUtf8("\xc3\xb8")        << ""           << "&oslash;"   << tr("small o with stroke")
        << QString::fromUtf8("\xc5\x93")        << ""           << "&oelig;"    << tr("small ligature oe")
        << QString::fromUtf8("\xc5\xa1")        << ""           << "&scaron;"   << tr("small s with caron")
        << QString::fromUtf8("\xc3\xb9")        << ""           << "&ugrave;"   << tr("small u with grave")
        << QString::fromUtf8("\xc3\xba")        << ""           << "&uacute;"   << tr("small u with acute")
        << QString::fromUtf8("\xc3\xbb")        << ""           << "&ucirc;"    << tr("small u with circumflex")
        << QString::fromUtf8("\xc3\xbc")        << ""           << "&uuml;"     << tr("small u with diaeresis")
        << QString::fromUtf8("\xc3\xbd")        << ""           << "&yacute;"   << tr("small y with acute")
        << QString::fromUtf8("\xc3\xbf")        << ""           << "&yuml;"     << tr("small y with diaeresis")
        << QString::fromUtf8("\xc3\xbe")        << ""           << "&thorn;"    << tr("small thorn")
        << QString::fromUtf8("\xc2\xaa")        << ""           << "&ordf;"     << tr("feminine ordinal indicator")
        << QString::fromUtf8("\xc2\xba")        << ""           << "&ordm;"     << tr("masculine ordinal indicator")
        << QString::fromUtf8("\xe2\x88\x9e")    << ""           << "&infin;"    << tr("infinity")
    ;

    QStringList characters2 = QStringList()
        << QString::fromUtf8("\xce\x91")        << ""           << "&Alpha;"    << tr("Greek capital letter Alpha")
        << QString::fromUtf8("\xce\xb1")        << ""           << "&alpha;"    << tr("Greek lower letter alpha")
        << QString::fromUtf8("\xce\x92")        << ""           << "&Beta;"     << tr("Greek capital letter Beta")
        << QString::fromUtf8("\xce\xb2")        << ""           << "&beta;"     << tr("Greek lower letter beta")
        << QString::fromUtf8("\xce\xa7")        << ""           << "&Chi;"      << tr("Greek capital letter Chi")
        << QString::fromUtf8("\xcf\x87")        << ""           << "&chi;"      << tr("Greek lower letter chi")
        << QString::fromUtf8("\xce\x94")        << ""           << "&Delta;"    << tr("Greek capital letter Delta")
        << QString::fromUtf8("\xce\xb4")        << ""           << "&delta;"    << tr("Greek lower letter delta")
        << QString::fromUtf8("\xce\x95")        << ""           << "&Epsilon;"  << tr("Greek capital letter Epsilon")
        << QString::fromUtf8("\xce\xb5")        << ""           << "&epsilon;"  << tr("Greek lower letter epsilon")
        << QString::fromUtf8("\xce\x97")        << ""           << "&Eta;"      << tr("Greek capital letter Eta")
        << QString::fromUtf8("\xce\xb7")        << ""           << "&eta;"      << tr("Greek lower letter eta")
        << QString::fromUtf8("\xce\x93")        << ""           << "&Gamma;"    << tr("Greek capital letter Gamma")
        << QString::fromUtf8("\xce\xb3")        << ""           << "&gamma;"    << tr("Greek lower letter gamma")
        << QString::fromUtf8("\xce\x99")        << ""           << "&Iota;"     << tr("Greek capital letter Iota")
        << QString::fromUtf8("\xce\xb9")        << ""           << "&iota;"     << tr("Greek lower letter iota")
        << QString::fromUtf8("\xce\x9a")        << ""           << "&Kappa;"    << tr("Greek capital letter Kappa")
        << QString::fromUtf8("\xce\xba")        << ""           << "&kappa;"    << tr("Greek lower letter kappa")
        << QString::fromUtf8("\xce\x9b")        << ""           << "&Lambda;"   << tr("Greek capital letter Lambda")
        << QString::fromUtf8("\xce\xbb")        << ""           << "&lambda;"   << tr("Greek lower letter lambda")
        << QString::fromUtf8("\xce\x9c")        << ""           << "&Mu;"       << tr("Greek capital letter Mu")
        << QString::fromUtf8("\xce\xbc")        << ""           << "&mu;"       << tr("Greek lower letter mu")
        << QString::fromUtf8("\xce\x9d")        << ""           << "&Nu;"       << tr("Greek capital letter Nu")
        << QString::fromUtf8("\xce\xbd")        << ""           << "&nu;"       << tr("Greek lower letter nu")
        << QString::fromUtf8("\xce\xa9")        << ""           << "&Omega;"    << tr("Greek capital letter Omega")
        << QString::fromUtf8("\xcf\x89")        << ""           << "&omega;"    << tr("Greek lower letter omega")
        << QString::fromUtf8("\xce\x9f")        << ""           << "&Omicron;"  << tr("Greek capital letter Omicron")
        << QString::fromUtf8("\xce\xbf")        << ""           << "&omicron;"  << tr("Greek lower letter omicron")
        << QString::fromUtf8("\xce\xa6")        << ""           << "&Phi;"      << tr("Greek capital letter Phi")
        << QString::fromUtf8("\xcf\x86")        << ""           << "&phi;"      << tr("Greek lower letter phi")
        << QString::fromUtf8("\xce\xa0")        << ""           << "&Pi;"       << tr("Greek capital letter Pi")
        << QString::fromUtf8("\xcf\x80")        << ""           << "&pi;"       << tr("Greek lower letter pi")
        << QString::fromUtf8("\xe2\x80\xb3")    << ""           << "&Prime;"    << tr("Greek double prime")
        << QString::fromUtf8("\xe2\x80\xb2")    << ""           << "&prime;"    << tr("Greek single prime")
        << QString::fromUtf8("\xce\xa8")        << ""           << "&Psi;"      << tr("Greek capital letter Psi")
        << QString::fromUtf8("\xcf\x88")        << ""           << "&psi;"      << tr("Greek lower letter psi")
        << QString::fromUtf8("\xce\xa1")        << ""           << "&Rho;"      << tr("Greek capital letter Rho")
        << QString::fromUtf8("\xcf\x81")        << ""           << "&rho;"      << tr("Greek lower letter rho")
        << QString::fromUtf8("\xce\xa3")        << ""           << "&Sigma;"    << tr("Greek capital letter Sigma")
        << QString::fromUtf8("\xcf\x83")        << ""           << "&sigma;"    << tr("Greek lower letter sigma")
        << QString::fromUtf8("\xce\xa4")        << ""           << "&Tau;"      << tr("Greek capital letter Tau")
        << QString::fromUtf8("\xcf\x84")        << ""           << "&tau;"      << tr("Greek lower letter tau")
        << QString::fromUtf8("\xce\x98")        << ""           << "&Theta;"    << tr("Greek capital letter Theta")
        << QString::fromUtf8("\xce\xb8")        << ""           << "&theta;"    << tr("Greek lower letter theta")
        << QString::fromUtf8("\xce\xa5")        << ""           << "&Upsilon;"  << tr("Greek capital letter Upsilon")
        << QString::fromUtf8("\xcf\x85")        << ""           << "&upsilon;"  << tr("Greek lower letter upsilon")
        << QString::fromUtf8("\xce\x9e")        << ""           << "&Xi;"       << tr("Greek capital letter Xi")
        << QString::fromUtf8("\xce\xbe")        << ""           << "&xi;"       << tr("Greek lower letter xi")
        << QString::fromUtf8("\xce\x96")        << ""           << "&Zeta;"     << tr("Greek capital letter Zeta")
        << QString::fromUtf8("\xce\xb6")        << ""           << "&zeta;"     << tr("Greek lower letter zeta")
    ;

    QStringList characters3 = QStringList()
        << QString::fromUtf8("\xe2\x84\xb5")    << ""           << "&alefsym;"  << tr("alef symbol")
        << QString::fromUtf8("\xe2\x88\xa7")    << ""           << "&and;"      << tr("logical and")
        << QString::fromUtf8("\xe2\x88\xa8")    << ""           << "&or;"       << tr("logical or")
        << QString::fromUtf8("\xe2\x88\xa9")    << ""           << "&cap;"      << tr("intersection")
        << QString::fromUtf8("\xe2\x88\xaa")    << ""           << "&cup;"      << tr("union")
        << QString::fromUtf8("\xe2\x89\x85")    << ""           << "&cong;"     << tr("congruent to")
        << QString::fromUtf8("\xe2\x86\xb5")    << ""           << "&crarr;"    << tr("downwards arrow with corner leftwards")
        << QString::fromUtf8("\xc2\xa4")        << ""           << "&curren;"   << tr("currency sign")
        << QString::fromUtf8("\xe2\x87\x93")    << ""           << "&dArr;"     << tr("downwards double arrow")
        << QString::fromUtf8("\xe2\x87\x91")    << ""           << "&uArr;"     << tr("upwards double arrow")
        << QString::fromUtf8("\xe2\x86\x93")    << ""           << "&darr;"     << tr("downwards arrow")
        << QString::fromUtf8("\xe2\x86\x91")    << ""           << "&uarr;"     << tr("upwards arrow")
        << QString::fromUtf8("\xe2\x88\x85")    << ""           << "&empty;"    << tr("empty set")
        << QString::fromUtf8("\xe2\x89\xa1")    << ""           << "&equiv;"    << tr("identical to")
        << QString::fromUtf8("\xe2\x88\x83")    << ""           << "&exist;"    << tr("there exists")
        << QString::fromUtf8("\xc6\x92")        << ""           << "&fnof;"     << tr("Latin small letter f with hook")
        << QString::fromUtf8("\xe2\x88\x80")    << ""           << "&forall;"   << tr("for all")
        << QString::fromUtf8("\xe2\x81\x84")    << ""           << "&frasl;"    << tr("fraction slash")
        << QString::fromUtf8("\xe2\x87\x94")    << ""           << "&hArr;"     << tr("left right double arrow")
        << QString::fromUtf8("\xe2\x86\x94")    << ""           << "&harr;"     << tr("left right single arrow")
        << QString::fromUtf8("\xe2\x84\x91")    << ""           << "&image;"    << tr("black-letter capital I")
        << QString::fromUtf8("\xe2\x88\xab")    << ""           << "&int;"      << tr("integral")
        << QString::fromUtf8("\xe2\x88\x88")    << ""           << "&isin;"     << tr("element of")
        << QString::fromUtf8("\xe2\x87\x90")    << ""           << "&lArr;"     << tr("leftwards double arrow")
        << QString::fromUtf8("\xe2\x87\x92")    << ""           << "&rArr;"     << tr("double right arrow")
        << QString::fromUtf8("\xe2\x8c\xa9")    << ""           << "&lang;"     << tr("left-pointing angle bracket")
        << QString::fromUtf8("\xe2\x8c\xaa")    << ""           << "&rang;"     << tr("right-pointing angle bracket")
        << QString::fromUtf8("\xe2\x8c\x88")    << ""           << "&lceil;"    << tr("left ceiling")
        << QString::fromUtf8("\xe2\x8c\x89")    << ""           << "&rceil;"    << tr("right ceiling")
        << QString::fromUtf8("\xe2\x89\xa4")    << ""           << "&le;"       << tr("less-than or equal to")
        << QString::fromUtf8("\xe2\x89\xa5")    << ""           << "&ge;"       << tr("greater-than or equal to")
        << QString::fromUtf8("\xe2\x8c\x8a")    << ""           << "&lfloor;"   << tr("left floor")
        << QString::fromUtf8("\xe2\x8c\x8b")    << ""           << "&rfloor;"   << tr("right floor")
        << QString::fromUtf8("\xe2\x88\x97")    << ""           << "&lowast;"   << tr("asterisk operator")
        << QString::fromUtf8("\xe2\x97\x8a")    << ""           << "&loz;"      << tr("lozenge")
        << QString::fromUtf8("\xc2\xaf")        << ""           << "&macr;"     << tr("macron")
        << QString::fromUtf8("\xe2\x88\x87")    << ""           << "&nabla;"    << tr("nabla")
        << QString::fromUtf8("\xe2\x89\xa0")    << ""           << "&ne;"       << tr("not equal to")
        << QString::fromUtf8("\xe2\x88\x8b")    << ""           << "&ni;"       << tr("contains as member")
        << QString::fromUtf8("\xc2\xac")        << ""           << "&not;"      << tr("not sign")
        << QString::fromUtf8("\xe2\x88\x89")    << ""           << "&notin;"    << tr("not an element of")
        << QString::fromUtf8("\xe2\x8a\x84")    << ""           << "&nsub;"     << tr("not a subset of")
        << QString::fromUtf8("\xe2\x80\xbe")    << ""           << "&oline;"    << tr("overline")
        << QString::fromUtf8("\xe2\x8a\x95")    << ""           << "&oplus;"    << tr("circled plus")
        << QString::fromUtf8("\xe2\x8a\x97")    << ""           << "&otimes;"   << tr("circled times")
        << QString::fromUtf8("\xe2\x88\x82")    << ""           << "&part;"     << tr("partial differential")
        << QString::fromUtf8("\xe2\x80\xb0")    << ""           << "&permil;"   << tr("per mille sign")
        << QString::fromUtf8("\xe2\x8a\xa5")    << ""           << "&perp;"     << tr("up tack")
        << QString::fromUtf8("\xcf\x96")        << ""           << "&piv;"      << tr("Greek pi symbol")
        << QString::fromUtf8("\xe2\x88\x8f")    << ""           << "&prod;"     << tr("n-ary product")
        << QString::fromUtf8("\xe2\x88\x9d")    << ""           << "&prop;"     << tr("proportional to")
        << QString::fromUtf8("\xe2\x88\x9a")    << ""           << "&radic;"    << tr("square root")
        << QString::fromUtf8("\xe2\x84\x9c")    << ""           << "&real;"     << tr("black-letter capital R")
        << QString::fromUtf8("\xe2\x8b\x85")    << ""           << "&sdot;"     << tr("dot operator")
        << QString::fromUtf8("\xcf\x82")        << ""           << "&sigmaf;"   << tr("Greek small letter final sigma")
        << QString::fromUtf8("\xe2\x88\xbc")    << ""           << "&sim;"      << tr("tilde operator")
        << QString::fromUtf8("\xe2\x8a\x82")    << ""           << "&sub;"      << tr("subset of")
        << QString::fromUtf8("\xe2\x8a\x83")    << ""           << "&sup;"      << tr("superset of")
        << QString::fromUtf8("\xe2\x8a\x86")    << ""           << "&sube;"     << tr("subset of or equal to")
        << QString::fromUtf8("\xe2\x8a\x87")    << ""           << "&supe;"     << tr("superset of or equal to")
        << QString::fromUtf8("\xe2\x88\x91")    << ""           << "&sum;"      << tr("n-ary summation")
        << QString::fromUtf8("\xc2\xb9")        << ""           << "&sup1;"     << tr("superscript one")
        << QString::fromUtf8("\xc2\xb2")        << ""           << "&sup2;"     << tr("superscript two")
        << QString::fromUtf8("\xc2\xb3")        << ""           << "&sup3;"     << tr("superscript three")
        << QString::fromUtf8("\xe2\x88\xb4")    << ""           << "&there4;"   << tr("therefore sign")
        << QString::fromUtf8("\xcf\x91")        << ""           << "&thetasym;" << tr("Greek theta symbol")
        << QString::fromUtf8("\xcf\x92")        << ""           << "&upsih;"    << tr("Greek Upsilon with hook symbol")
        << QString::fromUtf8("\xe2\x84\x98")    << ""           << "&weierp;"   << tr("script capital P")
        << QString::fromUtf8("\xc2\xa5")        << ""           << "&yen;"      << tr("yen sign")
    ;
    AddGrid(spaces, spaces.count());
    AddGrid(characters, 12);
    AddGrid(characters2, 10);
    AddGrid(characters3, 12);
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

void SelectCharacter::SetSelectedCharacter(const QString &text)
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
    connect(m_buttonMapper, SIGNAL(mapped(const QString &)), this, SLOT(SetSelectedCharacter(const QString &)));
}
