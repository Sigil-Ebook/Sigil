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

#include "BookManipulation/XercesCppUse.h"
#include "BookManipulation/XhtmlDoc.h"
#include "ResourceObjects/HTMLResource.h"
#include "Misc/Utility.h"
#include "sigil_constants.h"
#include "SourceUpdates/CleanContentUpdates.h"

using boost::shared_ptr;

void CleanContentUpdates::CleanContentInAllFiles(const QList<HTMLResource *> &html_resources,
                                                 const CleanContentParams &params)
{
    QtConcurrent::blockingMap(html_resources,
                              boost::bind(CleanContentInOneFile, _1, params));
}

void CleanContentUpdates::CleanContentInOneFile(HTMLResource *html_resource,
                                                const CleanContentParams &params)
{
    Q_ASSERT(html_resource);
    QWriteLocker locker(&html_resource->GetLock());
    shared_ptr<xc::DOMDocument> d = XhtmlDoc::LoadTextIntoDocument(html_resource->GetText());
    xc::DOMDocument &doc = *d.get();

    if (params.remove_page_numbers) {
        RemovePageNumbers(doc, params.page_number_format);
    }

    if (params.remove_empty_paragraphs) {
        RemoveEmptyParagraphs(doc);
    }

    html_resource->SetText(XhtmlDoc::GetDomDocumentAsString(doc));
}

void CleanContentUpdates::RemovePageNumbers(xc::DOMDocument &doc, const QString &page_number_format)
{
    QRegularExpression re(ConvertSamplePageNumberToRegExp(page_number_format));

    // body should only appear once
    xc::DOMNodeList *bodys = doc.getElementsByTagName(QtoX("body"));
    xc::DOMElement &body_element = *static_cast<xc::DOMElement *>(bodys->item(0));

    xc::DOMNodeList *paragraphs = body_element.getElementsByTagName(QtoX("p"));

    // change in reverse to preserve location information
    uint paragraph_count = paragraphs->getLength();
    for (uint i = paragraph_count; i > 0; i--) {
        // always delete the top element since list is dynamic
        xc::DOMElement &element = *static_cast<xc::DOMElement *>(paragraphs->item(i - 1));
        Q_ASSERT(&element);

        QString element_text = XtoQ(element.getTextContent());
        if (re.match(element_text).hasMatch()) {
            body_element.removeChild(&element);
        }
    }
}

void CleanContentUpdates::RemoveEmptyParagraphs(xc::DOMDocument &doc)
{
    // body should only appear once
    xc::DOMNodeList *bodys = doc.getElementsByTagName(QtoX("body"));
    xc::DOMElement &body_element = *static_cast<xc::DOMElement *>(bodys->item(0));

    xc::DOMNodeList *paragraphs = body_element.getElementsByTagName(QtoX("p"));

    // change in reverse to preserve location information
    uint paragraph_count = paragraphs->getLength();
    for (uint i = paragraph_count; i > 0; i--) {
        // always delete the top element since list is dynamic
        xc::DOMElement &element = *static_cast<xc::DOMElement *>(paragraphs->item(i - 1));
        Q_ASSERT(&element);

        QString element_text = XtoQ(element.getTextContent());
        element_text = element_text.trimmed();
        if (element_text.length() == 0) {
            body_element.removeChild(&element);
        }
    }
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
