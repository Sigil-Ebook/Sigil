/************************************************************************
**
**  Copyright (C) 2009  Strahinja Markovic
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

#include <stdafx.h>
#include "../Misc/Utility.h"
#include "../BookManipulation/CleanSource.h"
#include <tidy.h>
#include <buffio.h>
#include "../BookManipulation/XHTMLDoc.h"

static const QString SIGIL_CLASS_NAME       = "sgc";
static const QString SIGIL_CLASS_NAME_REG   = SIGIL_CLASS_NAME + "-(\\d+)";

// Use with <QRegExp>.setMinimal( true )
static const QString STYLE_TAG_CSS_ONLY     = "<\\s*style[^>]*type\\s*=\\s*\"text/css\"[^>]*>.*</\\s*style[^>]*>";

static const QString CLASS_REMOVE_START     = "<[^>]*class\\s*=\\s*\"[^\"]*";
static const QString CLASS_REMOVE_END       = "[^\"]*\"[^>]*>";

// Use with <QRegExp>.setMinimal( true )
static const QString TIDY_NEW_STYLE         = "(\\w+)\\.[\\w-]+\\s*(\\{.*\\})";

// The value was picked arbitrarily
static const int TAG_SIZE_THRESHOLD         = 1000;

static const QString SVG_ELEMENTS           =   "a,altGlyph,altGlyphDef,altGlyphItem,animate,animateColor,animateMotion"
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


// Performs general cleaning (and improving)
// of provided book XHTML source code
QString CleanSource::Clean( const QString &source )
{
    QString newsource = source;

    // We store the number of CSS style tags before
    // running Tidy so CleanCSS can remove redundant classes
    // if tidy added a new style tag
    int old_num_styles = CSSStyleTags( newsource ).count();
    
    newsource = HTMLTidy( newsource );
    newsource = CleanCSS( newsource, old_num_styles );

    // Once more, so we get the nice pretty-printed
    // output of our CSS code too.
    // TODO: create a special tidy func that only pretty-prints
    newsource = HTMLTidy( newsource );

    return newsource;
}


// No cleaning, just convert the source to valid XHTML
QString CleanSource::ToValidXHTML( const QString &source )
{
    return FastXHTMLTidy( source );
}


// Cleans CSS; currently it removes the redundant CSS classes
// that Tidy sometimes adds because it doesn't parse existing
// CSS classes, it only adds new ones; this also merges smaller
// style tags into larger ones
QString CleanSource::CleanCSS( const QString &source, int old_num_styles )
{
    QString newsource           = source;
    QStringList css_style_tags  = CSSStyleTags( newsource );

    // If Tidy added a new tag, we remove the redundant ones
    if ( css_style_tags.count() > old_num_styles )
    {
        SourceAndStyles cleaned = RemoveRedundantClasses( newsource, css_style_tags );

        newsource       = cleaned.source;
        css_style_tags  = cleaned.css_style_tags;
    }

    css_style_tags = RemoveEmptyComments( css_style_tags );
    css_style_tags = MergeSmallerStyles(  css_style_tags );

    newsource = WriteNewCSSStyleTags( newsource, css_style_tags );

    return newsource;
}


// Returns the content of all CSS style tags in a list,
// where each element is a QString representing the content
// of a single CSS style tag
QStringList CleanSource::CSSStyleTags( const QString &source )
{
    QStringList css_style_tags;

    QList< XHTMLDoc::XMLElement > style_tag_nodes = XHTMLDoc::GetTagsInHead( source, "style" );

    foreach( XHTMLDoc::XMLElement element, style_tag_nodes )
    {
        if (    element.attributes.contains( "type" ) && 
              ( element.attributes.value( "type" ) == "text/css" ) 
           )  
        {
            css_style_tags.append( element.text );
        }
    }

    return css_style_tags;
}

// Removes empty comments that are
// sometimes left after CDATA comments
QStringList CleanSource::RemoveEmptyComments( const QStringList &css_style_tags )
{
    QStringList new_tags = css_style_tags;

    for ( int i = 0; i < new_tags.count(); ++i )
    {
        new_tags[ i ].replace( "/**/", "" );
        new_tags[ i ] = new_tags[ i ].trimmed();
    }

    return new_tags;
}


