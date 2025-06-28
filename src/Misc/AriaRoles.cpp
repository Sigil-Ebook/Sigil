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
#include "Misc/AriaRoles.h"

static const QStringList REF_TAGS          = QStringList() << "a";
static const QStringList NOTE_TAGS         = QStringList() << "aside" << "header" << "footer" << "div" << "p";
static const QStringList BREAK_TAGS        = QStringList() << "span" << "hr";
static const QStringList H1H6_TAGS         = QStringList() << "h1" << "h2" << "h3" << "h4" << "h5" << "h6";
static const QStringList SECTION_TAGS      = QStringList() << "section" << "div";
static const QStringList SECTION_SIDE_TAGS = QStringList() << "section" << "div" << "aside" << "p";
static const QStringList SIDE_TAGS         = QStringList() << "aside" << "div" << "p";
static const QStringList NAV_TAGS          = QStringList() << "section" << "nav" << "div"; 
static const QStringList CVR_TAGS          = QStringList() << "img";
static const QStringList ENTRY_TAGS        = QStringList() << "li" << "dt" << "dd" << "div" << "p";
static const QStringList ENTRY_SIDE_TAGS   = QStringList() << "li" << "dt" << "dd" << "aside" << "div" << "p";

static const QStringList NAV_ROLES          = QStringList() << "doc-index" << "doc-pagelist" << "doc-toc";
static const QStringList REF_ROLES          = QStringList() << "doc-backlink" << "doc-biblioref" <<
                                                  "doc-glossref" << "doc-noteref";
static const QStringList SECTION_SIDE_ROLES = QStringList() << "doc-dedication" << "doc-example" <<
                                                  "doc-glossary" << "doc-pullquote";

AriaRoles *AriaRoles::m_instance = 0;

AriaRoles *AriaRoles::instance()
{
    if (m_instance == 0) {
        m_instance = new AriaRoles();
    }

    return m_instance;
}

AriaRoles::AriaRoles()
{
    SetAriaRolesMap();
    SetEpubTypeMap();
    SetCodeToRawTitleMap();
}


QString AriaRoles::GetName(const QString &code)
{
    DescriptiveInfo rel = m_CodeMap.value(code, DescriptiveInfo() );
    return rel.name;
}


QString AriaRoles::GetTitle(const QString &code, const QString &lang)
{
    if (!m_CodeToRawTitle.contains(code)) return code;

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
    // if no translator matched, use the user interface language so that
    // the dev can fix them manually
    if (!translation_loaded) {
        return GetName(code);
    }
    QString title = bookTranslator.translate("AriaRoles", m_CodeToRawTitle[code].toUtf8().constData());
    return title;
}


QString AriaRoles::GetDescriptionByCode(const QString &code)
{
    DescriptiveInfo rel = m_CodeMap.value(code, DescriptiveInfo());
    return rel.description;
}


QString AriaRoles::GetDescriptionByName(const QString &name)
{
    QString code = m_NameMap.value(name, QString());
    return GetDescriptionByCode(code);
}


QString AriaRoles::GetCode(const QString &name)
{
    return m_NameMap.value(name, QString());
}

bool AriaRoles::isAriaRolesCode(const QString &code)
{
    return m_CodeMap.contains(code);
}

bool AriaRoles::isAriaRolesName(const QString &name)
{
    return m_NameMap.contains(name);
}

QStringList AriaRoles::GetSortedNames()
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

QStringList AriaRoles::GetAllCodes()
{
    return m_CodeMap.keys();
}

const QHash<QString, DescriptiveInfo>& AriaRoles::GetCodeMap()
{
    return m_CodeMap;
}

QString AriaRoles::EpubTypeMapping(const QString &code)
{
  return m_EpubTypeMap.value(code, QString());
}

