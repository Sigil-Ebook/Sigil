/************************************************************************
**
**  Copyright (C) 2011  John Schember <john@nachtimwald.com>
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

#include "Misc/Language.h"

Language *Language::m_instance = 0;

Language *Language::instance()
{
    if ( m_instance == 0 ) {
        m_instance = new Language();
    }

    return m_instance;
}

QString Language::GetLanguageName(QString code)
{
    return m_languageCodeMap.value(code, QString());
}

QString Language::GetLanguageCode(QString name)
{
    return m_languageNameMap.value(name, QString());
}

QStringList Language::GetSortedPrimaryLanguageNames()
{
    if (!m_sortedPrimaryLanguageNames.isEmpty()) {
        return m_sortedPrimaryLanguageNames;
    }

    foreach (QString code, m_languageCodeMap.keys()) {
        m_sortedPrimaryLanguageNames.append(m_languageCodeMap.value(code));
    }
    m_sortedPrimaryLanguageNames.sort();

    return m_sortedPrimaryLanguageNames;
}

Language::Language()
{
    SetLanguageMap();
}

void Language::SetLanguageMap()
{
    if (!m_languageCodeMap.isEmpty()) {
        return;
    }

    // Must be a 1 to 1 relationship between codes and names.
    // The keys are the ISO 639-2 language codes
    // If a code has 2 and 3 character versions, use 2 character version
    // and the values are the the full English language names.
    // See http://www.loc.gov/standards/iso639-2/php/English_list.php 
    QStringList data;
    data <<
        "ab"    << tr("Abkhazian") <<
        "aa"    << tr("Afar") <<
        "af"    << tr("Afrikaans") <<
        "ak"    << tr("Akan") <<
        "sq"    << tr("Albanian") <<
        "am"    << tr("Amharic") <<
        "ar"    << tr("Arabic") <<
        "ar_DZ" << tr("Arabic") % " - " % tr("Algeria") <<
        "ar_BH" << tr("Arabic") % " - " % tr("Bahrain") <<
        "ar_EG" << tr("Arabic") % " - " % tr("Egypt") <<
        "ar_IQ" << tr("Arabic") % " - " % tr("Iraq") <<
        "ar_JO" << tr("Arabic") % " - " % tr("Jordan") <<
        "ar_KW" << tr("Arabic") % " - " % tr("Kuwait") <<
        "ar_LB" << tr("Arabic") % " - " % tr("Lebanon") <<
        "ar_LY" << tr("Arabic") % " - " % tr("Libya") <<
        "ar_MA" << tr("Arabic") % " - " % tr("Morocco") <<
        "ar_OM" << tr("Arabic") % " - " % tr("Oman") <<
        "ar_QA" << tr("Arabic") % " - " % tr("Qatar") <<
        "ar_SY" << tr("Arabic") % " - " % tr("Syria") <<
        "ar_TN" << tr("Arabic") % " - " % tr("Tunisia") <<
        "ar_AE" << tr("Arabic") % " - " % tr("United Arab Emirates") <<
        "ar_YE" << tr("Arabic") % " - " % tr("Yemen") <<
        "an"    << tr("Aragonese") <<
        "hy"    << tr("Armenian") <<
        "as"    << tr("Assamese") <<
        "av"    << tr("Avaric") <<
        "ae"    << tr("Avestan") <<
        "ay"    << tr("Aymara") <<
        "az"    << tr("Azerbaijani") <<
        "az_AZ" << tr("Azerbaijani") % " - " % tr( "Azerbaijan" ) <<
        "bm"    << tr("Bambara") <<
        "ba"    << tr("Bashkir") <<
        "eu"    << tr("Basque") <<
        "be"    << tr("Belarusian") <<
        "bn"    << tr("Bengali") <<
        "bh"    << tr("Bihari") <<
        "bi"    << tr("Bislama") <<
        "bs"    << tr("Bosnian") <<
        "br"    << tr("Breton") <<
        "bg"    << tr("Bulgarian") <<
        "my"    << tr("Burmese") <<
        "ca"    << tr("Catalan") <<
        "km"    << tr("Central Khmer") <<
        "ch"    << tr("Chamorro") <<
        "ce"    << tr("Chechen") <<
        "zh"    << tr("Chinese") <<
        "zh_CN" << tr("Chinese") % " - " % tr("China") <<
        "zh_HK" << tr("Chinese") % " - " % tr("Hong Kong") <<
        "zh_MO" << tr("Chinese") % " - " % tr("Macau") <<
        "zh_SG" << tr("Chinese") % " - " % tr("Singapore") <<
        "zh_TW" << tr("Chinese") % " - " % tr("Taiwan") <<
        "cu"    << tr("Church Slavic") <<
        "cv"    << tr("Chuvash") <<
        "kw"    << tr("Cornish") <<
        "co"    << tr("Corsican") <<
        "cr"    << tr("Cree") <<
        "hr"    << tr("Croatian") <<
        "cs"    << tr("Czech") <<
        "da"    << tr("Danish") <<
        "da_DK" << tr("Danish") % " - " % tr("Denmark") <<
        "dv"    << tr("Dhivehi") <<
        "nl"    << tr("Dutch") <<
        "nl_BE" << tr("Dutch") % " - " % tr ("Belgium") <<
        "nl_NL" << tr("Dutch") % " - " % tr ("Netherlands") <<
        "dz"    << tr("Dzongkha") <<
        "en"    << tr("English") <<
        "en_AU" << tr("English") % " - " % tr ("Australia") <<
        "en_BZ" << tr("English") % " - " % tr ("Belize") <<
        "en_CA" << tr("English") % " - " % tr ("Canada") <<
        "en_CB" << tr("English") % " - " % tr ("Caribbean") <<
        "en_GB" << tr("English") % " - " % tr ("Great Britain") <<
        "en_IN" << tr("English") % " - " % tr ("India") <<
        "en_IE" << tr("English") % " - " % tr ("Ireland") <<
        "en_JM" << tr("English") % " - " % tr ("Jamaica") <<
        "en_PH" << tr("English") % " - " % tr ("Phillippines") <<
        "en_TT" << tr("English") % " - " % tr ("Trinidad") <<
        "en_ZA" << tr("English") % " - " % tr ("South Africa") <<
        "en_US" << tr("English") % " - " % tr ("United States") <<
        "eo"    << tr("Esperanto") <<
        "et"    << tr("Estonian") <<
        "ee"    << tr("Ewe") <<
        "fo"    << tr("Faroese") <<
        "fj"    << tr("Fijian") <<
        "fi"    << tr("Finnish") <<
        "fr"    << tr("French") <<
        "fr_BE" << tr("French") % " - " % tr ("Belgium") <<
        "fr_CA" << tr("French") % " - " % tr ("Canada") <<
        "fr_FR" << tr("French") % " - " % tr ("France") <<
        "fr_LU" << tr("French") % " - " % tr ("Luxembourg") <<
        "fr_CH" << tr("French") % " - " % tr ("Switzerland") <<
        "ff"    << tr("Fulah") <<
        "gd"    << tr("Gaelic") % " - " % tr ("Scotland") <<
        "gd_IE" << tr("Gaelic") % " - " % tr ("Ireland") <<
        "gl"    << tr("Galician") <<
        "lg"    << tr("Ganda") <<
        "ka"    << tr("Georgian") <<
        "de"    << tr("German") <<
        "de_AT" << tr("German") % " - " % tr ("Austria") <<
        "de_DE" << tr("German") % " - " % tr ("Germany") <<
        "de_LI" << tr("German") % " - " % tr ("Liechtenstein") <<
        "de_LU" << tr("German") % " - " % tr ("Luxembourg") <<
        "de_CH" << tr("German") % " - " % tr ("Switzerland") <<
        "el"    << tr("Greek, Modern") <<
        "gn"    << tr("Guarani") <<
        "gu"    << tr("Gujarati") <<
        "ht"    << tr("Haitian") <<
        "ha"    << tr("Hausa") <<
        "he"    << tr("Hebrew") <<
        "hz"    << tr("Herero") <<
        "hi"    << tr("Hindi") <<
        "ho"    << tr("Hiri Motu") <<
        "hu"    << tr("Hungarian") <<
        "is"    << tr("Icelandic") <<
        "io"    << tr("Ido") <<
        "ig"    << tr("Igbo") <<
        "id"    << tr("Indonesian") <<
        "ia"    << tr("Interlingua") <<
        "ie"    << tr("Interlingue") <<
        "iu"    << tr("Inuktitut") <<
        "ik"    << tr("Inupiaq") <<
        "ga"    << tr("Irish") <<
        "it"    << tr("Italian") <<
        "it_IT" << tr("Italian") % " - " % tr ("Italy") <<
        "it_CH" << tr("Italian") % " - " % tr ("Switzerland") <<
        "ja"    << tr("Japanese") <<
        "jv"    << tr("Javanese") <<
        "kl"    << tr("Kalaallisut") <<
        "kn"    << tr("Kannada") <<
        "kr"    << tr("Kanuri") <<
        "ks"    << tr("Kashmiri") <<
        "kk"    << tr("Kazakh") <<
        "ki"    << tr("Kikuyu") <<
        "rw"    << tr("Kinyarwanda") <<
        "ky"    << tr("Kirghiz") <<
        "kv"    << tr("Komi") <<
        "kg"    << tr("Kongo") <<
        "ko"    << tr("Korean") <<
        "kj"    << tr("Kuanyama") <<
        "ku"    << tr("Kurdish") <<
        "lo"    << tr("Lao") <<
        "la"    << tr("Latin") <<
        "lv"    << tr("Latvian") <<
        "li"    << tr("Limburgan") <<
        "ln"    << tr("Lingala") <<
        "lt"    << tr("Lithuanian") <<
        "lu"    << tr("Luba-Katanga") <<
        "lb"    << tr("Luxembourgish") <<
        "mk"    << tr("Macedonian") <<
        "mg"    << tr("Malagasy") <<
        "ml"    << tr("Malayalam") <<
        "ms"    << tr("Malay") <<
        "ms_BN" << tr("Malay") % " - " % tr ("Brunei") <<
        "ms_MY" << tr("Malay") % " - " % tr ("Malaysia") <<
        "mt"    << tr("Maltese") <<
        "gv"    << tr("Manx") <<
        "mi"    << tr("Maori") <<
        "mr"    << tr("Marathi") <<
        "mh"    << tr("Marshallese") <<
        "mn"    << tr("Mongolian") <<
        "na"    << tr("Nauru") <<
        "nv"    << tr("Navajo") <<
        "nd"    << tr("Ndebele") % " - " % tr ("North") <<
        "nr"    << tr("Ndebele") % " - " % tr ("South") <<
        "ng"    << tr("Ndonga") <<
        "ne"    << tr("Nepali") <<
        "se"    << tr("Northern, Sami") <<
        "no"    << tr("Norwegian") <<
        "nb"    << tr("Norwegian") % " - " % tr ("Bokmal") <<
        "nn"    << tr("Norwegian") % " - " % tr ("Nynorsk") <<
        "ny"    << tr("Nyanja") <<
        "oc"    << tr("Occitan") <<
        "oj"    << tr("Ojibwa") <<
        "or"    << tr("Oriya") <<
        "om"    << tr("Oromo") <<
        "os"    << tr("Ossetian") <<
        "pi"    << tr("Pali") <<
        "pa"    << tr("Panjabi") <<
        "fa"    << tr("Persian") <<
        "pl"    << tr("Polish") <<
        "pt"    << tr("Portuguese") <<
        "pt_BR" << tr("Portuguese") % " - " % tr ("Brazil") <<
        "pt_TT" << tr("Portuguese") % " - " % tr ("Portugal") <<
        "ps"    << tr("Pushto") <<
        "qu"    << tr("Quechua") <<
        "ro"    << tr("Romanian") <<
        "ro_MO" << tr("Romanian") % " - " % tr ("Moldova") <<
        "rm"    << tr("Romansh") <<
        "rn"    << tr("Rundi") <<
        "ru"    << tr("Russian") <<
        "ru_MO" << tr("Russian") % " - " % tr ("Moldova") <<
        "sm"    << tr("Samoan") <<
        "sg"    << tr("Sango") <<
        "sa"    << tr("Sanskrit") <<
        "sc"    << tr("Sardinian") <<
        "sr"    << tr("Serbian") <<
        "sr_SP" << tr("Serbian") % " - " % tr ("SP") <<
        "sn"    << tr("Shona") <<
        "ii"    << tr("Sichuan Yi") <<
        "sd"    << tr("Sindhi") <<
        "si"    << tr("Sinhala") <<
        "sk"    << tr("Slovak") <<
        "sl"    << tr("Slovenian") <<
        "so"    << tr("Somali") <<
        "st"    << tr("Sotho, Southern") <<
        "es"    << tr("Spanish") <<
        "es_AR" << tr("Spanish") % " - " % tr ("Argentina") <<
        "es_BO" << tr("Spanish") % " - " % tr ("Bolivia") <<
        "es_CL" << tr("Spanish") % " - " % tr ("Chile") <<
        "es_CO" << tr("Spanish") % " - " % tr ("Columbia") <<
        "es_CR" << tr("Spanish") % " - " % tr ("Costa Rica") <<
        "es_DO" << tr("Spanish") % " - " % tr ("Dominican Republic") <<
        "es_EC" << tr("Spanish") % " - " % tr ("Ecuador") <<
        "es_SV" << tr("Spanish") % " - " % tr ("El Salvador") <<
        "es_GT" << tr("Spanish") % " - " % tr ("Guatemala") <<
        "es_HN" << tr("Spanish") % " - " % tr ("Honduras") <<
        "es_MX" << tr("Spanish") % " - " % tr ("Mexico") <<
        "es_NI" << tr("Spanish") % " - " % tr ("Nicaragua") <<
        "es_PA" << tr("Spanish") % " - " % tr ("Panama") <<
        "es_PY" << tr("Spanish") % " - " % tr ("Paraguay") <<
        "es_PE" << tr("Spanish") % " - " % tr ("Peru") <<
        "es_PR" << tr("Spanish") % " - " % tr ("Puerto Rico") <<
        "es_ES" << tr("Spanish") % " - " % tr ("Spain") <<
        "es_UY" << tr("Spanish") % " - " % tr ("Uruguay") <<
        "es_VE" << tr("Spanish") % " - " % tr ("Venezuela") <<
        "su"    << tr("Sundanese") <<
        "sw"    << tr("Swahili") <<
        "ss"    << tr("Swati") <<
        "sv"    << tr("Swedish") <<
        "sv_FI" << tr("Swedish") % " - " % tr ("Finland") <<
        "sv_SE" << tr("Swedish") % " - " % tr ("Sweden") <<
        "tl"    << tr("Tagalog") <<
        "ty"    << tr("Tahitian") <<
        "tg"    << tr("Tajik") <<
        "ta"    << tr("Tamil") <<
        "tt"    << tr("Tatar") <<
        "te"    << tr("Telugu") <<
        "th"    << tr("Thai") <<
        "bo"    << tr("Tibetan") <<
        "ti"    << tr("Tigrinya") <<
        "to"    << tr("Tonga") <<
        "ts"    << tr("Tsonga") <<
        "tn"    << tr("Tswana") <<
        "tr"    << tr("Turkish") <<
        "tr_TR" << tr("Turkish") % " - " % tr ("Turkey") <<
        "tk"    << tr("Turkmen") <<
        "tw"    << tr("Twi") <<
        "ug"    << tr("Uighur") <<
        "uk"    << tr("Ukrainian") <<
        "ur"    << tr("Urdu") <<
        "uz"    << tr("Uzbek") <<
        "uz_UX" << tr("Uzbek") % " - " % tr ("Uzbekistan") <<
        "ve"    << tr("Venda") <<
        "vi"    << tr("Vietnamese") <<
        "vo"    << tr("Volapuk") <<
        "wa"    << tr("Walloon") <<
        "cy"    << tr("Welsh") <<
        "fy"    << tr("Western Frisian") <<
        "wo"    << tr("Wolof") <<
        "xh"    << tr("Xhosa") <<
        "yi"    << tr("Yiddish") <<
        "yo"    << tr("Yoruba") <<
        "za"    << tr("Zhuang") <<
        "zu"    << tr("Zulu") <<
        "ace"   << tr( "Achinese" ) <<
        "ach"   << tr( "Acoli" ) <<
        "ada"   << tr( "Adangme" ) <<
        "ady"   << tr( "Adygei, Adyghe" ) <<
        "afh"   << tr( "Afrihili" ) <<
        "afa"   << tr( "Afro-Asiatic languages" ) <<
        "ain"   << tr( "Ainu" ) <<
        "akk"   << tr( "Akkadian" ) <<
        "ale"   << tr( "Aleut" ) <<
        "alg"   << tr( "Algonquian languages" ) <<
        "tut"   << tr( "Altaic languages" ) <<
        "anp"   << tr( "Angika" ) <<
        "apa"   << tr( "Apache languages" ) <<
        "arp"   << tr( "Arapaho" ) <<
        "arw"   << tr( "Arawak" ) <<
        "rup"   << tr( "Aromanian, Arumanian, Macedo-Romanian" ) <<
        "art"   << tr( "Artificial languages" ) <<
        "ast"   << tr( "Asturian, Asturleonese, Bable, Leonese" ) <<
        "ath"   << tr( "Athapascan languages" ) <<
        "aus"   << tr( "Australian languages" ) <<
        "map"   << tr( "Austronesian languages" ) <<
        "awa"   << tr( "Awadhi" ) <<
        "ban"   << tr( "Balinese" ) <<
        "bat"   << tr( "Baltic languages" ) <<
        "bal"   << tr( "Baluchi" ) <<
        "bai"   << tr( "Bamileke languages" ) <<
        "bad"   << tr( "Banda languages" ) <<
        "bnt"   << tr( "Bantu languages" ) <<
        "bas"   << tr( "Basa" ) <<
        "btk"   << tr( "Batak languages" ) <<
        "bej"   << tr( "Bedawiyet, Beja" ) <<
        "bem"   << tr( "Bemba" ) <<
        "ber"   << tr( "Berber languages" ) <<
        "bho"   << tr( "Bhojpuri" ) <<
        "bik"   << tr( "Bikol" ) <<
        "byn"   << tr( "Bilin, Blin" ) <<
        "bin"   << tr( "Bini, Edo" ) <<
        "zbl"   << tr( "Bliss, Blissymbols, Blissymbolics" ) <<
        "bra"   << tr( "Braj" ) <<
        "bug"   << tr( "Buginese" ) <<
        "bua"   << tr( "Buriat" ) <<
        "cad"   << tr( "Caddo" ) <<
        "cau"   << tr( "Caucasian languages" ) <<
        "ceb"   << tr( "Cebuano" ) <<
        "cel"   << tr( "Celtic languages" ) <<
        "cai"   << tr( "Central American Indian languages" ) <<
        "chg"   << tr( "Chagatai" ) <<
        "cmc"   << tr( "Chamic languages" ) <<
        "chr"   << tr( "Cherokee" ) <<
        "chy"   << tr( "Cheyenne" ) <<
        "chb"   << tr( "Chibcha" ) <<
        "chn"   << tr( "Chinook jargon" ) <<
        "chp"   << tr( "Chipewyan, Dene Suline" ) <<
        "cho"   << tr( "Choctaw" ) <<
        "chk"   << tr( "Chuukese" ) <<
        "nwc"   << tr( "Classical Nepal Bhasa/Newari, Old Newari" ) <<
        "syc"   << tr( "Classical Syriac" ) <<
        "cop"   << tr( "Coptic" ) <<
        "mus"   << tr( "Creek" ) <<
        "crp"   << tr( "Creoles and pidgins" ) <<
        "cpe"   << tr( "Creoles and pidgins- English based" ) <<
        "cpf"   << tr( "Creoles and pidgins- French-based" ) <<
        "cpp"   << tr( "Creoles and pidgins- Portuguese-based" ) <<
        "crh"   << tr( "Crimean Tatar/Turkish" ) <<
        "cus"   << tr( "Cushitic languages" ) <<
        "dak"   << tr( "Dakota" ) <<
        "dar"   << tr( "Dargwa" ) <<
        "del"   << tr( "Delaware" ) <<
        "zza"   << tr( "Dimili, Dimli, Zaza, Zazaki, Kirdki, Kirmanjki" ) <<
        "din"   << tr( "Dinka" ) <<
        "doi"   << tr( "Dogri" ) <<
        "dgr"   << tr( "Dogrib" ) <<
        "dra"   << tr( "Dravidian languages" ) <<
        "dua"   << tr( "Duala" ) <<
        "dum"   << tr( "Dutch- Middle (ca.1050-1350)" ) <<
        "dyu"   << tr( "Dyula" ) <<
        "frs"   << tr( "Eastern Frisian" ) <<
        "efi"   << tr( "Efik" ) <<
        "egy"   << tr( "Egyptian (Ancient)" ) <<
        "eka"   << tr( "Ekajuk" ) <<
        "elx"   << tr( "Elamite" ) <<
        "enm"   << tr( "English- Middle (1100-1500)" ) <<
        "ang"   << tr( "English- Old (ca.450-1100)" ) <<
        "myv"   << tr( "Erzya" ) <<
        "ewo"   << tr( "Ewondo" ) <<
        "fan"   << tr( "Fang" ) <<
        "fat"   << tr( "Fanti" ) <<
        "fil"   << tr( "Filipino, Pilipino" ) <<
        "fiu"   << tr( "Finno-Ugrian languages" ) <<
        "fon"   << tr( "Fon" ) <<
        "frm"   << tr( "French- Middle (ca.1400-1600)" ) <<
        "fro"   << tr( "French- Old (842-ca.1400)" ) <<
        "fur"   << tr( "Friulian" ) <<
        "gaa"   << tr( "Ga" ) <<
        "car"   << tr( "Galibi Carib" ) <<
        "gay"   << tr( "Gayo" ) <<
        "gba"   << tr( "Gbaya" ) <<
        "gez"   << tr( "Geez" ) <<
        "gmh"   << tr( "German- Middle High (ca.1050-1500)" ) <<
        "goh"   << tr( "German- Old High (ca.750-1050)" ) <<
        "gem"   << tr( "Germanic languages" ) <<
        "gil"   << tr( "Gilbertese" ) <<
        "gon"   << tr( "Gondi" ) <<
        "gor"   << tr( "Gorontalo" ) <<
        "got"   << tr( "Gothic" ) <<
        "grb"   << tr( "Grebo" ) <<
        "grc"   << tr( "Greek- Ancient (to 1453)" ) <<
        "gwi"   << tr( "Gwich'in" ) <<
        "hai"   << tr( "Haida" ) <<
        "haw"   << tr( "Hawaiian" ) <<
        "hil"   << tr( "Hiligaynon" ) <<
        "him"   << tr( "Himachali, Western Pahari languages" ) <<
        "hit"   << tr( "Hittite" ) <<
        "hmn"   << tr( "Hmong, Mong" ) <<
        "hup"   << tr( "Hupa" ) <<
        "iba"   << tr( "Iban" ) <<
        "ijo"   << tr( "Ijo languages" ) <<
        "ilo"   << tr( "Iloko" ) <<
        "smn"   << tr( "Inari Sami" ) <<
        "inc"   << tr( "Indic languages" ) <<
        "ine"   << tr( "Indo-European languages" ) <<
        "inh"   << tr( "Ingush" ) <<
        "ira"   << tr( "Iranian languages" ) <<
        "mga"   << tr( "Irish- Middle (900-1200)" ) <<
        "sga"   << tr( "Irish- Old (to 900)" ) <<
        "iro"   << tr( "Iroquoian languages" ) <<
        "kac"   << tr( "Jingpho, Kachin" ) <<
        "jrb"   << tr( "Judeo-Arabic" ) <<
        "jpr"   << tr( "Judeo-Persian" ) <<
        "kbd"   << tr( "Kabardian" ) <<
        "kab"   << tr( "Kabyle" ) <<
        "xal"   << tr( "Kalmyk, Oirat" ) <<
        "kam"   << tr( "Kamba" ) <<
        "pam"   << tr( "Kapampangan, Pampanga" ) <<
        "kaa"   << tr( "Kara-Kalpak" ) <<
        "krc"   << tr( "Karachay-Balkar" ) <<
        "krl"   << tr( "Karelian" ) <<
        "kar"   << tr( "Karen languages" ) <<
        "csb"   << tr( "Kashubian" ) <<
        "kaw"   << tr( "Kawi" ) <<
        "kha"   << tr( "Khasi" ) <<
        "khi"   << tr( "Khoisan languages" ) <<
        "kho"   << tr( "Khotanese, Sakan" ) <<
        "kmb"   << tr( "Kimbundu" ) <<
        "tlh"   << tr( "Klingon, tlhIngan-Hol" ) <<
        "kok"   << tr( "Konkani" ) <<
        "kos"   << tr( "Kosraean" ) <<
        "kpe"   << tr( "Kpelle" ) <<
        "kro"   << tr( "Kru languages" ) <<
        "kum"   << tr( "Kumyk" ) <<
        "kru"   << tr( "Kurukh" ) <<
        "kut"   << tr( "Kutenai" ) <<
        "lad"   << tr( "Ladino" ) <<
        "lah"   << tr( "Lahnda" ) <<
        "lam"   << tr( "Lamba" ) <<
        "day"   << tr( "Land Dayak languages" ) <<
        "lez"   << tr( "Lezghian" ) <<
        "jbo"   << tr( "Lojban" ) <<
        "nds"   << tr( "German-Low, Low Saxon" ) <<
        "dsb"   << tr( "Lower Sorbian" ) <<
        "loz"   << tr( "Lozi" ) <<
        "lua"   << tr( "Luba-Lulua" ) <<
        "lui"   << tr( "Luiseno" ) <<
        "smj"   << tr( "Lule Sami" ) <<
        "lun"   << tr( "Lunda" ) <<
        "luo"   << tr( "Luo (Kenya and Tanzania)" ) <<
        "lus"   << tr( "Lushai" ) <<
        "mad"   << tr( "Madurese" ) <<
        "mag"   << tr( "Magahi" ) <<
        "mai"   << tr( "Maithili" ) <<
        "mak"   << tr( "Makasar" ) <<
        "mnc"   << tr( "Manchu" ) <<
        "mdr"   << tr( "Mandar" ) <<
        "man"   << tr( "Mandingo" ) <<
        "mni"   << tr( "Manipuri" ) <<
        "mno"   << tr( "Manobo languages" ) <<
        "arn"   << tr( "Mapuche/Mapudungun" ) <<
        "chm"   << tr( "Mari" ) <<
        "mwr"   << tr( "Marwari" ) <<
        "mas"   << tr( "Masai" ) <<
        "myn"   << tr( "Mayan languages" ) <<
        "men"   << tr( "Mende" ) <<
        "mic"   << tr( "Mi'kmaq, Micmac" ) <<
        "min"   << tr( "Minangkabau" ) <<
        "mwl"   << tr( "Mirandese" ) <<
        "moh"   << tr( "Mohawk" ) <<
        "mdf"   << tr( "Moksha" ) <<
        "mkh"   << tr( "Mon-Khmer languages" ) <<
        "lol"   << tr( "Mongo" ) <<
        "mos"   << tr( "Mossi" ) <<
        "mul"   << tr( "Multiple languages" ) <<
        "mun"   << tr( "Munda languages" ) <<
        "nqo"   << tr( "N'Ko" ) <<
        "nah"   << tr( "Nahuatl languages" ) <<
        "nap"   << tr( "Neapolitan" ) <<
        "new"   << tr( "Nepal Bhasa/Newari" ) <<
        "nia"   << tr( "Nias" ) <<
        "nic"   << tr( "Niger-Kordofanian languages" ) <<
        "ssa"   << tr( "Nilo-Saharan languages" ) <<
        "niu"   << tr( "Niuean" ) <<
        "zxx"   << tr( "No linguistic content/Not applicable" ) <<
        "nog"   << tr( "Nogai" ) <<
        "non"   << tr( "Norse- Old" ) <<
        "nai"   << tr( "North American Indian languages" ) <<
        "frr"   << tr( "Northern Frisian" ) <<
        "nso"   << tr( "Northern Sotho, Sepedi, Pedi" ) <<
        "nub"   << tr( "Nubian languages" ) <<
        "nym"   << tr( "Nyamwezi" ) <<
        "nyn"   << tr( "Nyankole" ) <<
        "nyo"   << tr( "Nyoro" ) <<
        "nzi"   << tr( "Nzima" ) <<
        "pro"   << tr( "Occitan/Provençal- Old (to 1500)" ) <<
        "arc"   << tr( "Official/Imperial Aramaic (700-300 BCE)" ) <<
        "osa"   << tr( "Osage" ) <<
        "oto"   << tr( "Otomian languages" ) <<
        "pal"   << tr( "Pahlavi" ) <<
        "pau"   << tr( "Palauan" ) <<
        "pag"   << tr( "Pangasinan" ) <<
        "pap"   << tr( "Papiamento" ) <<
        "paa"   << tr( "Papuan languages" ) <<
        "peo"   << tr( "Persian- Old (ca.600-400 B.C.)" ) <<
        "phi"   << tr( "Philippine languages" ) <<
        "phn"   << tr( "Phoenician" ) <<
        "pon"   << tr( "Pohnpeian" ) <<
        "pra"   << tr( "Prakrit languages" ) <<
        "raj"   << tr( "Rajasthani" ) <<
        "rap"   << tr( "Rapanui" ) <<
        "rar"   << tr( "Rarotongan, Cook Islands Maori" ) <<
        "qaa-qtz"   << tr( "Reserved for local use" ) <<
        "roa"   << tr( "Romance languages" ) <<
        "rom"   << tr( "Romany" ) <<
        "sal"   << tr( "Salishan languages" ) <<
        "sam"   << tr( "Samaritan Aramaic" ) <<
        "smi"   << tr( "Sami languages" ) <<
        "sad"   << tr( "Sandawe" ) <<
        "sat"   << tr( "Santali" ) <<
        "sas"   << tr( "Sasak" ) <<
        "sco"   << tr( "Scots" ) <<
        "sel"   << tr( "Selkup" ) <<
        "sem"   << tr( "Semitic languages" ) <<
        "srr"   << tr( "Serer" ) <<
        "shn"   << tr( "Shan" ) <<
        "scn"   << tr( "Sicilian" ) <<
        "sid"   << tr( "Sidamo" ) <<
        "sgn"   << tr( "Sign Languages" ) <<
        "bla"   << tr( "Siksika" ) <<
        "sit"   << tr( "Sino-Tibetan languages" ) <<
        "sio"   << tr( "Siouan languages" ) <<
        "sms"   << tr( "Skolt Sami" ) <<
        "den"   << tr( "Slave (Athapascan)" ) <<
        "sla"   << tr( "Slavic languages" ) <<
        "sog"   << tr( "Sogdian" ) <<
        "son"   << tr( "Songhai languages" ) <<
        "snk"   << tr( "Soninke" ) <<
        "wen"   << tr( "Sorbian languages" ) <<
        "sai"   << tr( "South American Indian languages" ) <<
        "alt"   << tr( "Southern Altai" ) <<
        "sma"   << tr( "Southern Sami" ) <<
        "srn"   << tr( "Sranan Tongo" ) <<
        "suk"   << tr( "Sukuma" ) <<
        "sux"   << tr( "Sumerian" ) <<
        "sus"   << tr( "Susu" ) <<
        "gsw"   << tr( "Alsatian, Swiss German, Alemannic" ) <<
        "syr"   << tr( "Syriac" ) <<
        "tai"   << tr( "Tai languages" ) <<
        "tmh"   << tr( "Tamashek" ) <<
        "ter"   << tr( "Tereno" ) <<
        "tet"   << tr( "Tetum" ) <<
        "tig"   << tr( "Tigre" ) <<
        "tem"   << tr( "Timne" ) <<
        "tiv"   << tr( "Tiv" ) <<
        "tli"   << tr( "Tlingit" ) <<
        "tpi"   << tr( "Tok Pisin" ) <<
        "tkl"   << tr( "Tokelau" ) <<
        "tog"   << tr( "Tonga (Nyasa)" ) <<
        "tsi"   << tr( "Tsimshian" ) <<
        "tum"   << tr( "Tumbuka" ) <<
        "tup"   << tr( "Tupi languages" ) <<
        "ota"   << tr( "Turkish- Ottoman (1500-1928)" ) <<
        "tvl"   << tr( "Tuvalu" ) <<
        "tyv"   << tr( "Tuvinian" ) <<
        "udm"   << tr( "Udmurt" ) <<
        "uga"   << tr( "Ugaritic" ) <<
        "umb"   << tr( "Umbundu" ) <<
        "mis"   << tr( "Uncoded languages" ) <<
        "und"   << tr( "Undetermined" ) <<
        "hsb"   << tr( "Upper Sorbian" ) <<
        "vai"   << tr( "Vai" ) <<
        "vot"   << tr( "Votic" ) <<
        "wak"   << tr( "Wakashan languages" ) <<
        "war"   << tr( "Waray" ) <<
        "was"   << tr( "Washo" ) <<
        "wal"   << tr( "Wolaitta, Wolaytta" ) <<
        "sah"   << tr( "Yakut" ) <<
        "yao"   << tr( "Yao" ) <<
        "yap"   << tr( "Yapese" ) <<
        "ypk"   << tr( "Yupik languages" ) <<
        "znd"   << tr( "Zande languages" ) <<
        "zap"   << tr( "Zapotec" ) <<
        "zen"   << tr( "Zenaga" ) <<
        "zun"   << tr( "Zuni" );

    for (int i = 0; i < data.count(); i++) {
        QString code = data.at(i++);
        QString name = data.at(i);

        m_languageCodeMap.insert(code, name);
        m_languageNameMap.insert(name, code);
    }
}
