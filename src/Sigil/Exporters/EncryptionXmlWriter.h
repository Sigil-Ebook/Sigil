/************************************************************************
**
**  Copyright (C) 2009, 2010  Strahinja Markovic
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
#ifndef ENCRYPTIONXMLWRITER_H
#define ENCRYPTIONXMLWRITER_H

#include "XMLWriter.h"

class EncryptionXmlWriter : private XMLWriter
{
    
public:

    /**
     * Constructor.
     *
     * @param book The book for which we're writing the OPF.
     * @param device The IODevice into which we should write the XML.
     */
    EncryptionXmlWriter( QSharedPointer< Book > book, QIODevice &device );

    void WriteXML();    

private:

    /**
     * Writes the EncryptedData element and its children for the
     * specified font.
     *
     * @param The font for which we want to write the encryption info.
     */
    void WriteEncryptedData( const FontResource &font_resource );

};

#endif // ENCRYPTIONXMLWRITER_H