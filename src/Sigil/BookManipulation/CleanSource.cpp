/************************************************************************
**
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
#include <boost/tuple/tuple.hpp>
#include <buffio.h>

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
#include "Misc/SettingsStore.h"
#include "sigil_constants.h"
#include "sigil_exception.h"
#include "Misc/Utility.h"
#include <utility>

using boost::make_tuple;
using boost::tie;
using boost::tuple;

static const QString HEAD_END = "</\\s*head\\s*>";

static const QString SIGIL_CLASS_NAME     = "sgc";
static const QString SIGIL_CLASS_NAME_REG = SIGIL_CLASS_NAME + "-(\\d+)";

static const QString CSS_STYLE_TAG_START  = "<\\s*style[^>]*type\\s*=\\s*\"text/css\"[^>]*>";

// Use with minimal matching.
static const QString STYLE_TAG_CSS_ONLY   = CSS_STYLE_TAG_START + ".*</\\s*style[^>]*>";

static const QString CLASS_REMOVE_START   = "<[^>]*class\\s*=\\s*\"[^\"]*";
static const QString CLASS_REMOVE_END     = "[^\"]*\"[^>]*>";

// Use with minimal matching.
static const QString TIDY_NEW_STYLE       = "(\\w+)\\.[\\w-]+\\s*(\\{.*\\})";

// The value was picked arbitrarily
static const int TAG_SIZE_THRESHOLD       = 1000;

static const QString SVG_BLOCK_ELEMENTS         = "a,altGlyph,altGlyphDef,altGlyphItem,animate,animateColor,animateMotion"
        ",animateTransform,circle,clipPath,color-profile,cursor,definition-src,defs,desc"
        ",ellipse,feBlend,feColorMatrix,feComponentTransfer,feComposite,feConvolveMatrix"
        ",feDiffuseLighting,feDisplacementMap,feDistantLight,feFlood,feFuncA,feFuncB"
        ",feFuncG,feFuncR,feGaussianBlur,feImage,feMerg,feMergeNode,feMorphology,feOffset"
        ",fePointLight,feSpecularLighting,feSpotLight,feTile,feTurbulence,filter"
        ",font,font-face,font-face-format,font-face-name,font-face-src,font-face-uri"
        ",foreignObject,g,glyph,glyphRef,hkern,line,linearGradient,marker,mask"
        ",metadata,missing-glyph,mpath,path,pattern,polygon,polyline,radialGradient"
        ",rect,script,set,stop,style,svg,switch,symbol,text,textPath,title,tref,tspan"
        ",use,view,vkern";

static const QString SVG_INLINE_ELEMENTS = "image";
static const QString SVG_EMPTY_ELEMENTS = "image";

static QString HTML5_BLOCK_ELEMENTS       = "article,aside,audio,canvas,datagrid,details,dialog"
        ",figcaption,figure,footer,header,hgroup,menu,nav,section,source,summary,video";

static QString HTML5_INLINE_ELEMENTS      = "command,mark,meter,progress,rp,rt,ruby,time";
static QString HTML5_EMPTY_ELEMENTS       = "";

// Don't mix inline with block but inline can be duplicated with empty according to tidy docs
static QString BLOCK_ELEMENTS             = HTML5_BLOCK_ELEMENTS  + "," + SVG_BLOCK_ELEMENTS;
static QString INLINE_ELEMENTS            = HTML5_INLINE_ELEMENTS + "," + SVG_INLINE_ELEMENTS;;
static QString EMPTY_ELEMENTS             = HTML5_EMPTY_ELEMENTS  + "," + SVG_EMPTY_ELEMENTS;

// Performs general cleaning (and improving)
// of provided book XHTML source code
QString CleanSource::Clean(const QString &source)
{
    SettingsStore settings;
    SettingsStore::CleanLevel level = settings.cleanLevel();
    QString newsource = PreprocessSpecialCases(source);

    switch (level) {
        case SettingsStore::CleanLevel_PrettyPrint:
        case SettingsStore::CleanLevel_PrettyPrintBS4: {
            newsource = level == SettingsStore::CleanLevel_PrettyPrint ? PrettyPrint(newsource) : PrettyPrintBS4(newsource);
            // Remove any empty comments left over from pretty printing.
            QStringList css_style_tags  = CSSStyleTags(newsource);
            css_style_tags = RemoveEmptyComments(css_style_tags);
            return WriteNewCSSStyleTags(newsource, css_style_tags);
        }
        case SettingsStore::CleanLevel_BS4: {
            // We store the number of CSS style tags before
            // running any cleaning so CleanCSS can remove redundant classes
            // if new style tags added
            int old_num_styles = RobustCSSStyleTagCount(newsource);
            newsource = CleanBS4(newsource);
            newsource = CleanCSS(newsource, old_num_styles);
            return newsource;
        }
        default:
            // The only thing we will do for Clean when set to None is just prettify
            // the XML declaration at the top. Xerces puts the xml, DOCTYPE and opening
            // html tag all on the same line which is ugly to read.
            return PrettifyDOCTYPEHeader(newsource);
    }
}

// Repair XHTML if needed  using BeautifulSoup4
QString CleanSource::CleanBS4(const QString &source)
{
    int rv = 0;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(source));
    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("bs4repair"),
                                         QString("repairXHTML"),
                                         args,
                                         &rv,
                                         error_traceback);    
    if (rv != 0) {
        Utility::DisplayStdWarningDialog(QString("error in bs4repair repairXHTML: ") + QString::number(rv), 
                                         error_traceback);
        // an error happened, return unchanged original
        return QString(source);
    }
    return res.toString();
}

// Repair XHTML if needed and PrettyPrint using BeautifulSoup4
QString CleanSource::PrettyPrintBS4(const QString &source)
{
    int rv = 0;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(source));
    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("bs4repair"),
                                         QString("repairPrettyPrintXHTML"),
                                         args,
                                         &rv,
                                         error_traceback);    
    if (rv != 0) {
        Utility::DisplayStdWarningDialog(QString("error in bs4repair repairPrettyPrintXHTML: ") + QString::number(rv), 
                                         error_traceback);
        // an error happened, return unchanged original
        return QString(source);
    }
    return res.toString();
}

// Repair XML if needed and PrettyPrint using BeautifulSoup4
QString CleanSource::XMLPrettyPrintBS4(const QString &source)
{
    int rv = 0;
    QString error_traceback;
    QList<QVariant> args;
    args.append(QVariant(source));
    EmbeddedPython * epython  = EmbeddedPython::instance();

    QVariant res = epython->runInPython( QString("bs4repair"),
                                         QString("repairPrettyPrintXML"),
                                         args,
                                         &rv,
                                         error_traceback);    
    if (rv != 0) {
        Utility::DisplayStdWarningDialog(QString("error in bs4repair repairPrettyPrintXML: ") + QString::number(rv), 
                                         error_traceback);
        // an error happened, return unchanged original
        return QString(source);
    }
    return res.toString();
}

QString CleanSource::RemoveBlankStyleLines(const QString &source)
{
    // Remove the extra blank lines in the style section
    QStringList css_style_tags  = CSSStyleTags(source);
    css_style_tags = RemoveEmptyComments(css_style_tags);

    for (int i = 0; i < css_style_tags.count(); ++i) {
        css_style_tags[ i ].replace("^\n\n", "\n");
    }

    return WriteNewCSSStyleTags(source, css_style_tags);
}


// convert the source to valid XHTML
QString CleanSource::ToValidXHTML(const QString &source)
{
    QString newsource = source;

    if (!XhtmlDoc::IsDataWellFormed(source)) {
        newsource = CleanBS4(source);
        newsource = PrettyPrint(newsource);
        newsource = RemoveBlankStyleLines(newsource);
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
#if 0
    HTMLPrettyPrint pp(source);
    pp.setIndentLevel(0);
    return pp.prettyPrint();
#endif
    return XMLPrettyPrintBS4(source);
}


int CleanSource::RobustCSSStyleTagCount(const QString &source)
{
    int head_end_index = source.indexOf(QRegularExpression(HEAD_END));
    return Utility::Substring(0, head_end_index, source).count(QRegularExpression(CSS_STYLE_TAG_START));
}


// Cleans CSS; currently it removes the redundant CSS classes
// this also merges smaller style tags into larger ones
QString CleanSource::CleanCSS(const QString &source, int old_num_styles)
{
    QString newsource           = source;
    QStringList css_style_tags  = CSSStyleTags(newsource);

    // If added a new tag, we remove the redundant ones
    if (css_style_tags.count() > old_num_styles) {
        tie(newsource, css_style_tags) = RemoveRedundantClasses(newsource, css_style_tags);
    }

    css_style_tags = RemoveEmptyComments(css_style_tags);
    css_style_tags = MergeSmallerStyles(css_style_tags);
    newsource = WriteNewCSSStyleTags(newsource, css_style_tags);
    return newsource;
}


// Returns the content of all CSS style tags in a list,
// where each element is a QString representing the content
// of a single CSS style tag
QStringList CleanSource::CSSStyleTags(const QString &source)
{
    QList<XhtmlDoc::XMLElement> style_tag_nodes;

    try {
        style_tag_nodes = XhtmlDoc::GetTagsInHead(source, "style");
    } catch (ErrorParsingXml) {
        // Nothing really. If we can't get the CSS style tags,
        // than that's it. No CSS returned.
        // TODO: log this error.
    }

    QStringList css_style_tags;
    foreach(XhtmlDoc::XMLElement element, style_tag_nodes) {
        if (element.attributes.contains("type") &&
            (element.attributes.value("type") == "text/css")
           ) {
            css_style_tags.append(element.text);
        }
    }
    return css_style_tags;
}

// Removes empty comments that are
// sometimes left after CDATA comments
QStringList CleanSource::RemoveEmptyComments(const QStringList &css_style_tags)
{
    QStringList new_tags = css_style_tags;

    for (int i = 0; i < new_tags.count(); ++i) {
        new_tags[ i ].replace("/**/", "");
        new_tags[ i ] = new_tags[ i ].trimmed();
    }

    return new_tags;
}


