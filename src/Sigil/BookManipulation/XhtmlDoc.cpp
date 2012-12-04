/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
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

#include <boost/bind/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
//FlightCrew
#include <Misc/ErrorResultCollector.h>
// XercesExtensions
#include <LocationAwareDOMParser.h>
#include <NodeLocationInfo.h>
#include <XmlUtils.h>

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtWebKit/QWebFrame>
#include <QtWebKit/QWebPage>
#include <QtXml/QXmlInputSource>
#include <QtXml/QXmlSimpleReader>
#include <QtXml/QXmlStreamReader>

#include "BookManipulation/CleanSource.h"
#include "BookManipulation/XercesCppUse.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/Utility.h"
#include "sigil_constants.h"
#include "sigil_exception.h"

using boost::shared_ptr;

const QStringList BLOCK_LEVEL_TAGS = QStringList() << "address" << "blockquote" << "center" << "dir" << "div" <<
                                     "dl" << "fieldset" << "form" << "h1" << "h2" << "h3" <<
                                     "h4" << "h5" << "h6" << "hr" << "isindex" << "menu" <<
                                     "noframes" << "noscript" << "ol" << "p" << "pre" <<
                                     "table" << "ul" << "body";

const QStringList IMAGE_TAGS = QStringList() << "img" << "image";

static const QStringList INVALID_ID_TAGS = QStringList() << "base" << "head" << "meta" << "param" << "script" << "style" << "title";
static const QStringList SKIP_ID_TAGS = QStringList() << "html" << "#document" << "body";
const QStringList ID_TAGS = QStringList() << BLOCK_LEVEL_TAGS <<
                            "dd" << "dt" << "li" << "tbody" << "td" << "tfoot" <<
                            "th" << "thead" << "tr" << "a" << "abbr" <<
                            "acronym" << "address" << "b" << "big" <<
                            "caption" << "center" << "cite" << "code" << "font" <<
                            "label" << "i" << "pre" << "small" << "span" <<
                            "strike" << "strong" << "sub" << "sup" << "u";
const QStringList ANCHOR_TAGS = QStringList() << "a";
const QStringList SRC_TAGS = QStringList() << "link" << "img";


const int XML_DECLARATION_SEARCH_PREFIX_SIZE = 150;
static const int XML_CUSTOM_ENTITY_SEARCH_PREFIX_SIZE = 500;
static const QString ENTITY_SEARCH = "<!ENTITY\\s+(\\w+)\\s+\"([^\"]+)\">";

const QString BREAK_TAG_SEARCH  = "(<div>\\s*)?<hr\\s*class\\s*=\\s*\"[^\"]*(sigil_split_marker|sigilChapterBreak)[^\"]*\"\\s*/>(\\s*</div>)?";

namespace FlightCrew
{
extern const char         *NCX_2005_1_DTD_ID;
extern const unsigned int  NCX_2005_1_DTD_LEN;
extern const unsigned char NCX_2005_1_DTD[];
}

namespace fc = FlightCrew;


// Resolves custom ENTITY declarations
QString XhtmlDoc::ResolveCustomEntities(const QString &source)
{
    QString search_prefix = source.left(XML_CUSTOM_ENTITY_SEARCH_PREFIX_SIZE);

    if (!search_prefix.contains("<!ENTITY")) {
        return source;
    }

    QString new_source = source;
    QRegExp entity_search(ENTITY_SEARCH);
    QHash< QString, QString > entities;
    int main_index = 0;

    // Catch all custom entity declarations...
    while (true) {
        main_index = new_source.indexOf(entity_search, main_index);

        if (main_index == -1) {
            break;
        }

        entities[ "&" + entity_search.cap(1) + ";" ] = entity_search.cap(2);
        // Erase the entity declaration
        new_source.replace(entity_search.cap(0), "");
    }

    // ...and now replace all occurrences
    foreach(QString key, entities.keys()) {
        new_source.replace(key, entities[ key ]);
    }
    // Clean up what's left of the custom entity declaration field
    new_source.replace(QRegExp("\\[\\s*\\]>"), "");
    return new_source;
}


// Returns a list of XMLElements representing all
// the elements of the specified tag name
// in the head section of the provided XHTML source code
QList< XhtmlDoc::XMLElement > XhtmlDoc::GetTagsInHead(const QString &source, const QString &tag_name)
{
    // TODO: how about replacing uses of this function
    // with XPath expressions? Profile for speed.
    QXmlStreamReader reader(source);
    bool in_head = false;
    QList< XMLElement > matching_elements;

    while (!reader.atEnd()) {
        reader.readNext();

        if (reader.isStartElement()) {
            if (reader.name() == "head" || reader.name() == "HEAD") {
                in_head = true;
            } else if (in_head && reader.name() == tag_name) {
                matching_elements.append(CreateXMLElement(reader));
            }
        } else if (reader.isEndElement() &&
                   (reader.name() == "head" || reader.name() == "HEAD")
                  ) {
            break;
        }
    }

    if (reader.hasError()) {
        boost_throw(ErrorParsingXml()
                    << errinfo_XML_parsing_error_string(reader.errorString().toStdString())
                    << errinfo_XML_parsing_line_number(reader.lineNumber())
                    << errinfo_XML_parsing_column_number(reader.columnNumber())
                   );
    }

    return matching_elements;
}


