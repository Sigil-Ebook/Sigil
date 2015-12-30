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
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n"
    "    \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n"
    "\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
    "<head>\n"
    "<title>Index</title>\n"
    "<link href=\"../Styles/" % SGC_INDEX_CSS_FILENAME % "\" rel=\"stylesheet\" type=\"text/css\" />\n"
    "</head>\n"
    "<body>\n";

static const QString TEMPLATE3_BEGIN_TEXT =
    "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"no\"?>\n"
    "<!DOCTYPE html>\n\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" xmlns:epub=\"http://www.idpf.org/2007/ops\">\n"
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

QString IndexHTMLWriter::WriteXML(const QString &version)
{
    if (version.startsWith('2')) {
        m_IndexHTMLFile += TEMPLATE_BEGIN_TEXT;
    } else {
        m_IndexHTMLFile += TEMPLATE3_BEGIN_TEXT;
    }
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
    for (int i = 0; i < parent_item->rowCount(); i++) {
        QStandardItem *item = parent_item->child(i, 0);

        // If this is a target entry then skip.
        if (!item->rowCount()) {
            continue;
        }

        // Need to html escape entry text
        // If the first letter of this entry is different than the last
        // entry then insert a special separator.
        // Get actual first letter not htmlescaped
        QChar new_letter = item->text()[0].toLower();
        if (new_letter != letter && parent_item == root_item) {
            letter = new_letter;
            m_IndexHTMLFile += "<div class=\"sgc-index-new-letter\">";
            // starting letter may be an & or > or < - therefore html escape it
            m_IndexHTMLFile += QString(letter.toUpper()).toHtmlEscaped();
            m_IndexHTMLFile += "</div>";
        }

        m_IndexHTMLFile += "<div class=\"sgc-index-entry\">";
        // make sure to use the html escaped text here for entry
        QString etext = item->text().toHtmlEscaped();
        m_IndexHTMLFile += etext % "\n";
        m_IndexHTMLFile += " ";

        // Print all the targets for this entry
        int ref_count = 1;
        for (int j = 0; j < item->rowCount(); j++) {
            // If the entry has no children then its a target id.
            if (item->child(j, 0)->rowCount() == 0) {
                QString target = "../Text/" % item->child(j, 0)->text();
                if (ref_count > 1) {
                    m_IndexHTMLFile += ", ";
                }
                m_IndexHTMLFile += "<a href=\"" % target % "\">" % QString::number(ref_count) % "</a>";
                ref_count++;

            }
        }

        // Print any subentries and their targets
        WriteEntries(item);
        m_IndexHTMLFile += "</div>";
    }
}