// Merges smaller styles into bigger ones
QStringList CleanSource::MergeSmallerStyles( const QStringList &css_style_tags )
{
    if ( css_style_tags.count() < 2 )

        return css_style_tags;

    QStringList new_tags = css_style_tags;

    int index = 1;

    while ( index < new_tags.count() )
    {
        if ( new_tags[ index ].length() < TAG_SIZE_THRESHOLD )
        {
            new_tags[ index - 1 ].append( "\n\n" + new_tags[ index ] );

            new_tags.removeAt( index );
        }

        else
        {
            index++;
        }
    }

    return new_tags;
}


// Returns the largest index of all the Sigil CSS classes
int CleanSource::MaxSigilCSSClassIndex( const QStringList &css_style_tags  )
{
    int max_class_index = 0;    

    foreach( QString style_tag, css_style_tags )
    {
        QRegExp sigil_class( SIGIL_CLASS_NAME_REG );

        int main_index = 0;

        while ( true )
        {
            main_index = style_tag.indexOf( sigil_class, main_index );

            if ( main_index == -1 )

                break;

            main_index += sigil_class.matchedLength();

            int class_index = sigil_class.cap( 1 ).toInt();

            if ( class_index > max_class_index )

                max_class_index = class_index;
        }
    }

    return max_class_index;
}


// Runs HTML Tidy on the provided XHTML source code
QString CleanSource::HTMLTidy( const QString &source )
{
    TidyDoc tidy_document = tidyCreate();

    TidyBuffer output = { 0 };
    TidyBuffer errbuf = { 0 };

    // For more information on Tidy configuration
    // options, see http://tidy.sourceforge.net/docs/quickref.html

    // "output-xhtml"
    tidyOptSetBool( tidy_document, TidyXhtmlOut, yes );

    // "add-xml-decl"
    tidyOptSetBool( tidy_document, TidyXmlDecl, yes );

    // "clean"
    tidyOptSetBool( tidy_document, TidyMakeClean, yes );

    // "preserve-entities"
    tidyOptSetBool( tidy_document, TidyPreserveEntities, yes );	

    // Turning these two options on produces ugly markup
    // from WYSIWYG actions... for now, it's better we turn it off.

    // "merge-divs"
    //tidyOptSetInt( tidy_document, TidyMergeDivs, no );

    // "merge-spans"
    //tidyOptSetInt( tidy_document, TidyMergeSpans, no );

    // "doctype"
    tidyOptSetValue( tidy_document, TidyDoctype, "strict" );

    // "enclose-text"
    tidyOptSetBool( tidy_document, TidyEncloseBodyText, yes );	

    // "wrap"
    tidyOptSetInt( tidy_document, TidyWrapLen, 0 );	

    // "css-prefix"
    tidyOptSetValue( tidy_document, TidyCSSPrefix, SIGIL_CLASS_NAME.toUtf8().data() );	

    // Needed so that Tidy doesn't kill off SVG elements
    // "new-blocklevel-tags"
    tidyOptSetValue( tidy_document, TidyBlockTags, SVG_ELEMENTS.toUtf8().data() );

    // This option doesn't exist in "normal" Tidy. It has been hacked on
    // and enables us to direct Tidy to start numbering new CSS classes
    // from an index we provide it, and not always from 1 (which causes clashes).
    tidyOptSetInt( tidy_document, TidyClassStartID, MaxSigilCSSClassIndex( CSSStyleTags( source ) ) );	

    // "indent"
    tidyOptSetInt( tidy_document, TidyIndentContent, TidyAutoState );	

    // "tidy-mark"
    tidyOptSetBool( tidy_document, TidyMark, no );	

    // UTF-8 for input and output
    tidySetCharEncoding( tidy_document, "utf8" );  	

    // Force output
    tidyOptSetBool( tidy_document, TidyForceOutput, yes);

    // Write all errors to error buffer
    tidySetErrorBuffer( tidy_document, &errbuf );

    // Set the input
    tidyParseString( tidy_document, source.toUtf8().constData() );

    // GO BABY GO!
    tidyCleanAndRepair( tidy_document );

    // Run diagnostics
    tidyRunDiagnostics( tidy_document );

    // TODO: read and report any possible errors
    // from the error buffer

    // Store the cleaned up XHTML
    tidySaveBuffer( tidy_document, &output );

    QString clean = QString::fromUtf8( (const char*) output.bp );

    // Free memory
    tidyBufFree( &output );
    tidyBufFree( &errbuf );
    tidyRelease( tidy_document );

    return clean;
}