void AriaRoles::SetAriaRolesMap()
{
    if (!m_CodeMap.isEmpty()) {
        return;
    }

    QStringList data;
    data <<
        tr("Abstract") << "doc-abstract" << tr("A short summary of the principle ideas, concepts and conclusions of the work, or of section or excerpt within it.") <<
        tr("Acknowledgments") << "doc-acknowledgments" << tr("A section or statement that acknowledges significant contributions by persons, organizations, governments, and other entities to the realization of the work.") <<
        tr("Afterword") << "doc-afterword" << tr("A closing statement from the author or a person of importance, typically providing insight into how the content came to be written, its significance or related events that have transpired since its timeline.") <<
        tr("Appendix") << "doc-appendix" << tr("A section of supplemental information located after the primary content that informs the content but is not central to it.") <<
        tr("Back Link") << "doc-backlink" << tr("A link that allows the user to return to a related location in the content (e.g., from a footnote to its references or from a glossary definition to where a term is used.)") <<
        tr("Bibliography") << "doc-bibliography" << tr("A list of external references cited in the work, which may be to print or digital sources.") <<
        tr("Bibliography Entry") << "biblioentry" << tr("A single reference to an external source in a bibliography. [epub:type ONLY]") <<
        tr("Bibliography Reference") << "doc-biblioref" << tr("A reference to a bibliography entry.") <<
        tr("Chapter") << "doc-chapter" << tr("A major thematic section of content in a work.") <<
        tr("Colophon") << "doc-colophon" << tr("A short section of production notes particular to the edition (e.g., describing the typeface used), often located at the end of a work.") <<
        tr("Conclusion") << "doc-conclusion" << tr("A concluding section or statement that summarizes the work or wraps up the narrative.") <<
        tr("Cover") << "doc-cover" << tr("An image that sets the mood or tone for the work and typically includes the title and author.") <<
        tr("Credit") << "doc-credit" << tr("An acknowledgement of the source of integrated content from third-party sources, such as photos.  Typically identifies the creator, copyright, and any restrictions on reuse.") <<
        tr("Credits") << "doc-credits" << tr("A collection of credits.") <<
        tr("Dedication") << "doc-dedication" << tr("An inscription at the front of the work, typically addressed in tribute to one or more persons close to the author.") <<
        tr("Endnote") << "endnote" << tr("One of a collection of notes that occur at the end of a work, or a section within it that provides additional context to a referenced passage of text. [epub:type ONLY]") <<
        tr("Endnotes") << "doc-endnotes" << tr("A collection of notes at the end of a work or a section within it.") <<
        tr("Epigraph") << "doc-epigraph" << tr("A quotation set at the start of the work or a section that establishes the theme or sets the mood.") <<
        tr("Epilogue") << "doc-epilogue" << tr("A concluding section of narrative that wraps up or comments on the actions and events of the work, typically from a future perspective.") <<
        tr("Errata") << "doc-errata" << tr("A set of corrections discovered after initial publication of the work, sometimes referred to as corrigenda.") <<
        tr("Example") << "doc-example" << tr("An illustration of a key concept of the work, such as a code listing, case study or problem. [aria role ONLY]") <<
        tr("Footnote") << "doc-footnote" << tr("Ancillary information, such as a citation or commentary, that provides additional context to a referenced passage of text.") <<
        tr("Footnotes") << "footnotes" << tr("A collection of footnotes. [epub:type ONLY]") <<
        tr("Foreword") << "doc-foreword" << tr("An introductory section that precedes the work, typically not written by the author of the work.") <<
        tr("Glossary") << "doc-glossary" << tr("A brief dictionary of new, uncommon, or specialized terms used in the content.") <<
        tr("Glossary Reference") << "doc-glossref" << tr("A reference to a glossary definition.") <<
        tr("Index") << "doc-index" << tr("A navigational aid that provides a detailed list of links to key subjects, names and other important topics covered in the work.") <<
        tr("Introduction") << "doc-introduction" << tr("A preliminary section that typically introduces the scope or nature of the work.") <<
        tr("Note Reference") << "doc-noteref" << tr("A reference to a footnote or endnote, typically appearing as a superscripted number or symbol in the main body of the text.") <<
        tr("Notice") << "doc-notice" << tr("Notifies the user of consequences that might arise from an action or event.  Examples include warnings, cautions and dangers.") <<
        tr("Pagebreak") << "doc-pagebreak" << tr("A separator denoting the position before which a break occurs between contiguous pages in a statically paginated version of the content.") <<
        tr("Page Footer") << "doc-pagefooter" << tr("A section of text appearing at the bottom of a page that provides context about the current work and location within it. The page footer is distinct from the body text and normally follows a repeating template that contains (possibly truncated) items such as the document title, current section, author name(s), and page number.") <<
        tr("Page Header") << "doc-pageheader" << tr("A section of text appearing at the top of a page that provides context about the current work and location within it. The page header is distinct from the body text and normally follows a repeating template that contains (possibly truncated) items such as the document title, current section, author name(s), and page number.") <<
        tr("Page List") << "doc-pagelist" << tr("A navigational aid that provides a list of links to the page breaks in the content.") <<
        tr("Part") << "doc-part" << tr("A major structural division in a work that contains a set of related sections dealing with a particular subject, narrative arc, or similar encapsulated theme.") <<
        tr("Preamble") << "preamble" << tr("A section at the beginning of a work, typically containing introductory and/or explanatory prose regarding the scope or nature of the work's content. [epub:type ONLY]") <<
        tr("Preface") << "doc-preface" << tr("An introductory section that precedes the work, typically written by the author of the work.") <<
        tr("Prologue") << "doc-prologue" << tr("An introductory section that sets the background to a work, typically part of the narrative.") <<
        tr("Pull Quote") << "doc-pullquote" << tr("A distinctively placed or highlighted quotation from the current content designed to draw attention to a topic or highlight a key point.") <<
        tr("Questions and Answers") << "doc-qna" << tr("A section of content structured as a series of questions and answers, such as an interview or list of frequently asked questions.") <<
        tr("Subtitle") << "doc-subtitle" << tr("An explanatory or alternate title for the work, or a section or components within it.") <<
        tr("Tip") << "doc-tip" << tr("Helpful information that clarifies some aspect of the content or assists in comprehension.") <<
        tr("Table of Contents") << "doc-toc" << tr("A navigational aid that provides an ordered list of links to the major sectional headings in the content.  A table of contents may cover an entire work, or only a smaller section of it.");
 

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


void AriaRoles::SetEpubTypeMap()
{
    if (!m_EpubTypeMap.isEmpty()) {
        return;
    }

    m_EpubTypeMap[ "doc-abstract"        ] = "abstract";
    m_EpubTypeMap[ "doc-acknowledgments" ] = "acknowledgments";
    m_EpubTypeMap[ "doc-afterword"       ] = "afterword";
    m_EpubTypeMap[ "doc-appendix"        ] = "appendix";
    m_EpubTypeMap[ "doc-backlink"        ] = "backlink";
    m_EpubTypeMap[ "doc-bibliography"    ] = "bibliography";
    m_EpubTypeMap[ "biblioentry"         ] = "biblioentry";   // epub:type ONLY
    m_EpubTypeMap[ "doc-biblioref"       ] = "biblioref";
    m_EpubTypeMap[ "doc-chapter"         ] = "chapter";
    m_EpubTypeMap[ "doc-colophon"        ] = "colophon";
    m_EpubTypeMap[ "doc-conclusion"      ] = "conclusion";
    m_EpubTypeMap[ "doc-cover"           ] = "cover-image";
    m_EpubTypeMap[ "doc-credit"          ] = "credit";
    m_EpubTypeMap[ "doc-credits"         ] = "credits";
    m_EpubTypeMap[ "doc-dedication"      ] = "dedication";
    m_EpubTypeMap[ "endnote"             ] = "endnote";       // epub:type ONLY
    m_EpubTypeMap[ "doc-endnotes"        ] = "endnotes";
    m_EpubTypeMap[ "doc-epigraph"        ] = "epigraph";
    m_EpubTypeMap[ "doc-epilogue"        ] = "epilogue";
    m_EpubTypeMap[ "doc-errata"          ] = "errata";
    m_EpubTypeMap[ "doc-example"         ] = "";              // aria role ONLY 
    m_EpubTypeMap[ "doc-footnote"        ] = "footnote";
    m_EpubTypeMap[ "footnotes"           ] = "footnotes";     // epub:type ONLY
    m_EpubTypeMap[ "doc-foreword"        ] = "foreword";
    m_EpubTypeMap[ "doc-glossary"        ] = "glossary";
    m_EpubTypeMap[ "doc-glossref"        ] = "glossref";
    m_EpubTypeMap[ "doc-index"           ] = "index";
    m_EpubTypeMap[ "doc-introduction"    ] = "introduction";
    m_EpubTypeMap[ "doc-noteref"         ] = "noteref";
    m_EpubTypeMap[ "doc-notice"          ] = "notice";
    m_EpubTypeMap[ "doc-pagebreak"       ] = "pagebreak";
    m_EpubTypeMap[ "doc-pagelist"        ] = "page-list";
    m_EpubTypeMap[ "doc-part"            ] = "part";
    m_EpubTypeMap[ "preamble"            ] = "preamble";      // epub:type ONLY
    m_EpubTypeMap[ "doc-preface"         ] = "preface";
    m_EpubTypeMap[ "doc-prologue"        ] = "prologue";
    m_EpubTypeMap[ "doc-pullquote"       ] = "pullquote";
    m_EpubTypeMap[ "doc-qna"             ] = "qna";
    m_EpubTypeMap[ "doc-subtitle"        ] = "subtitle";
    m_EpubTypeMap[ "doc-tip"             ] = "tip";
    m_EpubTypeMap[ "doc-toc"             ] = "toc";
}


void AriaRoles::SetCodeToRawTitleMap()
{
    if (!m_CodeToRawTitle.isEmpty()) {
        return;
    }
    m_CodeToRawTitle[ "doc-abstract"        ] = "Abstract";
    m_CodeToRawTitle[ "doc-acknowledgments" ] = "Acknowledgments";
    m_CodeToRawTitle[ "doc-afterword"       ] = "Afterword";
    m_CodeToRawTitle[ "doc-appendix"        ] = "Appendix";
    m_CodeToRawTitle[ "doc-backlink"        ] = "Back Link";
    m_CodeToRawTitle[ "doc-bibliography"    ] = "Bibliography";
    m_CodeToRawTitle[ "biblioentry"         ] = "Bibliography Entry";
    m_CodeToRawTitle[ "doc-biblioref"       ] = "Bibliography Reference";
    m_CodeToRawTitle[ "doc-chapter"         ] = "Chapter";
    m_CodeToRawTitle[ "doc-colophon"        ] = "Colophon";
    m_CodeToRawTitle[ "doc-conclusion"      ] = "Conclusion";
    m_CodeToRawTitle[ "doc-cover"           ] = "Cover";
    m_CodeToRawTitle[ "doc-credit"          ] = "Credit";
    m_CodeToRawTitle[ "doc-credits"         ] = "Credits";
    m_CodeToRawTitle[ "doc-dedication"      ] = "Dedication";
    m_CodeToRawTitle[ "endnote"             ] = "Endnote";
    m_CodeToRawTitle[ "doc-endnotes"        ] = "Endnotes";
    m_CodeToRawTitle[ "doc-epigraph"        ] = "Epigraph";
    m_CodeToRawTitle[ "doc-epilogue"        ] = "Epilogue";
    m_CodeToRawTitle[ "doc-errata"          ] = "Errata";
    m_CodeToRawTitle[ "doc-example"         ] = "Example";
    m_CodeToRawTitle[ "doc-footnote"        ] = "Footnote";
    m_CodeToRawTitle[ "footnotes"           ] = "Footnotes";
    m_CodeToRawTitle[ "doc-foreword"        ] = "Foreword";
    m_CodeToRawTitle[ "doc-glossary"        ] = "Glossary";
    m_CodeToRawTitle[ "doc-glossref"        ] = "Glossary Reference";
    m_CodeToRawTitle[ "doc-index"           ] = "Index";
    m_CodeToRawTitle[ "doc-introduction"    ] = "Introduction";
    m_CodeToRawTitle[ "doc-noteref"         ] = "Note Reference";
    m_CodeToRawTitle[ "doc-notice"          ] = "Notice";
    m_CodeToRawTitle[ "doc-pagebreak"       ] = "Pagebreak";
    m_CodeToRawTitle[ "doc-pagefooter"      ] = "Page Footer";
    m_CodeToRawTitle[ "doc-pageheader"      ] = "Page Header";
    m_CodeToRawTitle[ "doc-pagelist"        ] = "Page List";
    m_CodeToRawTitle[ "doc-part"            ] = "Part";
    m_CodeToRawTitle[ "preamble"            ] = "Preamble";
    m_CodeToRawTitle[ "doc-preface"         ] = "Preface";
    m_CodeToRawTitle[ "doc-prologue"        ] = "Prologue";
    m_CodeToRawTitle[ "doc-pullquote"       ] = "Pull Quote";
    m_CodeToRawTitle[ "doc-qna"             ] = "Questions and Answers";
    m_CodeToRawTitle[ "doc-subtitle"        ] = "Subtitle";
    m_CodeToRawTitle[ "doc-tip"             ] = "Tip";
    m_CodeToRawTitle[ "doc-toc"             ] = "Table of Contents";
}

// According to the aria spec the following elements accept any role value:
// But most of these combinations make no sense whatsoever
//
// a (without an href), abbr, address, b, bdi, bdo, blockquote, canvas, cite, code, 
// data, del, dfn, div, em, figure (no figcaption), hgroup, i, ins, kbd, mark, output,
// p, pre, q, rp, rt, ruby, s, samp, small, span, strong, sub, sup, svg, table, tbody,
// tfoot, thead, [th, td, tr (if table not exposed as table, grid, or treegrid)], 
// time, u, var
//
// So use a more limited but normal list of allowed tags

QStringList AriaRoles::AllowedTags(const QString& code)
{
    if (REF_ROLES.contains(code)) return REF_TAGS;
    if (NAV_ROLES.contains(code)) return NAV_TAGS;
    if (SECTION_SIDE_ROLES.contains(code)) return SECTION_SIDE_TAGS;
    if (code == "doc-footnote") return NOTE_TAGS;
    if (code == "doc-cover") return CVR_TAGS;
    if (code == "doc-pagebreak") return BREAK_TAGS;
    if (code == "doc-subtitle") return H1H6_TAGS; 
    if (code == "doc-tip") return SIDE_TAGS;
    if (code == "biblioentry") return ENTRY_TAGS;  // epub:type ONLY
    if (code == "footnotes") return SECTION_TAGS;  // epub:type ONLY
    if (code == "endnote") return ENTRY_SIDE_TAGS; // epub:type ONLY
    if (code == "preamble") return SECTION_TAGS;   // epub:type ONLY
    // all others
    // doc-abstract, doc-acknowledgments, doc-afterword, doc-appendix, doc-bibliography
    // doc-chapter, doc-colophon, doc-conclusion, doc-credit, doc-credits
    // doc-endnotes, doc-epigraph, doc-epilogue, doc-errata, doc-foreword
    // doc-introduction, doc-notice, doc-part, doc-preface, doc-prologue, doc-qna
    return SECTION_TAGS;
}


