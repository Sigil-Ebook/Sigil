/************************************************************************
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2014      John Schember <john@nachtimwald.com> 
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

#ifndef VALIDATIONRESULT_H
#define VALIDATIONRESULT_H

class QString;

class ValidationResult
{
public:
    enum ResType {
        ResType_Info = 0,
        ResType_Warn,
        ResType_Error
    };

    // Use int to allow negative values to indicate if field is valid or not for linenumber and charoffset
    ValidationResult(ValidationResult::ResType type, const QString &bookpath, int linenumber, const QString &message);
    ValidationResult(ValidationResult::ResType type, const QString &bookpath, int linenumber, int charoffset, const QString &message);
    ~ValidationResult();

    ValidationResult::ResType Type();
    QString BookPath();
    int LineNumber();
    int CharOffset();
    QString Message();

private:
    ValidationResult::ResType m_type;
    QString m_bookpath;
    int m_linenumber;
    int m_charoffset;
    QString m_message;
};

#endif // VALIDATIONRESULT_H
