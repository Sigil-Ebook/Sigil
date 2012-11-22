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

#include <boost/tuple/tuple.hpp>
#include <buffio.h>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QWriteLocker>
#include <QtGui/QApplication>
#include <QtGui/QProgressDialog>

#include "BookManipulation/CleanSource.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/HTMLPrettyPrint.h"
#include "Misc/SettingsStore.h"
#include "sigil_constants.h"
#include "sigil_exception.h"
#include "Misc/Utility.h"

using boost::make_tuple;
using boost::tie;
using boost::tuple;

static const QString SIGIL_CLASS_NAME     = "sgc";
static const QString SIGIL_CLASS_NAME_REG = SIGIL_CLASS_NAME + "-(\\d+)";

static const QString CSS_STYLE_TAG_START  = "<\\s*style[^>]*type\\s*=\\s*\"text/css\"[^>]*>";

// Use with <QRegExp>.setMinimal( true )
static const QString STYLE_TAG_CSS_ONLY   = CSS_STYLE_TAG_START + ".*</\\s*style[^>]*>";

static const QString CLASS_REMOVE_START   = "<[^>]*class\\s*=\\s*\"[^\"]*";
static const QString CLASS_REMOVE_END     = "[^\"]*\"[^>]*>";

// Use with <QRegExp>.setMinimal( true )
static const QString TIDY_NEW_STYLE       = "(\\w+)\\.[\\w-]+\\s*(\\{.*\\})";

// The value was picked arbitrarily
static const int TAG_SIZE_THRESHOLD       = 1000;

static const QString SVG_ELEMENTS         = "a,altGlyph,altGlyphDef,altGlyphItem,animate,animateColor,animateMotion"
        ",animateTransform,circle,clipPath,color-profile,cursor,definition-src,defs,desc"
        ",ellipse,feBlend,feColorMatrix,feComponentTransfer,feComposite,feConvolveMatrix"
        ",feDiffuseLighting,feDisplacementMap,feDistantLight,feFlood,feFuncA,feFuncB"
        ",feFuncG,feFuncR,feGaussianBlur,feImage,feMerg,feMergeNode,feMorphology,feOffset"
        ",fePointLight,feSpecularLighting,feSpotLight,feTile,feTurbulence,filter"
        ",font,font-face,font-face-format,font-face-name,font-face-src,font-face-uri"
        ",foreignObject,g,glyph,glyphRef,hkern,image,line,linearGradient,marker,mask"
        ",metadata,missing-glyph,mpath,path,pattern,polygon,polyline,radialGradient"
        ",rect,script,set,stop,style,svg,switch,symbol,text,textPath,title,tref,tspan"
        ",use,view,vkern";

static QString HTML5_BLOCK_ELEMENTS       = "article,aside,datagrid,details,dialog,figcaption,figure,footer,header,hgroup,menu,nav,section,summary";
static QString HTML5_INLINE_ELEMENTS      = "audio,canvas,command,mark,meter,progress,rp,rt,ruby,time,video";

static QString BLOCK_ELEMENTS             = SVG_ELEMENTS + "," + HTML5_BLOCK_ELEMENTS;
static QString INLINE_ELEMENTS            = HTML5_INLINE_ELEMENTS;


