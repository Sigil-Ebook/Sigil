/************************************************************************
**
**  Copyright (C) 2020-2021 Kevin B. Hendricks, Stratford Ontario Canada
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
#include <QDebug>
#include "Parsers/TagAtts.h"

TagAtts::TagAtts()
    : m_n(0), m_mapping(QHash<QString, TagAtts::TAttribute*>()), m_anchor(new TagAtts::TAttribute("",""))
{
    m_anchor->prev = m_anchor;
    m_anchor->next = m_anchor;
}

// copy constructor linear time
TagAtts::TagAtts(const TagAtts &other)
    : m_n(0), m_mapping(QHash<QString, TagAtts::TAttribute*>()), m_anchor(new TagAtts::TAttribute("",""))
{
     m_anchor->prev = m_anchor;
     m_anchor->next = m_anchor;
     QList<std::pair<QString, QString> > pairlist = other.pairs();
     for(unsigned int i=0; i < other.size(); i++) {
         insert(pairlist.at(i).first, pairlist.at(i).second);
     }
}

// assignment linear time
TagAtts& TagAtts::operator=(const TagAtts &other)
{
    if (m_n > 0) {
        m_mapping.clear();
        TagAtts::TAttribute * patt = m_anchor->next;
        while(patt && (patt != m_anchor)) {
            TagAtts::TAttribute * natt = patt->next;
            delete patt;
            patt = natt;
            m_n--;
        }
        m_anchor->next = m_anchor;
        m_anchor->prev = m_anchor;
        m_n = 0;
    }
    QList<std::pair<QString, QString> > pairlist = other.pairs();
    for(unsigned int i=0; i < other.size(); i++) {
        insert(pairlist.at(i).first, pairlist.at(i).second);
    }
    return *this;
}

// comparison
bool TagAtts::operator==(const TagAtts &other)
{
    if (m_n != other.size()) return false;
    if (other.keys() != keys()) return false;
    if (other.values() != values()) return false;
    return true;
}

bool TagAtts::operator!=(const TagAtts &other)
{
    if (m_n != other.size()) return true;
    if (other.keys() != keys()) return true;
    if (other.values() != values()) return true;
    return false;
}

QString& TagAtts::operator[](const QString & key)
{
    if (!m_mapping.contains(key)) insert(key, "");
    return m_mapping[key]->value;
}

const QString TagAtts::operator[](const QString & key) const noexcept
{
    return value(key);
}

TagAtts::~TagAtts()
{
    m_n = 0;
    m_mapping.clear();
    TagAtts::TAttribute * patt = m_anchor->next;
    while(patt && (patt != m_anchor)) {
        TagAtts::TAttribute * natt = patt->next;
        delete patt;
        patt = natt;
    }
    delete m_anchor;
    m_anchor = nullptr;
}

void TagAtts::insert(const QString &key, const QString &value)
{
    if (m_mapping.contains(key)) {
        TagAtts::TAttribute* patt = m_mapping[key];
        patt->value = value;
    } else {
        TagAtts::TAttribute* patt = new TAttribute(key, value);
        // circular all prev and next values should exist
        // insert at end of list (just before anchor)
        TagAtts::TAttribute* last = m_anchor->prev;
        last->next = patt;
        patt->prev = last;
        patt->next = m_anchor;
        m_anchor->prev = patt;
        m_mapping[key] = patt;
        m_n++;
    }
}

void TagAtts::remove(const QString &key)
{
    if (!m_mapping.contains(key)) return;
    TagAtts::TAttribute* patt = m_mapping[key];
    // remove it from doubly-linked list
    TagAtts::TAttribute* pnext = patt->next;
    TagAtts::TAttribute* pprev = patt->prev;
    pprev->next = pnext;
    pnext->prev = pprev;
    m_mapping.remove(key);
    delete patt;
    m_n--;
}    

QString TagAtts::value(const QString &key, const QString &altvalue) const
{
    if (!m_mapping.contains(key)) return altvalue;
    TagAtts::TAttribute* patt = m_mapping[key];
    return patt->value;
}

QStringList TagAtts::keys() const
{
    QStringList keylist;
    if (m_n == 0) return keylist;
    TagAtts::TAttribute* patt = m_anchor->next;
    while (patt && (patt != m_anchor)) {
        keylist << patt->key;
        patt = patt->next;
    }
    return keylist;
}

QStringList TagAtts::values() const
{
    QStringList vallist;
    if (m_n == 0) return vallist;
    TagAtts::TAttribute* patt = m_anchor->next;
    while (patt && (patt != m_anchor)) {
        vallist << patt->value;
        patt = patt->next;
    }
    return vallist;
}

QList<std::pair<QString, QString> > TagAtts::pairs() const
{
    QList<std::pair<QString, QString> > plist;
    if (m_n == 0) return plist;
    TagAtts::TAttribute* patt = m_anchor->next;
    while (patt && (patt != m_anchor)) {
        std::pair<QString, QString> apair;
        apair.first = patt->key;
        apair.second = patt->value;
        plist << apair;
        patt = patt->next;
    }
    return plist;
}