// Returns a list of XMLElements representing all
// the elements of the specified tag name
// in the entire document of the provided XHTML source code
QList< XhtmlDoc::XMLElement > XhtmlDoc::GetTagsInDocument(const QString &source, const QString &tag_name)
{
    // TODO: how about replacing uses of this function
    // with XPath expressions? Profile for speed.
    QXmlStreamReader reader(source);
    QList< XMLElement > matching_elements;

    while (!reader.atEnd()) {
        reader.readNext();

        if (reader.isStartElement() &&
            reader.name() == tag_name) {
            matching_elements.append(CreateXMLElement(reader));
        }
    }

    if (reader.hasError()) {
        boost_throw(ErrorParsingXml()
                    << errinfo_XML_parsing_error_string(reader.errorString().toStdString())
                    << errinfo_XML_parsing_line_number(reader.lineNumber())
                    << errinfo_XML_parsing_column_number(reader.columnNumber())
                   );
    }

    return matching_elements;
}


QList< xc::DOMNode * > XhtmlDoc::GetNodeChildren(const xc::DOMNode &node)
{
    xc::DOMNodeList *children = node.getChildNodes();
    int num_children = children->getLength();
    QList< xc::DOMNode * > qtchildren;

    for (int i = 0; i < num_children; ++i) {
        qtchildren.append(children->item(i));
    }

    return qtchildren;
}


QHash< QString, QString > XhtmlDoc::GetNodeAttributes(const xc::DOMNode &node)
{
    QHash< QString, QString > attributes_hash;
    xc::DOMNamedNodeMap *attributes = node.getAttributes();

    if (!attributes) {
        return attributes_hash;
    }

    for (uint i = 0; i < attributes->getLength(); ++i) {
        xc::DOMAttr &attribute = *static_cast< xc::DOMAttr * >(attributes->item(i));
        attributes_hash[ GetAttributeName(attribute) ] = XtoQ(attribute.getValue());
    }

    return attributes_hash;
}


QList< xc::DOMElement * > XhtmlDoc::GetTagMatchingDescendants(const xc::DOMNode &node, const QStringList &tag_names)
{
    QList< xc::DOMElement * > matching_nodes;

    if (tag_names.contains(GetNodeName(node), Qt::CaseInsensitive)) {
        matching_nodes.append((xc::DOMElement *) &node);
    }

    if (node.hasChildNodes()) {
        QList< xc::DOMNode * > children = GetNodeChildren(node);

        for (int i = 0; i < children.count(); ++i) {
            matching_nodes.append(GetTagMatchingDescendants(*children.at(i), tag_names));
        }
    }

    return matching_nodes;
}


// TODO: turn the overloads into a template
QList< xc::DOMElement * > XhtmlDoc::GetTagMatchingDescendants(const xc::DOMElement &node, const QString &tag_name)
{
    xc::DOMNodeList *children = node.getElementsByTagName(QtoX(tag_name));
    int num_children = children->getLength();
    QList< xc::DOMElement * > qtchildren;

    for (int i = 0; i < num_children; ++i) {
        qtchildren.append(static_cast< xc::DOMElement * >(children->item(i)));
    }

    return qtchildren;
}


QList< xc::DOMElement * > XhtmlDoc::GetTagMatchingDescendants(const xc::DOMDocument &node, const QString &tag_name)
{
    return GetTagMatchingDescendants(node, tag_name, "*");
}


QList< xc::DOMElement * > XhtmlDoc::GetTagMatchingDescendants(
    const xc::DOMDocument &node,
    const QString &tag_name,
    const QString &namespace_name)
{
    xc::DOMNodeList *children = node.getElementsByTagNameNS(QtoX(namespace_name), QtoX(tag_name));
    int num_children = children->getLength();
    QList< xc::DOMElement * > qtchildren;

    for (int i = 0; i < num_children; ++i) {
        qtchildren.append(static_cast< xc::DOMElement * >(children->item(i)));
    }

    return qtchildren;
}

