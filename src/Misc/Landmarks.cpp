/************************************************************************
**
**  Copyright (C) 2016-2020 Kevin B. Hendricks, Stratford, Ontario, Canada
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
#include "Misc/Landmarks.h"

Landmarks *Landmarks::m_instance = 0;

Landmarks *Landmarks::instance()
{
    if (m_instance == 0) {
        m_instance = new Landmarks();
    }

    return m_instance;
}

Landmarks::Landmarks()
{
    SetLandmarksMap();
    SetGuideLandMap();
    SetCodeToRawTitleMap();
}


QString Landmarks::GetName(const QString &code)
{
    DescriptiveInfo rel = m_CodeMap.value(code, DescriptiveInfo() );
    return rel.name;
}


QString Landmarks::GetTitle(const QString &code, const QString &lang)
{
    if (!m_CodeToRawTitle.contains(code)) return code;

    // Setup the book language translator and load the translation for the selected language
    // Note the book language may differ from the ui language
    bool translation_loaded = false;
    QTranslator bookTranslator;
    const QString qm_name = QString("sigil_%1").arg(lang);
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
    // if no translator matched, use the user interface language so that
    // the dev can fix them manually
    if (!translation_loaded) {
        return GetName(code);
    }
    
    QString title = bookTranslator.translate("Landmarks", m_CodeToRawTitle[code].toUtf8().constData());
    return title;
}


QString Landmarks::GetDescriptionByCode(const QString &code)
{
    DescriptiveInfo rel = m_CodeMap.value(code, DescriptiveInfo());
    return rel.description;
}


QString Landmarks::GetDescriptionByName(const QString &name)
{
    QString code = m_NameMap.value(name, QString());
    return GetDescriptionByCode(code);
}


QString Landmarks::GetCode(const QString &name)
{
    return m_NameMap.value(name, QString());
}

bool Landmarks::isLandmarksCode(const QString &code)
{
    return m_CodeMap.contains(code);
}

bool Landmarks::isLandmarksName(const QString &name)
{
    return m_NameMap.contains(name);
}

QStringList Landmarks::GetSortedNames()
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

const QHash<QString, DescriptiveInfo> & Landmarks::GetCodeMap()
{
    return m_CodeMap;
}

QString Landmarks::GuideLandMapping(const QString &code)
{
  return m_GuideLandMap.value(code, QString());
}

void Landmarks::SetLandmarksMap()
{
    if (!m_CodeMap.isEmpty()) {
        return;
    }

    // See http://www.idpf.org/epub/vocab/structure/ (you need to look at previous versions to find the one
    // specific to the epub 3.0.1 spec

    QStringList data;
    data <<
        tr("Acknowledgments") << "acknowledgments" << tr("A passage containing acknowledgments to entities involved in the realization of the work.") <<
        tr("Afterword") << "afterword" << tr("A closing statement from the author or a person of importance to the story, typically providing insight into how the story came to be written, its significance or related events that have transpired since its timeline.") <<
        tr("Annotation") << "annotation" << tr("Explanatory information about passages in the work. Status: Deprecated") <<
        tr("Appendix") << "appendix" << tr("Supplemental information.") <<
        tr("Assessment") << "assessment" << tr("A test, quiz, or other activity that helps measure a student's understanding of what is being taught.") <<
        tr("Back Matter") << "backmatter" << tr("Ancillary material occurring after the main content of a publication, such as indices, appendices, etc.") <<
        tr("Bibliography") << "bibliography" << tr("A list of works cited.") <<
        tr("Body Matter") << "bodymatter" << tr("The main content of a publication.") <<
        tr("Chapter") << "chapter" << tr("A major structural division of a piece of writing.") <<
        tr("Colophon") << "colophon" << tr("A brief description usually located at the end of a publication, describing production notes relevant to the edition.") <<
        tr("Conclusion") << "conclusion" << tr("An ending section that typically wraps up the work.") <<
        tr("Contributors") << "contributors" << tr("A list of contributors to the work.") <<
        tr("Copyright Page") << "copyright-page" << tr("The copyright page of the work.") <<
        tr("Cover") << "cover" << tr("The publications cover(s), jacket information, etc.") <<
        tr("Dedication") << "dedication" << tr("An inscription addressed to one or several particular person(s).") <<
        tr("Division") << "division" << tr("A major structural division that may also appear as a substructure of a part (esp. in legislation).") <<
        tr("Epigraph") << "epigraph" << tr("A quotation that is pertinent but not integral to the text.") <<
        tr("Epilogue") << "epilogue" << tr("A concluding section that is typically written from a later point in time than the main story, although still part of the narrative.") <<
        tr("Errata") << "errata" << tr("Publication errata, in printed works typically a loose sheet inserted by hand; sometimes a bound page that contains corrections for mistakes in the work.") <<
        tr("Footnotes") << "footnotes" << tr("A collection of notes appearing at the bottom of a page.") <<
        tr("Foreword") << "foreword" << tr("An introductory section that precedes the work, typically not written by the work's author.") <<
        tr("Front Matter") << "frontmatter" << tr("Preliminary material to the main content of a publication, such as tables of contents, dedications, etc.") <<
        tr("Glossary") << "glossary" << tr("An alphabetical list of terms in a particular domain of knowledge, with the definitions for those terms.") <<
        tr("Half Title Page") << "halftitlepage" << tr("The half title page of the work which carries just the title itself.") <<
        tr("Imprimatur") << "imprimatur" << tr("A formal statement authorizing the publication of the work.") <<
        tr("Imprint") << "imprint" << tr("Information relating to the publication or distribution of the work.") <<
        tr("Index") << "index" << tr("A detailed list, usually arranged alphabetically, of the specific information in a publication.") <<
        tr("Introduction") << "introduction" << tr("A section in the beginning of the work, typically introducing the reader to the scope or nature of the work's content.") <<
        tr("Landmarks") << "landmarks" << tr("A collection of references to well-known/recurring components within the publication") <<
        tr("List of Audio Clips") << "loa" << tr("A listing of audio clips included in the work.") <<
        tr("List of Illustrations") << "loi" << tr("A listing of illustrations included in the work.") <<
        tr("List of Tables") << "lot" << tr("A listing of tables included in the work.") <<
        tr("List of Video Clips") << "lov" << tr("A listing of video clips included in the work.") <<
        tr("Notice") << "notice" << tr("Information that requires special attention, and that must not be skipped or suppressed. Examples include: alert, warning, caution, danger, important.") <<
        tr("Other Credits") << "other-credits" << tr("Acknowledgments of previously published parts of the work, illustration credits, and permission to quote from copyrighted material.") <<
        tr("Page List") << "page-list" << tr("A list of references to pagebreaks (start locations) from a print version of the ebook") <<
        tr("Part") << "part" << tr("A major structural division of a piece of writing, typically encapsulating a set of related chapters.") <<
        tr("Preamble") << "preamble" << tr("A section in the beginning of the work, typically containing introductory and/or explanatory prose regarding the scope or nature of the work's content") <<
        tr("Preface") << "preface" << tr("An introductory section that precedes the work, typically written by the work's author.") <<
        tr("Prologue") << "prologue" << tr("An introductory section that sets the background to a story, typically part of the narrative.") <<
        tr("Questions and Answers") << "qna" << tr("A question and answer section.") <<
        tr("Rear Notes") << "rearnotes" << tr("A collection of notes appearing at the rear (backmatter) of the work, or at the end of a section.") <<
        tr("Revision History") << "revision-history" << tr("A record of changes made to a work.") <<
        tr("Subchapter") << "subchapter" << tr("A major sub-division of a chapter.") <<
        tr("Title Page") << "titlepage" << tr("A page at the beginning of a book giving its title, authors, publisher and other publication information.") <<
        tr("Table of Contents") << "toc" << tr("A table of contents which is a list of the headings or parts of the book or document, organized in the order in which they appear. Typically appearing in the work's frontmatter, or at the beginning of a section.") <<
        tr("Volume") << "volume" << tr("A component of a collection.") <<
        tr("Warning") << "warning" << tr("A warning or caution about specific material. Status: Deprecated - Replaced by 'notice'.");


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


void Landmarks::SetGuideLandMap()
{
    if (!m_GuideLandMap.isEmpty()) {
        return;
    }

    m_GuideLandMap[ "acknowledgements" ] = "acknowledgments";
    m_GuideLandMap[ "acknowledgments"  ] = "acknowledgements";
    m_GuideLandMap[ "bibliography"     ] = "bibliography";
    m_GuideLandMap[ "text"             ] = "bodymatter";
    m_GuideLandMap[ "bodymatter"       ] = "text";
    m_GuideLandMap[ "colophon"         ] = "colophon";
    m_GuideLandMap[ "copyright-page"   ] = "copyright-page";
    m_GuideLandMap[ "cover"            ] = "cover";
    m_GuideLandMap[ "dedication"       ] = "dedication";
    m_GuideLandMap[ "epigraph"         ] = "epigraph";
    m_GuideLandMap[ "foreword"         ] = "foreword";
    m_GuideLandMap[ "glossary"         ] = "glossary";
    m_GuideLandMap[ "index"            ] = "index";
    m_GuideLandMap[ "loi"              ] = "loi";
    m_GuideLandMap[ "lot"              ] = "lot";
    m_GuideLandMap[ "preface"          ] = "preface";
    m_GuideLandMap[ "title-page"       ] = "titlepage";
    m_GuideLandMap[ "titlepage"        ] = "title-page";
    m_GuideLandMap[ "toc"              ] = "toc";
    // extended other. entries
    m_GuideLandMap["other.afterword"]     = "afterword";
    m_GuideLandMap["other.appendix"]      = "appendix";
    m_GuideLandMap["other.backmatter"]    = "backmatter";
    m_GuideLandMap["other.conclusion"]    = "conclusion";
    m_GuideLandMap["other.contributors"]  = "contributors";
    m_GuideLandMap["other.epilogue"]      = "epilogue";
    m_GuideLandMap["other.errata"]        = "errata";
    m_GuideLandMap["other.footnotes"]     = "footnotes";
    m_GuideLandMap["other.frontmatter"]   = "frontmatter";
    m_GuideLandMap["other.halftitlepage"] = "halftitlepage";
    m_GuideLandMap["other.imprimatur"]    = "imprimatur";
    m_GuideLandMap["other.imprint"]       = "imprint";
    m_GuideLandMap["other.introduction"]  = "introduction";
    m_GuideLandMap["other.loa"]           = "loa";
    m_GuideLandMap["other.lov"]           = "lov";
    m_GuideLandMap["other.other-credits"] = "other-credits";
    m_GuideLandMap["other.preamble"]      = "preamble";
    m_GuideLandMap["other.prologue"]      = "prologue";
    m_GuideLandMap["other.rearnotes"]     = "rearnotes";
    // and their reverse
    m_GuideLandMap["afterword"]     = "other.afterword";
    m_GuideLandMap["appendix"]      = "other.appendix";
    m_GuideLandMap["backmatter"]    = "other.backmatter";
    m_GuideLandMap["conclusion"]    = "other.conclusion";
    m_GuideLandMap["contributors"]  = "other.contributors";
    m_GuideLandMap["epilogue"]      = "other.epilogue";
    m_GuideLandMap["errata"]        = "other.errata";
    m_GuideLandMap["footnotes"]     = "other.footnotes";
    m_GuideLandMap["frontmatter"]   = "other.frontmatter";
    m_GuideLandMap["halftitlepage"] = "other.halftitlepage";
    m_GuideLandMap["imprimatur"]    = "other.imprimatur";
    m_GuideLandMap["imprint"]       = "other.imprint";
    m_GuideLandMap["introduction"]  = "other.introduction";
    m_GuideLandMap["loa"]           = "other.loa";
    m_GuideLandMap["lov"]           = "other.lov";
    m_GuideLandMap["other-credits"] = "other.other-credits";
    m_GuideLandMap["preamble"]      = "other.preamble";
    m_GuideLandMap["prologue"]      = "other.prologue";
    m_GuideLandMap["rearnotes"]     = "other.rearnotes";
}


void Landmarks::SetCodeToRawTitleMap()
{
    if (!m_CodeToRawTitle.isEmpty()) {
        return;
    }

    m_CodeToRawTitle["acknowledgments"]  = "Acknowledgments";
    m_CodeToRawTitle["afterword"]        = "Afterword";
    m_CodeToRawTitle["annotation"]       = "Annotation";
    m_CodeToRawTitle["appendix"]         = "Appendix";
    m_CodeToRawTitle["assessment"]       = "Assessment";
    m_CodeToRawTitle["backmatter"]       = "Back Matter";
    m_CodeToRawTitle["bibliography"]     = "Bibliography";
    m_CodeToRawTitle["bodymatter"]       = "Body Matter";
    m_CodeToRawTitle["chapter"]          = "Chapter";
    m_CodeToRawTitle["colophon"]         = "Colophon";
    m_CodeToRawTitle["conclusion"]       = "Conclusion";
    m_CodeToRawTitle["contributors"]     = "Contributors";
    m_CodeToRawTitle["copyright-page"]   = "Copyright Page";
    m_CodeToRawTitle["cover"]            = "Cover";
    m_CodeToRawTitle["dedication"]       = "Dedication";
    m_CodeToRawTitle["division"]         = "Division";
    m_CodeToRawTitle["epigraph"]         = "Epigraph";
    m_CodeToRawTitle["epilogue"]         = "Epilogue";
    m_CodeToRawTitle["errata"]           = "Errata";
    m_CodeToRawTitle["footnotes"]        = "Footnotes";
    m_CodeToRawTitle["foreword"]         = "Foreword";
    m_CodeToRawTitle["frontmatter"]      = "Front Matter";
    m_CodeToRawTitle["glossary"]         = "Glossary";
    m_CodeToRawTitle["halftitlepage"]    = "Half Title Page";
    m_CodeToRawTitle["imprimatur"]       = "Imprimatur";
    m_CodeToRawTitle["imprint"]          = "Imprint";
    m_CodeToRawTitle["index"]            = "Index";
    m_CodeToRawTitle["introduction"]     = "Introduction";
    m_CodeToRawTitle["landmarks"]        = "Landmarks";
    m_CodeToRawTitle["loa"]              = "List of Audio Clips";
    m_CodeToRawTitle["loi"]              = "List of Illustrations";
    m_CodeToRawTitle["lot"]              = "List of Tables";
    m_CodeToRawTitle["lov"]              = "List of Video Clips";
    m_CodeToRawTitle["notice"]           = "Notice";
    m_CodeToRawTitle["other-credits"]    = "Other Credits";
    m_CodeToRawTitle["page-list"]        = "Page List";
    m_CodeToRawTitle["part"]             = "Part";
    m_CodeToRawTitle["preamble"]         = "Preamble";
    m_CodeToRawTitle["preface"]          = "Preface";
    m_CodeToRawTitle["prologue"]         = "Prologue";
    m_CodeToRawTitle["qna"]              = "Questions and Answers";
    m_CodeToRawTitle["rearnotes"]        = "Rear Notes";
    m_CodeToRawTitle["revision-history"] = "Revision History";
    m_CodeToRawTitle["subchapter"]       = "Subchapter";
    m_CodeToRawTitle["titlepage"]        = "Title Page";
    m_CodeToRawTitle["toc"]              = "Table of Contents";
    m_CodeToRawTitle["volume"]           = "Volume";
    m_CodeToRawTitle["warning"]          = "Warning";
}