// Merges smaller styles into bigger ones
QStringList CleanSource::MergeSmallerStyles(const QStringList &css_style_tags)
{
    if (css_style_tags.count() < 2) {
        return css_style_tags;
    }

    QStringList new_tags = css_style_tags;
    int index = 1;

    while (index < new_tags.count()) {
        if (new_tags[ index ].length() < TAG_SIZE_THRESHOLD) {
            new_tags[ index - 1 ].append("\n\n" + new_tags[ index ]);
            new_tags.removeAt(index);
        } else {
            index++;
        }
    }

    return new_tags;
}


// Returns the largest index of all the Sigil CSS classes
int CleanSource::MaxSigilCSSClassIndex(const QStringList &css_style_tags)
{
    int max_class_index = 0;
    foreach(QString style_tag, css_style_tags) {
        QRegularExpression sigil_class(SIGIL_CLASS_NAME_REG);
        int main_index = 0;

        while (true) {
            QRegularExpressionMatch match = sigil_class.match(style_tag, main_index);
            main_index = match.capturedStart();
            if (main_index == -1) {
                break;
            }

            main_index += match.capturedLength();
            int class_index = match.captured(1).toInt();

            if (class_index > max_class_index) {
                max_class_index = class_index;
            }
        }
    }
    return max_class_index;
}

