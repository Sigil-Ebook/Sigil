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

#include <stdafx.h>
#include "FontObfuscation.h"

namespace 
{

QByteArray IdpfKeyFromIdentifier( const QString &identifier )
{
    QString whitespace_free = QString( identifier )
                              .remove( "\x20" )
                              .remove( "\x09" )
                              .remove( "\x0D" )
                              .remove( "\x0A" );

    return QCryptographicHash::hash( whitespace_free.toAscii(), QCryptographicHash::Sha1 );
}


QByteArray AdobeKeyFromIdentifier( const QString &identifier )
{
    QString cruft_free = QString( identifier )
                         .remove( "urn:uuid:" )
                         .remove( "-" )
                         .remove( ":" );

    return QByteArray::fromHex( cruft_free.toAscii() );
}


void IdpfObfuscate( const QString &filepath, const QString &identifier )
{
    QFile file( filepath );
    if ( !file.open( QFile::ReadWrite ) )

        return;

    QByteArray contents = file.readAll();

    QByteArray key = IdpfKeyFromIdentifier( identifier );
    int key_size   = key.size();

    // The IDPF method specifies that we use 1040 bytes, NOT 1024
    for ( int i = 0; ( i < 1040 ) && ( i < contents.size() ); ++i )
    {
        contents[ i ] = contents[ i ] ^ key[ i % key_size ]; 
    }

    file.seek( 0 );
    file.write( contents );
}


void AdobeObfuscate( const QString &filepath, const QString &identifier )
{
    QFile file( filepath );
    if ( !file.open( QFile::ReadWrite ) )

        return;

    QByteArray contents = file.readAll();

    QByteArray key = AdobeKeyFromIdentifier( identifier );
    int key_size   = key.size();

    for ( int i = 0; ( i < 1024 ) && ( i < contents.size() ); ++i )
    {
        contents[ i ] = contents[ i ] ^ key[ i % key_size ]; 
    }

    file.seek( 0 );
    file.write( contents );
}

};


void FontObfuscation::ObfuscateFile( const QString &filepath, 
                                     const QString &algorithm, 
                                     const QString &identifier )
{
    if ( !QFileInfo( filepath ).exists() )

        return;

    if ( algorithm == ADOBE_FONT_ALGO_ID )

        AdobeObfuscate( filepath, identifier );

    else 

        IdpfObfuscate( filepath, identifier );
}


