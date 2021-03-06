/************************************************************************
**
**  Copyright (C) 2021 Kevin B. Hendricks, Stratford, ON, Canada
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
#ifndef PCREERRORS_H
#define PCREERRORS_H

#include <QCoreApplication>
#include <QTranslator>
#include <QHash>

class QString;


/**
 * Singleton.
 *
 * PCREErrors
 */
 

class PCREErrors
{
    Q_DECLARE_TR_FUNCTIONS(PCREErrors)

public:

    static PCREErrors *instance();

    QString GetError(const QString &code, const QString &ow);

private:

    PCREErrors();

    void SetErrorMap();

    QHash<QString, QString> m_XlateError;
    
    static PCREErrors *m_instance;

};

#endif // PCREERRORS_H