// Tries to run Tidy's error correcting parser
// as fast as possible, with no unnecessary cleaning
QString CleanSource::FastXHTMLTidy( const QString &source )
{
    TidyDoc tidy_document = tidyCreate();

    TidyBuffer output = { 0 };
    TidyBuffer errbuf = { 0 };

    // For more information on Tidy configuration
    // options, see http://tidy.sourceforge.net/docs/quickref.html

    // "output-xhtml"
    tidyOptSetBool( tidy_document, TidyXhtmlOut, yes );

    // "add-xml-decl"
    tidyOptSetBool( tidy_document, TidyXmlDecl, yes );

    // "preserve-entities"
    tidyOptSetBool( tidy_document, TidyPreserveEntities, yes );	

    // "join-styles"
    tidyOptSetBool( tidy_document, TidyJoinStyles, no );

    // "merge-divs"
    tidyOptSetInt( tidy_document, TidyMergeDivs, TidyNoState );

    // "merge-spans"
    tidyOptSetInt( tidy_document, TidyMergeSpans, TidyNoState );

    // "wrap"
    tidyOptSetInt( tidy_document, TidyWrapLen, 0 );

    // "doctype"
    tidyOptSetValue( tidy_document, TidyDoctype, "strict" );

    // Needed so that Tidy doesn't kill off SVG elements
    // "new-blocklevel-tags"
    tidyOptSetValue( tidy_document, TidyBlockTags, SVG_ELEMENTS.toUtf8().data() );

    // "tidy-mark"
    tidyOptSetBool( tidy_document, TidyMark, no );	

    // UTF-8 for input and output
    tidySetCharEncoding( tidy_document, "utf8" );  	

    // Force output
    tidyOptSetBool( tidy_document, TidyForceOutput, yes );

    // Write all errors to error buffer
    tidySetErrorBuffer( tidy_document, &errbuf );

    // Set the input
    tidyParseString( tidy_document, source.toUtf8().constData() );

    // GO BABY GO!
    tidyCleanAndRepair( tidy_document );

    // Run diagnostics
    tidyRunDiagnostics( tidy_document );

    // TODO: read and report any possible errors
    // from the error buffer

    // Store the cleaned up XHTML
    tidySaveBuffer( tidy_document, &output );

    QString clean = QString::fromUtf8( (const char*) output.bp );

    // Free memory
    tidyBufFree( &output );
    tidyBufFree( &errbuf );
    tidyRelease( tidy_document );

    return clean;
}


// Writes the new CSS style tags to the source, replacing the old ones
QString CleanSource::WriteNewCSSStyleTags( const QString &source, const QStringList &css_style_tags )
{
    QRegExp body_start_tag( BODY_START );

    int body_begin = source.indexOf( body_start_tag, 0 );

    QString header = Utility::Substring( 0, body_begin, source );

    QRegExp css_styles_reg( STYLE_TAG_CSS_ONLY );
    css_styles_reg.setMinimal( true );

    // We delete the old CSS style tags
    header.remove( css_styles_reg );

    // For each new style tag, create it
    // and add it to the end of the <head> section
    foreach( QString styles, css_style_tags )
    {
        QString style_tag = "<style type=\"text/css\">\n" + styles + "\n</style>\n";

        header.insert( header.indexOf( "</head>" ), style_tag );
    }    

    return header + Utility::Substring( body_begin, source.length(), source );
}