QList<QString> XhtmlDoc::GetAllDescendantClasses(const xc::DOMNode &node)
{
    if (node.getNodeType() != xc::DOMNode::ELEMENT_NODE) {
        return QList< QString >();
    }

    const xc::DOMElement *element = static_cast< const xc::DOMElement * >(&node);
    QList< QString > classes;
    QString element_name = GetNodeName(*element);

    if (element->hasAttribute(QtoX("class"))) {
        QString class_values = XtoQ(element->getAttribute(QtoX("class")));
        foreach(QString class_name, class_values.split(" ")) {
            classes.append(element_name + "." + class_name);
        }
    }

    if (node.hasChildNodes()) {
        QList< xc::DOMNode * > children = GetNodeChildren(node);

        for (int i = 0; i < children.count(); ++i) {
            classes.append(GetAllDescendantClasses(*children.at(i)));
        }
    }

    return classes;
}

QList< QString > XhtmlDoc::GetAllDescendantIDs(const xc::DOMNode &node)
{
    if (node.getNodeType() != xc::DOMNode::ELEMENT_NODE) {
        return QList< QString >();
    }

    const xc::DOMElement *element = static_cast< const xc::DOMElement * >(&node);
    QList< QString > IDs;

    if (element->hasAttribute(QtoX("id"))) {
        IDs.append(XtoQ(element->getAttribute(QtoX("id"))));
    } else if (element->hasAttribute(QtoX("name"))) {
        // This is supporting legacy html of <a name="xxx"> (deprecated).
        // Make sure we don't return names of other elements like <meta> tags.
        if (XtoQ(element->getTagName()).toLower() == "a") {
            IDs.append(XtoQ(element->getAttribute(QtoX("name"))));
        }
    }

    if (node.hasChildNodes()) {
        QList< xc::DOMNode * > children = GetNodeChildren(node);

        for (int i = 0; i < children.count(); ++i) {
            IDs.append(GetAllDescendantIDs(*children.at(i)));
        }
    }

    return IDs;
}

QList< QString > XhtmlDoc::GetAllDescendantHrefs(const xc::DOMNode &node)
{
    if (node.getNodeType() != xc::DOMNode::ELEMENT_NODE) {
        return QList< QString >();
    }

    const xc::DOMElement *element = static_cast< const xc::DOMElement * >(&node);
    QList< QString > hrefs;

    if (element->hasAttribute(QtoX("href"))) {
        hrefs.append(XtoQ(element->getAttribute(QtoX("href"))));
    }

    if (node.hasChildNodes()) {
        QList< xc::DOMNode * > children = GetNodeChildren(node);

        for (int i = 0; i < children.count(); ++i) {
            hrefs.append(GetAllDescendantHrefs(*children.at(i)));
        }
    }

    return hrefs;
}


// DO NOT USE FOR DOMDOCUMENTS! Use GetDomDocumentAsString for such needs!
QString XhtmlDoc::GetDomNodeAsString(const xc::DOMNode &node)
{
    XMLCh LS[] = { xc::chLatin_L, xc::chLatin_S, xc::chNull };
    xc::DOMImplementation *impl = xc::DOMImplementationRegistry::getDOMImplementation(LS);
    shared_ptr< xc::DOMLSSerializer > serializer(
        ((xc::DOMImplementationLS *) impl)->createLSSerializer(),
        XercesExt::XercesDeallocator< xc::DOMLSSerializer >);
    serializer->getDomConfig()->setParameter(xc::XMLUni::fgDOMWRTDiscardDefaultContent, false);
    serializer->getDomConfig()->setParameter(xc::XMLUni::fgDOMWRTBOM, true);
    shared_ptr< XMLCh > xwritten(serializer->writeToString(&node), XercesExt::XercesStringDeallocator);
    return XtoQ(xwritten.get());
}


// This func makes sure that the UTF-8 encoding is set for the XML declaration
QString XhtmlDoc::GetDomDocumentAsString(const xc::DOMDocument &document)
{
    QString raw_source = GetDomNodeAsString(document);
    QRegExp encoding(ENCODING_ATTRIBUTE);
    int encoding_start = raw_source.indexOf(encoding);
    return raw_source.replace(encoding_start, encoding.matchedLength(), "encoding=\"UTF-8\"");
}


shared_ptr< xc::DOMDocument > XhtmlDoc::CopyDomDocument(const xc::DOMDocument &document)
{
    return RaiiWrapDocument(static_cast< xc::DOMDocument * >(document.cloneNode(true)));
}