// Performs general cleaning (and improving)
// of provided book XHTML source code
QString CleanSource::Clean(const QString &source)
{
    SettingsStore settings;
    SettingsStore::CleanLevel level = settings.cleanLevel();
    QString newsource = PreprocessSpecialCases(source);

    switch (level) {
        case SettingsStore::CleanLevel_PrettyPrint:
        case SettingsStore::CleanLevel_PrettyPrintTidy: {
            newsource = level == SettingsStore::CleanLevel_PrettyPrint ? PrettyPrint(newsource) : PrettyPrintTidy(newsource);
            // Remove any empty comments left over from pretty printing.
            QStringList css_style_tags  = CSSStyleTags(newsource);
            css_style_tags = RemoveEmptyComments(css_style_tags);
            return WriteNewCSSStyleTags(newsource, css_style_tags);
        }
        case SettingsStore::CleanLevel_Tidy: {
            // We store the number of CSS style tags before
            // running Tidy so CleanCSS can remove redundant classes
            // if tidy added a new style tag
            int old_num_styles = RobustCSSStyleTagCount(newsource);
            newsource = HTMLTidy(newsource, Tidy_Clean);
            newsource = CleanCSS(newsource, old_num_styles);
            return newsource;
        }
        default:
            // The only thing we will do for Clean when set to None is just prettify
            // the XML declaration at the top. Xerces puts the xml, DOCTYPE and opening
            // html tag all on the same line which is ugly to read.
            return PrettifyDOCTYPEHeader(source);
    }
}


// Remove blank lines at the top of style tag added by Tidy
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


// No cleaning, just convert the source to valid XHTML
QString CleanSource::ToValidXHTML(const QString &source)
{
    QString newsource = HTMLTidy(source, Tidy_Fast);
    newsource = RemoveBlankStyleLines(newsource);
    // Run Clean to ensure proper formatting based on the user's settings.
    newsource = Clean(newsource);
    return newsource;
}

QString CleanSource::PrettyPrint(const QString &source)
{
    HTMLPrettyPrint pp(source);
    return pp.prettyPrint();
}

QString CleanSource::PrettyPrintTidy(const QString &source)
{
    return HTMLTidy(source, Tidy_PrettyPrint);
}

QString CleanSource::ProcessXML(const QString &source)
{
    HTMLPrettyPrint pp(source);
    pp.setIndentLevel(0);
    return pp.prettyPrint();
    //return HTMLTidy( source, Tidy_XML );
}


int CleanSource::RobustCSSStyleTagCount(const QString &source)
{
    int head_end_index = source.indexOf(QRegExp(HEAD_END));
    return Utility::Substring(0, head_end_index, source).count(QRegExp(CSS_STYLE_TAG_START));
}


// Cleans CSS; currently it removes the redundant CSS classes
// that Tidy sometimes adds because it doesn't parse existing
// CSS classes, it only adds new ones; this also merges smaller
// style tags into larger ones
QString CleanSource::CleanCSS(const QString &source, int old_num_styles)
{
    QString newsource           = source;
    QStringList css_style_tags  = CSSStyleTags(newsource);

    // If Tidy added a new tag, we remove the redundant ones
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
    QList< XhtmlDoc::XMLElement > style_tag_nodes;

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
        QRegExp sigil_class(SIGIL_CLASS_NAME_REG);
        int main_index = 0;

        while (true) {
            main_index = style_tag.indexOf(sigil_class, main_index);

            if (main_index == -1) {
                break;
            }

            main_index += sigil_class.matchedLength();
            int class_index = sigil_class.cap(1).toInt();

            if (class_index > max_class_index) {
                max_class_index = class_index;
            }
        }
    }
    return max_class_index;
}


