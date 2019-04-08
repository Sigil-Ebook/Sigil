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

#pragma once
#ifndef XHTMLDOC_H
#define XHTMLDOC_H

#include <memory>

#include <QtWebKit/QWebElement>
#include "Misc/GumboInterface.h"
#include "ViewEditors/ViewEditor.h"

class QString;
class QStringList;
class QXmlStreamReader;

const QList<GumboTag> GIMAGE_TAGS = QList<GumboTag>() << GUMBO_TAG_IMG << GUMBO_TAG_IMAGE;
const QList<GumboTag> GVIDEO_TAGS = QList<GumboTag>() << GUMBO_TAG_VIDEO;
const QList<GumboTag> GAUDIO_TAGS = QList<GumboTag>() << GUMBO_TAG_AUDIO;

class XhtmlDoc
{

public:

    struct XMLElement {
        // The name of the element
        QString name;

        // The text of the element
        QString text;

        // The line number of the source
        int lineno;

        // The attributes of the element;
        // the keys are the attribute names,
        // the values are the attribute values
        QHash<QString, QString> attributes;
    };

    // Resolves custom ENTITY declarations
    static QString ResolveCustomEntities(const QString &source);

    // Returns a list of XMLElements representing all
    // the elements of the specified tag name
    // in the head section of the provided XHTML source code
    static QList<XMLElement> GetTagsInHead(const QString &source, const QString &tag_name);

    // Returns a list of XMLElements representing all
    // the elements of the specified tag name
    // in the entire document of the provided XHTML source code
    static QList<XMLElement> GetTagsInDocument(const QString &source, const QString &tag_name);

    // static QList<xc::DOMNode *> GetNodeChildren(const xc::DOMNode &node);

    static QList<QString> GetAllDescendantStyleUrls(const QString & source);
    static QList<QString> GetAllDescendantHrefs(const QString & source);
    static QList<QString> GetAllDescendantIDs(const QString & );
    static QList<QString> GetAllDescendantClasses(const QString & source);

    struct WellFormedError {
        int line;
        int column;
        QString message;

        WellFormedError() : line(-1), column(-1) {}
    };

    static WellFormedError GumboWellFormedErrorForSource(const QString &source, QString version="2.0");

    static WellFormedError WellFormedErrorForSource(const QString &source, QString version="2.0");

    static bool IsDataWellFormed(const QString &data, QString version="2.0");

    // Accepts a string with HTML and returns the text
    // in that HTML fragment. For instance:
    //   <h1>Hello <b>Qt</b>&nbsp;this is great</h1>
    // returns
    //   Hello Qt this is great
    // static QString GetTextInHtml(const QString &source);

    // Resolves HTML entities in the provided string.
    // For instance:
    //    Bonnie &amp; Clyde
    // returns
    //    Bonnie & Clyde
    // static QString ResolveHTMLEntities(const QString &text);

    // static QList<QWebElement> QWebElementChildren(const QWebElement &element);

    /**
     * Splits the provided source on SGF section breaks.
     *
     * @param source The source which we want to split.
     * @param custom_header An option custom header to be used instead of
     *                      the one in the current source.
     * @return The split sections.
     */
    static QStringList GetSGFSectionSplits(const QString &source,
                                           const QString &custom_header = QString());

    // Return a list of all linked CSS stylesheets
    static QStringList GetLinkedStylesheets(const QString &source);

    // Returns a list of all the "visible" text nodes that are descendants
    // of the specified node. "Visible" means we ignore style tags, script tags etc...
    static QList<GumboNode *> GetVisibleTextNodes(GumboInterface &gi, GumboNode *node);

    // Returns a list of all the nodes that are suitable for use with "id" attributes
    static QList<GumboNode *> GetIDNodes(GumboInterface &gi, GumboNode* node);

    // Returns the text for the node plus any children's text if they are not ID nodes
    static QString GetIDElementText(GumboInterface &gi, GumboNode *node);

    // Returns the first block element ancestor of the specified node
    static GumboNode *GetAncestorBlockElement(GumboInterface &gi, GumboNode *node);

    static GumboNode *GetAncestorIDElement(GumboInterface &gi, GumboNode *node);

    static QStringList GetPathsToMediaFiles(const QString &source);

    static QStringList GetPathsToStyleFiles(const QString &source);

    static QStringList GetAllURLPathsFromStylesheet(const QString & source, const QString & csspath);

    static QStringList GetAllMediaPathsFromMediaChildren(const QString &source, QList<GumboTag> tags);


private:

    // Accepts a reference to an XML stream reader positioned on an XML element.
    // Returns an XMLElement struct with the data in the stream.
    static XMLElement CreateXMLElement(QXmlStreamReader &reader);

};

#endif // XHTMLDOC_H

