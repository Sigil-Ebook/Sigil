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

#include <QtCore/QFile>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "Dialogs/About.h"
#include "Misc/Utility.h"
#include "sigil_constants.h"

const QString VERSION_NUMBERS = "(\\d+)\\.(\\d+)\\.(\\d+)";
const QString SIGIL_VERSION   = QString(SIGIL_FULL_VERSION);
const QString SIGIL_HOMEPAGE  = "http://code.google.com/p/sigil/";
const QString GNU_LICENSE     = "http://www.gnu.org/licenses/gpl-3.0-standalone.html";


About::About(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    ui.lbHomepageDisplay->setText("<a href=\"" % SIGIL_HOMEPAGE % "\">" % SIGIL_HOMEPAGE % "</a>");
    ui.lbLicenseDisplay->setText("<a href=\"" % GNU_LICENSE % "\">" % tr("GNU General Public License v3") % "</a>");
    ui.lbBuildTimeDisplay->setText(GetUTCBuildTime().toString("yyyy.MM.dd HH:mm:ss") + " UTC");
    ui.lbLoadedQtDisplay->setText(QString(qVersion()));
    QRegularExpression version_number(VERSION_NUMBERS);
    QRegularExpressionMatch mo = version_number.match(SIGIL_VERSION);
    QString version_text = QString("%1.%2.%3")
                               .arg(mo.captured(1).toInt())
                               .arg(mo.captured(2).toInt())
                               .arg(mo.captured(3).toInt());
    ui.lbVersionDisplay->setText(version_text);
    QString credits = "<h4>" + tr("Maintainer / Lead Developer") + "</h4>" +
        "<ul><li>John Schember</li></ul>" +
        "<h4>" + tr("Code Contributors") + "</h4>" +
        "<ul>" + 
        "<li>Grant Drake</li>" + 
        "<li>Dave Heiland</li>" + 
        "<li>Charles King</li>" + 
        "<li>Daniel Pavel</li>" + 
        "<li>Grzegorz Wolszczak</li>" + 
        "</ul>" + 
        "<h4>" + tr("Translators") + "</h4>" +
        "<ul><li><a href=\"https://www.transifex.net/projects/p/sigil/\">https://www.transifex.net/projects/p/sigil/teams/</a></li></ul>";
        "<h4>" + tr("Original Creator") + "</h4>" +
        "<ul><li>Strahinja Marković  (" + tr("retired") + ")</li></ul>";
    ui.creditsDisplay->setText(credits);
}


QDateTime About::GetUTCBuildTime()
{
    QString time_string = QString::fromLatin1(__TIME__);
    QString date_string = QString::fromLatin1(__DATE__);
    Q_ASSERT(!date_string.isEmpty());
    Q_ASSERT(!time_string.isEmpty());
    QRegularExpression date_match("(\\w{3})\\s+(\\d+)\\s+(\\d{4})");
    QRegularExpressionMatch mo = date_match.match(date_string);
    QDate date(mo.captured(3).toInt(), MonthIndexFromString(mo.captured(1)), mo.captured(2).toInt());
    return QDateTime(date, QTime::fromString(time_string, "hh:mm:ss")).toUTC();
}


// Needed because if we use the "MMM" string in the QDate::fromString
// function, it will match on localized month names, not English ones.
// The __DATE__ macro *always* uses English month names.
int About::MonthIndexFromString(const QString &three_letter_string)
{
    Q_ASSERT(three_letter_string.count() == 3);
    Q_ASSERT(three_letter_string[ 0 ].isUpper());

    if (three_letter_string == "Jan") {
        return 1;
    }

    if (three_letter_string == "Feb") {
        return 2;
    }

    if (three_letter_string == "Mar") {
        return 3;
    }

    if (three_letter_string == "Apr") {
        return 4;
    }

    if (three_letter_string == "May") {
        return 5;
    }

    if (three_letter_string == "Jun") {
        return 6;
    }

    if (three_letter_string == "Jul") {
        return 7;
    }

    if (three_letter_string == "Aug") {
        return 8;
    }

    if (three_letter_string == "Sep") {
        return 9;
    }

    if (three_letter_string == "Oct") {
        return 10;
    }

    if (three_letter_string == "Nov") {
        return 11;
    }

    if (three_letter_string == "Dec") {
        return 12;
    }

    Q_ASSERT(false);
    return 0;
}