TidyDoc CleanSource::TidyOptions(TidyDoc tidy_document, TidyType type, int max_class_index)
{
    // For more information on Tidy configuration
    // options, see http://tidy.sourceforge.net/docs/quickref.html
    if (type == Tidy_XML) {
        // "output-xml"
        tidyOptSetBool(tidy_document, TidyXmlOut, yes);
        // "input-xml"
        tidyOptSetBool(tidy_document, TidyXmlTags, yes);
    } else {
        // "output-xhtml"
        tidyOptSetBool(tidy_document, TidyXhtmlOut, yes);
    }

    // "add-xml-decl"
    tidyOptSetBool(tidy_document, TidyXmlDecl, yes);
    // "preserve-entities"
    tidyOptSetBool(tidy_document, TidyPreserveEntities, yes);
    // "anchor-as-name"
    tidyOptSetBool(tidy_document, TidyAnchorAsName, no);
    // "alt-text"
    tidyOptSetValue(tidy_document, TidyAltText, "");

    if (type != Tidy_Clean) {
        // Turning the two merge options on produces ugly markup
        // for WYSIWYG actions...
        // "merge-divs"
        tidyOptSetInt(tidy_document, TidyMergeDivs, no);
        // "merge-spans"
        tidyOptSetInt(tidy_document, TidyMergeSpans, no);
        // "join-styles"
        tidyOptSetBool(tidy_document, TidyJoinStyles, no);
    } else {
        // "clean"
        tidyOptSetBool(tidy_document, TidyMakeClean, yes);
        // "css-prefix"
        tidyOptSetValue(tidy_document, TidyCSSPrefix, SIGIL_CLASS_NAME.toUtf8().data());
        // This option doesn't exist in "normal" Tidy. It has been hacked on
        // and enables us to direct Tidy to start numbering new CSS classes
        // from an index we provide it, and not always from 1 (which causes clashes).
        tidyOptSetInt(tidy_document, TidyClassStartID, max_class_index);
    }

    // "doctype"
    tidyOptSetValue(tidy_document, TidyDoctype, "strict");
    // "enclose-text"
    tidyOptSetBool(tidy_document, TidyEncloseBodyText, yes);
    // "wrap"
    tidyOptSetInt(tidy_document, TidyWrapLen, 0);
    // "newline"
    tidyOptSetValue(tidy_document, TidyNewline, "LF");
    // Needed so that Tidy doesn't kill off SVG elements and HTML5 Tags
    // "new-blocklevel-tags"
    tidyOptSetValue(tidy_document, TidyBlockTags, BLOCK_ELEMENTS.toUtf8().data());
    // "new-inline-tags"
    tidyOptSetValue(tidy_document, TidyInlineTags, INLINE_ELEMENTS.toUtf8().data());

    if (type != Tidy_Fast)
        // "indent"
    {
        tidyOptSetInt(tidy_document, TidyIndentContent, TidyAutoState);
    }

    // "tidy-mark"
    tidyOptSetBool(tidy_document, TidyMark, no);
    // UTF-8 for input and output
    tidySetCharEncoding(tidy_document, "utf8");
    // Force output
    tidyOptSetBool(tidy_document, TidyForceOutput, yes);
    return tidy_document;
}


// Runs HTML Tidy on the provided XHTML source code
QString CleanSource::HTMLTidy(const QString &source, TidyType type)
{
    if (source.isEmpty()) {
        return QString();
    }

    TidyDoc tidy_document = tidyCreate();
    TidyBuffer output = { 0 };
    TidyBuffer errbuf = { 0 };

    if (type == Tidy_Clean) {
        tidy_document = TidyOptions(tidy_document, type, MaxSigilCSSClassIndex(CSSStyleTags(source)));
    } else {
        tidy_document = TidyOptions(tidy_document, type);
    }

    // Write all errors to error buffer
    tidySetErrorBuffer(tidy_document, &errbuf);
    // Set the input
    tidyParseString(tidy_document, source.toUtf8().constData());
    // GO BABY GO!
    tidyCleanAndRepair(tidy_document);
    // Run diagnostics
    tidyRunDiagnostics(tidy_document);
    // TODO: read and report any possible errors
    // from the error buffer
    // Store the cleaned up XHTML
    tidySaveBuffer(tidy_document, &output);
    QString clean = QString::fromUtf8((const char *) output.bp);
    // Free memory
    tidyBufFree(&output);
    tidyBufFree(&errbuf);
    tidyRelease(tidy_document);
    clean = RemoveMetaCharset(clean);
    return clean;
}