shared_ptr< xc::DOMDocument > XhtmlDoc::LoadTextIntoDocument(const QString &source)
{
    XercesExt::LocationAwareDOMParser parser;
    // This scanner ignores schemas
    parser.useScanner(xc::XMLUni::fgDGXMLScanner);
    parser.setValidationScheme(xc::AbstractDOMParser::Val_Never);
    parser.useCachedGrammarInParse(true);
    parser.setLoadExternalDTD(true);
    parser.setDoNamespaces(true);
    xc::MemBufInputSource xhtml_dtd(XHTML_ENTITIES_DTD, XHTML_ENTITIES_DTD_LEN, XHTML_ENTITIES_DTD_ID);
    parser.loadGrammar(xhtml_dtd, xc::Grammar::DTDGrammarType, true);
    xc::MemBufInputSource ncx_dtd(fc::NCX_2005_1_DTD, fc::NCX_2005_1_DTD_LEN, fc::NCX_2005_1_DTD_ID);
    parser.loadGrammar(ncx_dtd, xc::Grammar::DTDGrammarType, true);
    QString prepared_source = PrepareSourceForXerces(source);
    // We use source.count() * 2 because count returns
    // the number of QChars, which are 2 bytes long
    xc::MemBufInputSource input(
        reinterpret_cast< const XMLByte * >(prepared_source.utf16()),
        prepared_source.count() * 2,
        "empty");
    XMLCh UTF16[] = { xc::chLatin_U, xc::chLatin_T, xc::chLatin_F, xc::chDigit_1, xc::chDigit_6, xc::chNull };
    input.setEncoding(UTF16);
    parser.parse(input);
    return RaiiWrapDocument(parser.adoptDocument());
}


shared_ptr< xc::DOMDocument > XhtmlDoc::RaiiWrapDocument(xc::DOMDocument *document)
{
    return shared_ptr< xc::DOMDocument >(document, XercesExt::XercesDeallocator< xc::DOMDocument >);
}


int XhtmlDoc::NodeLineNumber(const xc::DOMNode &node)
{
    return XercesExt::GetNearestNodeLocationInfo(node).LineNumber;
}


int XhtmlDoc::NodeColumnNumber(const xc::DOMNode &node)
{
    return XercesExt::GetNearestNodeLocationInfo(node).ColumnNumber;
}


XhtmlDoc::WellFormedError XhtmlDoc::WellFormedErrorForSource(const QString &source)
{
    boost::scoped_ptr< xc::SAX2XMLReader > parser(xc::XMLReaderFactory::createXMLReader());
    parser->setFeature(xc::XMLUni::fgSAX2CoreValidation,            false);
    parser->setFeature(xc::XMLUni::fgXercesSchema,                  false);
    parser->setFeature(xc::XMLUni::fgXercesLoadSchema,              false);
    parser->setFeature(xc::XMLUni::fgXercesUseCachedGrammarInParse, true);
    parser->setFeature(xc::XMLUni::fgXercesSkipDTDValidation,       true);
    // We need the DGXMLScanner because of the entities
    parser->setProperty(xc::XMLUni::fgXercesScannerName,
                        (void *) xc::XMLUni::fgDGXMLScanner);
    xc::MemBufInputSource xhtml_dtd(XHTML_ENTITIES_DTD, XHTML_ENTITIES_DTD_LEN, XHTML_ENTITIES_DTD_ID);
    parser->loadGrammar(xhtml_dtd, xc::Grammar::DTDGrammarType, true);
    xc::MemBufInputSource ncx_dtd(fc::NCX_2005_1_DTD, fc::NCX_2005_1_DTD_LEN, fc::NCX_2005_1_DTD_ID);
    parser->loadGrammar(ncx_dtd, xc::Grammar::DTDGrammarType, true);
    fc::ErrorResultCollector collector;
    parser->setErrorHandler(&collector);
    QString prepared_source = PrepareSourceForXerces(source);
    // We use source.count() * 2 because count returns
    // the number of QChars, which are 2 bytes long
    xc::MemBufInputSource input(
        reinterpret_cast< const XMLByte * >(prepared_source.utf16()),
        prepared_source.count() * 2,
        "empty");
    XMLCh UTF16[] = { xc::chLatin_U, xc::chLatin_T, xc::chLatin_F, xc::chDigit_1, xc::chDigit_6, xc::chNull };
    input.setEncoding(UTF16);

    try {
        parser->parse(input);
    } catch (xc::SAXException &exception) {
        collector.AddNewExceptionAsResult(exception);
    } catch (xc::XMLException &exception) {
        collector.AddNewExceptionAsResult(exception);
    }

    std::vector< fc::Result > results = collector.GetResults();

    if (!results.empty()) {
        XhtmlDoc::WellFormedError error;
        error.line    = results[ 0 ].GetErrorLine();
        error.column  = results[ 0 ].GetErrorColumn();
        error.message = QString::fromUtf8(results[ 0 ].GetMessage().data());
        return error;
    }

    return XhtmlDoc::WellFormedError();
}


