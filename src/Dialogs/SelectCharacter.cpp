/************************************************************************
**
**  Copyright (C) 2018 Kevin B. Hendricks, Stratford, ON Canada
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

#include <QtWidgets/QCompleter>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QApplication>
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
        << QString::fromUtf8("\xc2\xa0")        << "nbsp"       << "&nbsp;"   << "non-breaking space &#xa0;"
        << QString::fromUtf8("\xe2\x80\x82")    << "ensp"       << "&ensp;"   << "en space &#x2002;"
        << QString::fromUtf8("\xe2\x80\x83")    << "emsp"       << "&emsp;"   << "em space &#x2003;"
        << QString::fromUtf8("\xe2\x80\x89")    << "thinsp"     << "&thinsp;" << "thin space &#x2009;"
        << "&#173;"                             << "shy"        << "&shy;"    << "soft hyphen &#xad;"
        << QString::fromUtf8("\xe2\x80\xaf")    << "nnbsp"      << "&#x202f;" << "narrow non-breaking space &#x202f"
    ;

    QStringList characters = QStringList()
        << QString::fromUtf8("\xe2\x80\x98")    << ""           << "&lsquo;"    << "left single quote"
        << QString::fromUtf8("\xe2\x80\x99")    << ""           << "&rsquo;"    << "right single quote"
        << QString::fromUtf8("\xe2\x80\x9C")    << ""           << "&ldquo;"    << "left double quote"
        << QString::fromUtf8("\xe2\x80\x9D")    << ""           << "&rdquo;"    << "right double quote"
        << QString::fromUtf8("\xe2\x80\xB9")    << ""           << "&lsaquo;"   << "left-pointing single angle quote"
        << QString::fromUtf8("\xe2\x80\xBA")    << ""           << "&rsaquo;"   << "right-pointing single angle quote"
        << QString::fromUtf8("\xc2\xab")        << ""           << "&laquo;"    << "left-pointing double angle quote"
        << QString::fromUtf8("\xc2\xbb")        << ""           << "&raquo;"    << "right-pointing double angle quote"
        << "'"                                  << ""           << "&apos;"     << "apostrophe"
        << "\""                                 << ""           << "&quot;"     << "double quote"
        << QString::fromUtf8("\xe2\x80\x9A")    << ""           << "&sbquo;"    << "single low-9 quote"
        << QString::fromUtf8("\xe2\x80\x9E")    << ""           << "&bdquo;"    << "double low-9 quote"
        << QString::fromUtf8("\xe2\x80\x94")    << ""           << "&mdash;"    << "em dash"
        << QString::fromUtf8("\xe2\x80\x93")    << ""           << "&ndash;"    << "en dash"
        << QString::fromUtf8("\xc2\xa7")        << ""           << "&sect;"     << "section sign"
        << QString::fromUtf8("\xc2\xb6")        << ""           << "&para;"     << "pilcrow - paragraph sign"
        << QString::fromUtf8("\xe2\x80\xa0")    << ""           << "&dagger;"   << "dagger"
        << QString::fromUtf8("\xe2\x80\xa1")    << ""           << "&Dagger;"   << "double dagger"
        << "&amp;"                              << "&&"         << "&amp;"      << "ampersand"
        << "&lt;"                               << "<"          << "&lt;"       << "less-than sign"
        << "&gt;"                               << ">"          << "&gt;"       << "greater-than sign"
        << QString::fromUtf8("\xc2\xa9")        << ""           << "&copy;"     << "copyright"
        << QString::fromUtf8("\xc2\xae")        << ""           << "&reg;"      << "registered sign"
        << QString::fromUtf8("\xe2\x84\xa2")    << ""           << "&trade;"    << "trademark symbol"
        << QString::fromUtf8("\xe2\x86\x90")    << ""           << "&larr;"     << "left arrow"
        << QString::fromUtf8("\xe2\x86\x92")    << ""           << "&rarr;"     << "right arrow"
        << QString::fromUtf8("\xe2\x80\xa2")    << ""           << "&bull;"     << "bullet"
        << QString::fromUtf8("\xc2\xb7")        << ""           << "&middot;"   << "middle dot"
        << QString::fromUtf8("\xc2\xb0")        << ""           << "&deg;"      << "degree sign"
        << QString::fromUtf8("\xc2\xb1")        << ""           << "&plusmn;"   << "plus minus sign"
        << QString::fromUtf8("\xe2\x88\x92")    << ""           << "&minus;"    << "minus sign"
        << QString::fromUtf8("\xc3\x97")        << ""           << "&times;"    << "multiplication sign"
        << QString::fromUtf8("\xc3\xb7")        << ""           << "&divide;"   << "division sign"
        << QString::fromUtf8("\xc2\xbc")        << ""           << "&frac14;"   << "fraction 1/4"
        << QString::fromUtf8("\xc2\xbd")        << ""           << "&frac12;"   << "fraction 1/2"
        << QString::fromUtf8("\xc2\xbe")        << ""           << "&frac34;"   << "fraction 3/4"
        << QString::fromUtf8("\xe2\x85\x93")    << ""           << "&#8531;"    << "fraction 1/3"
        << QString::fromUtf8("\xe2\x85\x94")    << ""           << "&#8532;"    << "fraction 2/3"
        << QString::fromUtf8("\xe2\x85\x9b")    << ""           << "&#8539;"    << "fraction 1/8"
        << QString::fromUtf8("\xe2\x85\x9c")    << ""           << "&#8540;"    << "fraction 3/8"
        << QString::fromUtf8("\xe2\x85\x9d")    << ""           << "&#8541;"    << "fraction 5/8"
        << QString::fromUtf8("\xe2\x85\x9e")    << ""           << "&#8542;"    << "fraction 7/8"
        << QString::fromUtf8("\xe2\x80\xa6")    << ""           << "&hellip;"   << "horizontal ellipsis"
        << QString::fromUtf8("\xc2\xb5")        << ""           << "&micro;"    << "micron"
        << QString::fromUtf8("\xc2\xa2")        << ""           << "&cent;"     << "cent sign"
        << QString::fromUtf8("\xc2\xa3")        << ""           << "&pound;"    << "pound sign"
        << QString::fromUtf8("\xe2\x82\xac")    << ""           << "&euro;"     << "euro sign"
        << QString::fromUtf8("\xc2\xbf")        << ""           << "&iquest;"   << "inverted question mark"
        << QString::fromUtf8("\xc2\xa1")        << ""           << "&iexcl;"    << "inverted exclamation mark"
        << QString::fromUtf8("\xc2\xa8")        << ""           << "&uml;"      << "diaeresis"
        << QString::fromUtf8("\xc2\xb4")        << ""           << "&acute;"    << "acute accent"
        << QString::fromUtf8("\xc2\xb8")        << ""           << "&cedil;"    << "cedilla"
        << QString::fromUtf8("\xcb\x86")        << ""           << "&circ;"     << "circumflex accent"
        << QString::fromUtf8("\xcb\x9c")        << ""           << "&tilde;"    << "small tilde"
        << QString::fromUtf8("\xc3\x80")        << ""           << "&Agrave;"   << "capital A with grave"
        << QString::fromUtf8("\xc3\x81")        << ""           << "&Aacute;"   << "capital A with acute"
        << QString::fromUtf8("\xc3\x82")        << ""           << "&Acirc;"    << "capital A with circumflex"
        << QString::fromUtf8("\xc3\x83")        << ""           << "&Atilde;"   << "capital A with tilde"
        << QString::fromUtf8("\xc3\x84")        << ""           << "&Auml;"     << "capital A with diaeresis"
        << QString::fromUtf8("\xc3\x85")        << ""           << "&Aring;"    << "capital A with ring above"
        << QString::fromUtf8("\xc3\x86")        << ""           << "&AElig;"    << "capital AE"
        << QString::fromUtf8("\xc3\x87")        << ""           << "&Ccedil;"   << "capital C with cedilla"
        << QString::fromUtf8("\xc3\x88")        << ""           << "&Egrave;"   << "capital E with grave"
        << QString::fromUtf8("\xc3\x89")        << ""           << "&Eacute;"   << "capital E with acute"
        << QString::fromUtf8("\xc3\x8a")        << ""           << "&Ecirc;"    << "capital E with circumflex"
        << QString::fromUtf8("\xc3\x8b")        << ""           << "&Euml;"     << "capital E with diaeresis"
        << QString::fromUtf8("\xc3\x8c")        << ""           << "&Igrave;"   << "capital I with grave"
        << QString::fromUtf8("\xc3\x8d")        << ""           << "&Iacute;"   << "capital I with acute"
        << QString::fromUtf8("\xc3\x8e")        << ""           << "&Icirc;"    << "capital I with circumflex"
        << QString::fromUtf8("\xc3\x8f")        << ""           << "&Iuml;"     << "capital I with diaeresis"
        << QString::fromUtf8("\xc3\x90")        << ""           << "&ETH;"      << "capital eth"
        << QString::fromUtf8("\xc3\x91")        << ""           << "&Ntilde;"   << "capital N with tilde"
        << QString::fromUtf8("\xc3\x92")        << ""           << "&Ograve;"   << "capital O with grave"
        << QString::fromUtf8("\xc3\x93")        << ""           << "&Oacute;"   << "capital O with acute"
        << QString::fromUtf8("\xc3\x94")        << ""           << "&Ocirc;"    << "capital O with circumflex"
        << QString::fromUtf8("\xc3\x95")        << ""           << "&Otilde;"   << "capital O with tilde"
        << QString::fromUtf8("\xc3\x96")        << ""           << "&Ouml;"     << "capital O with diaeresis"
        << QString::fromUtf8("\xc3\x98")        << ""           << "&Oslash;"   << "capital O with stroke"
        << QString::fromUtf8("\xc5\x92")        << ""           << "&OElig;"    << "capital ligature OE"
        << QString::fromUtf8("\xc5\xa0")        << ""           << "&Scaron;"   << "capital S with caron"
        << QString::fromUtf8("\xc3\x99")        << ""           << "&Ugrave;"   << "capital U with grave"
        << QString::fromUtf8("\xc3\x9a")        << ""           << "&Uacute;"   << "capital U with acute"
        << QString::fromUtf8("\xc3\x9b")        << ""           << "&Ucirc;"    << "capital U with circumflex"
        << QString::fromUtf8("\xc3\x9c")        << ""           << "&Uuml;"     << "capital U with diaeresis"
        << QString::fromUtf8("\xc3\x9d")        << ""           << "&Yacute;"   << "capital Y with acute"
        << QString::fromUtf8("\xc5\xb8")        << ""           << "&Yuml;"     << "capital Y with diaeresis"
        << QString::fromUtf8("\xc3\x9e")        << ""           << "&THORN;"    << "capital THORN"
        << QString::fromUtf8("\xc3\x9f")        << ""           << "&szlig;"    << "small sharp s"
        << QString::fromUtf8("\xc3\xa0")        << ""           << "&agrave;"   << "small a with grave"
        << QString::fromUtf8("\xc3\xa1")        << ""           << "&aacute;"   << "small a with acute"
        << QString::fromUtf8("\xc3\xa2")        << ""           << "&acirc;"    << "small a with circumflex"
        << QString::fromUtf8("\xc3\xa3")        << ""           << "&atilde;"   << "small a with tilde"
        << QString::fromUtf8("\xc3\xa4")        << ""           << "&auml;"     << "small a with diaresis"
        << QString::fromUtf8("\xc3\xa5")        << ""           << "&aring;"    << "small a with ring above"
        << QString::fromUtf8("\xc3\xa6")        << ""           << "&aelig;"    << "small ae"
        << QString::fromUtf8("\xc3\xa7")        << ""           << "&ccedil;"   << "small c with cedilia"
        << QString::fromUtf8("\xc3\xa8")        << ""           << "&egrave;"   << "small e with grave"
        << QString::fromUtf8("\xc3\xa9")        << ""           << "&eacute;"   << "small e with acute"
        << QString::fromUtf8("\xc3\xaa")        << ""           << "&ecirc;"    << "small e with circumflex"
        << QString::fromUtf8("\xc3\xab")        << ""           << "&euml;"     << "small e with diaeresis"
        << QString::fromUtf8("\xc3\xac")        << ""           << "&igrave;"   << "small i with grave"
        << QString::fromUtf8("\xc3\xad")        << ""           << "&iacute;"   << "small i with acute"
        << QString::fromUtf8("\xc3\xae")        << ""           << "&icirc;"    << "small i with circumflex"
        << QString::fromUtf8("\xc3\xaf")        << ""           << "&iuml;"     << "small i with diaeresis"
        << QString::fromUtf8("\xc3\xb0")        << ""           << "&eth;"      << "small eth"
        << QString::fromUtf8("\xc3\xb1")        << ""           << "&ntilde;"   << "small n with tilde"
        << QString::fromUtf8("\xc3\xb2")        << ""           << "&ograve;"   << "small o with grave"
        << QString::fromUtf8("\xc3\xb3")        << ""           << "&oacute;"   << "small o with acute"
        << QString::fromUtf8("\xc3\xb4")        << ""           << "&ocirc;"    << "small o with circumflex"
        << QString::fromUtf8("\xc3\xb5")        << ""           << "&otilde;"   << "small o with tilde"
        << QString::fromUtf8("\xc3\xb6")        << ""           << "&ouml;"     << "small o with diaeresis"
        << QString::fromUtf8("\xc3\xb8")        << ""           << "&oslash;"   << "small o with stroke"
        << QString::fromUtf8("\xc5\x93")        << ""           << "&oelig;"    << "small ligature oe"
        << QString::fromUtf8("\xc5\xa1")        << ""           << "&scaron;"   << "small s with caron"
        << QString::fromUtf8("\xc3\xb9")        << ""           << "&ugrave;"   << "small u with grave"
        << QString::fromUtf8("\xc3\xba")        << ""           << "&uacute;"   << "small u with acute"
        << QString::fromUtf8("\xc3\xbb")        << ""           << "&ucirc;"    << "small u with circumflex"
        << QString::fromUtf8("\xc3\xbc")        << ""           << "&uuml;"     << "small u with diaeresis"
        << QString::fromUtf8("\xc3\xbd")        << ""           << "&yacute;"   << "small y with acute"
        << QString::fromUtf8("\xc3\xbf")        << ""           << "&yuml;"     << "small y with diaeresis"
        << QString::fromUtf8("\xc3\xbe")        << ""           << "&thorn;"    << "small thorn"
        << QString::fromUtf8("\xc2\xaa")        << ""           << "&ordf;"     << "feminine ordinal indicator"
        << QString::fromUtf8("\xc2\xba")        << ""           << "&ordm;"     << "masculine ordinal indicator"
        << QString::fromUtf8("\xe2\x88\x9e")    << ""           << "&infin;"    << "infinity"
    ;
    QStringList characters2 = QStringList()
        << QString::fromUtf8("\xce\x91")        << ""           << "&Alpha;"    << "Greek capital letter Alpha"
        << QString::fromUtf8("\xce\xb1")        << ""           << "&alpha;"    << "Greek lower letter alpha"
        << QString::fromUtf8("\xce\x92")        << ""           << "&Beta;"     << "Greek capital letter Beta"
        << QString::fromUtf8("\xce\xb2")        << ""           << "&beta;"     << "Greek lower letter beta"
        << QString::fromUtf8("\xce\xa7")        << ""           << "&Chi;"      << "Greek capital letter Chi"
        << QString::fromUtf8("\xcf\x87")        << ""           << "&chi;"      << "Greek lower letter chi"
        << QString::fromUtf8("\xce\x94")        << ""           << "&Delta;"    << "Greek capital letter Delta"
        << QString::fromUtf8("\xce\xb4")        << ""           << "&delta;"    << "Greek lower letter delta"
        << QString::fromUtf8("\xce\x95")        << ""           << "&Epsilon;"  << "Greek capital letter Epsilon"
        << QString::fromUtf8("\xce\xb5")        << ""           << "&epsilon;"  << "Greek lower letter epsilon"
        << QString::fromUtf8("\xce\x97")        << ""           << "&Eta;"      << "Greek capital letter Eta"
        << QString::fromUtf8("\xce\xb7")        << ""           << "&eta;"      << "Greek lower letter eta"
        << QString::fromUtf8("\xce\x93")        << ""           << "&Gamma;"    << "Greek capital letter Gamma"
        << QString::fromUtf8("\xce\xb3")        << ""           << "&gamma;"    << "Greek lower letter gamma"
        << QString::fromUtf8("\xce\x99")        << ""           << "&Iota;"     << "Greek capital letter Iota"
        << QString::fromUtf8("\xce\xb9")        << ""           << "&iota;"     << "Greek lower letter iota"
        << QString::fromUtf8("\xce\x9a")        << ""           << "&Kappa;"    << "Greek capital letter Kappa"
        << QString::fromUtf8("\xce\xba")        << ""           << "&kappa;"    << "Greek lower letter kappa"
        << QString::fromUtf8("\xce\x9b")        << ""           << "&Lambda;"   << "Greek capital letter Lambda"
        << QString::fromUtf8("\xce\xbb")        << ""           << "&lambda;"   << "Greek lower letter lambda"
        << QString::fromUtf8("\xce\x9c")        << ""           << "&Mu;"       << "Greek capital letter Mu"
        << QString::fromUtf8("\xce\xbc")        << ""           << "&mu;"       << "Greek lower letter mu"
        << QString::fromUtf8("\xce\x9d")        << ""           << "&Nu;"       << "Greek capital letter Nu"
        << QString::fromUtf8("\xce\xbd")        << ""           << "&nu;"       << "Greek lower letter mu"
        << QString::fromUtf8("\xce\xa9")        << ""           << "&Omega;"    << "Greek capital letter Omega"
        << QString::fromUtf8("\xcf\x89")        << ""           << "&omega;"    << "Greek lower letter omega"
        << QString::fromUtf8("\xce\x9f")        << ""           << "&Omicron;"  << "Greek capital letter Omicron"
        << QString::fromUtf8("\xce\xbf")        << ""           << "&omicron;"  << "Greek lower letter omicron"
        << QString::fromUtf8("\xce\xa6")        << ""           << "&Phi;"      << "Greek capital letter Phi"
        << QString::fromUtf8("\xcf\x86")        << ""           << "&phi;"      << "Greek lower letter phi"
        << QString::fromUtf8("\xce\xa0")        << ""           << "&Pi;"       << "Greek capital letter Pi"
        << QString::fromUtf8("\xcf\x80")        << ""           << "&pi;"       << "Greek lower letter pi"
        << QString::fromUtf8("\xe2\x80\xb3")    << ""           << "&Prime;"    << "Greek double prime"
        << QString::fromUtf8("\xe2\x80\xb2")    << ""           << "&prime;"    << "Greek single prime"
        << QString::fromUtf8("\xce\xa8")        << ""           << "&Psi;"      << "Greek capital letter Psi"
        << QString::fromUtf8("\xcf\x88")        << ""           << "&psi;"      << "Greek lower letter psi"
        << QString::fromUtf8("\xce\xa1")        << ""           << "&Rho;"      << "Greek capital letter Rho"
        << QString::fromUtf8("\xcf\x81")        << ""           << "&rho;"      << "Greek lower letter rho"
        << QString::fromUtf8("\xce\xa3")        << ""           << "&Sigma;"    << "Greek capital letter Sigma"
        << QString::fromUtf8("\xcf\x83")        << ""           << "&sigma;"    << "Greek lower letter sigma"
        << QString::fromUtf8("\xce\xa4")        << ""           << "&Tau;"      << "Greek capital letter Tau"
        << QString::fromUtf8("\xcf\x84")        << ""           << "&tau;"      << "Greek lower letter tau"
        << QString::fromUtf8("\xce\x98")        << ""           << "&Theta;"    << "Greek capital letter Theta"
        << QString::fromUtf8("\xce\xb8")        << ""           << "&theta;"    << "Greek lower letter theta"
        << QString::fromUtf8("\xce\xa5")        << ""           << "&Upsilon;"  << "Greek capital letter Upsilon"
        << QString::fromUtf8("\xcf\x85")        << ""           << "&upsilon;"  << "Greek lower letter upsilon"
        << QString::fromUtf8("\xce\x9e")        << ""           << "&Xi;"       << "Greek capital letter Xi"
        << QString::fromUtf8("\xce\xbe")        << ""           << "&xi;"       << "Greek lower letter xi"
        << QString::fromUtf8("\xce\x96")        << ""           << "&Zeta;"     << "Greek capital letter Zeta"
        << QString::fromUtf8("\xce\xb6")        << ""           << "&zeta;"     << "Greek lower letter zeta"
    ;

    QStringList characters3 = QStringList()
        << QString::fromUtf8("\xe2\x84\xb5")    << ""           << "&alefsym;"  << "alef symbol"
        << QString::fromUtf8("\xe2\x88\xa7")    << ""           << "&and;"      << "logical and"
        << QString::fromUtf8("\xe2\x88\xa8")    << ""           << "&or;"       << "logical or"
        << QString::fromUtf8("\xe2\x88\xa9")    << ""           << "&cap;"      << "intersection"
        << QString::fromUtf8("\xe2\x88\xaa")    << ""           << "&cup;"      << "union"
        << QString::fromUtf8("\xe2\x89\x85")    << ""           << "&cong;"     << "congruent to"
        << QString::fromUtf8("\xe2\x86\xb5")    << ""           << "&crarr;"    << "downwards arrow with corner leftwards"
        << QString::fromUtf8("\xc2\xa4")        << ""           << "&curren;"   << "currency sign"
        << QString::fromUtf8("\xe2\x87\x93")    << ""           << "&dArr;"     << "downwards double arrow"
        << QString::fromUtf8("\xe2\x87\x91")    << ""           << "&uArr;"     << "upwards double arrow"
        << QString::fromUtf8("\xe2\x86\x93")    << ""           << "&darr;"     << "downwards arrow"
        << QString::fromUtf8("\xe2\x86\x91")    << ""           << "&uarr;"     << "upwards arrow"
        << QString::fromUtf8("\xe2\x88\x85")    << ""           << "&empty;"    << "empty set"
        << QString::fromUtf8("\xe2\x89\xa1")    << ""           << "&equiv;"    << "identical to"
        << QString::fromUtf8("\xe2\x88\x83")    << ""           << "&exist;"    << "there exists"
        << QString::fromUtf8("\xc6\x92")        << ""           << "&fnof;"     << "Latin small letter f with hook"
        << QString::fromUtf8("\xe2\x88\x80")    << ""           << "&forall;"   << "for all"
        << QString::fromUtf8("\xe2\x81\x84")    << ""           << "&frasl;"    << "fraction slash"
        << QString::fromUtf8("\xe2\x87\x94")    << ""           << "&hArr;"     << "left right double arrow"
        << QString::fromUtf8("\xe2\x86\x94")    << ""           << "&harr;"     << "left right single arrow"
        << QString::fromUtf8("\xe2\x84\x91")    << ""           << "&image;"    << "black-letter capital I"
        << QString::fromUtf8("\xe2\x88\xab")    << ""           << "&int;"      << "integral"
        << QString::fromUtf8("\xe2\x88\x88")    << ""           << "&isin;"     << "element of"
        << QString::fromUtf8("\xe2\x87\x90")    << ""           << "&lArr;"     << "leftwards double arrow"
        << QString::fromUtf8("\xe2\x87\x92")    << ""           << "&rArr;"     << "double right arrow"
        << QString::fromUtf8("\xe2\x8c\xa9")    << ""           << "&lang;"     << "left-pointing angle bracket"
        << QString::fromUtf8("\xe2\x8c\xaa")    << ""           << "&rang;"     << "right-pointing angle bracket"
        << QString::fromUtf8("\xe2\x8c\x88")    << ""           << "&lceil;"    << "left ceiling"
        << QString::fromUtf8("\xe2\x8c\x89")    << ""           << "&rceil;"    << "right ceiling"
        << QString::fromUtf8("\xe2\x89\xa4")    << ""           << "&le;"       << "less-than or equal to"
        << QString::fromUtf8("\xe2\x89\xa5")    << ""           << "&ge;"       << "greater-than or equal to"
        << QString::fromUtf8("\xe2\x8c\x8a")    << ""           << "&lfloor;"   << "left floor"
        << QString::fromUtf8("\xe2\x8c\x8b")    << ""           << "&rfloor;"   << "right floor"
        << QString::fromUtf8("\xe2\x88\x97")    << ""           << "&lowast;"   << "asterisk operator"
        << QString::fromUtf8("\xe2\x97\x8a")    << ""           << "&loz;"      << "lozenge"
        << QString::fromUtf8("\xc2\xaf")        << ""           << "&macr;"     << "macron"
        << QString::fromUtf8("\xe2\x88\x87")    << ""           << "&nabla;"    << "nabla"
        << QString::fromUtf8("\xe2\x89\xa0")    << ""           << "&ne;"       << "not equal to"
        << QString::fromUtf8("\xe2\x88\x8b")    << ""           << "&ni;"       << "contains as member"
        << QString::fromUtf8("\xc2\xac")        << ""           << "&not;"      << "not sign"
        << QString::fromUtf8("\xe2\x88\x89")    << ""           << "&notin;"    << "not an element of"
        << QString::fromUtf8("\xe2\x8a\x84")    << ""           << "&nsub;"     << "not a subset of"
        << QString::fromUtf8("\xe2\x80\xbe")    << ""           << "&oline;"    << "overline"
        << QString::fromUtf8("\xe2\x8a\x95")    << ""           << "&oplus;"    << "circled plus"
        << QString::fromUtf8("\xe2\x8a\x97")    << ""           << "&otimes;"   << "circled times"
        << QString::fromUtf8("\xe2\x88\x82")    << ""           << "&part;"     << "partial differential"
        << QString::fromUtf8("\xe2\x80\xb0")    << ""           << "&permil;"   << "per mille sign"
        << QString::fromUtf8("\xe2\x8a\xa5")    << ""           << "&perp;"     << "up tack"
        << QString::fromUtf8("\xcf\x96")        << ""           << "&piv;"      << "Greek pi symbol"
        << QString::fromUtf8("\xe2\x88\x8f")    << ""           << "&prod;"     << "n-ary product"
        << QString::fromUtf8("\xe2\x88\x9d")    << ""           << "&prop;"     << "proportional to"
        << QString::fromUtf8("\xe2\x88\x9a")    << ""           << "&radic;"    << "square root"
        << QString::fromUtf8("\xe2\x84\x9c")    << ""           << "&real;"     << "black-letter capital R"
        << QString::fromUtf8("\xe2\x8b\x85")    << ""           << "&sdot;"     << "dot operator"
        << QString::fromUtf8("\xcf\x82")        << ""           << "&sigmaf;"   << "Greek small letter final sigma"
        << QString::fromUtf8("\xe2\x88\xbc")    << ""           << "&sim;"      << "tilde operator"
        << QString::fromUtf8("\xe2\x8a\x82")    << ""           << "&sub;"      << "subset of"
        << QString::fromUtf8("\xe2\x8a\x83")    << ""           << "&sup;"      << "superset of"
        << QString::fromUtf8("\xe2\x8a\x86")    << ""           << "&sube;"     << "subset of or equal to"
        << QString::fromUtf8("\xe2\x8a\x87")    << ""           << "&supe;"     << "superset of or equal to"
        << QString::fromUtf8("\xe2\x88\x91")    << ""           << "&sum;"      << "n-ary summation"
        << QString::fromUtf8("\xc2\xb9")        << ""           << "&sup1;"     << "superscript one"
        << QString::fromUtf8("\xc2\xb2")        << ""           << "&sup2;"     << "superscript two"
        << QString::fromUtf8("\xc2\xb3")        << ""           << "&sup3;"     << "superscript three"
        << QString::fromUtf8("\xe2\x88\xb4")    << ""           << "&there4;"   << "therefore sign"
        << QString::fromUtf8("\xcf\x91")        << ""           << "&thetasym;" << "Greek theta symbol"
        << QString::fromUtf8("\xcf\x92")        << ""           << "&upsih;"    << "Greek Upsilon with hook symbol"
        << QString::fromUtf8("\xe2\x84\x98")    << ""           << "&weierp;"   << "script capital P"
        << QString::fromUtf8("\xc2\xa5")        << ""           << "&yen;"      << "yen sign"
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
