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

#include "Misc/Language.h"

Language *Language::m_instance = 0;

Language *Language::instance()
{
    if ( m_instance == 0 ) {
        m_instance = new Language();
    }

    return m_instance;
}

Language::Language()
{
    SetLanguageMap();
}

QString Language::GetLanguageName( QString code )
{
    return m_languageCodeMap.value( code );
}

QString Language::GetLanguageCode( QString name )
{
    return m_languageNameMap.value( name );
}

QStringList Language::GetSortedPrimaryLanguageNames()
{
    if ( !m_sortedPrimaryLanguageNames.isEmpty() )
    {
        return m_sortedPrimaryLanguageNames;
    }

    foreach( QString code, m_languageCodeMap.keys() )
    {
        if ( code.length() == 2 )
        {
            m_sortedPrimaryLanguageNames.append( m_languageCodeMap.value( code ) );
        }
    }

    m_sortedPrimaryLanguageNames.sort();

    return m_sortedPrimaryLanguageNames;
}

void Language::SetLanguageMap()
{
    if ( !m_languageCodeMap.isEmpty() )
    {
        return;
    }

    // Must be a 1 to 1 relationship between codes and names.
    // The keys are the ISO 639-1 language codes
    // and the values are the the full English language names.
    // See http://www.loc.gov/standards/iso639-2/php/English_list.php 
    QStringList data;
    data <<
        "ab"    << tr( "Abkhazian" ) <<
        "aa"    << tr( "Afar" ) <<
        "af"    << tr( "Afrikaans" ) <<
        "ak"    << tr( "Akan" ) <<
        "sq"    << tr( "Albanian" ) <<
        "am"    << tr( "Amharic" ) <<
        "ar"    << tr( "Arabic" ) <<
        "ar_DZ" << tr( "Arabic" ) % " - " % tr( "Algeria" ) <<
        "ar_BH" << tr( "Arabic" ) % " - " % tr( "Bahrain" ) <<
        "ar_EG" << tr( "Arabic" ) % " - " % tr( "Egypt" ) <<
        "ar_IQ" << tr( "Arabic" ) % " - " % tr( "Iraq" ) <<
        "ar_JO" << tr( "Arabic" ) % " - " % tr( "Jordan" ) <<
        "ar_KW" << tr( "Arabic" ) % " - " % tr( "Kuwait" ) <<
        "ar_LB" << tr( "Arabic" ) % " - " % tr( "Lebanon" ) <<
        "ar_LY" << tr( "Arabic" ) % " - " % tr( "Libya" ) <<
        "ar_MA" << tr( "Arabic" ) % " - " % tr( "Morocco" ) <<
        "ar_OM" << tr( "Arabic" ) % " - " % tr( "Oman" ) <<
        "ar_QA" << tr( "Arabic" ) % " - " % tr( "Qatar" ) <<
        "ar_SY" << tr( "Arabic" ) % " - " % tr( "Syria" ) <<
        "ar_TN" << tr( "Arabic" ) % " - " % tr( "Tunisia" ) <<
        "ar_AE" << tr( "Arabic" ) % " - " % tr( "United Arab Emirates" ) <<
        "ar_YE" << tr( "Arabic" ) % " - " % tr( "Yemen" ) <<
        "an"    << tr( "Aragonese" ) <<
        "hy"    << tr( "Armenian" ) <<
        "as"    << tr( "Assamese" ) <<
        "av"    << tr( "Avaric" ) <<
        "ae"    << tr( "Avestan" ) <<
        "ay"    << tr( "Aymara" ) <<
        "az"    << tr( "Azerbaijani" ) <<
        "az_AZ" << tr( "Azerbaijani" ) % " - " % tr( "Azerbaijan" ) <<
        "bm"    << tr( "Bambara" ) <<
        "ba"    << tr( "Bashkir" ) <<
        "eu"    << tr( "Basque" ) <<
        "be"    << tr( "Belarusian" ) <<
        "bn"    << tr( "Bengali" ) <<
        "bh"    << tr( "Bihari" ) <<
        "bi"    << tr( "Bislama" ) <<
        "bs"    << tr( "Bosnian" ) <<
        "br"    << tr( "Breton" ) <<
        "bg"    << tr( "Bulgarian" ) <<
        "my"    << tr( "Burmese" ) <<
        "ca"    << tr( "Catalan" ) <<
        "km"    << tr( "Central Khmer" ) <<
        "ch"    << tr( "Chamorro" ) <<
        "ce"    << tr( "Chechen" ) <<
        "zh"    << tr( "Chinese" ) <<
        "zh_CN" << tr( "Chinese" ) % " - " % tr( "China" ) <<
        "zh_HK" << tr( "Chinese" ) % " - " % tr( "Hong Kong" ) <<
        "zh_MO" << tr( "Chinese" ) % " - " % tr( "Macau" ) <<
        "zh_SG" << tr( "Chinese" ) % " - " % tr( "Singapore" ) <<
        "zh_TW" << tr( "Chinese" ) % " - " % tr( "Taiwan" ) <<
        "cu"    << tr( "Church Slavic" ) <<
        "cv"    << tr( "Chuvash" ) <<
        "kw"    << tr( "Cornish" ) <<
        "co"    << tr( "Corsican" ) <<
        "cr"    << tr( "Cree" ) <<
        "hr"    << tr( "Croatian" ) <<
        "cs"    << tr( "Czech" ) <<
        "da"    << tr( "Danish" ) <<
        "da_DK" << tr( "Danish" ) % " - " % tr( "Denmark" ) <<
        "dv"    << tr( "Dhivehi" ) <<
        "nl"    << tr( "Dutch" ) <<
        "nl_BE" << tr( "Dutch" ) % " - " % tr ( "Belgium" ) <<
        "nl_NL" << tr( "Dutch" ) % " - " % tr ( "Netherlands" ) <<
        "dz"    << tr( "Dzongkha" ) <<
        "en"    << tr( "English" ) <<
        "en_AU" << tr( "English" ) % " - " % tr ( "Australia" ) <<
        "en_BZ" << tr( "English" ) % " - " % tr ( "Belize" ) <<
        "en_CA" << tr( "English" ) % " - " % tr ( "Canada" ) <<
        "en_CB" << tr( "English" ) % " - " % tr ( "Caribbean" ) <<
        "en_GB" << tr( "English" ) % " - " % tr ( "Great Britain" ) <<
        "en_IN" << tr( "English" ) % " - " % tr ( "India" ) <<
        "en_IE" << tr( "English" ) % " - " % tr ( "Ireland" ) <<
        "en_JM" << tr( "English" ) % " - " % tr ( "Jamaica" ) <<
        "en_PH" << tr( "English" ) % " - " % tr ( "Phillippines" ) <<
        "en_TT" << tr( "English" ) % " - " % tr ( "Trinidad" ) <<
        "en_ZA" << tr( "English" ) % " - " % tr ( "South Africa" ) <<
        "en_US" << tr( "English" ) % " - " % tr ( "United States" ) <<
        "eo"    << tr( "Esperanto" ) <<
        "et"    << tr( "Estonian" ) <<
        "ee"    << tr( "Ewe" ) <<
        "fo"    << tr( "Faroese" ) <<
        "fj"    << tr( "Fijian" ) <<
        "fi"    << tr( "Finnish" ) <<
        "fr"    << tr( "French" ) <<
        "fr_BE" << tr( "French" ) % " - " % tr ( "Belgium" ) <<
        "fr_CA" << tr( "French" ) % " - " % tr ( "Canada" ) <<
        "fr_FR" << tr( "French" ) % " - " % tr ( "France" ) <<
        "fr_LU" << tr( "French" ) % " - " % tr ( "Luxembourg" ) <<
        "fr_CH" << tr( "French" ) % " - " % tr ( "Switzerland" ) <<
        "ff"    << tr( "Fulah" ) <<
        "gd"    << tr( "Gaelic" ) % " - " % tr ( "Scotland" ) <<
        "gd_IE" << tr( "Gaelic" ) % " - " % tr ( "Ireland" ) <<
        "gl"    << tr( "Galician" ) <<
        "lg"    << tr( "Ganda" ) <<
        "ka"    << tr( "Georgian" ) <<
        "de"    << tr( "German" ) <<
        "de_AT" << tr( "German" ) % " - " % tr ( "Austria" ) <<
        "de_DE" << tr( "German" ) % " - " % tr ( "Germany" ) <<
        "de_LI" << tr( "German" ) % " - " % tr ( "Liechtenstein" ) <<
        "de_LU" << tr( "German" ) % " - " % tr ( "Luxembourg" ) <<
        "de_CH" << tr( "German" ) % " - " % tr ( "Switzerland" ) <<
        "el"    << tr( "Greek, Modern" ) <<
        "gn"    << tr( "Guarani" ) <<
        "gu"    << tr( "Gujarati" ) <<
        "ht"    << tr( "Haitian" ) <<
        "ha"    << tr( "Hausa" ) <<
        "he"    << tr( "Hebrew" ) <<
        "hz"    << tr( "Herero" ) <<
        "hi"    << tr( "Hindi" ) <<
        "ho"    << tr( "Hiri Motu" ) <<
        "hu"    << tr( "Hungarian" ) <<
        "is"    << tr( "Icelandic" ) <<
        "io"    << tr( "Ido" ) <<
        "ig"    << tr( "Igbo" ) <<
        "id"    << tr( "Indonesian" ) <<
        "ia"    << tr( "Interlingua" ) <<
        "ie"    << tr( "Interlingue" ) <<
        "iu"    << tr( "Inuktitut" ) <<
        "ik"    << tr( "Inupiaq" ) <<
        "ga"    << tr( "Irish" ) <<
        "it"    << tr( "Italian" ) <<
        "it_IT" << tr( "Italian" ) % " - " % tr ( "Italy" ) <<
        "it_CH" << tr( "Italian" ) % " - " % tr ( "Switzerland" ) <<
        "ja"    << tr( "Japanese" ) <<
        "jv"    << tr( "Javanese" ) <<
        "kl"    << tr( "Kalaallisut" ) <<
        "kn"    << tr( "Kannada" ) <<
        "kr"    << tr( "Kanuri" ) <<
        "ks"    << tr( "Kashmiri" ) <<
        "kk"    << tr( "Kazakh" ) <<
        "ki"    << tr( "Kikuyu" ) <<
        "rw"    << tr( "Kinyarwanda" ) <<
        "ky"    << tr( "Kirghiz" ) <<
        "kv"    << tr( "Komi" ) <<
        "kg"    << tr( "Kongo" ) <<
        "ko"    << tr( "Korean" ) <<
        "kj"    << tr( "Kuanyama" ) <<
        "ku"    << tr( "Kurdish" ) <<
        "lo"    << tr( "Lao" ) <<
        "la"    << tr( "Latin" ) <<
        "lv"    << tr( "Latvian" ) <<
        "li"    << tr( "Limburgan" ) <<
        "ln"    << tr( "Lingala" ) <<
        "lt"    << tr( "Lithuanian" ) <<
        "lu"    << tr( "Luba-Katanga" ) <<
        "lb"    << tr( "Luxembourgish" ) <<
        "mk"    << tr( "Macedonian" ) <<
        "mg"    << tr( "Malagasy" ) <<
        "ml"    << tr( "Malayalam" ) <<
        "ms"    << tr( "Malay" ) <<
        "ms_BN" << tr( "Malay" ) % " - " % tr ( "Brunei" ) <<
        "ms_MY" << tr( "Malay" ) % " - " % tr ( "Malaysia" ) <<
        "mt"    << tr( "Maltese" ) <<
        "gv"    << tr( "Manx" ) <<
        "mi"    << tr( "Maori" ) <<
        "mr"    << tr( "Marathi" ) <<
        "mh"    << tr( "Marshallese" ) <<
        "mn"    << tr( "Mongolian" ) <<
        "na"    << tr( "Nauru" ) <<
        "nv"    << tr( "Navajo" ) <<
        "nd"    << tr( "Ndebele" ) % " - " % tr ( "North" ) <<
        "nr"    << tr( "Ndebele" ) % " - " % tr ( "South" ) <<
        "ng"    << tr( "Ndonga" ) <<
        "ne"    << tr( "Nepali" ) <<
        "se"    << tr( "Northern, Sami" ) <<
        "no"    << tr( "Norwegian" ) <<
        "nb"    << tr( "Norwegian" ) % " - " % tr ( "Bokmal" ) <<
        "nn"    << tr( "Norwegian" ) % " - " % tr ( "Nynorsk" ) <<
        "ny"    << tr( "Nyanja" ) <<
        "oc"    << tr( "Occitan" ) <<
        "oj"    << tr( "Ojibwa" ) <<
        "or"    << tr( "Oriya" ) <<
        "om"    << tr( "Oromo" ) <<
        "os"    << tr( "Ossetian" ) <<
        "pi"    << tr( "Pali" ) <<
        "pa"    << tr( "Panjabi" ) <<
        "fa"    << tr( "Persian" ) <<
        "pl"    << tr( "Polish" ) <<
        "pt"    << tr( "Portuguese" ) <<
        "pt_BR" << tr( "Portuguese" ) % " - " % tr ( "Brazil" ) <<
        "pt_TT" << tr( "Portuguese" ) % " - " % tr ( "Portugal" ) <<
        "ps"    << tr( "Pushto" ) <<
        "qu"    << tr( "Quechua" ) <<
        "ro"    << tr( "Romanian" ) <<
        "ro_MO" << tr( "Romanian" ) % " - " % tr ( "Moldova" ) <<
        "rm"    << tr( "Romansh" ) <<
        "rn"    << tr( "Rundi" ) <<
        "ru"    << tr( "Russian" ) <<
        "ru_MO" << tr( "Russian" ) % " - " % tr ( "Moldova" ) <<
        "sm"    << tr( "Samoan" ) <<
        "sg"    << tr( "Sango" ) <<
        "sa"    << tr( "Sanskrit" ) <<
        "sc"    << tr( "Sardinian" ) <<
        "sr"    << tr( "Serbian" ) <<
        "sr_SP" << tr( "Serbian" ) % " - " % tr ( "SP" ) <<
        "sn"    << tr( "Shona" ) <<
        "ii"    << tr( "Sichuan Yi" ) <<
        "sd"    << tr( "Sindhi" ) <<
        "si"    << tr( "Sinhala" ) <<
        "sk"    << tr( "Slovak" ) <<
        "sl"    << tr( "Slovenian" ) <<
        "so"    << tr( "Somali" ) <<
        "st"    << tr( "Sotho, Southern" ) <<
        "es"    << tr( "Spanish" ) <<
        "es_AR" << tr( "Spanish" ) % " - " % tr ( "Argentina" ) <<
        "es_BO" << tr( "Spanish" ) % " - " % tr ( "Bolivia" ) <<
        "es_CL" << tr( "Spanish" ) % " - " % tr ( "Chile" ) <<
        "es_CO" << tr( "Spanish" ) % " - " % tr ( "Columbia" ) <<
        "es_CR" << tr( "Spanish" ) % " - " % tr ( "Costa Rica" ) <<
        "es_DO" << tr( "Spanish" ) % " - " % tr ( "Dominican Republic" ) <<
        "es_EC" << tr( "Spanish" ) % " - " % tr ( "Ecuador" ) <<
        "es_SV" << tr( "Spanish" ) % " - " % tr ( "El Salvador" ) <<
        "es_GT" << tr( "Spanish" ) % " - " % tr ( "Guatemala" ) <<
        "es_HN" << tr( "Spanish" ) % " - " % tr ( "Honduras" ) <<
        "es_MX" << tr( "Spanish" ) % " - " % tr ( "Mexico" ) <<
        "es_NI" << tr( "Spanish" ) % " - " % tr ( "Nicaragua" ) <<
        "es_PA" << tr( "Spanish" ) % " - " % tr ( "Panama" ) <<
        "es_PY" << tr( "Spanish" ) % " - " % tr ( "Paraguay" ) <<
        "es_PE" << tr( "Spanish" ) % " - " % tr ( "Peru" ) <<
        "es_PR" << tr( "Spanish" ) % " - " % tr ( "Puerto Rico" ) <<
        "es_ES" << tr( "Spanish" ) % " - " % tr ( "Spain" ) <<
        "es_UY" << tr( "Spanish" ) % " - " % tr ( "Uruguay" ) <<
        "es_VE" << tr( "Spanish" ) % " - " % tr ( "Venezuela" ) <<
        "su"    << tr( "Sundanese" ) <<
        "sw"    << tr( "Swahili" ) <<
        "ss"    << tr( "Swati" ) <<
        "sv"    << tr( "Swedish" ) <<
        "sv_FI" << tr( "Swedish" ) % " - " % tr ( "Finland" ) <<
        "sv_SE" << tr( "Swedish" ) % " - " % tr ( "Sweden" ) <<
        "tl"    << tr( "Tagalog" ) <<
        "ty"    << tr( "Tahitian" ) <<
        "tg"    << tr( "Tajik" ) <<
        "ta"    << tr( "Tamil" ) <<
        "tt"    << tr( "Tatar" ) <<
        "te"    << tr( "Telugu" ) <<
        "th"    << tr( "Thai" ) <<
        "bo"    << tr( "Tibetan" ) <<
        "ti"    << tr( "Tigrinya" ) <<
        "to"    << tr( "Tonga" ) <<
        "ts"    << tr( "Tsonga" ) <<
        "tn"    << tr( "Tswana" ) <<
        "tr"    << tr( "Turkish" ) <<
        "tr_TR" << tr( "Turkish" ) % " - " % tr ( "Turkey" ) <<
        "tk"    << tr( "Turkmen" ) <<
        "tw"    << tr( "Twi" ) <<
        "ug"    << tr( "Uighur" ) <<
        "uk"    << tr( "Ukrainian" ) <<
        "ur"    << tr( "Urdu" ) <<
        "uz"    << tr( "Uzbek" ) <<
        "uz_UX" << tr( "Uzbek" ) % " - " % tr ( "Uzbekistan" ) <<
        "ve"    << tr( "Venda" ) <<
        "vi"    << tr( "Vietnamese" ) <<
        "vo"    << tr( "Volapuk" ) <<
        "wa"    << tr( "Walloon" ) <<
        "cy"    << tr( "Welsh" ) <<
        "fy"    << tr( "Western Frisian" ) <<
        "wo"    << tr( "Wolof" ) <<
        "xh"    << tr( "Xhosa" ) <<
        "yi"    << tr( "Yiddish" ) <<
        "yo"    << tr( "Yoruba" ) <<
        "za"    << tr( "Zhuang" ) <<
        "zu"    << tr( "Zulu" );

    for ( int i = 0; i < data.count(); i++ )
    {
        QString code = data.at(i++);
        QString name = data.at(i);

        m_languageCodeMap.insert( code, name );
        m_languageNameMap.insert( name, code );
    }
}
