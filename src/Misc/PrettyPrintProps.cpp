/************************************************************************
**
**  Copyright (C) 2026 Kevin B. Hendricks, Stratford, Ontario, Canada
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

#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <QFile>
#include <QDebug>

#include "sigil_constants.h"
#include "Misc/Utility.h"
#include "Parsers/QuickParser.h"
#include "Misc/PrettyPrintProps.h"

const QString DEFAULTXML =
  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
  "<prettyprint>\n"
  " \n"
  "  <!-- actual string added in front to indent one level, typically 2 or 4 blanks, quotes needed -->\n"
  "  <indent_string>\"  \"</indent_string>\n"
  "   \n"
  "  <doublespace>true</doublespace>\n"
  "   \n"
  "  <!-- set membership of tags determine when and where whitespace is compressed, and newlines added -->\n"
  "   \n"
  "  <!-- Structural tags open and close on their own lines with contents properly indented -->\n"
  "  <structural_tags>\n"
  "      annotation, annotation-xml, article, aside, blockquote,\n"
  "      body, canvas, colgroup, div, dl, figure, footer, head, header,\n"
  "      hr, html, maction, math, menclose, mfrac, mmultiscripts, mover,\n"
  "      mpadded, mphantom, mroot, mrow, msqrt, mstyle, mtable, mtd, mtr,\n"
  "      munder, munderover, nav, ol, section, semantics, table, tbody,\n"
  "      tfoot, thead, td, th, tr, ul\n"
  "  </structural_tags>\n"
  "   \n"
  "  <!-- Inline tags have no added space nor newlines -->\n"
  "  <inline_tags>\n"
  "      a, abbr, acronym, b, bdo, big, br, button, cite, code, del, dfn, em,\n"
  "      font, i, image, img, input, ins, kbd, label, map, mark, mbp:nu, mi,\n"
  "      mn, mo, ms, mspace, mtext, msub, msup, msubsup, nobr, object, q,\n"
  "      ruby, rp, rt, s, samp, select, small, span, strike, strong, sub,\n"
  "      sup, textarea, tt, u, var, wbr\n"
  "  </inline_tags>\n"
  "   \n"
  "  <!-- Void tags are tags that can not have contents and therefore must self-close -->\n"
  "  <void_tags>\n"
  "      area, base, basefont, bgsound, br, col, command, embed, event-source,\n"
  "      frame, hr, img, input, keygen, link, maligngroup, malignmark,\n"
  "      mbp:pagebreak, meta, mglyph, mprescripts, msline, mspace, none,\n"
  "      param, source, spacer, track, wbr\n"
  "  </void_tags>\n"
  "   \n"
  "  <!-- Preserve Space tags are tags whose content's whitespace can not be changed or compressed -->\n"
  "  <preservespace_tags>\n"
  "      code, cs, pre, textarea, script, style\n"
  "  </preservespace_tags>\n"
  "   \n"
  "  <!-- No Entity Substituion tags are those whose contents should not have any entities expanded -->\n"
  "  <noentitysub_tags>\n"
  "      script, style\n"
  "  </noentitysub_tags>\n"
  "   \n"
  "  <!-- Text holder tags are tags that typically have text content -->\n"
  "  <textholder_tags>\n"
  "      address, caption, dd, div, dt, figcaption, h1, h2, h3, h4, h5, h6,\n"
  "      legend, li, option, p, td, th, title\n"
  "  </textholder_tags>\n"
  " \n"
  "</prettyprint>\n";

const QStringList TAGS_TO_PARSE = QStringList() << "indent_string" << "doublespace" <<
                                  "structural_tags" << "inline_tags" << "void_tags" <<
                                  "preservespace_tags" << "noentitysub_tags" << "textholder_tags";

const QStringList BOOL_TRUE = QStringList() << "true" << "on" << "1" << "yes";

PrettyPrintProps *PrettyPrintProps::m_instance = 0;


PrettyPrintProps *PrettyPrintProps::instance()
{
    if (m_instance == 0) {
        m_instance = new PrettyPrintProps();
    }

    return m_instance;
}

void PrettyPrintProps::ParsePrettyPrintXml()
{
    QString pp_path = Utility::DefinePrefsDir() + "/prettyprint.xml";
    if (!QFile::exists(pp_path)) {
        Utility::WriteUnicodeTextFile(DEFAULTXML, pp_path, false);
    }
    QString xmldata = Utility::ReadUnicodeTextFile(pp_path, false);
    if (xmldata.isEmpty()) xmldata = DEFAULTXML;
    QuickParser qp(xmldata);
    QString pstate;
    bool get_text = false;
    QString data;
    while(true) {
        QuickParser::MarkupInfo mi = qp.parse_next();
        if (mi.pos < 0) break;
	if (!mi.text.isEmpty() && get_text) {
	    data = data + mi.text;
	}
        if (mi.text.isEmpty()) {
	    if (mi.ttype == "begin") {
	        if (TAGS_TO_PARSE.contains(mi.tname)) {
		    pstate = mi.tname;
		    get_text = true;
		}
	    } else if (mi.ttype == "end") {
	        if (TAGS_TO_PARSE.contains(mi.tname)) {
		    get_text = false;
		    // parse collected text and assign it to its set
		    data = data.trimmed();
		    if (pstate == "indent_string") {
		        std::string ind = data.toStdString();
		        // remove any leading or trailing quotes
		        if (!ind.empty() && (ind.front() == '"' || ind.front() == '\'')) ind.erase(0,1);
		        if (!ind.empty() && (ind.back() == '"' || ind.back() == '\'')) ind.pop_back();
		        m_indent_string = ind;
		    } else if (pstate == "doublespace") {
		        m_doublespace = BOOL_TRUE.contains(data.toLower());
		    } else {
		        foreach(QString tag, data.split(",")) {
		            tag = tag.simplified();
		            if (pstate == "structural_tags") m_structural.insert(tag.toStdString());
		            if (pstate == "inline_tags") m_inline.insert(tag.toStdString());
		            if (pstate == "void_tags") m_void.insert(tag.toStdString());
		            if (pstate == "preservespace_tags") m_preservespace.insert(tag.toStdString());
		      	    if (pstate == "noentiyysub_tags") m_noentitysub.insert(tag.toStdString());
		            if (pstate == "textholder_tags") m_textholder.insert(tag.toStdString());
			}
		    }
		    pstate = "";
		    data = "";
	        }
	    }
	}
    }
}

PrettyPrintProps::PrettyPrintProps()
{
    ParsePrettyPrintXml();
}

bool PrettyPrintProps::inset_structural(const std::string &s) const
{
    return m_structural.find(s) != m_structural.end();
}


bool PrettyPrintProps::inset_inline(const std::string &s) const
{
    return m_inline.find(s) != m_inline.end();
}

bool PrettyPrintProps::inset_preservespace(const std::string &s) const
{
    return m_preservespace.find(s) != m_preservespace.end();
}

bool PrettyPrintProps::inset_noentitysub(const std::string &s) const
{
    return m_noentitysub.find(s) != m_noentitysub.end();
}

bool PrettyPrintProps::inset_void(const std::string &s) const
{
    return m_void.find(s) != m_void.end();
}

bool PrettyPrintProps::inset_textholder(const std::string &s) const
{
    return m_textholder.find(s) != m_textholder.end();
}
