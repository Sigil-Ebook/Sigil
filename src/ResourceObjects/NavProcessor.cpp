/************************************************************************
**
**  Copyright (C) 2016-2019, Kevin B. Hendricks, Stratford Ontario
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
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDir>
#include <QUrl>
#include <QFileInfo>

#include "Misc/Utility.h"
#include "Misc/SettingsStore.h"
#include "Misc/GumboInterface.h"
#include "Misc/Landmarks.h"
#include "BookManipulation/FolderKeeper.h"
#include "ResourceObjects/Resource.h"
#include "ResourceObjects/NavProcessor.h"

static const QString NAV_PAGELIST_PATTERN = "\\s*<!--\\s*SIGIL_REPLACE_PAGELIST_HERE\\s*-->\\s*";
static const QString NAV_LANDMARKS_PATTERN = "\\s*<!--\\s*SIGIL_REPLACE_LANDMARKS_HERE\\s*-->\\s*";
static const QString NAV_TOC_PATTERN = "\\s*<!--\\s*SIGIL_REPLACE_TOC_HERE\\s*-->\\s*";

static const QStringList SIGIL_FOLDERS = QStringList() << "Images" << "Fonts" << "Text" << "Styles" 
                                                       << "Audio" << "Video" << "Misc";


NavProcessor::NavProcessor(HTMLResource * nav_resource)
  : m_NavResource(nav_resource)
{
    QReadLocker locker(&m_NavResource->GetLock());
    QString source = m_NavResource->GetText();
    if (source.isEmpty()) {
          SettingsStore ss;
          QString lang = ss.defaultMetadataLang();
          QString newsource = 
            "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
            "<!DOCTYPE html>\n"
            "<html xmlns=\"http://www.w3.org/1999/xhtml\" xmlns:epub=\"http://www.idpf.org/2007/ops\" "
            "lang=\"%1\" xml:lang=\"%2\">\n"
            "<head>\n"
            "  <meta charset=\"utf-8\" />\n"
            "  <style type=\"text/css\">\n"
            "    nav#landmarks, nav#page-list { display:none; }\n"
            "    ol { list-style-type: none; }\n"
            "  </style>\n"
            "</head>\n"
            "<body epub:type=\"frontmatter\">\n"
            "  <nav epub:type=\"toc\" id=\"toc\">\n"
            "  </nav>\n"
            "  <nav epub:type=\"landmarks\" id=\"landmarks\" hidden=\"\">\n"
            "  </nav>\n"
            "</body>\n"
            "</html>";
          newsource = newsource.arg(lang).arg(lang);
          QWriteLocker locker(&m_NavResource->GetLock());
          m_NavResource->SetText(newsource);
    }
}


NavProcessor::~NavProcessor()
{
}


QList<NavLandmarkEntry> NavProcessor::GetLandmarks()
{
    QList<NavLandmarkEntry> landlist;
    if (!m_NavResource) return landlist; 

    QReadLocker locker(&m_NavResource->GetLock());
    QString source = m_NavResource->GetText();

    // user may leave nav in unparseable state so use
    // regular expressions to try and extract just the landmarks code only from main nav

    QRegularExpression landmarks_start("(<\\s*nav\\s[^>]*epub:type[^>]*[\"']landmarks[\"'][^>]*>)",QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch mo = landmarks_start.match(source, 0);
    if (mo.hasMatch()) {
        int end = source.indexOf("</nav>", mo.capturedStart());
        if (end > 0) {
            source = source.mid(mo.capturedStart(), end - mo.capturedStart()+ 6);
        }
    }

    GumboInterface gi = GumboInterface(source, "3.0");
    gi.parse();
    const QList<GumboNode*> nav_nodes = gi.get_all_nodes_with_tag(GUMBO_TAG_NAV);
    for (int i = 0; i < nav_nodes.length(); ++i) {
        GumboNode* node = nav_nodes.at(i);
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "epub:type");
        if (attr && (QString::fromUtf8(attr->value) == "landmarks")) {
            const QList<GumboTag> tags = QList<GumboTag>() << GUMBO_TAG_A;;
            const QList<GumboNode*> anchor_nodes = gi.get_nodes_with_tags(node, tags);
            for (int j = 0; j < anchor_nodes.length(); ++j) {
                NavLandmarkEntry le;
                GumboNode* ancnode = anchor_nodes.at(j);
                GumboAttribute* typeattr = gumbo_get_attribute(&ancnode->v.element.attributes, "epub:type");
                GumboAttribute* hrefattr = gumbo_get_attribute(&ancnode->v.element.attributes, "href");
                if (typeattr) le.etype = QString::fromUtf8(typeattr->value);
                if (hrefattr) le.href = Utility::URLDecodePath(QString::fromUtf8(hrefattr->value));
                le.title = Utility::DecodeXML(gi.get_local_text_of_node(ancnode));
                landlist.append(le);
            }
            break;
        }
    }
    return landlist;
}


QList<NavPageListEntry> NavProcessor::GetPageList()
{
    QList<NavPageListEntry> pagelist;
    if (!m_NavResource) return pagelist; 
        
    QReadLocker locker(&m_NavResource->GetLock());
    QString source = m_NavResource->GetText();

    // user may leave nav in unparseable state so use
    // regular expressions to try and extract just the page-list code only from main nav

    QRegularExpression pagelist_start("(<\\s*nav\\s[^>]*epub:type[^>]*[\"']page-list[\"'][^>]*>)",QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch mo = pagelist_start.match(source, 0);
    if (mo.hasMatch()) {
        int end = source.indexOf("</nav>", mo.capturedStart());
        if (end > 0) {
            source = source.mid(mo.capturedStart(), end - mo.capturedStart()+ 6);
        }
    }

    GumboInterface gi = GumboInterface(source, "3.0");
    gi.parse();
    const QList<GumboNode*> nav_nodes = gi.get_all_nodes_with_tag(GUMBO_TAG_NAV);
    for (int i = 0; i < nav_nodes.length(); ++i) {
        GumboNode* node = nav_nodes.at(i);
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "epub:type");
        if (attr && (QString::fromUtf8(attr->value) == "page-list")) {
            QList<GumboTag> tags = QList<GumboTag>() << GUMBO_TAG_A;
            const QList<GumboNode*> anchor_nodes = gi.get_nodes_with_tags(node, tags);
            for (int j = 0; j < anchor_nodes.length(); ++j) {
                NavPageListEntry pe;
                GumboNode* ancnode = anchor_nodes.at(j);
                GumboAttribute* hrefattr = gumbo_get_attribute(&ancnode->v.element.attributes, "href");
                if (hrefattr) pe.href = Utility::URLDecodePath(QString::fromUtf8(hrefattr->value));
                pe.pagename = Utility::DecodeXML(gi.get_local_text_of_node(ancnode));
                pagelist.append(pe);
            }
            break;
        }
    }
    return pagelist;
}


QList<NavTOCEntry> NavProcessor::GetTOC()
{
    QList<NavTOCEntry> toclist;
    if (!m_NavResource) return toclist; 
        
    QReadLocker locker(&m_NavResource->GetLock());
    QString source = m_NavResource->GetText();

    // user may leave nav in unparseable state so use
    // regular expressions to try and extract just the toc code only from main nav

    QRegularExpression toc_start("(<\\s*nav\\s[^>]*epub:type[^>]*[\"']toc[\"'][^>]*>)",QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch mo = toc_start.match(source, 0);
    if (mo.hasMatch()) {
        int end = source.indexOf("</nav>", mo.capturedStart());
        if (end > 0) {
            source = source.mid(mo.capturedStart(), end - mo.capturedStart()+ 6);
        }
    }

    GumboInterface gi = GumboInterface(source, "3.0");
    gi.parse();
    const QList<GumboNode*> nav_nodes = gi.get_all_nodes_with_tag(GUMBO_TAG_NAV);
    for (int i = 0; i < nav_nodes.length(); ++i) {
        GumboNode* node = nav_nodes.at(i);
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "epub:type");
        if (attr && (QString::fromUtf8(attr->value) == "toc")) {
            QList<GumboTag> tags = QList<GumboTag>() << GUMBO_TAG_OL;
            const QList<GumboNode*> ol_nodes = gi.get_nodes_with_tags(node, tags);
            for (int j = 0; j < ol_nodes.length(); ++j) {
                GumboNode * olnode = ol_nodes.at(j);
                toclist.append(GetNodeTOC(gi, olnode, 1));
                break;
            }
            break;              
        }
    }
    return toclist;
}


QList<NavTOCEntry> NavProcessor::GetNodeTOC(GumboInterface & gi, const GumboNode * node, int lvl)
{
    if ((node->type != GUMBO_NODE_ELEMENT) || (node->v.element.tag != GUMBO_TAG_OL)) {
        return QList<NavTOCEntry>();
    }
  
    QList<NavTOCEntry> toclist;
    const GumboVector* children = &node->v.element.children;

    for (unsigned int i = 0; i < children->length; ++i) {
        GumboNode * child = static_cast<GumboNode*>(children->data[i]);
        if (child->type == GUMBO_NODE_ELEMENT) {
            if (child->v.element.tag == GUMBO_TAG_LI) {
                const GumboVector* li_children = &child->v.element.children;
                for (unsigned int j = 0; j < li_children->length; ++j) {
                    GumboNode * li_child = static_cast<GumboNode*>(li_children->data[j]);
                    if (li_child->type == GUMBO_NODE_ELEMENT) {
                        if (li_child->v.element.tag == GUMBO_TAG_A) {
                            NavTOCEntry te;
                            te.lvl = lvl;
                            GumboAttribute* hrefattr = gumbo_get_attribute(&li_child->v.element.attributes, "href");
                            if (hrefattr) te.href = Utility::URLDecodePath(QString::fromUtf8(hrefattr->value));
                            te.title = Utility::DecodeXML(gi.get_local_text_of_node(li_child));
                            toclist.append(te);
                        } else if (li_child->v.element.tag == GUMBO_TAG_OL) {
                            toclist.append(GetNodeTOC(gi, li_child, lvl+1));
                        }
                    }
                }
            }  
        }
    } 
    return toclist;             
}


QString NavProcessor::BuildTOC(const QList<NavTOCEntry> & toclist)
{
    QStringList res;
    int curlvl = 1;
    bool initial = true;
    QString step = "  ";
    QString base = step.repeated(2);
    res << "\n" + step + "<nav epub:type=\"toc\" id=\"toc\">\n";
    res << base + "<h1>" + Landmarks::instance()->GetName("toc") + "</h1>\n";
    res << base + "<ol>\n";
    foreach(NavTOCEntry te, toclist) {
        int lvl = te.lvl;
        QString href = Utility::URLEncodePath(te.href);
        QString title = Utility::EncodeXML(te.title);
        if (lvl > curlvl) {
            while(lvl > curlvl) {
                QString indent = base + step.repeated(curlvl);
                res << indent + "<ol>\n";
                res << indent + step + "<li>\n";
                res << indent + step.repeated(2) + "<a href=\"" + href + "\">" + title + "</a>\n";
                curlvl++;
            }
        } else if (lvl < curlvl) {
            while(lvl < curlvl) {
                QString indent = base + step.repeated(curlvl-1);
                res << indent + step + "</li>\n";
                res << indent + "</ol>\n";
                curlvl--;
            }
            QString indent = base + step.repeated(lvl-1);
            res << indent + step  + "</li>\n";
            res << indent + step + "<li>\n";
            res << indent + step.repeated(2) + "<a href=\"" + href + "\">" + title + "</a>\n";              
        } else {
            QString indent = base + step.repeated(lvl-1);
            if (!initial) {
                res << indent + step  + "</li>\n";
            }
            res << indent + step + "<li>\n";
            res << indent + step.repeated(2) + "<a href=\"" + href + "\">" + title + "</a>\n";              
        }
        initial = false;
        curlvl = lvl;
    }
    while(curlvl > 0) {
        QString indent = base + step.repeated(curlvl-1);
        if (!initial) res << indent + step + "</li>\n";
        res << indent + "</ol>\n";
        --curlvl;
    }
    res << step + "</nav>\n";
    return res.join("");
}


QString NavProcessor::BuildLandmarks(const QList<NavLandmarkEntry> & landlist)
{
    QStringList res;
    QString step = "  ";
    QString base = step.repeated(2);
    res << "\n" + step + "<nav epub:type=\"landmarks\" id=\"landmarks\" hidden=\"\">\n";
    res << base + "<h1>" + Landmarks::instance()->GetName("landmarks") + "</h1>\n";
    res << base + "<ol>\n";
    foreach(NavLandmarkEntry le, landlist) {
        QString etype = le.etype;
        QString href = Utility::URLEncodePath(le.href);
        QString title = Utility::EncodeXML(le.title);
        res << base + step + "<li>\n";
        res << base + step.repeated(2) + "<a epub:type=\"" + etype + "\" href=\"" + href + "\">" + title + "</a>\n";
        res << base + step + "</li>\n";
    }
    res << base + "</ol>\n";
    res << step + "</nav>\n";
    return res.join("");
}


QString NavProcessor::BuildPageList(const QList<NavPageListEntry> & pagelist)
{
    QStringList res;
    QString step = "  ";
    QString base = step.repeated(3);
    res << "\n" + step + "<nav epub:type=\"page-list\" id=\"page-list\" hidden=\"\">\n";
    res << base + "<h1>" + Landmarks::instance()->GetName("page-list") + "</h1>\n";
    res << "\n" + base + "<ol>\n";
    foreach(NavPageListEntry pe, pagelist) {
        QString pagename = Utility::EncodeXML(pe.pagename);
        QString href = Utility::URLEncodePath(pe.href);
        res << base + step + "<li><a href=\"" + href + "\">" + pagename + "</a></li>\n";
    }
    res << base + "</ol>\n";
    res << step + "</nav>\n";
    return res.join("");
}


void NavProcessor::SetPageList(const QList<NavPageListEntry> & pagelist)
{
    if (!m_NavResource) return; 
        
    bool found_pagelist = false;
    // QWriteLocker locker(&m_NavResource->GetLock());
    GumboInterface gi = GumboInterface(m_NavResource->GetText(), "3.0");
    gi.parse();
    QList<GumboNode*> nav_nodes = gi.get_all_nodes_with_tag(GUMBO_TAG_NAV);
    for (int i = 0; i < nav_nodes.length(); ++i) {
        GumboNode* node = nav_nodes.at(i);
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "epub:type");
        if (attr && (QString::fromUtf8(attr->value) == "page-list")) {
            found_pagelist = true;
            GumboNode* parent = node->parent;
            unsigned int index_within_parent = node->index_within_parent;
            gumbo_remove_from_parent(node);
            gumbo_destroy_node(node);
            GumboNode * placeholder = gumbo_create_text_node(GUMBO_NODE_COMMENT,"SIGIL_REPLACE_PAGELIST_HERE");
            gumbo_insert_node(placeholder, parent, index_within_parent);
            break;
        }
    }
    if (!found_pagelist) {
        QList<GumboNode*> body_nodes = gi.get_all_nodes_with_tag(GUMBO_TAG_BODY);
        if (body_nodes.length() == 1) {
            GumboNode* body = body_nodes.at(0);
            GumboNode * placeholder = gumbo_create_text_node(GUMBO_NODE_COMMENT,"SIGIL_REPLACE_PAGELIST_HERE");
            gumbo_append_node(body, placeholder);
        }
    }
    QString nav_data = gi.getxhtml();
    QString page_xml = BuildPageList(pagelist);
    QRegularExpression pagelist_placeholder(NAV_PAGELIST_PATTERN, 
                       QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch mo = pagelist_placeholder.match(nav_data);
    if (mo.hasMatch()) {
        nav_data.replace(mo.capturedStart(), mo.capturedLength(), page_xml);
    }
    m_NavResource->SetText(nav_data);
}


void NavProcessor::SetLandmarks(const QList<NavLandmarkEntry> & landlist)
{
    if (!m_NavResource) return; 

    bool found_landmarks = false;
    // QWriteLocker locker(&m_NavResource->GetLock());
    GumboInterface gi = GumboInterface(m_NavResource->GetText(), "3.0");
    gi.parse();
    const QList<GumboNode*> nav_nodes = gi.get_all_nodes_with_tag(GUMBO_TAG_NAV);
    for (int i = 0; i < nav_nodes.length(); ++i) {
        GumboNode* node = nav_nodes.at(i);
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "epub:type");
        if (attr && (QString::fromUtf8(attr->value) == "landmarks")) {
            found_landmarks = true;
            GumboNode * parent = node->parent;
            unsigned int index_within_parent = node->index_within_parent;
            gumbo_remove_from_parent(node);
            gumbo_destroy_node(node);
            GumboNode * placeholder = gumbo_create_text_node(GUMBO_NODE_COMMENT,"SIGIL_REPLACE_LANDMARKS_HERE");
            gumbo_insert_node(placeholder, parent, index_within_parent);
            break;
        }
    }
    if (!found_landmarks) {
        QList<GumboNode*> body_nodes = gi.get_all_nodes_with_tag(GUMBO_TAG_BODY);
        if (body_nodes.length() == 1) {
            GumboNode* body = body_nodes.at(0);
            GumboNode * placeholder = gumbo_create_text_node(GUMBO_NODE_COMMENT,"SIGIL_REPLACE_LANDMARKS_HERE");
            gumbo_append_node(body, placeholder);
        }
    }
    QString nav_data = gi.getxhtml();
    QString land_xml = BuildLandmarks(landlist);
    QRegularExpression landmarks_placeholder(NAV_LANDMARKS_PATTERN,
                       QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch mo = landmarks_placeholder.match(nav_data);
    if (mo.hasMatch()) {
        nav_data.replace(mo.capturedStart(), mo.capturedLength(), land_xml);
    }
    m_NavResource->SetText(nav_data);
}



void NavProcessor::SetTOC(const QList<NavTOCEntry> & toclist)
{
    if (!m_NavResource) return; 

    bool found_toc = false;
    // QWriteLocker locker(&m_NavResource->GetLock());
    GumboInterface gi = GumboInterface(m_NavResource->GetText(), "3.0");
    gi.parse();
    const QList<GumboNode*> nav_nodes = gi.get_all_nodes_with_tag(GUMBO_TAG_NAV);
    for (int i = 0; i < nav_nodes.length(); ++i) {
        GumboNode* node = nav_nodes.at(i);
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "epub:type");
        if (attr && (QString::fromUtf8(attr->value) == "toc")) {
            found_toc = true;
            GumboNode * parent = node->parent;
            unsigned int index_within_parent = node->index_within_parent;
            gumbo_remove_from_parent(node);
            gumbo_destroy_node(node);
            GumboNode * placeholder = gumbo_create_text_node(GUMBO_NODE_COMMENT,"SIGIL_REPLACE_TOC_HERE");
            gumbo_insert_node(placeholder, parent, index_within_parent);
            break;
        }
    }
    if (!found_toc) {
        QList<GumboNode*> body_nodes = gi.get_all_nodes_with_tag(GUMBO_TAG_BODY);
        if (body_nodes.length() == 1) {
            GumboNode* body = body_nodes.at(0);
            GumboNode * placeholder = gumbo_create_text_node(GUMBO_NODE_COMMENT,"SIGIL_REPLACE_TOC_HERE");
            gumbo_append_node(body, placeholder);
        }
    }
    QString nav_data = gi.getxhtml();
    QString toc_xml = BuildTOC(toclist);
    QRegularExpression toc_placeholder(NAV_TOC_PATTERN, 
                       QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch mo = toc_placeholder.match(nav_data);
    if (mo.hasMatch()) {
        nav_data.replace(mo.capturedStart(), mo.capturedLength(), toc_xml);
    }
    m_NavResource->SetText(nav_data);
}


void NavProcessor::AddLandmarkCode(const Resource *resource, QString new_code, bool toggle)
{
    if (new_code.isEmpty()) return;
    QList<NavLandmarkEntry> landlist = GetLandmarks();
    QWriteLocker locker(&m_NavResource->GetLock());
    int pos = GetResourceLandmarkPos(resource, landlist);
    QString current_code;
    if (pos > -1) {
        NavLandmarkEntry le = landlist.at(pos);
        current_code = le.etype;
    }
    if ((current_code != new_code) || !toggle) {
        QString title = Landmarks::instance()->GetName(new_code);
        if (pos > -1) {
            NavLandmarkEntry le = landlist.at(pos);
            le.etype = new_code;
            le.title = title;
            landlist.replace(pos, le);
        } else {
            NavLandmarkEntry le;
            le.etype = new_code;
            le.title = title;
            le.href = ConvertOEBPSToNavRelative(resource->GetRelativePathToOEBPS());
            landlist.append(le);
        }
    } else {
        // if the current code is the same as the new one, we toggle it off.
        if (pos > -1) landlist.removeAt(pos);
    }
    SetLandmarks(landlist);
}

void NavProcessor::RemoveLandmarkForResource(const Resource * resource) 
{
    QList<NavLandmarkEntry> landlist = GetLandmarks();
    QWriteLocker locker(&m_NavResource->GetLock());
    int pos = GetResourceLandmarkPos(resource, landlist);
    if (pos > -1) {
        landlist.removeAt(pos);
        SetLandmarks(landlist);
    }
}

int NavProcessor::GetResourceLandmarkPos(const Resource *resource, const QList<NavLandmarkEntry> & landlist)
{
    QString resource_oebps_path = resource->GetRelativePathToOEBPS();
    for (int i=0; i < landlist.count(); ++i) {
        NavLandmarkEntry le = landlist.at(i);
        QString href = ConvertHREFToOEBPSRelative(le.href);
        QStringList parts = href.split('#', QString::KeepEmptyParts);
        if (parts.at(0) == resource_oebps_path) {
            return i;
        }
    }
    return -1;
}

QString NavProcessor::GetLandmarkCodeForResource(const Resource *resource)
{
    const QList<NavLandmarkEntry> landlist = GetLandmarks();
    QReadLocker locker(&m_NavResource->GetLock());
    int pos = GetResourceLandmarkPos(resource, landlist);
    QString etype;
    if (pos > -1) {
        NavLandmarkEntry le = landlist.at(pos);
        etype = le.etype;
    }
    return etype;
}

QString NavProcessor::GetLandmarkNameForResource(const Resource *resource)
{
    QString name;
    QString etype = GetLandmarkCodeForResource(resource);
    if (!etype.isEmpty()) {
        name = Landmarks::instance()->GetName(etype);
    }
    return name;
}

QHash <QString, QString> NavProcessor::GetLandmarkNameForPaths()
{
    const QList<NavLandmarkEntry> landlist = GetLandmarks();
    QReadLocker locker(&m_NavResource->GetLock());
    QHash <QString, QString> semantic_types;
    foreach(NavLandmarkEntry le, landlist) {
        QString href = ConvertHREFToOEBPSRelative(le.href);
        QStringList parts = href.split('#', QString::KeepEmptyParts);
        QString etype = le.etype;
        semantic_types[parts.at(0)] = Landmarks::instance()->GetName(etype);
    }
    return semantic_types;
}

QHash <QString, QString> NavProcessor::GetLandmarkCodeForPaths()
{
  const QList<NavLandmarkEntry> landlist = GetLandmarks();
  QReadLocker locker(&m_NavResource->GetLock());
  QHash <QString, QString> semantic_types;
  foreach(NavLandmarkEntry le, landlist) {
    QString href = ConvertHREFToOEBPSRelative(le.href);
    QStringList parts = href.split('#', QString::KeepEmptyParts);
    QString etype = le.etype;
    semantic_types[parts.at(0)] = etype;
  }
  return semantic_types;
}


// Interface to Set the Nav TOC directly from Book Contents (Headings)
// Get the Book headings and Make a Tree out of them and then convert
// That tree of headings to our flat NavTOCEntry list
bool NavProcessor::GenerateTOCFromBookContents(const Book* book)
{
    QString prev_xml = BuildTOC(GetTOC());
    QWriteLocker locker(&m_NavResource->GetLock());
    bool is_changed = false;

    // Remove the Nav resource from list of HTMLResources if it exists (EPUB3)
    QList<HTMLResource*> htmlresources = book->GetFolderKeeper()->GetResourceTypeList<HTMLResource>(true);
    if (m_NavResource) {
        htmlresources.removeOne(m_NavResource);
    }

    const QList<Headings::Heading> headings = Headings::MakeHeadingHeirarchy(Headings::GetHeadingList(htmlresources));
    QList<NavTOCEntry> toclist;
    foreach(const Headings::Heading & heading, headings) {
        toclist.append(HeadingWalker(heading, 1));
    }
    QString new_xml = BuildTOC(toclist);
    is_changed = new_xml != prev_xml;
    SetTOC(toclist);
    return is_changed;
}

// Used by GenerateTOCFromBookContents
QList<NavTOCEntry>  NavProcessor::HeadingWalker(const Headings::Heading & heading, int lvl)
{
    QList<NavTOCEntry> toclist;
    QString nav_file = m_NavResource->GetRelativePathToOEBPS();
    QString heading_file = heading.resource_file->GetRelativePathToOEBPS();
    if (heading.include_in_toc && (nav_file != heading_file)) {
        NavTOCEntry te;
        te.lvl = lvl;
        te.title = heading.text;
        QString id_to_use = heading.id;
        // If this heading appears right after a section break,
        // then it "represents" and links to its file; otherwise,
        // we link to the heading element directly
        // Prevent links back to to the nav itself form the nav
        if (heading.at_file_start) {
            te.href = ConvertOEBPSToNavRelative(heading_file);
        } else {
            te.href = ConvertOEBPSToNavRelative(heading_file) + "#" + id_to_use;
        }
        toclist.append(te);
    }
    foreach(Headings::Heading child_heading, heading.children) {
        toclist.append(HeadingWalker(child_heading, lvl+1));
    }
    return toclist;
}




// Interface from Dockable Table of Contents Widget to get the Nav TOC
// Need to take our flat Nav TOC list and create a tree then convert that
// tree to become a TOCModel::TOCEntry tree with returned root that is
// used in the Tabel Of Contents Dockable Widget
TOCModel::TOCEntry NavProcessor::GetRootTOCEntry()
{
    TOCModel::TOCEntry root;
    root.is_root = true;
    QList<NavTOCEntry> navtree = MakeHierarchy(GetTOC());
    foreach(NavTOCEntry nav_entry, navtree) {
        AddTOCEntry(nav_entry, root);
    }
    return root;
}

void NavProcessor::AddTOCEntry(const NavTOCEntry & nav_entry, TOCModel::TOCEntry & parent) 
{
    TOCModel::TOCEntry toc_entry;
    toc_entry.text = nav_entry.title;
    QString href = ConvertHREFToOEBPSRelative(nav_entry.href);
    toc_entry.target = href;
    foreach(NavTOCEntry nav_child, nav_entry.children) {
        AddTOCEntry(nav_child, toc_entry);
    }
    parent.children.append(toc_entry);
}

QList<NavTOCEntry> NavProcessor::MakeHierarchy(const QList<NavTOCEntry> & toclist)
{
    // Based on the approach used in Headings::
    QList<NavTOCEntry> navtree = toclist;
    for (int i = 0; i < navtree.size(); ++i) {
        // As long as entries after this one are higher in level, we 
        // continue adding them to this entry's children or grandchildren
        while(true) {
            if ((i == navtree.size() - 1) || (navtree[ i + 1 ].lvl  <= navtree[ i ].lvl)) {
                break;
            }
            AddChildEntry(navtree[ i ], navtree[ i + 1 ]);
            // The removeAt function will "push down" the rest
            // of the elements in the list by one after
            // it removes this element
            navtree.removeAt(i + 1);
        }
    }
    return navtree;
}

// Adds the new_child heading to the parent heading;
// the new_child is propagated down the tree (to its children) if needed
void NavProcessor::AddChildEntry(NavTOCEntry &parent, NavTOCEntry new_child)
{
    if ((!parent.children.isEmpty()) && (parent.children.last().lvl < new_child.lvl)) {
        AddChildEntry(parent.children.last(), new_child);
    } else {
        parent.children.append(new_child);
    }
}





// Interface from EditTOC Dialog to Set the Nav TOC 
// passes in TOCModel::TOCEntry root
// So convert to flat Nav TOC Entry list and rebuild the Nav TOC Section
void NavProcessor::GenerateNavTOCFromTOCEntries(const TOCModel::TOCEntry& root)
{
    QList<NavTOCEntry> toclist;
    foreach(TOCModel::TOCEntry entry, root.children) {
        toclist.append(AddEditTOCEntry(entry, 1));
    }
    SetTOC(toclist);   
}

// Used by GenerateNavTOCFromTOCEntries
QList<NavTOCEntry> NavProcessor::AddEditTOCEntry(TOCModel::TOCEntry & entry, int lvl)
{
    QList<NavTOCEntry> toclist;
    NavTOCEntry te;
    te.title = entry.text;
    QString href = ConvertOEBPSToNavRelative(entry.target);
    te.href = href;
    te.lvl = lvl;
    toclist.append(te);
    foreach(TOCModel::TOCEntry child, entry.children) {
        toclist.append(AddEditTOCEntry(child, lvl+1));
    }
    return toclist;
}



// Utility Routines to convert hrefs from NCX Relative to Nav Relative and Back

// convert nav relative paths to be relative to OEBPS folder
// Nav lives in OEBPS/Text/ in Sigil
// NCX lives in OEBPS/ in Sigil
QString NavProcessor::ConvertHREFToOEBPSRelative(const QString & href) 
{
    QString new_href = href;
    QString nav_href = m_NavResource->GetRelativePathToOEBPS();

    if (new_href.startsWith("./")) {
        // handles "./" and "./Section0001.xhtml"
        new_href.remove(0,2);
        if (new_href.isEmpty()) {
            new_href = nav_href;
        } else {
            new_href = "Text/" + new_href;
        }
    } else if (new_href.startsWith("#")) {
        // handles "#toc"
        new_href = nav_href + new_href;
    } else if (new_href.startsWith("../")) {
        // handles "../Text/blah" 
        new_href.remove(0,3);
        QStringList parts = new_href.split('/', QString::KeepEmptyParts);
        QString folder = parts.at(0);
        if (!SIGIL_FOLDERS.contains(folder)) {
            new_href = "Text/" + new_href;
        }
    } else {
        // handles "Section0001.xhtml#frag"
        QStringList parts = new_href.split('/', QString::KeepEmptyParts);
        QString folder = parts.at(0);
        if (!SIGIL_FOLDERS.contains(folder)) {
            new_href = "Text/" + new_href;
        }
    }
    return new_href;
}

// convert oebps relative paths to be relative to the Text folder
// Nav lives in OEBPS/Text/ in Sigil
// NCX lives in OEBPS/ in Sigil
QString NavProcessor::ConvertOEBPSToNavRelative(const QString & href) 
{
    QString new_href = href;
    QString nav_href = m_NavResource->GetRelativePathToOEBPS();
    QStringList pieces = new_href.split('#', QString::KeepEmptyParts);
    QString basepath = pieces.at(0);
    QString fragment = "";
    if (pieces.size() > 1) fragment = pieces.at(1);
    if ((basepath == nav_href) && !fragment.isEmpty()) {
        new_href = "#" + fragment;
    } else if (!new_href.startsWith("http://")) {
        new_href = "../" + new_href;
    }
    return new_href;
}
