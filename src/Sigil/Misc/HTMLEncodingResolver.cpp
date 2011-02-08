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

#include <stdafx.h>
#include "HTMLEncodingResolver.h"
#include "Utility.h"

const QString HEAD_END = "</\\s*head\\s*>";
const QString ENCODING_ATTRIBUTE = "encoding\\s*=\\s*(?:\"|')([^\"']+)(?:\"|')";
const QString STANDALONE_ATTRIBUTE = "standalone\\s*=\\s*(?:\"|')([^\"']+)(?:\"|')";


// Accepts a full path to an HTML file.
// Reads the file, detects the encoding
// and returns the text converted to Unicode. 
QString HTMLEncodingResolver::ReadHTMLFile( const QString &fullfilepath )
{
    QFile file( fullfilepath );

    // Check if we can open the file
    if ( !file.open( QFile::ReadOnly ) ) 
    {
        boost_throw( CannotOpenFile() 
                     << errinfo_file_fullpath( file.fileName().toStdString() )
                     << errinfo_file_errorstring( file.errorString().toStdString() ) 
                   );
    }

    QByteArray data = file.readAll();

    return Utility::ConvertLineEndings( GetCodecForHTML( data ).toUnicode( data ) );
}


// Accepts an HTML stream and tries to determine its encoding;
// if no encoding is detected, the default codec for this locale is returned.
// We use this function because Qt's QTextCodec::codecForHtml() function
// leaves a *lot* to be desired.
const QTextCodec& HTMLEncodingResolver::GetCodecForHTML( const QByteArray &raw_text )
{
    // Qt docs say Qt will take care of deleting
    // any QTextCodec objects on application exit

    QString ascii_data = raw_text;

    int head_end = ascii_data.indexOf( QRegExp( HEAD_END ) );

    if ( head_end != -1 )
    {
        QString head = Utility::Substring( 0, head_end, ascii_data );

        QRegExp encoding( ENCODING_ATTRIBUTE );
        head.indexOf( encoding );
        QTextCodec *encoding_codec = QTextCodec::codecForName( encoding.cap( 1 ).toAscii() );

        if ( encoding_codec != 0 )

            return *encoding_codec;

        QRegExp charset( "charset=([^\"]+)\"" );
        head.indexOf( charset );
        QTextCodec *charset_codec  = QTextCodec::codecForName( charset .cap( 1 ).toAscii() );

        if ( charset_codec != 0 )

            return *charset_codec;
    }

    // This is a workaround for a bug in QTextCodec which
    // expects the 'charset' attribute to always come after
    // the 'http-equiv' attribute
    ascii_data.replace( QRegExp( "<\\s*meta([^>]*)http-equiv=\"Content-Type\"([^>]*)>" ),
                                 "<meta http-equiv=\"Content-Type\" \\1 \\2>" );

    // If we couldn't find a codec ourselves,
    // we use Qt's function.
    QTextCodec &locale_codec   = *QTextCodec::codecForLocale();
    QTextCodec &detected_codec = *QTextCodec::codecForHtml( ascii_data.toAscii(), QTextCodec::codecForLocale() ); 

    if ( detected_codec.name() != locale_codec.name() )

        return detected_codec;

    // If that couldn't find anything, then let's test for UTF-8
    if ( IsValidUtf8( raw_text ) )

        return *QTextCodec::codecForName( "UTF-8" );

    // If everything fails, we fall back to the locale default
    return locale_codec;
}


// This function goes through the entire byte array 
// and tries to see whether this is a valid UTF-8 sequence.
// If it's valid, this is probably a UTF-8 string.
bool HTMLEncodingResolver::IsValidUtf8( const QByteArray &string )
{
    // This is an implementation of the Perl code written here:
    //   http://www.w3.org/International/questions/qa-forms-utf-8
    //
    // Basically, UTF-8 has a very specific byte-pattern. This function
    // checks if the sent byte-sequence conforms to this pattern.
    // If it does, chances are *very* high that this is UTF-8.
    //
    // This function is written to be fast, not pretty.    

    if ( string.isNull() )

        return false;

    int index = 0;

    while ( index < string.size() )
    {
        QByteArray dword = string.mid( index, 4 );

        if ( dword.size() < 4 )

            dword = dword.leftJustified( 4, '\0' );

        const unsigned char * bytes = (const unsigned char *) dword.constData();

        // ASCII
        if (   bytes[0] == 0x09 ||
               bytes[0] == 0x0A ||
               bytes[0] == 0x0D ||
               ( 0x20 <= bytes[0] && bytes[0] <= 0x7E )                    
           ) 
        {
            index += 1;
        }

        // non-overlong 2-byte
        else if (  ( 0xC2 <= bytes[0] && bytes[0] <= 0xDF ) &&
                   ( 0x80 <= bytes[1] && bytes[1] <= 0xBF )            
                ) 
        {
            index += 2;
        }
           
        else if (  (     bytes[0] == 0xE0                         &&         // excluding overlongs 
                         ( 0xA0 <= bytes[1] && bytes[1] <= 0xBF ) &&
                         ( 0x80 <= bytes[2] && bytes[2] <= 0xBF )        ) || 
                  
                   (     (   ( 0xE1 <= bytes[0] && bytes[0] <= 0xEC ) ||     // straight 3-byte
                             bytes[0] == 0xEE                         ||
                             bytes[0] == 0xEF                     ) &&
                    
                         ( 0x80 <= bytes[1] && bytes[1] <= 0xBF )   &&
                         ( 0x80 <= bytes[2] && bytes[2] <= 0xBF )        ) ||

                   (     bytes[0] == 0xED                         &&         // excluding surrogates
                         ( 0x80 <= bytes[1] && bytes[1] <= 0x9F ) &&
                         ( 0x80 <= bytes[2] && bytes[2] <= 0xBF )        )
                 ) 
        {
            index += 3;
        }
 
          
        else if (    (   bytes[0] == 0xF0                         &&         // planes 1-3
                         ( 0x90 <= bytes[1] && bytes[1] <= 0xBF ) &&
                         ( 0x80 <= bytes[2] && bytes[2] <= 0xBF ) &&
                         ( 0x80 <= bytes[3] && bytes[3] <= 0xBF )      ) ||

                     (   ( 0xF1 <= bytes[0] && bytes[0] <= 0xF3 ) &&         // planes 4-15
                         ( 0x80 <= bytes[1] && bytes[1] <= 0xBF ) &&
                         ( 0x80 <= bytes[2] && bytes[2] <= 0xBF ) &&
                         ( 0x80 <= bytes[3] && bytes[3] <= 0xBF )      ) ||
                
                     (   bytes[0] == 0xF4                         &&         // plane 16
                         ( 0x80 <= bytes[1] && bytes[1] <= 0x8F ) &&
                         ( 0x80 <= bytes[2] && bytes[2] <= 0xBF ) &&
                         ( 0x80 <= bytes[3] && bytes[3] <= 0xBF )      )
                ) 
        {
            index += 4;
        }

        else
        {
            return false;
        }
    }

    return true;
}