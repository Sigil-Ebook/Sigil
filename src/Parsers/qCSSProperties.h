/************************************************************************
 **
 **  Copyright (C) 2021  Kevin B. Hendricks, Stratford, ON, Canada
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
 ** Extracted and modified from:
 ** CSSTidy (https://github.com/csstidy-c/csstidy)
 **
 ** CSSTidy Portions Copyright:
 **   Florian Schmitz <floele@gmail.com>
 **   Thierry Charbonnel
 **   Will Mitchell <aethon@gmail.com>
 **   Brett Zamir <brettz9@yahoo.com>
 **   sined_ <sined_@users.sourceforge.net>
 **   Dmitry Leskov <git@dmitryleskov.com>
 **   Kevin Coyner <kcoyner@debian.org>
 **   Tuukka Pasanen <pasanen.tuukka@gmail.com>
 **   Frank W. Bergmann <csstidy-c@tuxad.com>
 **   Frank Dana <ferdnyc@gmail.com>
 **
 ** CSSTidy us Available under the LGPL 2.1
 *************************************************************************/

#pragma once
#ifndef CSS_PROPERTIES_H
#define CSS_PROPERTIES_H

#include <QChar>
#include <QString>
#include <QHash>

/**
 * Singleton.
 *
 * CSSProperties
 */
 

class CSSProperties
{

public:

    static CSSProperties *instance();

    bool contains(QString pname);

    QString levels(QString pname);

private:

    CSSProperties();

    QHash<QString, QString> m_all_properties;
    static CSSProperties *m_instance;
};

#endif // CSS_PROPERTIES_H
