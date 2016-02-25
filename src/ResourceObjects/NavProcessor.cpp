/************************************************************************
**
**  Copyright (C) 2016  Kevin B. Hendricks, Stratford Ontario
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
#include "Misc/GumboInterface.h"
#include "Misc/Landmarks.h"

#include "ResourceObjects/NavProcessor.h"

static const QString NAV_PAGELIST_PATTERN = "\\s*<!--\\s*SIGIL_REPLACE_PAGELIST_HERE\\s*-->\\s*";
static const QString NAV_LANDMARKS_PATTERN = "\\s*<!--\\s*SIGIL_REPLACE_LANDMARKS_HERE\\s*-->\\s*";
static const QString NAV_TOC_PATTERN = "\\s*<!--\\s*SIGIL_REPLACE_TOC_HERE\\s*-->\\s*";

NavProcessor::NavProcessor(HTMLResource * nav_resource)
: m_NavResource(nav_resource)
{
}


NavProcessor::~NavProcessor()
{
}


QList<NavLandmarkEntry> NavProcessor::GetLandmarks()
{
    QList<NavLandmarkEntry> landlist;
    if (!m_NavResource) return landlist; 
	
    QReadLocker locker(&m_NavResource->GetLock());

    GumboInterface gi = GumboInterface(m_NavResource->GetText(), "3.0");
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

    GumboInterface gi = GumboInterface(m_NavResource->GetText(), "3.0");
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

    GumboInterface gi = GumboInterface(m_NavResource->GetText(), "3.0");
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
	        }
	        break;		
	    }
    }
    return toclist;
}


QList<NavTOCEntry> NavProcessor::GetNodeTOC(GumboInterface gi, const GumboNode * node, int lvl)
{
    if ((node->type != GUMBO_NODE_ELEMENT) || (node->v.element.tag != GUMBO_TAG_OL)) {
        return QList<NavTOCEntry>();
    }
    QList<NavTOCEntry> toclist;
    const GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        GumboNode * child = static_cast<GumboNode*>(children->data[i]);
        if (child->type == GUMBO_NODE_ELEMENT) {
            if (child->v.element.tag == GUMBO_TAG_A) {
            	NavTOCEntry te;
            	te.lvl = lvl;
            	GumboAttribute* hrefattr = gumbo_get_attribute(&child->v.element.attributes, "href");
		        if (hrefattr) te.href = Utility::URLDecodePath(QString::fromUtf8(hrefattr->value));
	            te.title = Utility::DecodeXML(gi.get_local_text_of_node(child));
                toclist.append(te);
            } else if (child->v.element.tag == GUMBO_TAG_OL) {
                toclist.append(GetNodeTOC(gi, child, lvl+1));
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
    res << "\n" + base + "<ol>\n";
    foreach(NavTOCEntry te, toclist) {
	    int lvl = te.lvl;
	    QString href = Utility::URLEncodePath(te.href);
	    QString title = Utility::EncodeXML(te.title);
	    if (lvl > curlvl) {
	        while(lvl > curlvl) {
	            QString indent = base + step.repeated(curlvl);
	            res << indent + "<ol>/n";
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
	    res << indent + step + "</li>\n";
	    res << indent + "</ol>\n";
	    --curlvl;
    }
    return res.join("");
}


QString NavProcessor::BuildLandmarks(const QList<NavLandmarkEntry> & landlist)
{
    QStringList res;
    QString step = "  ";
    QString base = step.repeated(2);
    res << "\n" + base + "<ol>\n";
    foreach(NavLandmarkEntry le, landlist) {
	    QString etype = le.etype;
	    QString href = Utility::URLEncodePath(le.href);
	    QString title = Utility::EncodeXML(le.title);
	    res << base + step + "<li>\n";
	    res << base + step.repeated(2) + "<a epub:type=\"" + etype + "\" href=\"" + href + "\">" + title + "</a>\n";
	    res << base + step + "</li>\n";
    }
    res << base + "</ol>\n";
    return res.join("");
}


QString NavProcessor::BuildPageList(const QList<NavPageListEntry> & pagelist)
{
    QStringList res;
    QString step = "  ";
    QString base = step.repeated(3);
    res << "\n" + base + "<ol>\n";
    foreach(NavPageListEntry pe, pagelist) {
	    QString pagename = Utility::EncodeXML(pe.pagename);
	    QString href = Utility::URLEncodePath(pe.href);
	    res << base + step + "<li><a href=\"" + href + "\">" + pagename + "</a></li>\n";
    }
    res << base + "</ol>\n";
    return res.join("");
}


void NavProcessor::SetPageList(const QList<NavPageListEntry> & pagelist)
{
    if (!m_NavResource) return; 
	
    // QWriteLocker locker(&m_NavResource->GetLock());
    GumboInterface gi = GumboInterface(m_NavResource->GetText(), "3.0");
    gi.parse();
    const QList<GumboNode*> nav_nodes = gi.get_all_nodes_with_tag(GUMBO_TAG_NAV);
    for (int i = 0; i < nav_nodes.length(); ++i) {
	    GumboNode* node = nav_nodes.at(i);
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "epub:type");
	    if (attr && (QString::fromUtf8(attr->value) == "page-list")) {
            QList<GumboTag> tags = QList<GumboTag>() << GUMBO_TAG_OL;
	        const QList<GumboNode*> ol_nodes = gi.get_nodes_with_tags(node, tags);
	        for (int j = 0; j < ol_nodes.length(); ++j) {
		        GumboNode * olnode = ol_nodes.at(j);
		        GumboNode * parent = node;
		        gumbo_remove_from_parent(olnode);
		        gumbo_destroy_node(olnode);
		        GumboNode * placeholder = gumbo_create_text_node(GUMBO_NODE_COMMENT,"SIGIL_REPLACE_PAGELIST_HERE");
		        gumbo_append_node(parent, placeholder);
		        break;
	        }
	        break;
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
	
    // QWriteLocker locker(&m_NavResource->GetLock());
    GumboInterface gi = GumboInterface(m_NavResource->GetText(), "3.0");
    gi.parse();
    const QList<GumboNode*> nav_nodes = gi.get_all_nodes_with_tag(GUMBO_TAG_NAV);
    for (int i = 0; i < nav_nodes.length(); ++i) {
	    GumboNode* node = nav_nodes.at(i);
	    GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "epub:type");
	    if (attr && (QString::fromUtf8(attr->value) == "landmarks")) {
            QList<GumboTag> tags = QList<GumboTag>() << GUMBO_TAG_OL;
	        const QList<GumboNode*> ol_nodes = gi.get_nodes_with_tags(node, tags);
	        for (int j = 0; j < ol_nodes.length(); ++j) {
	            GumboNode * olnode = ol_nodes.at(j);
		        GumboNode * parent = node;
		        gumbo_remove_from_parent(olnode);
		        gumbo_destroy_node(olnode);
		        GumboNode * placeholder = gumbo_create_text_node(GUMBO_NODE_COMMENT,"SIGIL_REPLACE_LANDMARKS_HERE");
		        gumbo_append_node(parent, placeholder);
		        break;
	        }
	        break;
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
	
    // QWriteLocker locker(&m_NavResource->GetLock());
    GumboInterface gi = GumboInterface(m_NavResource->GetText(), "3.0");
    gi.parse();
    const QList<GumboNode*> nav_nodes = gi.get_all_nodes_with_tag(GUMBO_TAG_NAV);
    for (int i = 0; i < nav_nodes.length(); ++i) {
	    GumboNode* node = nav_nodes.at(i);
	    GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes, "epub:type");
	    if (attr && (QString::fromUtf8(attr->value) == "toc")) {
            QList<GumboTag> tags = QList<GumboTag>()  << GUMBO_TAG_OL;
	        const QList<GumboNode*> ol_nodes = gi.get_nodes_with_tags(node, tags);
	        for (int j = 0; j < ol_nodes.length(); ++j) {
	            GumboNode * olnode = ol_nodes.at(j);
		        GumboNode * parent = node;
		        gumbo_remove_from_parent(olnode);
		        gumbo_destroy_node(olnode);
		        GumboNode * placeholder = gumbo_create_text_node(GUMBO_NODE_COMMENT,"SIGIL_REPLACE_TOC_HERE");
		        gumbo_append_node(parent, placeholder);
		        break;
	        }
	    break;
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
            le.href = resource->GetRelativePathToOEBPS();
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
        QString href = le.href;
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

QHash <QString, QString> NavProcessor::GetLandmarkNameForPaths()
{
    const QList<NavLandmarkEntry> landlist = GetLandmarks();
    QReadLocker locker(&m_NavResource->GetLock());
    QHash <QString, QString> semantic_types;
    foreach(NavLandmarkEntry le, landlist) {
        QString href = le.href;
        QStringList parts = href.split('#', QString::KeepEmptyParts);
        QString title = le.title;
        semantic_types[parts.at(0)] = title;
    }
    return semantic_types;
}