// Writes the new CSS style tags to the source, replacing the old ones
QString CleanSource::WriteNewCSSStyleTags(const QString &source, const QStringList &css_style_tags)
{
    QRegularExpression body_start_tag(BODY_START);
    int body_begin = source.indexOf(body_start_tag, 0);
    QString header = Utility::Substring(0, body_begin, source);
    QRegularExpression css_styles_reg(STYLE_TAG_CSS_ONLY, QRegularExpression::InvertedGreedinessOption|QRegularExpression::DotMatchesEverythingOption);
    // We delete the old CSS style tags
    header.remove(css_styles_reg);
    // For each new style tag, create it
    // and add it to the end of the <head> section
    foreach(QString styles, css_style_tags) {
        QString style_tag = "<style type=\"text/css\">\n" + styles + "\n</style>\n";
        header.insert(header.indexOf("</head>"), style_tag);
    }
    return header + Utility::Substring(body_begin, source.length(), source);
}


tuple<QString, QStringList> CleanSource::RemoveRedundantClasses(const QString &source, const QStringList &css_style_tags)
{
    QHash<QString, QString> redundant_classes = GetRedundantClasses(css_style_tags);
    return make_tuple(RemoveRedundantClassesSource(source, redundant_classes),
                      RemoveRedundantClassesTags(css_style_tags, redundant_classes));
}

