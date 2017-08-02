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
const QString SVG_NAMESPACE_PREFIX = "<\\s*[^>]*(xmlns\\s*:\\s*svg\\s*=\\s*(?:\"|')[^\"']+(?:\"|'))[^>]*>";

// Performs general cleaning (and improving)
// of provided book XHTML source code
QString CleanSource::Mend(const QString &source, const QString &version)
{
    SettingsStore settings;
    QString newsource = PreprocessSpecialCases(source);

    // This hack should not be needed anymore as the epub version is known
    // so missing doctypes should be properly recreated by gumbo
#if 0
    // This is a real hackjob just to test the impact of doctype on gumbo repair functioning
    // Better code here would handle the case of the xml declaration if present
    // Instead of blindly prepending a doctype if none can be found 
    const int SAFE_LENGTH = 200;
    QRegularExpression doctype_present("<!DOCTYPE\\s*html", QRegularExpression::CaseInsensitiveOption);
    int index = newsource.indexOf(doctype_present);
    if (!(index > 0 && index < SAFE_LENGTH)) {
        QString doctype;
        if (version == "2.0") {
            doctype = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n  \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n";
        } else {
            doctype = "<!DOCTYPE html>\n";
        }
        newsource = doctype + newsource;
    }
#endif

    GumboInterface gp = GumboInterface(newsource, version);
    newsource = gp.repair();
    newsource = CharToEntity(newsource);
    newsource = PrettifyDOCTYPEHeader(newsource);
    return newsource;
}


// Mend and Prettify XHTML
QString CleanSource::MendPrettify(const QString &source, const QString &version)
{
    QString newsource = PreprocessSpecialCases(source);
    GumboInterface gi = GumboInterface(newsource, version);
    newsource = gi.prettyprint();
    newsource = CharToEntity(newsource);
    newsource = PrettifyDOCTYPEHeader(newsource);
    return newsource;
}


// Repair XML if needed and PrettyPrint using BeautifulSoup4
QString CleanSource::XMLPrettyPrintBS4(const QString &source, const QString mtype)
{
    int rv = 0;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(source));
    args.append(QVariant(mtype));
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
QString CleanSource::ToValidXHTML(const QString &source, const QString &version)
{
    QString newsource = source;
    if (!XhtmlDoc::IsDataWellFormed(source)) {
        newsource = Mend(source, version);
    }
    return newsource;
}

XhtmlDoc::WellFormedError CleanSource::WellFormedXMLCheck(const QString &source, const QString mtype)
{
    XhtmlDoc::WellFormedError error; 
    int rv = 0;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(source));
    args.append(QVariant(mtype));
    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("xmlprocessor"),
                                         QString("WellFormedXMLCheck"),
                                         args,
                                         &rv,
                                         error_traceback);    
    if (rv != 0) {
        Utility::DisplayStdWarningDialog(QString("error in xmlprocessor WellFormedXMLCheck: ") + QString::number(rv), 
                                         error_traceback);
        // an error happened during check, return well-formed as true
        return error;
    }
    QStringList errors = res.toStringList();
    error.line = errors.at(0).toInt();
    error.column = errors.at(1).toInt();
    error.message = errors.at(2);
    return error;
}

bool CleanSource::IsWellFormedXML(const QString &source, const QString mtype)
{
    int rv = 0;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(source));
    args.append(QVariant(mtype));
    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("xmlprocessor"),
                                         QString("IsWellFormedXML"),
                                         args,
                                         &rv,
                                         error_traceback);    
    if (rv != 0) {
        Utility::DisplayStdWarningDialog(QString("error in xmlprocessor IsWellFormedXML: ") + QString::number(rv), 
                                         error_traceback);
        // an error happened during check return well-formed as true
        return true;
    }
    return res.toBool();
}

QString CleanSource::ProcessXML(const QString &source, const QString mtype)
{
    return XMLPrettyPrintBS4(source, mtype);
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


// neither svg nor math tags need a namespace prefix defined
// especially as epub3 now includes them into the html5 spec
// So we need to remove the svg prefix from the tags before
// processing them with gumbo
QString CleanSource::PreprocessSpecialCases(const QString &source)
{
    QString newsource = source;
    // remove prefix from root tag and add unprefixed svg namespace to it
    QRegularExpression root_svg_tag_with_prefix("<\\s*svg\\s*:\\s*svg");
    QString root_svg_embeddedNS = "<svg xmlns=\"http://www.w3.org/2000/svg\"";
    newsource.replace(root_svg_tag_with_prefix, root_svg_embeddedNS);
    // search for any prefixed svg namespace in that root tag and remove it
    QRegularExpression svg_nsprefix(SVG_NAMESPACE_PREFIX);
    QRegularExpressionMatch mo = svg_nsprefix.match(newsource);
    if (mo.hasMatch()) {
        newsource.replace(mo.capturedStart(1), mo.capturedLength(1), "");
    } 
    // now strip the prefix from all child starting tags
    QRegularExpression starting_child_svg_tag_with_prefix("<\\s*svg\\s*:");
    QString starting_child_tag_no_prefix = "<";
    newsource.replace(starting_child_svg_tag_with_prefix, starting_child_tag_no_prefix);
    // do the same for any child ending tags
    QRegularExpression ending_child_svg_tag_with_prefix("<\\s*/\\s*svg\\s*:");
    QString ending_child_tag_no_prefix = "</";
    newsource.replace(ending_child_svg_tag_with_prefix, ending_child_tag_no_prefix);
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


bool CleanSource::ReformatAll(QList <HTMLResource *> resources, QString(clean_func)(const QString &source, const QString &version))
{
    QProgressDialog progress(QObject::tr("Cleaning..."), 0, 0, resources.count(), Utility::GetMainWindow());
    progress.setMinimumDuration(PROGRESS_BAR_MINIMUM_DURATION);
    int progress_value = 0;
    progress.setValue(progress_value);
    bool book_modified = false;
    foreach(HTMLResource * resource, resources) {
        progress.setValue(progress_value++);
        qApp->processEvents();
        QWriteLocker locker(&resource->GetLock());
        QString source = resource->GetText();
        QString version = resource->GetEpubVersion();
        QString newsource = clean_func(source, version);
        if (newsource != source) {
            book_modified = true;
            resource->SetText(newsource);
        }
    }
    return book_modified;
}
