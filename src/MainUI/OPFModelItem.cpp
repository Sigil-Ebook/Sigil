/************************************************************************
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include "MainUI/OPFModelItem.h"

// Reimplementation of QStandardItem to support sorting alphanumerically
AlphanumericItem::AlphanumericItem() : QStandardItem()
{
}

AlphanumericItem::AlphanumericItem(const QIcon icon, const QString text) : QStandardItem(icon, text)
{
}

// Override standard sort operator to allow sorting alphanumerically case-sensitive
bool AlphanumericItem::operator<(const QStandardItem &item) const
{
    if (model()->sortRole() == READING_ORDER_ROLE) {
        return data(READING_ORDER_ROLE).toInt() < item.data(READING_ORDER_ROLE).toInt();
    } else if (model()->sortRole() != ALPHANUMERIC_ORDER_ROLE) {
        return text().compare(item.text()) < 0;
    }

    QString s1 = data(ALPHANUMERIC_ORDER_ROLE).toString();
    QString s2 = item.data(ALPHANUMERIC_ORDER_ROLE).toString();

    if (s1 == NULL || s2 == NULL) {
        return false;
    }

    int len1 = s1.length();
    int len2 = s2.length();
    int marker1 = 0;
    int marker2 = 0;
    int result  = 0;

    // Loop through both strings with separate markers
    while (marker1 < len1 && marker2 < len2) {
        QChar ch1 = s1[marker1];
        QChar ch2 = s2[marker2];
        QString space1(s1);
        QString space2(s2);
        int loc1 = 0;
        int loc2 = 0;

        // Loop over first string at marker collecting consecutive digits (or non-digits)
        do {
            space1[loc1++] = ch1;
            marker1++;

            if (marker1 < len1) {
                ch1 = s1[marker1];
            } else {
                break;
            }
        } while (ch1.isDigit() == space1[0].isDigit());

        // Loop over second string at marker collecting consecutive digits (or non-digits)
        do {
            space2[loc2++] = ch2;
            marker2++;

            if (marker2 < len2) {
                ch2 = s2[marker2];
            } else {
                break;
            }
        } while (ch2.isDigit() == space2[0].isDigit());

        QString str1 = space1.left(loc1);
        QString str2 = space2.left(loc2);

        // If we collected numbers, compare them numerically
        if (space1[0].isDigit() && space2[0].isDigit()) {
            int n1 = str1.toInt();
            int n2 = str2.toInt();
            result = n1 - n2;
        } else {
            result = str1.compare(str2);
        }

        if (result != 0) {
            return result < 0;
        }
    }

    return len1 < len2;
}
