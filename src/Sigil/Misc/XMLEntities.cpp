/************************************************************************
**
**  Copyright (C) 2013  Dave Heiland
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

#include <QtCore/QString>
#include <QtCore/QStringList>

#include "Misc/XMLEntities.h"

XMLEntities *XMLEntities::m_instance = 0;

XMLEntities *XMLEntities::instance()
{
    if (m_instance == 0) {
        m_instance = new XMLEntities();
    }

    return m_instance;
}

QString XMLEntities::GetEntityName(ushort code)
{
    QString name;
    if (m_EntityName.contains(code)) {
        name = m_EntityName.value(code, QString());
    }
    return name;
}

QString XMLEntities::GetEntityDescription(ushort code)
{
    QString name;
    if (m_EntityDescription.contains(code)) {
        name = m_EntityDescription.value(code, QString());
    }
    return name;
}

XMLEntities::XMLEntities()
{
    SetXMLEntities();
}

void XMLEntities::SetXMLEntities()
{
    if (!m_EntityName.isEmpty()) {
        return;
    }

    QStringList data;
    data <<
    "34" << "quot" << tr("quotation mark") <<
    "38" << "amp" << tr("ampersand") <<
    "39" << "apos" << tr("apostrophe") <<
    "60" << "lt" << tr("less-than sign") <<
    "62" << "gt" << tr("greater-than sign") <<
    "160" << "nbsp" << tr("no-break space") <<
    "161" << "iexcl" << tr("inverted exclamation mark") <<
    "162" << "cent" << tr("cent sign") <<
    "163" << "pound" << tr("pound sign") <<
    "164" << "curren" << tr("currency sign") <<
    "165" << "yen" << tr("yen sign") <<
    "166" << "brvbar" << tr("broken bar") <<
    "167" << "sect" << tr("section sign") <<
    "168" << "uml" << tr("diaeresis") <<
    "169" << "copy" << tr("copyright symbol") <<
    "170" << "ordf" << tr("feminine ordinal indicator") <<
    "171" << "laquo" << tr("left-pointing double angle quotation mark") <<
    "172" << "not" << tr("not sign") <<
    "173" << "shy" << tr("soft hyphen") <<
    "174" << "reg" << tr("registered sign") <<
    "175" << "macr" << tr("macron") <<
    "176" << "deg" << tr("degree symbol") <<
    "177" << "plusmn" << tr("plus-minus sign") <<
    "178" << "sup2" << tr("superscript two") <<
    "179" << "sup3" << tr("superscript three") <<
    "180" << "acute" << tr("acute accent") <<
    "181" << "micro" << tr("micro sign") <<
    "182" << "para" << tr("pilcrow sign") <<
    "183" << "middot" << tr("middle dot") <<
    "184" << "cedil" << tr("cedilla") <<
    "185" << "sup1" << tr("superscript one") <<
    "186" << "ordm" << tr("masculine ordinal indicator") <<
    "187" << "raquo" << tr("right-pointing double angle quotation mark") <<
    "188" << "frac14" << tr("vulgar fraction one quarter") <<
    "189" << "frac12" << tr("vulgar fraction one half") <<
    "190" << "frac34" << tr("vulgar fraction three quarters") <<
    "191" << "iquest" << tr("inverted question mark") <<
    "192" << "Agrave" << tr("Latin capital letter A with grave accent") <<
    "193" << "Aacute" << tr("Latin capital letter A with acute accent") <<
    "194" << "Acirc" << tr("Latin capital letter A with circumflex") <<
    "195" << "Atilde" << tr("Latin capital letter A with tilde") <<
    "196" << "Auml" << tr("Latin capital letter A with diaeresis") <<
    "197" << "Aring" << tr("Latin capital letter A with ring above") <<
    "198" << "AElig" << tr("Latin capital letter AE") <<
    "199" << "Ccedil" << tr("Latin capital letter C with cedilla") <<
    "200" << "Egrave" << tr("Latin capital letter E with grave accent") <<
    "201" << "Eacute" << tr("Latin capital letter E with acute accent") <<
    "202" << "Ecirc" << tr("Latin capital letter E with circumflex") <<
    "203" << "Euml" << tr("Latin capital letter E with diaeresis") <<
    "204" << "Igrave" << tr("Latin capital letter I with grave accent") <<
    "205" << "Iacute" << tr("Latin capital letter I with acute accent") <<
    "206" << "Icirc" << tr("Latin capital letter I with circumflex") <<
    "207" << "Iuml" << tr("Latin capital letter I with diaeresis") <<
    "208" << "ETH" << tr("Latin capital letter Eth") <<
    "209" << "Ntilde" << tr("Latin capital letter N with tilde") <<
    "210" << "Ograve" << tr("Latin capital letter O with grave accent") <<
    "211" << "Oacute" << tr("Latin capital letter O with acute accent") <<
    "212" << "Ocirc" << tr("Latin capital letter O with circumflex") <<
    "213" << "Otilde" << tr("Latin capital letter O with tilde") <<
    "214" << "Ouml" << tr("Latin capital letter O with diaeresis") <<
    "215" << "times" << tr("multiplication sign") <<
    "216" << "Oslash" << tr("Latin capital letter O with stroke") <<
    "217" << "Ugrave" << tr("Latin capital letter U with grave accent") <<
    "218" << "Uacute" << tr("Latin capital letter U with acute accent") <<
    "219" << "Ucirc" << tr("Latin capital letter U with circumflex") <<
    "220" << "Uuml" << tr("Latin capital letter U with diaeresis") <<
    "221" << "Yacute" << tr("Latin capital letter Y with acute accent") <<
    "222" << "THORN" << tr("Latin capital letter THORN") <<
    "223" << "szlig" << tr("Latin small letter sharp s") <<
    "224" << "agrave" << tr("Latin small letter a with grave accent") <<
    "225" << "aacute" << tr("Latin small letter a with acute accent") <<
    "226" << "acirc" << tr("Latin small letter a with circumflex") <<
    "227" << "atilde" << tr("Latin small letter a with tilde") <<
    "228" << "auml" << tr("Latin small letter a with diaeresis") <<
    "229" << "aring" << tr("Latin small letter a with ring above") <<
    "230" << "aelig" << tr("Latin small letter ae") <<
    "231" << "ccedil" << tr("Latin small letter c with cedilla") <<
    "232" << "egrave" << tr("Latin small letter e with grave accent") <<
    "233" << "eacute" << tr("Latin small letter e with acute accent") <<
    "234" << "ecirc" << tr("Latin small letter e with circumflex") <<
    "235" << "euml" << tr("Latin small letter e with diaeresis") <<
    "236" << "igrave" << tr("Latin small letter i with grave accent") <<
    "237" << "iacute" << tr("Latin small letter i with acute accent") <<
    "238" << "icirc" << tr("Latin small letter i with circumflex") <<
    "239" << "iuml" << tr("Latin small letter i with diaeresis") <<
    "240" << "eth" << tr("Latin small letter eth") <<
    "241" << "ntilde" << tr("Latin small letter n with tilde") <<
    "242" << "ograve" << tr("Latin small letter o with grave accent") <<
    "243" << "oacute" << tr("Latin small letter o with acute accent") <<
    "244" << "ocirc" << tr("Latin small letter o with circumflex") <<
    "245" << "otilde" << tr("Latin small letter o with tilde") <<
    "246" << "ouml" << tr("Latin small letter o with diaeresis") <<
    "247" << "divide" << tr("division sign") <<
    "248" << "oslash" << tr("Latin small letter o with stroke") <<
    "249" << "ugrave" << tr("Latin small letter u with grave accent") <<
    "250" << "uacute" << tr("Latin small letter u with acute accent") <<
    "251" << "ucirc" << tr("Latin small letter u with circumflex") <<
    "252" << "uuml" << tr("Latin small letter u with diaeresis") <<
    "253" << "yacute" << tr("Latin small letter y with acute accent") <<
    "254" << "thorn" << tr("Latin small letter thorn") <<
    "255" << "yuml" << tr("Latin small letter y with diaeresis") <<
    "338" << "OElig" << tr("Latin capital ligature oe") <<
    "339" << "oelig" << tr("Latin small ligature oe") <<
    "352" << "Scaron" << tr("Latin capital letter s with caron") <<
    "353" << "scaron" << tr("Latin small letter s with caron") <<
    "376" << "Yuml" << tr("Latin capital letter y with diaeresis") <<
    "402" << "fnof" << tr("Latin small letter f with hook") <<
    "710" << "circ" << tr("modifier letter circumflex accent") <<
    "732" << "tilde" << tr("small tilde") <<
    "913" << "Alpha" << tr("Greek capital letter Alpha") <<
    "914" << "Beta" << tr("Greek capital letter Beta") <<
    "915" << "Gamma" << tr("Greek capital letter Gamma") <<
    "916" << "Delta" << tr("Greek capital letter Delta") <<
    "917" << "Epsilon" << tr("Greek capital letter Epsilon") <<
    "918" << "Zeta" << tr("Greek capital letter Zeta") <<
    "919" << "Eta" << tr("Greek capital letter Eta") <<
    "920" << "Theta" << tr("Greek capital letter Theta") <<
    "921" << "Iota" << tr("Greek capital letter Iota") <<
    "922" << "Kappa" << tr("Greek capital letter Kappa") <<
    "923" << "Lambda" << tr("Greek capital letter Lambda") <<
    "924" << "Mu" << tr("Greek capital letter Mu") <<
    "925" << "Nu" << tr("Greek capital letter Nu") <<
    "926" << "Xi" << tr("Greek capital letter Xi") <<
    "927" << "Omicron" << tr("Greek capital letter Omicron") <<
    "928" << "Pi" << tr("Greek capital letter Pi") <<
    "929" << "Rho" << tr("Greek capital letter Rho") <<
    "931" << "Sigma" << tr("Greek capital letter Sigma") <<
    "932" << "Tau" << tr("Greek capital letter Tau") <<
    "933" << "Upsilon" << tr("Greek capital letter Upsilon") <<
    "934" << "Phi" << tr("Greek capital letter Phi") <<
    "935" << "Chi" << tr("Greek capital letter Chi") <<
    "936" << "Psi" << tr("Greek capital letter Psi") <<
    "937" << "Omega" << tr("Greek capital letter Omega") <<
    "945" << "alpha" << tr("Greek small letter alpha") <<
    "946" << "beta" << tr("Greek small letter beta") <<
    "947" << "gamma" << tr("Greek small letter gamma") <<
    "948" << "delta" << tr("Greek small letter delta") <<
    "949" << "epsilon" << tr("Greek small letter epsilon") <<
    "950" << "zeta" << tr("Greek small letter zeta") <<
    "951" << "eta" << tr("Greek small letter eta") <<
    "952" << "theta" << tr("Greek small letter theta") <<
    "953" << "iota" << tr("Greek small letter iota") <<
    "954" << "kappa" << tr("Greek small letter kappa") <<
    "955" << "lambda" << tr("Greek small letter lambda") <<
    "956" << "mu" << tr("Greek small letter mu") <<
    "957" << "nu" << tr("Greek small letter nu") <<
    "958" << "xi" << tr("Greek small letter xi") <<
    "959" << "omicron" << tr("Greek small letter omicron") <<
    "960" << "pi" << tr("Greek small letter pi") <<
    "961" << "rho" << tr("Greek small letter rho") <<
    "962" << "sigmaf" << tr("Greek small letter final sigma") <<
    "963" << "sigma" << tr("Greek small letter sigma") <<
    "964" << "tau" << tr("Greek small letter tau") <<
    "965" << "upsilon" << tr("Greek small letter upsilon") <<
    "966" << "phi" << tr("Greek small letter phi") <<
    "967" << "chi" << tr("Greek small letter chi") <<
    "968" << "psi" << tr("Greek small letter psi") <<
    "969" << "omega" << tr("Greek small letter omega") <<
    "977" << "thetasym" << tr("Greek theta symbol") <<
    "978" << "upsih" << tr("Greek Upsilon with hook symbol") <<
    "982" << "piv" << tr("Greek pi symbol") <<
    "8194" << "ensp" << tr("en space") <<
    "8195" << "emsp" << tr("em space") <<
    "8201" << "thinsp" << tr("thin space") <<
    "8204" << "zwnj" << tr("zero-width non-joiner") <<
    "8205" << "zwj" << tr("zero-width joiner") <<
    "8206" << "lrm" << tr("left-to-right mark") <<
    "8207" << "rlm" << tr("right-to-left mark") <<
    "8211" << "ndash" << tr("en dash") <<
    "8212" << "mdash" << tr("em dash") <<
    "8216" << "lsquo" << tr("left single quotation mark") <<
    "8217" << "rsquo" << tr("right single quotation mark") <<
    "8218" << "sbquo" << tr("single low-9 quotation mark") <<
    "8220" << "ldquo" << tr("left double quotation mark") <<
    "8221" << "rdquo" << tr("right double quotation mark") <<
    "8222" << "bdquo" << tr("double low-9 quotation mark") <<
    "8224" << "dagger" << tr("dagger, obelisk") <<
    "8225" << "Dagger" << tr("double dagger, double obelisk") <<
    "8226" << "bull" << tr("bullet") <<
    "8230" << "hellip" << tr("horizontal ellipsis") <<
    "8240" << "permil" << tr("per mille sign") <<
    "8242" << "prime" << tr("prime") <<
    "8243" << "Prime" << tr("double prime") <<
    "8249" << "lsaquo" << tr("single left-pointing angle quotation mark") <<
    "8250" << "rsaquo" << tr("single right-pointing angle quotation mark") <<
    "8254" << "oline" << tr("overline") <<
    "8260" << "frasl" << tr("fraction slash") <<
    "8364" << "euro" << tr("euro sign") <<
    "8465" << "image" << tr("black-letter capital I") <<
    "8472" << "weierp" << tr("script capital P") <<
    "8476" << "real" << tr("black-letter capital R") <<
    "8482" << "trade" << tr("trademark symbol") <<
    "8501" << "alefsym" << tr("alef symbol") <<
    "8592" << "larr" << tr("leftwards arrow") <<
    "8593" << "uarr" << tr("upwards arrow") <<
    "8594" << "rarr" << tr("rightwards arrow") <<
    "8595" << "darr" << tr("downwards arrow") <<
    "8596" << "harr" << tr("left right arrow") <<
    "8629" << "crarr" << tr("downwards arrow with corner leftwards") <<
    "8656" << "lArr" << tr("leftwards double arrow") <<
    "8657" << "uArr" << tr("upwards double arrow") <<
    "8658" << "rArr" << tr("rightwards double arrow") <<
    "8659" << "dArr" << tr("downwards double arrow") <<
    "8660" << "hArr" << tr("left right double arrow") <<
    "8704" << "forall" << tr("for all") <<
    "8706" << "part" << tr("partial differential") <<
    "8707" << "exist" << tr("there exists") <<
    "8709" << "empty" << tr("empty set") <<
    "8711" << "nabla" << tr("nabla") <<
    "8712" << "isin" << tr("element of") <<
    "8713" << "notin" << tr("not an element of") <<
    "8715" << "ni" << tr("contains as member") <<
    "8719" << "prod" << tr("n-ary product") <<
    "8721" << "sum" << tr("n-ary summation") <<
    "8722" << "minus" << tr("minus sign") <<
    "8727" << "lowast" << tr("asterisk operator") <<
    "8730" << "radic" << tr("square root") <<
    "8733" << "prop" << tr("proportional to") <<
    "8734" << "infin" << tr("infinity") <<
    "8736" << "ang" << tr("angle") <<
    "8743" << "and" << tr("logical and") <<
    "8744" << "or" << tr("logical or") <<
    "8745" << "cap" << tr("intersection") <<
    "8746" << "cup" << tr("union") <<
    "8747" << "int" << tr("integral") <<
    "8756" << "there4" << tr("therefore sign") <<
    "8764" << "sim" << tr("tilde operator") <<
    "8773" << "cong" << tr("congruent to") <<
    "8776" << "asymp" << tr("almost equal to") <<
    "8800" << "ne" << tr("not equal to") <<
    "8801" << "equiv" << tr("identical to") <<
    "8804" << "le" << tr("less-than or equal to") <<
    "8805" << "ge" << tr("greater-than or equal to") <<
    "8834" << "sub" << tr("subset of") <<
    "8835" << "sup" << tr("superset of") <<
    "8836" << "nsub" << tr("not a subset of") <<
    "8838" << "sube" << tr("subset of or equal to") <<
    "8839" << "supe" << tr("superset of or equal to") <<
    "8853" << "oplus" << tr("circled plus") <<
    "8855" << "otimes" << tr("circled times") <<
    "8869" << "perp" << tr("up tack") <<
    "8901" << "sdot" << tr("dot operator") <<
    "8968" << "lceil" << tr("left ceiling") <<
    "8969" << "rceil" << tr("right ceiling") <<
    "8970" << "lfloor" << tr("left floor") <<
    "8971" << "rfloor" << tr("right floor") <<
    "9001" << "lang" << tr("left-pointing angle bracket") <<
    "9002" << "rang" << tr("right-pointing angle bracket") <<
    "9674" << "loz" << tr("lozenge") <<
    "9824" << "spades" << tr("black spade suit") <<
    "9827" << "clubs" << tr("black club suit") <<
    "9829" << "hearts" << tr("black heart suit") <<
    "9830" << "diams" << tr("black diamond suit");

    for (int i = 0; i < data.count(); i++) {
        ushort code = data.at(i++).toInt();
        QString name = data.at(i++);
        QString description = data.at(i);
        m_EntityName.insert(code, name);
        m_EntityDescription.insert(code, description);
    }
}
