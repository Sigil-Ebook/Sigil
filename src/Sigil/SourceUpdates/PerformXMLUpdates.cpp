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

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QStringList>

#include "BookManipulation/XercesCppUse.h"
#include "BookManipulation/XhtmlDoc.h"
#include "sigil_exception.h"
#include "Misc/Utility.h"
#include "SourceUpdates/PerformXMLUpdates.h"

static const QChar POUND_SIGN    = QChar::fromLatin1('#');
static const QChar FORWARD_SLASH = QChar::fromLatin1('/');
static const QString DOT         = ".";
static const QString DOT_DOT     = "..";

PerformXMLUpdates::PerformXMLUpdates(const QString &source,
                                     const QHash<QString, QString> &xml_updates)
    :
    m_XMLUpdates(xml_updates)
{
    m_Document = XhtmlDoc::LoadTextIntoDocument(source);
    InitPathAttributes();
}


PerformXMLUpdates::PerformXMLUpdates(const xc::DOMDocument &document,
                                     const QHash<QString, QString> &xml_updates)
    :
    m_XMLUpdates(xml_updates)
{
    m_Document = XhtmlDoc::CopyDomDocument(document);
    InitPathAttributes();
}


shared_ptr<xc::DOMDocument> PerformXMLUpdates::operator()()
{
    UpdateXMLReferences();
    return m_Document;
}


void PerformXMLUpdates::UpdateXMLReferences()
{
    xc::DOMElement *document_element = m_Document->getDocumentElement();

    if (!document_element) {
        boost_throw(ErrorBuildingDOM());
    }

    QList<xc::DOMElement *> nodes = XhtmlDoc::GetTagMatchingDescendants(*document_element, m_PathTags);
    int node_count = nodes.count();

    for (int i = 0; i < node_count; ++i) {
        UpdateReferenceInNode(nodes.at(i));
    }
}


// This function has been brutally optimized since it is the main
// bottleneck during loading (well, not anymore :) ).
// Be vewy, vewy careful when editing it.
void PerformXMLUpdates::UpdateReferenceInNode(xc::DOMElement *node)
{
    xc::DOMNamedNodeMap &attributes = *node->getAttributes();
    int num_attributes = attributes.getLength();
    const QList<QString> &keys = m_XMLUpdates.keys();
    int num_keys = keys.count();

    for (int i = 0; i < num_attributes; ++i) {
        xc::DOMAttr &attribute = *static_cast<xc::DOMAttr *>(attributes.item(i));
        Q_ASSERT(&attribute);

        if (!m_PathAttributes.contains(XhtmlDoc::GetAttributeName(attribute), Qt::CaseInsensitive)) {
            continue;
        }

        for (int j = 0; j < num_keys; ++j) {
            const QString &key_path        = keys.at(j);
            const QString &filename        = QFileInfo(key_path).fileName();
            const QString &atrribute_value = Utility::URLDecodePath(XtoQ(attribute.getValue()));
            int name_index = atrribute_value.lastIndexOf(filename);

            if (name_index == -1) {
                continue;
            }

            int filename_length  = filename.length();
            int name_end_index   = name_index + filename_length;
            bool has_fragment_id = name_end_index < atrribute_value.length() &&
                                   atrribute_value.at(name_end_index) == POUND_SIGN;
            // The left() call returns the part of the string before the
            // fragment ID, if any.
            const QString &attribute_path_dir_name =
                !has_fragment_id                                                   ?
                QFileInfo(atrribute_value).dir().dirName()                       :
                QFileInfo(atrribute_value.left(name_end_index)).dir().dirName();
            const QString &old_path_dir_name = QFileInfo(key_path).dir().dirName();

            // We need to make sure that files that have the same name,
            // but a different parent directory still compare differently.
            // This still isn't perfect since they could differ in the
            // grandfather directory, but comparing absolute values would
            // mean querying the filesystem and that would kill performance.
            // This is good enough (famous last words etc...).
            if (!attribute_path_dir_name.isEmpty()           &&
                attribute_path_dir_name != DOT               &&
                attribute_path_dir_name != DOT_DOT           &&
                attribute_path_dir_name != old_path_dir_name) {
                continue;
            }

            int atr_value_length = atrribute_value.length();
            QString new_path;

            // First we look at whether the filename matches the attribute value,
            // and then we determine whether it's actually a path that ends with the filename
            if (filename_length == atr_value_length ||
                (name_end_index == atr_value_length &&
                 atrribute_value.at(name_index - 1) == FORWARD_SLASH
                )
               ) {
                new_path = m_XMLUpdates.value(key_path);
            } else if (has_fragment_id &&
                       (name_index == 0 ||
                        atrribute_value.at(name_index - 1) == FORWARD_SLASH
                       )
                      ) {
                new_path = atrribute_value.mid(name_end_index).prepend(m_XMLUpdates.value(key_path));
            }

            if (!new_path.isEmpty()) {
                attribute.setValue(QtoX(Utility::URLEncodePath(new_path)));
                // We assign to "i" to break the outer loop
                i = num_attributes;
                break;
            }
        }
    }
}


void PerformXMLUpdates::InitPathAttributes()
{
    m_PathAttributes << "href" << "src";
}

