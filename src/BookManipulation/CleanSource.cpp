/************************************************************************
**
**  Copyright (C) 2015 Kevin B. Hendricks Stratford, ON, Canada 
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include "Misc/EmbeddedPython.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QWriteLocker>
#include <QtWidgets/QApplication>
#include <QtWidgets/QProgressDialog>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "BookManipulation/CleanSource.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/HTMLPrettyPrint.h"
#include "Misc/GumboInterface.h"
#include "Misc/SettingsStore.h"
#include "sigil_constants.h"
#include "sigil_exception.h"
#include "Misc/Utility.h"
#include <utility>

static const QString HEAD_END = "</\\s*head\\s*>";


// Performs general cleaning (and improving)
// of provided book XHTML source code
QString CleanSource::Clean(const QString &source)
{
    SettingsStore settings;
    SettingsStore::CleanLevel level = settings.cleanLevel();
    QString newsource = PreprocessSpecialCases(source);

    switch (level) {
        case SettingsStore::CleanLevel_PrettyPrint:
        case SettingsStore::CleanLevel_PrettyPrintGumbo: {
            newsource = level == SettingsStore::CleanLevel_PrettyPrint ? PrettyPrint(newsource) : PrettyPrintGumbo(newsource);
            newsource = PrettifyDOCTYPEHeader(newsource);
            return newsource;
        }
        case SettingsStore::CleanLevel_Gumbo: {
            GumboInterface gp = GumboInterface(newsource);
            newsource = gp.repair();
            newsource = CharToEntity(newsource);
            newsource = PrettifyDOCTYPEHeader(newsource);
            return newsource;
        }
        default:
            // The only thing we will do for Clean when set to None is just prettify
            // the XML declaration at the top. 
            return PrettifyDOCTYPEHeader(newsource);
    }
}


// Repair XHTML if needed using Gumbo and then PrettyPrint
QString CleanSource::PrettyPrintGumbo(const QString &source)
{
    QString newsource = source;
    GumboInterface gi = GumboInterface(newsource);
    newsource = gi.prettyprint();
    newsource = CharToEntity(newsource);
    newsource = PrettifyDOCTYPEHeader(newsource);
    return newsource;
}


// Repair XML if needed and PrettyPrint using BeautifulSoup4
QString CleanSource::XMLPrettyPrintBS4(const QString &source)
{
    int rv = 0;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(source));
    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("xmlprocessor"),
                                         QString("repairXML"),
                                         args,
                                         &rv,
                                         error_traceback);    
    if (rv != 0) {
        Utility::DisplayStdWarningDialog(QString("error in xmlprocessor repairXML: ") + QString::number(rv), 
                                         error_traceback);
        // an error happened, return unchanged original
        return QString(source);
    }
    return res.toString();
}

// convert the source to valid XHTML
QString CleanSource::ToValidXHTML(const QString &source)
{
    QString newsource = source;

    if (!XhtmlDoc::IsDataWellFormed(source)) {
        newsource = PrettyPrintGumbo(newsource);
    }
    return newsource;
}

QString CleanSource::PrettyPrint(const QString &source)
{
    HTMLPrettyPrint pp(source);
    return pp.prettyPrint();
}

QString CleanSource::ProcessXML(const QString &source)
{
    return XMLPrettyPrintBS4(source);
}

QString CleanSource::RemoveMetaCharset(const QString &source)
{
    int head_end = source.indexOf(QRegularExpression(HEAD_END));
    if (head_end == -1) {
        return source;
    }
    QString head = Utility::Substring(0, head_end, source);

    QRegularExpression metacharset("<meta[^>]+charset[^>]+>");
    QRegularExpressionMatch metacharset_match = metacharset.match(head);
    if (!metacharset_match.hasMatch()) {
        return source;
    }
    int meta_start = metacharset_match.capturedStart();

    head.remove(meta_start, metacharset_match.capturedLength());
    return head + Utility::Substring(head_end, source.length(), source);
}


QString CleanSource::PreprocessSpecialCases(const QString &source)
{
    QString newsource = source;
    // For content files we want to retain the functionality offered by an
    // auto-correcting html parser, but this can only handle one namespace
    // in the <html> tag and simply strips tags with namespace prefixes it
    // doesn't understand. The solution used here is to embed the namespace
    // definition in the root tag itself.
    QRegularExpression root_svg_tag_with_prefix("<\\s*svg\\s*:\\s*svg");
    QString root_svg_embeddedNS = "<svg xmlns=\"http://www.w3.org/2000/svg\"";
    newsource.replace(root_svg_tag_with_prefix, root_svg_embeddedNS);
    // Once the root tag has an embedded namespace, strip the prefix from all other tags
    QRegularExpression child_svg_tag_with_prefix("<\\s*svg\\s*:");
    QString child_tag_no_prefix = "<";
    newsource.replace(child_svg_tag_with_prefix, child_tag_no_prefix);
    return newsource;
}


// Be careful to make sure that we do not mess up epub3 <!DOCTYPE html> here
QString CleanSource::PrettifyDOCTYPEHeader(const QString &source)
{
    QString newsource = source;
    const int SAFE_LENGTH = 200;
    QRegularExpression doctype_invalid("<!DOCTYPE html PUBLIC \"W3C");
    int index = newsource.indexOf(doctype_invalid);

    if (index > 0 && index < SAFE_LENGTH) {
        newsource.insert(index + 23, "-//");
    }

    QRegularExpression doctype_missing_newline("\\?><!DOCTYPE");
    index = source.indexOf(doctype_missing_newline);

    if (index > 0 && index < SAFE_LENGTH) {
        newsource.insert(index + 2, "\n");
        QRegularExpression html_missing_newline("\"><html ");
        index = newsource.indexOf(html_missing_newline);

        if (index > 0 && index < SAFE_LENGTH) {
            newsource.insert(index + 2, "\n\n");
        }

        bool is_ncx = false;
        QRegularExpression ncx_missing_newline("\"><ncx ");
        index = newsource.indexOf(ncx_missing_newline);

        if (index > 0 && index < SAFE_LENGTH) {
            is_ncx = true;
            newsource.insert(index + 2, "\n");
        }

        QRegularExpression doctype_http_missing_newline("//EN\" \"http://");
        index = newsource.indexOf(doctype_http_missing_newline);

        if (index > 0 && index < SAFE_LENGTH) {
            newsource.insert(index + 5, is_ncx ? "\n" : "\n ");
        }
    }

    return newsource;
}


QString CleanSource::CharToEntity(const QString &source)
{
    SettingsStore settings;
    QString new_source = source;
    QList<std::pair <ushort, QString>> codenames = settings.preserveEntityCodeNames();
    std::pair <ushort, QString> epair;
    foreach(epair, codenames) {
        new_source.replace(QChar(epair.first), epair.second);
    }
    return new_source;
}


void CleanSource::ReformatAll(QList <HTMLResource *> resources, QString(clean_func)(const QString &source))
{
    //QList <Resource *> resources = m_OPFModel.GetResourceListInFolder( Resource::HTMLResourceType );
    QProgressDialog progress(QObject::tr("Cleaning..."), 0, 0, resources.count(), Utility::GetMainWindow());
    progress.setMinimumDuration(PROGRESS_BAR_MINIMUM_DURATION);
    int progress_value = 0;
    progress.setValue(progress_value);
    foreach(HTMLResource * resource, resources) {
        progress.setValue(progress_value++);
        qApp->processEvents();
        QWriteLocker locker(&resource->GetLock());
        resource->SetText(clean_func(resource->GetText()));
    }
}
