/************************************************************************
**
**  Copyright (C) 2025 Kevin B. Hendricks, Stratford, ON, Canada
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
#ifndef ARIAROLES_H
#define ARIAROLES_H

#include <QCoreApplication>
#include <QHash>
#include <QStringList>

class QString;

#include "Misc/DescriptiveInfo.h"


/**
 * Singleton.
 *
 * AriaRoles
 */
 

class AriaRoles
{
    Q_DECLARE_TR_FUNCTIONS(AriaRoles)

public:

    static AriaRoles *instance();

    QString GetName(const QString &code);
    QString GetTitle(const QString &code, const QString &lang);
    QString GetDescriptionByCode(const QString &code);
    QString GetDescriptionByName(const QString &name);
    QString GetCode(const QString &name);
    QStringList GetSortedNames();
    QStringList GetAllCodes();
    bool isAriaRolesCode(const QString &code);
    bool isAriaRolesName(const QString &name);
    const QHash<QString, DescriptiveInfo>& GetCodeMap();
    QString EpubTypeMapping(const QString &code);
    QStringList AllowedTags(const QString &code);

private:

    AriaRoles();

    void SetAriaRolesMap();

    void SetEpubTypeMap();

    void SetCodeToRawTitleMap();

    // code -> DescriptiveInfo -> name and description
    QHash<QString, DescriptiveInfo> m_CodeMap;
    
    // name -> code
    QHash<QString, QString> m_NameMap;

    QStringList m_sortedNames;

    QHash<QString, QString> m_EpubTypeMap;
    
    QHash<QString,QString> m_CodeToRawTitle;

    static AriaRoles *m_instance;


};

#endif // ARIAROLES_H
