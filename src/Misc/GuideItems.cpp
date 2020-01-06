/************************************************************************
**
**  Copyright (C) 2016-2020 Kevin B. Hendricks, Stratford Ontario Canada
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
// #include <QDebug>

#include "Misc/SettingsStore.h"
#include "Misc/UILanguage.h"
#include "Misc/GuideItems.h"

GuideItems *GuideItems::m_instance = 0;

GuideItems *GuideItems::instance()
{
    if (m_instance == 0) {
        m_instance = new GuideItems();
    }

    return m_instance;
}


GuideItems::GuideItems()
{
    SetGuideItemsMap();
    SetCodeToRawTitleMap();
}


QString GuideItems::GetName(const QString &code)
{
    DescriptiveInfo rel = m_CodeMap.value(code, DescriptiveInfo() );
    if (rel.name.isEmpty()) return code;
    return rel.name;
}


QString GuideItems::GetTitle(const QString &code, const QString &lang)
{
    // qDebug() << "GuideItems: " << code << lang;
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
                // qDebug() << "Loaded bookTranslator: " << qm_name << path;
                break;
            }
        }
    }
    // if no translator matched, use the user interface language so that
    // the dev can fix them manually
    if (!translation_loaded) {
        return GetName(code);
    }
    
    QString title = bookTranslator.translate("GuideItems", m_CodeToRawTitle[code].toUtf8().constData());
    if (title.isEmpty()) {
        title = bookTranslator.translate("Landmarks", m_CodeToRawTitle[code].toUtf8().constData());
    }
    return title;
}


QString GuideItems::GetDescriptionByCode(const QString &code)
{
    DescriptiveInfo rel = m_CodeMap.value(code, DescriptiveInfo());
    return rel.description;
}


QString GuideItems::GetDescriptionByName(const QString &name)
{
    QString code = m_NameMap.value(name, QString());
    return GetDescriptionByCode(code);
}


QString GuideItems::GetCode(const QString &name)
{
    return m_NameMap.value(name, QString());
}

bool GuideItems::isGuideItemsCode(const QString &code)
{
    return m_CodeMap.contains(code);
}

bool GuideItems::isGuideItemsName(const QString &name)
{
    return m_NameMap.contains(name);
}

QStringList GuideItems::GetSortedNames()
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

const QHash<QString, DescriptiveInfo> & GuideItems::GetCodeMap()
{
    return m_CodeMap;
}


void GuideItems::SetGuideItemsMap()
{
    if (!m_CodeMap.isEmpty()) {
        return;
    }

    // For the authoritiative relator list and descriptive definitions see http://www.loc.gov/marc/relators/
    // Names and codes must be unique between basic and advanced (except Publisher).
    QStringList data;
    data <<
        tr("Acknowledgements") << "acknowledgements" << tr("A passage containing acknowledgments to entities involved in the realization of the work.") <<
	tr("Afterword [other.]") << "other.afterword" << tr("A closing statement from the author or a person of importance to the story, typically providing insight into how the story came to be written, its significance or related events that have transpired since its timeline.") <<
	tr("Appendix [other.]") << "other.appendix" << tr("Supplemental information.") <<
	tr("Back Matter [other.]") << "other.backmatter" << tr("Ancillary material occurring after the main content of a publication, such as indices, appendices, etc.") <<
        tr("Bibliography") << "bibliography" << tr("A list of works cited.") <<
        tr("Text") << "text" << tr("The start of the main text content of a publication.") <<
        tr("Colophon") << "colophon" << tr("A brief description usually located at the end of a publication, describing production notes relevant to the edition.") <<
	tr("Conclusion [other.]") << "other.conclusion" << tr("An ending section that typically wraps up the work.") <<
	tr("Contributors [other.]") << "other.contributors" << tr("A list of contributors to the work.") <<
        tr("Copyright Page") << "copyright-page" << tr("The copyright page of the work.") <<
        tr("Cover") << "cover" << tr("The publications cover(s), jacket information, etc.") <<
        tr("Dedication") << "dedication" << tr("An inscription addressed to one or several particular person(s).") <<
	tr("Epilogue [other.]") << "other.epilogue" << tr("A concluding section that is typically written from a later point in time than the main story, although still part of the narrative.") <<
        tr("Epigraph") << "epigraph" << tr("A quotation that is pertinent but not integral to the text.") <<
	tr("Errata [other.]") << "other.errata" << tr("Publication errata, in printed works typically a loose sheet inserted by hand; sometimes a bound page that contains corrections for mistakes in the work.") <<
	tr("Footnotes [other.]") << "other.footnotes" << tr("A collection of notes appearing at the bottom of a page.") <<
        tr("Foreword") << "foreword" << tr("An introductory section that precedes the work, typically not written by the work's author.") <<
	tr("Front Matter [other.]") << "other.frontmatter" << tr("Preliminary material to the main content of a publication, such as tables of contents, dedications, etc.") <<
        tr("Glossary") << "glossary" << tr("An alphabetical list of terms in a particular domain of knowledge, with the definitions for those terms.") <<
	tr("Half Title Page [other.]") << "other.halftitlepage" << tr("The half title page of the work which carries just the title itself.") <<
	tr("Imprimatur [other.]") << "other.imprimatur" << tr("A formal statement authorizing the publication of the work.") <<
	tr("Imprint [other.]") << "other.imprint" << tr("Information relating to the publication or distribution of the work.") <<
        tr("Index") << "index" << tr("A detailed list, usually arranged alphabetically, of the specific information in a publication.") <<
	tr("Introduction [other.]") << "other.introduction" << tr("A section in the beginning of the work, typically introducing the reader to the scope or nature of the work's content.") <<
        tr("List of Illustrations") << "loi" << tr("A listing of illustrations included in the work.") <<
	tr("List of Audio Clips [other.]") << "other.loa" << tr("A listing of audio clips included in the work.") <<
        tr("List of Tables") << "lot" << tr("A listing of tables included in the work.") <<
	tr("List of Video Clips [other.]") << "other.lov" << tr("A listing of video clips included in the work.") <<
        tr("Notes") << "notes" << tr("A collection of notes. It can be used to identify footnotes, rear notes, marginal notes, inline notes, and similar when legacy naming conventions are not desired. Status: Deprecated - Replaced by: 'footnotes', 'rearnotes'") <<
	tr("Other Credits [other.]") << "other.other-credits" << tr("Acknowledgments of previously published parts of the work, illustration credits, and permission to quote from copyrighted material.") <<
	tr("Preamble [other.]") << "other.preamble" << tr("A section in the beginning of the work, typically containing introductory and/or explanatory prose regarding the scope or nature of the work's content") <<
        tr("Preface") << "preface" << tr("An introductory section that precedes the work, typically written by the work's author.") <<
	tr("Prologue [other.]") << "other.prologue" << tr("An introductory section that sets the background to a story, typically part of the narrative.") <<
	tr("Rear Notes [other.]") << "other.rearnotes" << tr("A collection of notes appearing at the rear (backmatter) of the work, or at the end of a section.") <<
        tr("Title Page") << "title-page" << tr("A page at the beginning of a book giving its title, authors, publisher and other publication information.") <<
        tr("Table of Contents") << "toc" << tr("A table of contents which is a list of the headings or parts of the book or document, organized in the order in which they appear. Typically appearing in the work's frontmatter, or at the beginning of a section.");

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

void GuideItems::SetCodeToRawTitleMap()
{
    if (!m_CodeToRawTitle.isEmpty()) {
        return;
    }
    m_CodeToRawTitle["acknowledgements"]    = "Acknowledgements";
    m_CodeToRawTitle["other.afterword"]     = "Afterword";
    m_CodeToRawTitle["other.appendix"]      = "Appendix";
    m_CodeToRawTitle["other.backmatter"]    = "Back Matter";
    m_CodeToRawTitle["bibliography"]        = "Bibliography";
    m_CodeToRawTitle["colophon"]            = "Colophon";
    m_CodeToRawTitle["other.conclusion"]    = "Conclusion";
    m_CodeToRawTitle["other.contributors"]  = "Contributors";
    m_CodeToRawTitle["copyright-page"]      = "Copyright Page";
    m_CodeToRawTitle["cover"]               = "Cover";
    m_CodeToRawTitle["dedication"]          = "Dedication";
    m_CodeToRawTitle["other.epilogue"]      = "Epilogue";
    m_CodeToRawTitle["epigraph"]            = "Epigraph";
    m_CodeToRawTitle["other.errata"]        = "Errata";
    m_CodeToRawTitle["other.footnotes"]     = "Footnotes";
    m_CodeToRawTitle["foreword"]            = "Foreword";
    m_CodeToRawTitle["other.frontmatter"]   = "Front Matter";
    m_CodeToRawTitle["glossary"]            = "Glossary";
    m_CodeToRawTitle["other.halftitlepage"] = "Half Title Page";
    m_CodeToRawTitle["other.imprimatur"]    = "Imprimatur";
    m_CodeToRawTitle["other.imprint"]       = "Imprint";
    m_CodeToRawTitle["index"]               = "Index";
    m_CodeToRawTitle["other.introduction"]  = "Introduction";
    m_CodeToRawTitle["loi"]                 = "List of Illustrations";
    m_CodeToRawTitle["other.loa"]           = "List of Audio Clips";
    m_CodeToRawTitle["lot"]                 = "List of Tables";
    m_CodeToRawTitle["other.lov"]           = "List of Video Clips";
    m_CodeToRawTitle["notes"]               = "Notes";
    m_CodeToRawTitle["other.other-credits"] = "Other Credits";
    m_CodeToRawTitle["other.preamble"]      = "Preamble";
    m_CodeToRawTitle["preface"]             = "Preface";
    m_CodeToRawTitle["other.prologue"]      = "Prologue";
    m_CodeToRawTitle["other.rearnotes"]     = "Rear Notes";
    m_CodeToRawTitle["text"]                = "Text";
    m_CodeToRawTitle["title-page"]          = "Title Page";
    m_CodeToRawTitle["toc"]                 = "Table of Contents";
}
