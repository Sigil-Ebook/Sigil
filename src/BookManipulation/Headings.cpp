/************************************************************************
**
**  Copyright (C) 2015 Kevin B. Hendricks Stratford, ON, Canada
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

#include <memory>
#include <functional>

#include <QtCore/QtCore>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtConcurrent/QtConcurrent>

#include "BookManipulation/Headings.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/GumboInterface.h"
#include "Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "sigil_constants.h"


// The maximum allowed distance (in lines) that a heading
// can be located from a body tag and still
// be detected as the "name" for that chapter.
// The value was picked arbitrarily.
static const int ALLOWED_HEADING_DISTANCE = 20;

const QList<GumboTag> GHEADING_TAGS = QList<GumboTag>() << GUMBO_TAG_H1 << GUMBO_TAG_H2 << GUMBO_TAG_H3 << GUMBO_TAG_H4 << GUMBO_TAG_H5 << GUMBO_TAG_H6;

const QStringList HEADING_TAGS = QStringList() << "h1" << "h2" << "h3" << "h4" << "h5" << "h6";

const QString SIGIL_NOT_IN_TOC_CLASS = "sigil_not_in_toc";
const QString OLD_SIGIL_NOT_IN_TOC_CLASS = "sigilNotInTOC";


// Returns a list of headings from the provided XHTML source;
// the list is flat, the headings are *not* in a hierarchy tree
QList<Headings::Heading> Headings::GetHeadingList(QList<HTMLResource *> html_resources,
        bool include_unwanted_headings)
{
    QList<Headings::Heading> heading_list;
    QList<QList<Headings::Heading>> per_file_headings =
                                     QtConcurrent::blockingMapped(html_resources,
                                             std::bind(GetHeadingListForOneFile, std::placeholders::_1, include_unwanted_headings));

    for (int i = 0; i < per_file_headings.count(); ++i) {
        heading_list.append(per_file_headings.at(i));
    }

    return heading_list;
}


QList<Headings::Heading> Headings::GetHeadingListForOneFile(HTMLResource *html_resource,
        bool include_unwanted_headings)
{
    Q_ASSERT(html_resource);
    QString source = html_resource->GetText();
    QString version = html_resource->GetEpubVersion();
    GumboInterface gi = GumboInterface(source, version);
    gi.parse();

    // get original source line number of body element
    unsigned int body_line = 0;
    QList<GumboNode*> bodylist = gi.get_all_nodes_with_tag(GUMBO_TAG_BODY);
    if (!bodylist.isEmpty()) {
        GumboNode* body = bodylist.at(0);
        body_line = body->v.element.start_pos.line;
    }

    QList<GumboNode*> heading_nodes = gi.get_all_nodes_with_tags(GHEADING_TAGS);
    int num_heading_nodes = heading_nodes.count();
    QList<Headings::Heading> headings;

    for (int i = 0; i < num_heading_nodes; ++i) {

        GumboNode* node = heading_nodes.at(i);

        Heading heading;

        heading.resource_file  = html_resource;
        heading.path_to_node = gi.get_path_to_node(node);

        heading.title = QString();
        GumboAttribute* attr = gumbo_get_attribute(&node->v.element.attributes,"title");
        if (attr) {
           heading.title = QString::fromUtf8(attr->value);
        }
        heading.orig_title     = heading.title;
        heading.id = QString();
        attr = gumbo_get_attribute(&node->v.element.attributes,"id");
        if (attr) {
           heading.id = QString::fromUtf8(attr->value);
        }

        if (!heading.title.isEmpty()) {
            heading.text = heading.title.simplified();
        } else {
            heading.text = gi.get_local_text_of_node(node).simplified();
        }
        heading.level = QString( QString::fromStdString(gi.get_tag_name(node)).at(1) ).toInt();
        heading.orig_level     = heading.level;

        QString classes  = QString();
        attr = gumbo_get_attribute(&node->v.element.attributes,"class");
        if (attr) {
            classes = QString::fromUtf8(attr->value);
        }

        heading.include_in_toc = !(classes.contains(SIGIL_NOT_IN_TOC_CLASS) ||
                                   classes.contains(OLD_SIGIL_NOT_IN_TOC_CLASS));

        unsigned int node_line = node->v.element.start_pos.line;

        heading.at_file_start = (i == 0) && ((node_line - body_line) < ALLOWED_HEADING_DISTANCE);
        heading.is_changed     = false;

        if (heading.include_in_toc || include_unwanted_headings) {
            headings.append(heading);
        }
    }

    return headings;
}


// Takes a flat list of headings and returns a list with those
// headings sorted into a hierarchy
QList<Headings::Heading> Headings::MakeHeadingHeirarchy(const QList<Heading> &headings)
{
    QList<Heading> ordered_headings = headings;

    for (int i = 0; i < ordered_headings.size(); ++i) {
        // As long as the headings after this one are
        // higher in level (smaller in size), we continue
        // adding them as this heading's children
        while (true) {
            if ((i == ordered_headings.size() - 1) ||
                (ordered_headings[ i + 1 ].level <= ordered_headings[ i ].level)
               ) {
                break;
            }

            AddChildHeading(ordered_headings[ i ], ordered_headings[ i + 1 ]);
            // The removeAt function will "push down" the rest
            // of the elements in the list by one after
            // it removes this element
            ordered_headings.removeAt(i + 1);
        }
    }
    return ordered_headings;
}


QList<Headings::Heading> Headings::GetFlattenedHeadings(const QList<Heading> &headings)
{
    QList<Heading> flat_headings;
    foreach(Heading heading, headings) {
        flat_headings.append(FlattenHeadingNode(heading));
    }
    return flat_headings;
}


// Flattens the provided heading node and its children
// into a list and returns it
QList<Headings::Heading> Headings::FlattenHeadingNode(Heading heading)
{
    QList<Heading> my_headings;
    my_headings.append(heading);
    foreach(Heading child_heading, heading.children) {
        my_headings.append(FlattenHeadingNode(child_heading));
    }
    return my_headings;
}


// Adds the new_child heading to the parent heading;
// the new_child is propagated down the tree if necessary
void Headings::AddChildHeading(Heading &parent, Heading new_child)
{
    if ((!parent.children.isEmpty()) &&
        (parent.children.last().level < new_child.level)
       ) {
        AddChildHeading(parent.children.last(), new_child);
    } else {
        parent.children.append(new_child);
    }
}