// Removes redundant classes from the style tags and source code;
// Calls more specific version.
CleanSource::SourceAndStyles CleanSource::RemoveRedundantClasses( const QString &source, const QStringList &css_style_tags )
{
    QHash< QString, QString > redundant_classes = GetRedundantClasses( css_style_tags );
    
    SourceAndStyles cleaned;
    cleaned.source          = RemoveRedundantClassesSource( source, redundant_classes );
    cleaned.css_style_tags  = RemoveRedundantClassesTags( css_style_tags, redundant_classes );   

    return cleaned;
}

// Removes redundant CSS classes from the provided CSS style tags
QStringList CleanSource::RemoveRedundantClassesTags( const QStringList &css_style_tags, const QHash< QString, QString > redundant_classes )
{
    QStringList new_css_style_tags  = css_style_tags;
    QStringList last_tag_styles     = new_css_style_tags.last().split( QChar( '\n' ) );

    // Searches for the old class in every line;
    // Tidy always creates class definitions as one line
    foreach( QString key, redundant_classes.keys() )
    {
        QRegExp remove_old( "^.*" + QRegExp::escape( key ) + ".*$" );
        remove_old.setMinimal( true );

        last_tag_styles.replaceInStrings( remove_old, "" );
    }

    new_css_style_tags[ new_css_style_tags.count() - 1 ] = last_tag_styles.join( QChar( '\n' ) );

    return new_css_style_tags;
}

// Removes redundant CSS classes from the provided XHTML source code;
// Updates references to older classes that do the same thing
QString CleanSource::RemoveRedundantClassesSource( const QString &source, const QHash< QString, QString > redundant_classes )
{
    QString newsource = source;

    foreach( QString key, redundant_classes.keys() )
    {
        QRegExp remove_old( CLASS_REMOVE_START + key + CLASS_REMOVE_END );

        while ( newsource.indexOf( remove_old ) != -1 )
        {
            QString matched = remove_old.cap( 0 );

            matched.replace( key, redundant_classes.value( key ) );
            newsource.replace( remove_old.cap( 0 ), matched );
        }
    }

    return newsource;
}


// Returns a QHash with keys being the new redundant CSS classes,
// and the values being the old classes that already do the job of the new ones.
QHash< QString, QString > CleanSource::GetRedundantClasses( const QStringList &css_style_tags )
{
    QHash< QString, QString > redundant_classes;

    // HACK: This whole concept is really ugly.
    // a) We need to fix Tidy so it doesn't create useless new classes.
    // b) We need a real CSS parser. One that knows which HTML element
    // has which style/class.

    // Tidy always create ONE style tag for its new classes,
    // and it is always the last one
    QString new_style_tag = css_style_tags.last();
    QStringList new_style_tag_lines = new_style_tag.split( QChar( '\n' ) );

    // We search through all the tags that come before this new one 
    for ( int i = 0; i < css_style_tags.count() - 1; ++i )
    {
        QStringList old_lines = css_style_tags[ i ].split( QChar( '\n' ) );

        // We go through all the lines in the last CSS style tag.
        // It contains the new styles Tidy added.
        foreach( QString line_in_new_styles, new_style_tag_lines )
        {
            QRegExp class_definition( TIDY_NEW_STYLE );
            class_definition.setMinimal( true );

            if ( line_in_new_styles.indexOf( class_definition ) != -1 )
            {                
                QRegExp matching_style( QRegExp::escape( class_definition.cap( 1 ) ) + "\\.[\\w-]+\\s*" +
                                        QRegExp::escape( class_definition.cap( 2 ) ) );

                // There should always be just one that matches
                QStringList matching_lines = old_lines.filter( matching_style );

                if ( matching_lines.count() != 0 )
                {
                    QRegExp sgc_class( SIGIL_CLASS_NAME_REG );

                    matching_lines[ 0 ].indexOf( sgc_class );
                    QString oldclass = sgc_class.cap( 0 );

                    line_in_new_styles.indexOf( sgc_class );
                    QString newclass = sgc_class.cap( 0 );

                    redundant_classes[ newclass ] = oldclass;
                }

                else
                {   
                    continue;                    
                }
            }
        }
    }

    return redundant_classes;
}


