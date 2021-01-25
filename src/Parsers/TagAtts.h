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

#pragma once
#ifndef TAGATTS_H
#define TAGATTS_H

#include <QObject>
#include <QString>
#include <QHash>

// implement a very simple ordered hash for tag attributes by using a qhash to store
// key to node mappings where each node (see struct TAttribute) is a key/value pair in 
// a circular doubly-linked list.
// both key and value are QStrings

class TagAtts : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor.
     */
     TagAtts();
     TagAtts(const TagAtts &other);
     TagAtts& operator=(const TagAtts &other);
     bool operator==(const TagAtts &other);
     bool operator!=(const TagAtts &other);
     QString& operator[](const QString &key);
     const QString operator[](const QString &key) const noexcept;
     
    ~TagAtts();
    
    struct TAttribute {
        QString key;
        QString value;
        struct TAttribute * prev;
        struct TAttribute * next;
        TAttribute(const QString &akey, const QString &avalue): key(akey), value(avalue) { prev = nullptr; next=nullptr; };
        ~TAttribute() { prev=nullptr; next=nullptr; }
    };

    void insert(const QString &key, const QString &value);
    void remove(const QString &key);
    QString value(const QString &key, const QString &altvalue="") const;
    bool contains(const QString &key) { return m_mapping.contains(key); }
    bool isEmpty() { return m_n == 0; }
    unsigned int size() const { return m_n; }
    QStringList keys() const;
    QStringList values() const;
    QList< std::pair< QString,QString > > pairs() const;

private:
    unsigned int m_n;
    QHash<QString, TAttribute*> m_mapping;
    // anchor for the circular doubly linked list
    TAttribute* m_anchor;
};

#endif // TAGATTS_H