// This only exist because of a bug in Apple's GCC.
// It has problems with templates in default arguments.
xc::DOMElement *XhtmlDoc::CreateElementInDocument(
    const QString &tag_name,
    const QString &namespace_name,
    xc::DOMDocument &document)
{
    return CreateElementInDocument(tag_name, namespace_name, document, QHash< QString, QString >());
}


xc::DOMElement *XhtmlDoc::CreateElementInDocument(
    const QString &tag_name,
    const QString &namespace_name,
    xc::DOMDocument &document,
    QHash< QString, QString > attributes)
{
    xc::DOMElement *element = document.createElementNS(QtoX(namespace_name), QtoX(tag_name));
    foreach(QString attribute_name, attributes.keys()) {
        element->setAttribute(QtoX(attribute_name), QtoX(attributes[ attribute_name ]));
    }
    return element;
}


xc::DOMElement *XhtmlDoc::RenameElementInDocument(xc::DOMDocument &document, xc::DOMNode &node, QString tag_name)
{
    xc::DOMElement *element = static_cast< xc::DOMElement * >(&node);
    xc::DOMElement *new_element = CreateElementInDocument(tag_name, XtoQ(element->getNamespaceURI()), document, GetNodeAttributes(node));

    // Move all the children
    while (node.hasChildNodes()) {
        new_element->appendChild(element->getFirstChild());
    }

    // Replace the old node with the new node
    node.getParentNode()->replaceChild(new_element, element);
    return new_element;
}


// Accepts a string with HTML and returns the text
// in that HTML fragment. For instance:
//   <h1>Hello <b>Qt</b>&nbsp;this is great</h1>
// returns
//   Hello Qt this is great
QString XhtmlDoc::GetTextInHtml(const QString &source)
{
    QWebPage page;
    page.mainFrame()->setHtml(source);
    return page.mainFrame()->toPlainText();
}


// Resolves HTML entities in the provided string.
// For instance:
//    Bonnie &amp; Clyde
// returns
//    Bonnie & Clyde
QString XhtmlDoc::ResolveHTMLEntities(const QString &text)
{
    // Faking some HTML... this is the easiest way to do it
    QString newsource = "<div>" + text + "</div>";
    return GetTextInHtml(newsource);
}


// A tree node class without a children() function...
// appallingly stupid, isn't it?
QList< QWebElement > XhtmlDoc::QWebElementChildren(const QWebElement &element)
{
    QList< QWebElement > children;
    const QWebElement &first_child = element.firstChild();

    if (!first_child.isNull()) {
        children.append(first_child);
    }

    QWebElement next_sibling = first_child.nextSibling();

    while (!next_sibling.isNull()) {
        children.append(next_sibling);
        next_sibling = next_sibling.nextSibling();
    }

    return children;
}


QStringList XhtmlDoc::GetSGFSectionSplits(const QString &source,
        const QString &custom_header)
{
    QRegExp body_start_tag(BODY_START);
    QRegExp body_end_tag(BODY_END);
    int body_begin = source.indexOf(body_start_tag, 0) + body_start_tag.matchedLength();
    int body_end   = source.indexOf(body_end_tag,   0);
    int main_index = body_begin;
    QString header = !custom_header.isEmpty() ? custom_header + "<body>\n" : source.left(body_begin);
    QStringList sections;
    QRegExp break_tag(BREAK_TAG_SEARCH);

    while (main_index != body_end) {
        // We search for our HR break tag
        int break_index = source.indexOf(break_tag, main_index);
        QString body;

        // We break up the remainder of the file on the HR tag index if it's found
        if (break_index > -1) {
            body = Utility::Substring(main_index, break_index, source);
            main_index = break_index + break_tag.matchedLength();
        }
        // Otherwise, we take the rest of the file
        else {
            body = Utility::Substring(main_index, body_end, source);
            main_index = body_end;
        }

        sections.append(header + body + "</body> </html>");
    }

    return sections;
}


QStringList XhtmlDoc::GetLinkedStylesheets(const QString &source)
{
    QList< XhtmlDoc::XMLElement > link_tag_nodes;

    try {
        link_tag_nodes = XhtmlDoc::GetTagsInHead(source, "link");
    } catch (ErrorParsingXml) {
        // Nothing really. If we can't get the CSS style tags,
        // than that's it. No CSS returned.
    }

    QStringList linked_css_paths;
    foreach(XhtmlDoc::XMLElement element, link_tag_nodes) {
        if (element.attributes.contains("type") &&
            (element.attributes.value("type").toLower() == "text/css") &&
            element.attributes.contains("rel") &&
            (element.attributes.value("rel").toLower() == "stylesheet") &&
            element.attributes.contains("href")) {
            linked_css_paths.append(element.attributes.value("href"));
        }
    }
    return linked_css_paths;
}


