/************************************************************************
**
**  Copyright (C) 2025 Kevin B. Hendricks, Stratford, Ontario, Canada
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
#include <QStringList>
#include <QHash>
#include <QTranslator>
#include <QDir>

#include "Misc/SettingsStore.h"
#include "Misc/UILanguage.h"
#include "Misc/AriaClips.h"

static const QString ph_chapter_title       = AriaClips::tr("CHAPTER_TITLE_HERE");

static const QString ph_href_to_endnote     = AriaClips::tr("HREF_TO_ENDNOTE");
static const QString ph_href_endnote_return = AriaClips::tr("HREF_RETURN_FROM_ENDNOTE");

static const QString ph_fn_book_label       = AriaClips::tr("Footnotes");
static const QString ph_tip_book_label      = AriaClips::tr("Tip");
static const QString ph_sidebar_book_label  = AriaClips::tr("Sidebar");
static const QString ph_page_book_label     = AriaClips::tr("Page");

static const QString ph_backlink_book_label = AriaClips::tr("Back to");

static const QString ph_fn_link_book_label = AriaClips::tr("To footnote");
static const QString ph_en_link_book_label = AriaClips::tr("To endnote");

AriaClips *AriaClips::m_instance = 0;

AriaClips *AriaClips::instance()
{
    if (m_instance == 0) {
        m_instance = new AriaClips();
    }

    return m_instance;
}

AriaClips::AriaClips()
{
    SetAriaClipsMap();
    SetCodeToRawTitleMap();
}


QString AriaClips::GetName(const QString &code)
{
    DescriptiveInfo rel = m_CodeMap.value(code, DescriptiveInfo() );
    return rel.name;
}


QString AriaClips::GetBookTranslationFor(const QString& term, const QString& lang, const QString& fallback)
{
    // Setup the book language translator and load the translation for the selected language
    // Note the book language may differ from the ui language
    bool translation_loaded = false;
    QTranslator bookTranslator;
    QString qm_name = QString("sigil_%1").arg(lang);
    // Run though all locations and stop once we find and are able to load
    // an appropriate translation.
    foreach(QString path, UILanguage::GetPossibleTranslationPaths()) {
        if (QDir(path).exists()) {
            if (bookTranslator.load(qm_name, path)) {
                translation_loaded = true;
                break;
            }
        }
    }
    // try again with just the base part of the langcode before any "-"
    if (!translation_loaded && lang.contains("-")) {
        const QString langcut = lang.split("-").at(0);
        qm_name = QString("sigil_%1").arg(langcut);
        foreach(QString path, UILanguage::GetPossibleTranslationPaths()) {
            if (QDir(path).exists()) {
                if (bookTranslator.load(qm_name, path)) {
                    translation_loaded = true;
                    break;
                }
            }
        }
    }    
    if (!translation_loaded) {
        return fallback;
    }
    QString title = bookTranslator.translate("AriaClips", term.toUtf8().constData());
    return title;
}


QString AriaClips::GetTitle(const QString &code, const QString &lang)
{
    if (!m_CodeToRawTitle.contains(code)) return code;
    QString title = GetBookTranslationFor(m_CodeToRawTitle[code], lang, GetName(code));
    return title;
}


QString AriaClips::GetDescriptionByCode(const QString &code)
{
    DescriptiveInfo rel = m_CodeMap.value(code, DescriptiveInfo());
    return rel.description;
}


QString AriaClips::GetDescriptionByName(const QString &name)
{
    QString code = m_NameMap.value(name, QString());
    return GetDescriptionByCode(code);
}


QString AriaClips::GetCode(const QString &name)
{
    return m_NameMap.value(name, QString());
}

bool AriaClips::isAriaClipsCode(const QString &code)
{
    return m_CodeMap.contains(code);
}

bool AriaClips::isAriaClipsName(const QString &name)
{
    return m_NameMap.contains(name);
}

QStringList AriaClips::GetSortedNames()
{
    if (!m_sortedNames.isEmpty()) {
        return m_sortedNames;
    }

    foreach(QString code, m_CodeMap.keys()) {
        DescriptiveInfo rel = m_CodeMap.value(code);
        m_sortedNames.append(rel.name);
    }
    m_sortedNames.sort();
    return m_sortedNames;
}

QStringList AriaClips::GetAllCodes()
{
    return m_CodeMap.keys();
}

const QHash<QString, DescriptiveInfo>& AriaClips::GetCodeMap()
{
    return m_CodeMap;
}


void AriaClips::SetAriaClipsMap()
{
    if (!m_CodeMap.isEmpty()) {
        return;
    }

    QStringList data;
    data <<
        
tr("Aside") << "aside" << "<aside>\n  \\1\n</aside>\n" <<

tr("Aside: Sidebar") << "sidebar" << "<aside aria-label=\"LABEL_FOR_SIDEBAR\">\n  \\1\n</aside>\n" <<

tr("Aside: Tip") << "tip" << "<aside role=\"doc-tip\" epub:type=\"tip\" aria-label=\"LABEL_FOR_TIP\">\n  \\1\n</aside>\n" <<

tr("Backlink from Footnote") << "fn_backlink" << "<a href=\"#ref_fn_N_\" epub:type=\"backlink\" role=\"doc-backlink\">LABEL_FOR_FOOTNOTE_BACKLINK [_N_]</a>" <<

tr("Backlink from Endnote") << "endnote_backlink" << "<a href=\"HREF_RETURN_FROM_ENDNOTE#en_N_\" epub:type=\"backlink\" role=\"doc-backlink\">LABEL_FOR_ENDNOTE_BACKLINK [_N_]</a>" <<

tr("Endnote in li") << "endnote_li" << "    <li id=\"en_N_\" epub:type=\"endnote\">\n      <p>\\1\n        <a href=\"HREF_RETURN_FROM_ENDNOTE#en_N_\" epub:type=\"backlink\" role=\"doc-backlink\">LABEL_FOR_ENDNOTE_BACKLINK [_N_]</a>\n      </p>\n    </li>\n" <<

tr("Footnote in aside") << "fn_aside" << "<aside id=\"fn_N_\" epub:type=\"footnote\" role=\"doc-footnote\">\n  <p>\\1\n    <a href=\"#ref_fn_N_\" epub:type=\"backlink\" role=\"doc-backlink\">LABEL_FOR_FOOTNOTE_BACKLINK [_N_]</a>\n  </p>\n</aside>\n" <<

tr("Footnote in div") << "fn_div" << "<div id=\"fn_N_\" epub:type=\"footnote\" role=\"doc-footnote\">\n  <p>\\1\n    <a href=\"#ref_fn_N_\" epub:type=\"backlink\" role=\"doc-backlink\">LABEL_FOR_FOOTNOTE_BACKLINK [_N_]</a>\n  </p>\n</div>\n" <<

tr("Footnote in p") << "fn_p" << "<p id=\"fn_N_\" epub:type=\"footnote\" role=\"doc-footnote\">\n    \\1\n     <a href=\"#ref_fn_N_\" epub:type=\"backlink\" role=\"doc-backlink\">LABEL_FOR_FOOTNOTE_BACKLINK [_N_]</a>\n  </p>\n" <<

tr("PageBreak in hr") << "pagebreak_hr" << "<hr epub:type=\"pagebreak\" role=\"doc-pagebreak\" />\n" <<

tr("PageBreak in span") << "pagebreak_span" << "<span id=\"page_N_\" epub:type=\"pagebreak\" role=\"doc-pagebreak\" aria-label=\"LABEL_FOR_PAGE _N_\" />" <<

tr("Reference to Footnote") << "fn_ref" << "<a id=\"ref_fn_N_\" href=\"#fn_N_\" epub:type=\"noteref\" role=\"doc-noteref\" aria-label=\"LABEL_FOR_FOOTNOTE_REFERENCE _N_\">[_N_]</a>" <<

tr("Reference to Endnote") << "endnote_ref" << "<a id=\"ref_en_N_\" href=\"HREF_TO_ENDNOTE#en_N_\" epub:type=\"noteref\" role=\"doc-noteref\" aria-label=\"LABEL_FOR_ENDNOTE_REFERENCE _N_\">[_N_]</a>" <<

tr("Section") << "section" << "<section>\n  \\1\n</section>\n" <<

tr("Section: Chapter")<< "chapter" << "<section role=\"doc-chapter\" epub:type=\"chapter\" aria-labelledby=\"heading1\">\n  <h1 id=\"heading1\">CHAPTER_TITLE_HERE</h1>\n  \\1\n</section>\n" <<

tr("Section: Endnotes") << "endnotes" << "<section epub:type=\"endnotes\" role=\"doc-endnotes\">\n  <ol>\n  </ol>\n</section>\n" <<

tr("Section: Footnotes") << "footnotes" << "<section epub:type=\"footnotes\" aria-label=\"LABEL_FOR_FOOTNOTES\">\n  \\1\n</section>\n";

    for (int i = 0; i < data.count(); i++) {
        QString name = data.at(i++);
        QString code = data.at(i++);
        QString description = data.at(i);
        DescriptiveInfo rel;
        rel.name = name;
        rel.description  = description;
        m_CodeMap.insert(code, rel);
        m_NameMap.insert(name, code);
    }
}


void AriaClips::SetCodeToRawTitleMap()
{
    if (!m_CodeToRawTitle.isEmpty()) {
        return;
    }
    m_CodeToRawTitle[ "section"          ] = "Section";
    m_CodeToRawTitle[ "aside"            ] = "Aside";
    m_CodeToRawTitle[ "chapter"          ] = "Chapter";
    m_CodeToRawTitle[ "footnotes"        ] = "Footnotes";
    m_CodeToRawTitle[ "fn ref"           ] = "Footnote reference";
    m_CodeToRawTitle[ "fn_backlink"      ] = "Backlink from Footnote";
    m_CodeToRawTitle[ "fn_aside"         ] = "Footnote in aside";
    m_CodeToRawTitle[ "fn_div"           ] = "Footnote in div";
    m_CodeToRawTitle[ "fn_p"             ] = "Footnote in p";
    m_CodeToRawTitle[ "endnote_ref"      ] = "Endnote reference";
    m_CodeToRawTitle[ "endnote_backlink" ] = "Backlink from Endnote";
    m_CodeToRawTitle[ "endnotes"         ] = "Endnotes";
    m_CodeToRawTitle[ "endnote_li"       ] = "Endnote in li";
    m_CodeToRawTitle[ "sidebar"          ] = "Sidebar";
    m_CodeToRawTitle[ "tip"              ] = "Tip";
    m_CodeToRawTitle[ "pagebreak_hr"     ] = "Pagebreak in hr";
    m_CodeToRawTitle[ "pagebreak_span"   ] = "PageBreak in span";
}


QString AriaClips::TranslatePlaceholders(const QString& cliptext, const QString& book_lang)
{
    QString newtext = cliptext;
    
    // Placeholders should be translated in UI lang as they are meant for epub dev to fill in
    newtext.replace("CHAPTER_TITLE_HERE", ph_chapter_title);
    newtext.replace("HREF_TO_ENDNOTE", ph_href_to_endnote);
    newtext.replace("HREF_RETURN_FROM_ENDNOTE", ph_href_endnote_return);
    
    // aria-label attributes and any other reader facing text should be in the book language    
    QString fns_lbl = GetBookTranslationFor("Footnotes", book_lang, ph_fn_book_label); 
    newtext.replace("LABEL_FOR_FOOTNOTES", fns_lbl);
    
    QString tip_lbl = GetBookTranslationFor("Tip", book_lang, ph_tip_book_label); 
    newtext.replace("LABEL_FOR_TIP", tip_lbl);
    
    QString sidebar_lbl = GetBookTranslationFor("Sidebar", book_lang, ph_sidebar_book_label); 
    newtext.replace("LABEL_FOR_SIDEBAR", sidebar_lbl);
    
    QString page_lbl = GetBookTranslationFor("Page", book_lang, ph_page_book_label); 
    newtext.replace("LABEL_FOR_PAGE", page_lbl);

    QString fn_backlink_lbl = GetBookTranslationFor("Back to", book_lang, ph_backlink_book_label); 
    newtext.replace("LABEL_FOR_FOOTNOTE_BACKLINK", fn_backlink_lbl);

    QString en_backlink_lbl = GetBookTranslationFor("Back to", book_lang, ph_backlink_book_label); 
    newtext.replace("LABEL_FOR_ENDNOTE_BACKLINK", en_backlink_lbl);

    QString fn_reference_lbl = GetBookTranslationFor("To footnote", book_lang, ph_fn_link_book_label); 
    newtext.replace("LABEL_FOR_FOOTNOTE_REFERENCE", fn_reference_lbl);

    QString en_reference_lbl = GetBookTranslationFor("To endnote", book_lang, ph_en_link_book_label); 
    newtext.replace("LABEL_FOR_ENDNOTE_REFERENCE", en_reference_lbl);

    return newtext;
}