// Writes the new CSS style tags to the source, replacing the old ones
QString CleanSource::WriteNewCSSStyleTags(const QString &source, const QStringList &css_style_tags)
{
    QRegExp body_start_tag(BODY_START);
    int body_begin = source.indexOf(body_start_tag, 0);
    QString header = Utility::Substring(0, body_begin, source);
    QRegExp css_styles_reg(STYLE_TAG_CSS_ONLY);
    css_styles_reg.setMinimal(true);
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


tuple< QString, QStringList > CleanSource::RemoveRedundantClasses(const QString &source, const QStringList &css_style_tags)
{
    QHash< QString, QString > redundant_classes = GetRedundantClasses(css_style_tags);
    return make_tuple(RemoveRedundantClassesSource(source, redundant_classes),
                      RemoveRedundantClassesTags(css_style_tags, redundant_classes));
}

// Removes redundant CSS classes from the provided CSS style tags
QStringList CleanSource::RemoveRedundantClassesTags(const QStringList &css_style_tags, const QHash< QString, QString > redundant_classes)
{
    QStringList new_css_style_tags  = css_style_tags;
    QStringList last_tag_styles     = new_css_style_tags.last().split(QChar('\n'));
    // Searches for the old class in every line;
    // Tidy always creates class definitions as one line
    foreach(QString key, redundant_classes.keys()) {
        QRegExp remove_old("^.*" + QRegExp::escape(key) + ".*$");
        remove_old.setMinimal(true);
        last_tag_styles.replaceInStrings(remove_old, "");
    }
    new_css_style_tags[ new_css_style_tags.count() - 1 ] = last_tag_styles.join(QChar('\n'));
    return new_css_style_tags;
}

// Removes redundant CSS classes from the provided XHTML source code;
// Updates references to older classes that do the same thing
QString CleanSource::RemoveRedundantClassesSource(const QString &source, const QHash< QString, QString > redundant_classes)
{
    QString newsource = source;
    foreach(QString key, redundant_classes.keys()) {
        QRegExp remove_old(CLASS_REMOVE_START + key + CLASS_REMOVE_END);

        while (newsource.indexOf(remove_old) != -1) {
            QString matched = remove_old.cap(0);
            matched.replace(key, redundant_classes.value(key));
            newsource.replace(remove_old.cap(0), matched);
        }
    }
    return newsource;
}


// Returns a QHash with keys being the new redundant CSS classes,
// and the values being the old classes that already do the job of the new ones.
QHash< QString, QString > CleanSource::GetRedundantClasses(const QStringList &css_style_tags)
{
    QHash< QString, QString > redundant_classes;
    // HACK: This whole concept is really ugly.
    // a) We need to fix Tidy so it doesn't create useless new classes.
    // b) We need a real CSS parser. One that knows which HTML element
    // has which style/class.
    // Tidy always create ONE style tag for its new classes,
    // and it is always the last one
    QString new_style_tag = css_style_tags.last();
    QStringList new_style_tag_lines = new_style_tag.split(QChar('\n'));

    // We search through all the tags that come before this new one
    for (int i = 0; i < css_style_tags.count() - 1; ++i) {
        QStringList old_lines = css_style_tags[ i ].split(QChar('\n'));
        // We go through all the lines in the last CSS style tag.
        // It contains the new styles Tidy added.
        foreach(QString line_in_new_styles, new_style_tag_lines) {
            QRegExp class_definition(TIDY_NEW_STYLE);
            class_definition.setMinimal(true);

            if (line_in_new_styles.indexOf(class_definition) != -1) {
                QRegExp matching_style(QRegExp::escape(class_definition.cap(1)) + "\\.[\\w-]+\\s*" +
                                       QRegExp::escape(class_definition.cap(2)));
                // There should always be just one that matches
                QStringList matching_lines = old_lines.filter(matching_style);

                if (matching_lines.count() != 0) {
                    QRegExp sgc_class(SIGIL_CLASS_NAME_REG);
                    matching_lines[ 0 ].indexOf(sgc_class);
                    QString oldclass = sgc_class.cap(0);
                    line_in_new_styles.indexOf(sgc_class);
                    QString newclass = sgc_class.cap(0);
                    redundant_classes[ newclass ] = oldclass;
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
    int head_end = source.indexOf(QRegExp(HEAD_END));

    if (head_end == -1) {
        return source;
    }

    QString head = Utility::Substring(0, head_end, source);
    QRegExp metacharset("<meta[^>]+charset[^>]+>");
    int meta_start = head.indexOf(metacharset);

    if (meta_start == -1) {
        return source;
    }

    head.remove(meta_start, metacharset.matchedLength());
    return head + Utility::Substring(head_end, source.length(), source);
}


QString CleanSource::PreprocessSpecialCases(const QString &source)
{
    QString newsource = source;
    // For content files we want to retain the functionality offered by HTMLTidy's
    // auto-correcting html parser, but this can only handle one namespace
    // in the <html> tag and simply strips tags with namespace prefixes it
    // doesn't understand. The solution used here is to embed the namespace
    // definition in the root tag itself.
    QRegExp root_svg_tag_with_prefix("<\\s*svg\\s*:\\s*svg");
    QString root_svg_embeddedNS = "<svg xmlns=\"http://www.w3.org/2000/svg\"";
    newsource.replace(root_svg_tag_with_prefix, root_svg_embeddedNS);
    // Once the root tag has an embedded namespace, strip the prefix from all other tags
    QRegExp child_svg_tag_with_prefix("<\\s*svg\\s*:");
    QString child_tag_no_prefix = "<";
    newsource.replace(child_svg_tag_with_prefix, child_tag_no_prefix);
    return newsource;
}

QString CleanSource::PrettifyDOCTYPEHeader(const QString &source)
{
    QString newsource = source;
    const int SAFE_LENGTH = 200;
    QRegExp doctype_invalid("<!DOCTYPE html PUBLIC \"W3C");
    int index = doctype_invalid.indexIn(newsource);

    if (index > 0 && index < SAFE_LENGTH) {
        newsource.insert(index + 23, "-//");
    }

    QRegExp doctype_missing_newline("\\?><!DOCTYPE");
    index = doctype_missing_newline.indexIn(source);

    if (index > 0 && index < SAFE_LENGTH) {
        newsource.insert(index + 2, "\n");
        QRegExp html_missing_newline("\"><html ");
        index = html_missing_newline.indexIn(newsource);

        if (index > 0 && index < SAFE_LENGTH) {
            newsource.insert(index + 2, "\n\n");
        }

        bool is_ncx = false;
        QRegExp ncx_missing_newline("\"><ncx ");
        index = ncx_missing_newline.indexIn(newsource);

        if (index > 0 && index < SAFE_LENGTH) {
            is_ncx = true;
            newsource.insert(index + 2, "\n");
        }

        QRegExp doctype_http_missing_newline("//EN\" \"http://");
        index = doctype_http_missing_newline.indexIn(newsource);

        if (index > 0 && index < SAFE_LENGTH) {
            newsource.insert(index + 5, is_ncx ? "\n" : "\n ");
        }
    }

    return newsource;
}


void CleanSource::ReformatAll(QList <TextResource *> resources, QString(clean_func)(const QString &source))
{
    //QList <Resource *> resources = m_OPFModel.GetResourceListInFolder( Resource::HTMLResourceType );
    QProgressDialog progress(QObject::tr("Cleaning..."), 0, 0, resources.count(), Utility::GetMainWindow());
    progress.setMinimumDuration(PROGRESS_BAR_MINIMUM_DURATION);
    int progress_value = 0;
    progress.setValue(progress_value);
    foreach(TextResource * resource, resources) {
        progress.setValue(progress_value++);
        qApp->processEvents();
        QWriteLocker locker(&resource->GetLock());
        resource->SetText(clean_func(resource->GetText()));
    }
}