void XhtmlDoc::RemoveChildren(xc::DOMNode &node)
{
    while (true) {
        xc::DOMNode *child = node.getFirstChild();

        if (!child) {
            break;
        }

        node.removeChild(child);
    }
}


// Returns the node's "real" name. We don't care
// about namespace prefixes and whatnot.
QString XhtmlDoc::GetNodeName(const xc::DOMNode &node)
{
    QString local_name = XtoQ(node.getLocalName());

    if (local_name.isEmpty()) {
        return XtoQ(node.getNodeName());
    } else {
        return local_name;
    }
}


// TODO: this should be covered by attribute.localName(), no?
QString XhtmlDoc::GetAttributeName(const xc::DOMAttr &attribute)
{
    QString name = XtoQ(attribute.getName());
    int colon_index = name.lastIndexOf(QChar(':'));

    if (colon_index < 0) {
        return name;
    } else {
        return name.mid(colon_index + 1);
    }
}


xc::DOMDocumentFragment *XhtmlDoc::ConvertToDocumentFragment(const xc::DOMNodeList &list)
{
    if (list.getLength() == 0) {
        return NULL;
    }

    xc::DOMDocumentFragment *fragment = list.item(0)->getOwnerDocument()->createDocumentFragment();
    // Since a DomNodeList is "live", we store the count
    // so we don't have to recalculate it every loop iteration
    int count = list.getLength();

    for (int i = 0; i < count; ++i) {
        // We need to clone the node before inserting it in the
        // fragment so as to pick up the node's descendants too
        fragment->appendChild(list.item(i)->cloneNode(true));
    }

    return fragment;
}


// Converts a DomNodeList to a regular QList
QList< xc::DOMNode * > XhtmlDoc::ConvertToRegularList(const xc::DOMNodeList &list)
{
    // Since a DomNodeList is "live", we store the count
    // so we don't have to recalculate it every loop iteration
    int count = list.getLength();
    QList< xc::DOMNode * > nodes;

    for (int i = 0; i < count; ++i) {
        nodes.append(list.item(i));
    }

    return nodes;
}


// Returns a list with only the element nodes
QList< xc::DOMNode * > XhtmlDoc::ExtractElements(const xc::DOMNodeList &list)
{
    // Since a DomNodeList is "live", we store the count
    // so we don't have to recalculate it every loop iteration
    int count = list.getLength();
    QList< xc::DOMNode * > element_nodes;

    for (int i = 0; i < count; ++i) {
        xc::DOMNode *node = list.item(i);

        if (node->getNodeType() == xc::DOMNode::ELEMENT_NODE) {
            element_nodes.append(node);
        }
    }

    return element_nodes;
}


// Returns the node's real index in the list
int XhtmlDoc::GetRealIndexInList(const xc::DOMNode &node, const xc::DOMNodeList &list)
{
    // Since a DomNodeList is "live", we store the count
    // so we don't have to recalculate it every loop iteration
    int count = list.getLength();

    for (int i = 0; i < count; ++i) {
        if (list.item(i)->isSameNode(&node)) {
            return i;
        }
    }

    return -1;
}

// Returns the node's "element" index
// (pretending the list is only made up of element nodes).
int XhtmlDoc::GetElementIndexInList(const xc::DOMNode &node, const xc::DOMNodeList &list)
{
    // Since a DomNodeList is "live", we store the count
    // so we don't have to recalculate it every loop iteration
    int count = list.getLength();
    int element_index = 0;

    for (int i = 0; i < count; ++i) {
        if (list.item(i)->isSameNode(&node)) {
            return element_index;
        }

        if (list.item(i)->getNodeType() == xc::DOMNode::ELEMENT_NODE) {
            element_index++;
        }
    }

    return -1;
}

// Returns the index of node in the specified list
// depending on the node type. Text nodes get the "real"
// index, element nodes get the "element" index
// (pretending the list is only made up of element nodes).
int XhtmlDoc::GetCustomIndexInList(const xc::DOMNode &node, const xc::DOMNodeList &list)
{
    if (node.getNodeType() == xc::DOMNode::TEXT_NODE) {
        return GetRealIndexInList(node, list);
    } else {
        return GetElementIndexInList(node, list);
    }
}


// Returns a list of all the "visible" text nodes that are descendants
// of the specified node. "Visible" means we ignore style tags, script tags etc...
QList< xc::DOMNode * > XhtmlDoc::GetVisibleTextNodes(const xc::DOMNode &node)
{
    // TODO: investigate possible parallelization
    // opportunities for this function (profile before and after!)
    if (node.getNodeType() == xc::DOMNode::TEXT_NODE) {
        return QList< xc::DOMNode * >() << const_cast< xc::DOMNode * >(&node);
    } else {
        QString node_name = GetNodeName(node);

        if (node.hasChildNodes()  &&
            node_name != "script" &&
            node_name != "style"
           ) {
            QList< xc::DOMNode * > children = GetNodeChildren(node);
            QList< xc::DOMNode * > text_nodes;

            for (int i = 0; i < children.count(); ++i) {
                text_nodes.append(GetVisibleTextNodes(*children.at(i)));
            }

            return text_nodes;
        }
    }

    return QList< xc::DOMNode * >();
}


