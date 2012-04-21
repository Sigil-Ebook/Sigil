/************************************************************************
**
**  Copyright (C) 2010  Strahinja Markovic
**
**  This file is part of FlightCrew.
**
**  FlightCrew is free software: you can redistribute it and/or modify
**  it under the terms of the GNU Lesser General Public License as published
**  by the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  FlightCrew is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU Lesser General Public License for more details.
**
**  You should have received a copy of the GNU Lesser General Public License
**  along with FlightCrew.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#include <stdafx.h>
#include "DetermineMimetype.h"
#include "Utilities.h"

namespace FlightCrew
{

const std::string OEBPS_MIME   = "application/oebps-package+xml";
const std::string XHTML_MIME   = "application/xhtml+xml";
const std::string NCX_MIME     = "application/x-dtbncx+xml";
const std::string PNG_MIME     = "image/png";
const std::string GIF_MIME     = "image/gif";
const std::string JPEG_MIME    = "image/jpeg";
const std::string SVG_MIME     = "image/svg+xml";
const std::string DTBOOK_MIME  = "application/x-dtbook+xml";
const std::string CSS_MIME     = "text/css";
const std::string XML_MIME     = "application/xml"; // used for out-of-line xml islands
const std::string XPGT_MIME    = "application/vnd.adobe-page-template+xml";
const std::string OTF_MIME     = "application/vnd.ms-opentype";

// For the "correct" truetype font mimetype, see this link
// http://mx.gw.com/pipermail/file/2009/000400.html
// Apparently ISO/IEC JTC 1/SC34 are working on a new font top-level medatype. But on
// the other hand they also recognize "application/x-font-ttf" as being the
// experimental (read: not standardized) defacto MIME type for Truetype fonts.
// Number of Google hits for all three possibilities:
//    "application/x-truetype-font"   2100
//    "application/x-font-truetype"   4100
//    "application/x-font-ttf"       45900
//
// So "application/x-font-ttf" it is.
const std::string TTF_MIME     = "application/x-font-ttf";
const std::string OEB_DOC_MIME = "text/x-oeb1-document";
const std::string OEB_CSS_MIME = "text/x-oeb1-css";

const std::string UNKNOWN_MIME = "unknown";

// It's just an arbitrary num of starting chars
// that we search for a fingerprint. Things like
// "<html>" should appear in this small section.
static const uint NUM_CHARS_FOR_FINGERPRINT = 1000;
static const boost::regex HTML_TAG_REGEX( "<\\s*html[^>]*>" );

static const std::string NCX_SYSTEM_ID    = "-//NISO//DTD ncx 2005-1//EN";
static const std::string DTBOOK_SYSTEM_ID = "-//NISO//DTD dtbook 2005-1//EN";

static const boost::regex NCX_TAG_REGEX(
    "<[^>]*ncx[^>]*\"http://www.daisy.org/z3986/2005/ncx/\"[^>]*>" );

static const boost::regex XPGT_TEMPLATE_REGEX(
    "<[^>]*template[^>]*\"http://ns.adobe.com/2006/ade\"[^>]*>" );

static const boost::regex DTBOOK_TAG_REGEX(
    "<[^>]*dtbook[^>]*\"http://www.daisy.org/z3986/2005/dtbook/\"[^>]*>" );


std::string MimetypeFromExtension( const fs::path &filepath )
{
    std::string extension = Util::BoostPathToUtf8Path( filepath.extension() );
    boost::erase_first( extension, "." );

    if ( extension == "xhtml" ||
         extension == "html"  ||
         extension == "htm" )
    {
        // Only the xhtml mimetype is valid
        // within epub, "text/html" is not
        return XHTML_MIME;
    }

    if ( extension == "png" )
    
        return PNG_MIME;    

    if ( extension == "gif" )
    
        return GIF_MIME;    

    if ( extension == "jpg" ||
         extension == "jpeg" )
    {
        return JPEG_MIME;
    }

    if ( extension == "css" )
    
        return CSS_MIME;    

    if ( extension == "ncx" )
    
        return NCX_MIME;
    
    if ( extension == "svg" )
    
        return SVG_MIME;    

    if ( extension == "otf" )
    
        return OTF_MIME;

    if ( extension == "ttf" )
    
        return TTF_MIME; 
        
    // We don't check for "xml" because
    // that's commonly used for several things.

    return UNKNOWN_MIME;
}


bool HasHtmlFingerprint( const std::string &contents )
{
    return boost::regex_search( contents, HTML_TAG_REGEX );
}


bool HasDtbookFingerprint( const std::string &contents )
{
    return 
        boost::contains( contents, DTBOOK_SYSTEM_ID ) || 
        boost::regex_search( contents, DTBOOK_TAG_REGEX );
}


bool HasNcxFingerprint( const std::string &contents )
{
    return 
        boost::contains( contents, NCX_SYSTEM_ID ) || 
        boost::regex_search( contents, NCX_TAG_REGEX );
}


bool HasXpgtFingerprint( const std::string &contents )
{
    return boost::regex_search( contents, XPGT_TEMPLATE_REGEX );
}


std::string GuessMimetypeFromFileContents( const fs::path &filepath )
{
    std::string contents;

    try
    {
        contents = Util::ReadUnicodFile( filepath );
    }

    catch ( std::exception& )
    {
    	return UNKNOWN_MIME;
    }    

    std::string contents_start = Util::GetFirstNumChars( contents, NUM_CHARS_FOR_FINGERPRINT );

    if ( HasHtmlFingerprint( contents_start ) )

        return XHTML_MIME;

    if ( HasNcxFingerprint( contents_start ) )

        return DTBOOK_MIME;

    if ( HasNcxFingerprint( contents_start ) )

        return NCX_MIME;

    if ( HasXpgtFingerprint( contents_start ) )

        return XPGT_MIME;

    return UNKNOWN_MIME;
}


std::string DetermineMimetype( const fs::path &filepath )
{
    std::string mimetype = MimetypeFromExtension( filepath );
    
    if ( mimetype != UNKNOWN_MIME )

        return mimetype;
    
    return GuessMimetypeFromFileContents( filepath );
}


} // namespace FlightCrew

