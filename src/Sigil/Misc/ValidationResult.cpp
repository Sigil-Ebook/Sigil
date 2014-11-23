/************************************************************************
**
**  Copyright (C) 2014 John Schember <john@nachtimwald.com> 
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

#include "Misc/ValidationResult.h"

ValidationResult::ValidationResult(ValidationResult::ResType type, const QString &filename, size_t linenumber, const QString &message)
{
    m_type       = type;
    m_filename   = filename;
    m_linenumber = linenumber;
    m_message    = message;
}

ValidationResult::~ValidationResult()
{
}

ValidationResult::ResType ValidationResult::Type()
{
	return m_type;
}

QString ValidationResult::Filename()
{
	return m_filename;
}

size_t ValidationResult::LineNumber()
{
	return m_linenumber;
}

QString ValidationResult::Message()
{
	return m_message;
}