// Returns a list of all nodes suitable for "id" element
QList<xc::DOMNode *> XhtmlDoc::GetIDNodes(const xc::DOMNode &node)
{
    QList<xc::DOMNode *> text_nodes = QList<xc::DOMNode *>();
    QString node_name = GetNodeName(node);

    if (node.hasChildNodes() && node_name != "head") {
        QList<xc::DOMNode *> children = GetNodeChildren(node);

        if (!INVALID_ID_TAGS.contains(node_name)) {
            if (ID_TAGS.contains(node_name)) {
                text_nodes.append(const_cast <xc::DOMNode *>(&node));
            } else if (!SKIP_ID_TAGS.contains(node_name)) {
                xc::DOMNode &ancestor_id_node = GetAncestorIDElement(node);

                if (!text_nodes.contains(const_cast <xc::DOMNode *>(&ancestor_id_node))) {
                    text_nodes.append(const_cast <xc::DOMNode *>(&ancestor_id_node));
                }
            }

            // Parse children after parent to keep index numbers in order
            for (int i = 0; i < children.count(); ++i) {
                QList<xc::DOMNode *> children_text_nodes = GetIDNodes(*children.at(i));
                foreach(xc::DOMNode * cnode, children_text_nodes) {
                    if (!text_nodes.contains(const_cast <xc::DOMNode *>(cnode))) {
                        text_nodes.append(cnode);
                    }
                }
            }
        }
    }

    return text_nodes;
}

QString XhtmlDoc::GetIDElementText(const xc::DOMNode &node)
{
    QString text;
    QList< xc::DOMNode * > children = GetNodeChildren(node);

    // Combine all text nodes for this node plus all text for non-ID element children
    for (int i = 0; i < children.count(); ++i) {
        xc::DOMNode *child_node = children.at(i);
        QString child_node_name = GetNodeName(*child_node);

        if ((*children.at(i)).getNodeType() == xc::DOMNode::TEXT_NODE) {
            xc::DOMNode *text_node = children.at(i);
            xc::DOMElement &text_element = static_cast<xc::DOMElement &>(*text_node);
            text += XtoQ(text_element.getTextContent());
        } else if (!ID_TAGS.contains(child_node_name)) {
            text += GetIDElementText(*child_node);
        }
    }

    return text;
}


// Returns a list of ALL text nodes that are descendants
// of the specified node.
QList< xc::DOMNode * > XhtmlDoc::GetAllTextNodes(const xc::DOMNode &node)
{
    // TODO: investigate possible parallelization
    // opportunities for this function (profile before and after!)
    if (node.getNodeType() == xc::DOMNode::TEXT_NODE) {
        return QList< xc::DOMNode * >() << const_cast< xc::DOMNode * >(&node);
    } else {
        if (node.hasChildNodes()) {
            QList< xc::DOMNode * > children = GetNodeChildren(node);
            QList< xc::DOMNode * > text_nodes;

            for (int i = 0; i < children.count(); ++i) {
                text_nodes.append(GetAllTextNodes(*children.at(i)));
            }

            return text_nodes;
        }
    }

    return QList< xc::DOMNode * >();
}


// Returns the first block element ancestor of the specified node
xc::DOMNode &XhtmlDoc::GetAncestorBlockElement(const xc::DOMNode &node)
{
    const xc::DOMNode *parent_node = &node;

    while (true) {
        parent_node = parent_node->getParentNode();

        if (BLOCK_LEVEL_TAGS.contains(GetNodeName(*parent_node))) {
            break;
        }
    }

    if (parent_node) {
        return const_cast< xc::DOMNode & >(*parent_node);
    } else {
        return *(node.getOwnerDocument()->getElementsByTagName(QtoX("body"))->item(0));
    }
}

xc::DOMNode &XhtmlDoc::GetAncestorIDElement(const xc::DOMNode &node)
{
    const xc::DOMNode *parent_node = &node;

    while (true) {
        parent_node = parent_node->getParentNode();

        if (ID_TAGS.contains(GetNodeName(*parent_node))) {
            break;
        }
    }

    if (parent_node) {
        return const_cast<xc::DOMNode &>(*parent_node);
    } else {
        return *(node.getOwnerDocument()->getElementsByTagName(QtoX("body"))->item(0));
    }
}

