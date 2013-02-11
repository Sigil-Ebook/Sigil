/************************************************************************
**
**  Copyright (C) 2012 Dave Heiland
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
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

#include "MiscEditors/IndexHTMLWriter.h"
#include "MiscEditors/IndexEntries.h"
#include "sigil_constants.h"

const QString SGC_INDEX_CSS_FILENAME = "sgc-index.css";

static const QString TEMPLATE_BEGIN_TEXT =
    "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"no\"?>\n"
    "\n"
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n"
    "    \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n"
    "\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
    "<head>\n"
    "<title>Index</title>\n"
    "<link href=\"../Styles/" % SGC_INDEX_CSS_FILENAME % "\" rel=\"stylesheet\" type=\"text/css\" />\n" 
    "</head>\n"
    "<body>\n";

static const QString TEMPLATE_END_TEXT =
    "</body>\n"
    "</html>\n";


IndexHTMLWriter::IndexHTMLWriter()
    :
    m_IndexHTMLFile(QString())
{
}

QString IndexHTMLWriter::WriteXML()
{
    m_IndexHTMLFile += TEMPLATE_BEGIN_TEXT;
    m_IndexHTMLFile += "<div class=\"sgc-index-title\">";
    m_IndexHTMLFile += QObject::tr("Index");
    m_IndexHTMLFile += "</div>\n";
    m_IndexHTMLFile += "<div class=\"sgc-index-body\">";
    WriteEntries();
    m_IndexHTMLFile += "</div>";
    m_IndexHTMLFile += TEMPLATE_END_TEXT;
    return m_IndexHTMLFile;
}

void IndexHTMLWriter::WriteEntries(QStandardItem *parent_item)
{
    QStandardItem *root_item = IndexEntries::instance()->GetRootItem();

    if (!parent_item) {
        parent_item = root_item;
    }

    if (!parent_item->rowCount()) {
        return;
    }

    QChar letter = ' ';

    if (parent_item->child(0, 0)->rowCount()) {
        // Print Index groups/entries
        for (int i = 0; i < parent_item->rowCount(); i++) {
            QString new_letter_text = "";
            // Space between top level entries if first letter changes
            QChar new_letter = parent_item->child(i, 0)->text()[0].toLower();

            if (new_letter != letter && parent_item == root_item) {
                letter = new_letter;
                new_letter_text = " sgc-index-new-letter";
            }

            if (parent_item->child(i, 0)->rowCount()) {
                if (parent_item->child(i, 0)->child(0, 0)->rowCount()) {
                    m_IndexHTMLFile += "<div class=\"sgc-index-key" % new_letter_text % "\">";
                } else {
                    m_IndexHTMLFile += "<div class=\"sgc-index-entry" % new_letter_text % "\">";
                }

                m_IndexHTMLFile += parent_item->child(i, 0)->text() % "\n";
                WriteEntries(parent_item->child(i, 0));
                m_IndexHTMLFile += "</div>";
            }
        }
    } else {
        // Print links
        m_IndexHTMLFile += " ";

        for (int i = 0; i < parent_item->rowCount(); i++) {
            QString target = "../Text/" % parent_item->child(i, 0)->text();
            m_IndexHTMLFile += "<a href=\"" % target % "\">" % QString::number(i + 1) % "</a>";

            if (i < parent_item->rowCount() - 1) {
                m_IndexHTMLFile += ", ";
            }
        }
    }
}