// Removes redundant CSS classes from the provided CSS style tags
QStringList CleanSource::RemoveRedundantClassesTags(const QStringList &css_style_tags, const QHash<QString, QString> redundant_classes)
{
    QStringList new_css_style_tags  = css_style_tags;
    QStringList last_tag_styles     = new_css_style_tags.last().split(QChar('\n'));
    // Searches for the old class in every line;
    // ***FIXME*** Deal with "Tidy always creates class definitions as one line given no Tidy now
    foreach(QString key, redundant_classes.keys()) {
        QRegularExpression remove_old("^.*" + QRegularExpression::escape(key) + ".*$", QRegularExpression::InvertedGreedinessOption|QRegularExpression::DotMatchesEverythingOption);
        last_tag_styles.replaceInStrings(remove_old, "");
    }
    new_css_style_tags[new_css_style_tags.count() - 1] = last_tag_styles.join(QChar('\n'));
    return new_css_style_tags;
}

// Removes redundant CSS classes from the provided XHTML source code;
// Updates references to older classes that do the same thing
QString CleanSource::RemoveRedundantClassesSource(const QString &source, const QHash<QString, QString> redundant_classes)
{
    QString newsource = source;
    foreach(QString key, redundant_classes.keys()) {
        QRegularExpression remove_old(CLASS_REMOVE_START + key + CLASS_REMOVE_END);
        QRegularExpressionMatch match = remove_old.match(newsource);

        while (match.hasMatch() && match.capturedStart() != -1) {
            QString matched = match.captured();
            matched.replace(key, redundant_classes.value(key));
            newsource.replace(match.captured(), matched);
            match = remove_old.match(newsource);
        }
    }
    return newsource;
}


// Returns a QHash with keys being the new redundant CSS classes,
// and the values being the old classes that already do the job of the new ones.
QHash<QString, QString> CleanSource::GetRedundantClasses(const QStringList &css_style_tags)
{
    QHash<QString, QString> redundant_classes;
    // HACK: This whole concept is really ugly.
    // We need a real CSS parser. One that knows which HTML element
    // has which style/class.
    // **FIXME** Deal with "Tidy always create ONE style tag for its new classes,
    // and it is always the last one"
    QString new_style_tag = css_style_tags.last();
    QStringList new_style_tag_lines = new_style_tag.split(QChar('\n'));

    // We search through all the tags that come before this new one
    for (int i = 0; i < css_style_tags.count() - 1; ++i) {
        QStringList old_lines = css_style_tags[ i ].split(QChar('\n'));
        // We go through all the lines in the last CSS style tag.
        // It contains the new styles Tidy added.
        foreach(QString line_in_new_styles, new_style_tag_lines) {
            QRegularExpression class_definition(TIDY_NEW_STYLE, QRegularExpression::InvertedGreedinessOption|QRegularExpression::DotMatchesEverythingOption);
            QRegularExpressionMatch class_definition_match = class_definition.match(line_in_new_styles);

            if (class_definition_match.hasMatch() && class_definition_match.capturedStart() != -1) {
                QRegularExpression matching_style(QRegularExpression::escape(class_definition_match.captured(1)) + "\\.[\\w-]+\\s*" +
                                                  QRegularExpression::escape(class_definition_match.captured(2)));
                // There should always be just one that matches
                QStringList matching_lines = old_lines.filter(matching_style);
                if (matching_lines.count() != 0) {
                    QRegularExpression sgc_class(SIGIL_CLASS_NAME_REG);
                    QRegularExpressionMatch sgc_class_match = sgc_class.match(matching_lines[0]);
                    if (!sgc_class_match.hasMatch()) {
                        continue;
                    }
                    QString oldclass = sgc_class_match.captured();
                    sgc_class_match = sgc_class.match(line_in_new_styles);
                    if (!sgc_class_match.hasMatch()) {
                        continue;
                    }
                    QString newclass = sgc_class_match.captured();
                    redundant_classes[newclass] = oldclass;
                } else {
                    continue;
                }
            }
        }
    }

    return redundant_classes;
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