QStringList XhtmlDoc::GetImagePathsFromImageChildren(const xc::DOMNode &node)
{
    QStringList image_links = GetAllImagePathsFromImageChildren(node);
    // Remove duplicate references
    image_links.removeDuplicates();
    return image_links;
}

QStringList XhtmlDoc::GetAllImagePathsFromImageChildren(const xc::DOMNode &node)
{
    // "Normal" HTML image elements
    QList< xc::DOMElement * > image_nodes = GetTagMatchingDescendants(node, IMAGE_TAGS);
    QStringList image_links;
    // Get a list of all images referenced
    foreach(xc::DOMElement * image_node, image_nodes) {
        QString url_reference;

        if (image_node->hasAttribute(QtoX("src"))) {
            url_reference = Utility::URLDecodePath(XtoQ(image_node->getAttribute(QtoX("src"))));
        } else { // This covers the SVG "image" tags
            url_reference = Utility::URLDecodePath(XtoQ(image_node->getAttribute(QtoX("xlink:href"))));
        }

        if (!url_reference.isEmpty()) {
            image_links << url_reference;
        }
    }
    return image_links;
}

QStringList XhtmlDoc::GetAllHrefPaths(const xc::DOMNode &node)
{
    // Anchor tags
    QList< xc::DOMElement * > nodes = GetTagMatchingDescendants(node, ANCHOR_TAGS);
    QStringList hrefs;
    // Get a list of all defined hrefs
    foreach(xc::DOMElement * node, nodes) {
        if (node->hasAttribute(QtoX("href"))) {
            hrefs.append(XtoQ(node->getAttribute(QtoX("href"))));
        }
    }
    return hrefs;
}

// Accepts a reference to an XML stream reader positioned on an XML element.
// Returns an XMLElement struct with the data in the stream.
XhtmlDoc::XMLElement XhtmlDoc::CreateXMLElement(QXmlStreamReader &reader)
{
    XMLElement element;
    foreach(QXmlStreamAttribute attribute, reader.attributes()) {
        QString attribute_name = attribute.name().toString();

        // We convert non-mixed case attribute names to lower case;
        // simplifies things later on so we for instance don't
        // have to check for both "src" and "SRC".
        if (!Utility::IsMixedCase(attribute_name)) {
            attribute_name = attribute_name.toLower();
        }

        element.attributes[ attribute_name ] = attribute.value().toString();
    }
    element.name = reader.name().toString();
    element.text = reader.readElementText();
    return element;
}


QString XhtmlDoc::PrepareSourceForXerces(const QString &source)
{
    QString prefix = source.left(XML_DECLARATION_SEARCH_PREFIX_SIZE);
    QRegExp standalone(STANDALONE_ATTRIBUTE);
    prefix.indexOf(standalone);
    return QString(source).remove(standalone.pos(), standalone.matchedLength());
}


// Returns the node identified by the specified ViewEditor element hierarchy
xc::DOMNode *XhtmlDoc::GetNodeFromHierarchy(const xc::DOMDocument &document,
        const QList< ViewEditor::ElementIndex > &hierarchy)
{
    xc::DOMNode *node = document.getElementsByTagName(QtoX("html"))->item(0);
    if (node == NULL) {
        return NULL;
    }
    xc::DOMNode *end_node = NULL;
    xc::DOMNodeList *tmp_node = NULL;

    for (int i = 0; i < hierarchy.count() - 1; ++i) {
        QList< xc::DOMNode * > children;

        tmp_node = node->getChildNodes();
        if (tmp_node != NULL) {
            if (hierarchy[ i + 1 ].name != "#text") {
                children = ExtractElements(*tmp_node);
            } else {
                children = ConvertToRegularList(*tmp_node);
            }
        }

        // If the index is within the range, descend
        if (hierarchy[ i ].index < children.count()) {
            node = children.at(hierarchy[ i ].index);

            if (node) {
                end_node = node;
            } else {
                break;
            }
        }
        // Error handling. The asked-for node cannot be found,
        // so we stop where we are.
        else {
            end_node = node;
            break;
        }
    }

    return end_node;
}

// Creates a ViewEditor element hierarchy from the specified node
QList< ViewEditor::ElementIndex > XhtmlDoc::GetHierarchyFromNode(const xc::DOMNode &node)
{
    xc::DOMNode *html_node = node.getOwnerDocument()->getElementsByTagName(QtoX("html"))->item(0);
    const xc::DOMNode *current_node = &node;
    QList< ViewEditor::ElementIndex > element_list;

    while (current_node != html_node) {
        xc::DOMNode *parent = current_node->getParentNode();
        ViewEditor::ElementIndex element;
        element.name  = GetNodeName(*parent);
        element.index = GetCustomIndexInList(*current_node, *parent->getChildNodes());
        element_list.prepend(element);
        current_node = parent;
    }

    return element_list;
}

