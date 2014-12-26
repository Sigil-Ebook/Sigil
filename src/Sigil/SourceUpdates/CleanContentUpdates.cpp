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

    xc::DOMNodeList *paragraphs = body_element.getElementsByTagName(QtoX("p"));

    // change in reverse to preserve location information
    int paragraph_count = paragraphs->getLength();
    for (int i = paragraph_count - 2; i >= 0; i--) {
        xc::DOMElement &element = *static_cast<xc::DOMElement *>(paragraphs->item(i));
        xc::DOMElement &element_next = *static_cast<xc::DOMElement *>(paragraphs->item(i + 1));

        QString element_text = XtoQ(element.getTextContent());
        QString element_next_text = XtoQ(element_next.getTextContent());

        QString element_text_trimmed = element_text.trimmed();
        QString element_next_text_trimmed = element_next_text.trimmed();

        QString last_char = element_text_trimmed.right(1);
        QString first_char = element_next_text_trimmed.left(1);

        xc::DOMNode* element_last_child = element.getLastChild();
        xc::DOMNode* element_next_first_child = element_next.getFirstChild();

        if (only_not_formatted &&
            (XhtmlDoc::GetNodeName(*element_last_child) != "#text" ||
             XhtmlDoc::GetNodeName(*element_next_first_child) != "#text")) {
            continue;
        }

        if (!ContainsLetters(element_text) ||
            !ContainsLetters(element_next_text)) {
            continue;
        }

        if (last_char == "." || last_char == "?" ||
            last_char == "!") {
            continue;
        }

        // join paragraphs

        if ((last_char == "-" || last_char == "—") &&
            element_text_trimmed.length() >= 2 &&
            element_text_trimmed[element_text_trimmed.length() - 2].isLetter()) {
            if (first_char.length() == 1 && first_char[0].isLower()) {
                RemoveLastChar(element);
            }
        } else {
            element.appendChild(doc.createTextNode(L" "));
        }

        MoveChildren(element, element_next);
        body_element.removeChild(&element_next);
        number_of_changes++;
    }

    return number_of_changes;
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
    element.setTextContent(QtoX(new_content));
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
