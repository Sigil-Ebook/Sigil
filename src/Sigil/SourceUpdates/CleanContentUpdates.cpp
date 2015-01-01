/************************************************************************
**
**  Copyright (C) 2014 Marek Gibek
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

#include <QtCore/QtCore>
#include <QtCore/QString>
#include <QtConcurrent/QtConcurrent>

#include "BookManipulation/CleanSource.h"
#include "BookManipulation/XercesCppUse.h"
#include "BookManipulation/XhtmlDoc.h"
#include "ResourceObjects/HTMLResource.h"
#include "Misc/Utility.h"
#include "sigil_constants.h"
#include "SourceUpdates/CleanContentUpdates.h"

using boost::shared_ptr;

ChangesCount CleanContentUpdates::CleanContentInAllFiles(const QList<HTMLResource *> &html_resources,
                                                 const CleanContentParams &params)
{
    return QtConcurrent::blockingMappedReduced(html_resources,
                                               boost::bind(CleanContentInOneFile, _1, params),
                                               &ReduceFunction);
}

ChangesCount CleanContentUpdates::CleanContentInOneFile(HTMLResource *html_resource,
                                                const CleanContentParams &params)
{
    ChangesCount changes_count;
    changes_count.number_of_files = 0;
    changes_count.number_of_changes = 0;

    QWriteLocker locker(&html_resource->GetLock());
    shared_ptr<xc::DOMDocument> d = XhtmlDoc::LoadTextIntoDocument(html_resource->GetText());
    xc::DOMDocument &doc = *d.get();

    if (params.remove_page_numbers) {
        changes_count.number_of_changes += RemovePageNumbers(doc, params.page_number_format,
                                                             params.page_number_remove_empty_paragraphs);
    }

    if (params.join_paragraphs) {
        changes_count.number_of_changes += JoinParagraphs(doc, params.join_paragraphs_only_not_formatted);
    }

    if (changes_count.number_of_changes > 0) {
        changes_count.number_of_files = 1;
        html_resource->SetText(CleanSource::Clean(XhtmlDoc::GetDomDocumentAsString(doc)));
    }

    return changes_count;
}

void CleanContentUpdates::ReduceFunction(ChangesCount &acc, ChangesCount one_result)
{
    acc.number_of_files += one_result.number_of_files;
    acc.number_of_changes += one_result.number_of_changes;
}

int CleanContentUpdates::RemovePageNumbers(xc::DOMDocument &doc,
                                           const QString &page_number_format,
                                           bool remove_empty_paragraphs_around)
{
    int number_of_changes = 0;

    QRegularExpression re(ConvertSamplePageNumberToRegExp(page_number_format));

    // body should only appear once
    xc::DOMNodeList *bodys = doc.getElementsByTagName(QtoX("body"));
    xc::DOMElement &body_element = *static_cast<xc::DOMElement *>(bodys->item(0));

    xc::DOMNodeList *paragraphs = body_element.getElementsByTagName(QtoX("p"));

    // change in reverse to preserve location information
    int paragraph_count = paragraphs->getLength();
    for (int i = paragraph_count - 1; i >= 0; i--) {
        xc::DOMElement &element = *static_cast<xc::DOMElement *>(paragraphs->item(i));
        QString element_text = XtoQ(element.getTextContent());

        if (re.match(element_text).hasMatch()) {
            body_element.removeChild(&element);
            number_of_changes++;

            if (remove_empty_paragraphs_around) {

                // remove paragraphs after (paragraph count is dynamic)
                paragraph_count = paragraphs->getLength();
                int j = i;
                while (j < paragraph_count) {
                    xc::DOMElement &el = *static_cast<xc::DOMElement *>(paragraphs->item(j));
                    element_text = XtoQ(el.getTextContent());
                    element_text = element_text.trimmed();
                    if (element_text.length() != 0) {
                        break;
                    }
                    body_element.removeChild(&el);
                    paragraph_count = paragraphs->getLength();
                }

                // remove paragraphs before
                while (i > 0) {
                    xc::DOMElement &el = *static_cast<xc::DOMElement *>(paragraphs->item(i - 1));
                    element_text = XtoQ(el.getTextContent());
                    element_text = element_text.trimmed();
                    if (element_text.length() != 0) {
                        break;
                    }
                    body_element.removeChild(&el);
                    i--;
                }
            }
        }
    }

    return number_of_changes;
}

int CleanContentUpdates::JoinParagraphs(xc::DOMDocument &doc, bool only_not_formatted)
{
    int number_of_changes = 0;

    // body should only appear once
    xc::DOMNodeList *bodys = doc.getElementsByTagName(QtoX("body"));
    xc::DOMElement &body_element = *static_cast<xc::DOMElement *>(bodys->item(0));

    // analyse only children of body tag
    xc::DOMNodeList *children = body_element.getChildNodes();
    int children_length = children->getLength();
    int pos = 0;
    while (pos < children_length - 1) {

        pos += SkipEmptyNodes(children, pos);
        if (pos >= children_length - 1) {
            break;
        }
        xc::DOMElement &e1 = *static_cast<xc::DOMElement *>(children->item(pos));
        int pos_e1 = pos;

        pos++;
        pos += SkipEmptyNodes(children, pos);
        if (pos >= children_length) {
            break;
        }
        xc::DOMElement &e2 = *static_cast<xc::DOMElement *>(children->item(pos));

        QString e1_name = XhtmlDoc::GetNodeName(e1);
        QString e2_name = XhtmlDoc::GetNodeName(e2);

        if (e1_name != "p" || e2_name != "p") {
            // we are looking for sibling "p" tags
            continue;
        }

        if (CanJoinParagraphs(e1, e2, only_not_formatted)) {
            JoinParagraphs(doc, e1, e2);
            number_of_changes++;
            pos = pos_e1;
            children_length--;
            continue;
        }
    }

    return number_of_changes;
}

int CleanContentUpdates::SkipEmptyNodes(xc::DOMNodeList *nodes, int pos)
{
    int skipped = 0;
    int nodes_length = nodes->getLength();
    while (pos + skipped < nodes_length) {
        xc::DOMElement &el = *static_cast<xc::DOMElement *>(nodes->item(pos + skipped));
        if (XhtmlDoc::GetNodeName(el) != "#text") {
            return skipped;
        }
        QString el_text = XtoQ(el.getTextContent()).trimmed();
        if (el_text.length() > 0) {
            return skipped;
        }
        skipped++;
    }
    return skipped;
}

bool CleanContentUpdates::CanJoinParagraphs(xc::DOMElement &p1, xc::DOMElement &p2,
                                            bool only_not_formatted)
{
    QString p1_text = XtoQ(p1.getTextContent());
    QString p2_text = XtoQ(p2.getTextContent());

    QString p1_text_trimmed = p1_text.trimmed();
    QString p2_text_trimmed = p2_text.trimmed();

    QString last_char = p1_text_trimmed.right(1);
    QString first_char = p2_text_trimmed.left(1);

    xc::DOMNode* p1_last_child = p1.getLastChild();
    xc::DOMNode* p2_first_child = p2.getFirstChild();

    if (!p1_last_child || !p2_first_child) {
        return false;
    }

    if (only_not_formatted &&
        (XhtmlDoc::GetNodeName(*p1_last_child) != "#text" ||
         XhtmlDoc::GetNodeName(*p2_first_child) != "#text")) {
        return false;
    }

    if (!ContainsLetters(p1_text) ||
        !ContainsLetters(p2_text)) {
        return false;
    }

    if (last_char == "." || last_char == "?" ||
        last_char == "!") {
        return false;
    }

    return true;
}

void CleanContentUpdates::JoinParagraphs(xc::DOMDocument &doc, xc::DOMElement &p1, xc::DOMElement &p2)
{
    QString p1_text = XtoQ(p1.getTextContent());
    QString p2_text = XtoQ(p2.getTextContent());

    QString p1_text_trimmed = p1_text.trimmed();
    QString p2_text_trimmed = p2_text.trimmed();

    QString last_char = p1_text_trimmed.right(1);
    QString first_char = p2_text_trimmed.left(1);

    if ((last_char == "-" || last_char == "—") &&
        p1_text_trimmed.length() >= 2 &&
        p1_text_trimmed[p1_text_trimmed.length() - 2].isLetter()) {
        if (first_char.length() == 1 && first_char[0].isLower()) {
            RemoveLastChar(p1);
        }
    } else {
        p1.appendChild(doc.createTextNode(L" "));
    }

    MoveChildren(p1, p2);
    p2.getParentNode()->removeChild(&p2);
}

QString CleanContentUpdates::ConvertSamplePageNumberToRegExp(const QString &page_number_format)
{
    QString res = page_number_format;
    res.replace("[", "\\[");
    res.replace("]", "\\]");
    res.replace("{", "\\{");
    res.replace("}", "\\}");
    res.replace("(", "\\(");
    res.replace(")", "\\)");
    res.replace("|", "\\|");
    res.replace("^", "\\^");
    res.replace("$", "\\$");
    res.replace(".", "\\.");
    res.replace("?", "\\?");
    res.replace("*", "\\*");
    res.replace("+", "\\+");
    res.replace(QRegExp("\\d+"), "\\d+");
    return "^" + res + "$";
}

void CleanContentUpdates::RemoveLastChar(xc::DOMElement &element)
{
    xc::DOMNode* last_child = element.getLastChild();
    QString content = XtoQ(last_child->getTextContent());
    QString new_content = content.left(content.length() - 1);
    last_child->setTextContent(QtoX(new_content));
}

void CleanContentUpdates::MoveChildren(xc::DOMElement &elementDest, xc::DOMElement &elementSrc)
{
    xc::DOMNodeList *childNodes = elementSrc.getChildNodes();
    uint length = childNodes->getLength();
    for (uint i = 0; i < length; i++) {
        elementDest.appendChild(childNodes->item(0));
    }
}

bool CleanContentUpdates::ContainsLetters(const QString &str)
{
    return str.contains(QRegExp("[A-Za-z]"));
}
