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
#include "ErrorMessages.h"

namespace FlightCrew
{

static const std::string NO_MESSAGE_PLACEHOLDER = 
    "No message available. "
    "Please report this on the FlightCrew issue tracker.";

boost::mutex ErrorMessages::s_AccessMutex;
ErrorMessages* ErrorMessages::s_Instance = NULL;

ErrorMessages& ErrorMessages::Instance()
{
    // We use a static local variable
    // to hold our singleton instance; using a pointer member
    // variable creates problems with object destruction;

    boost::lock_guard< boost::mutex > locker( s_AccessMutex );

    if ( !s_Instance )
    {
        static ErrorMessages messages;
        s_Instance = &messages;
    }

    return *s_Instance;
}


const std::string ErrorMessages::MessageForId( ResultId error_id )
{
    if ( m_Messages.count( error_id ) )

        return m_Messages[ error_id ];

    return NO_MESSAGE_PLACEHOLDER;
}


ErrorMessages::ErrorMessages()
{
    LoadMessages();
}


void ErrorMessages::LoadMessages()
{
    // It would be great if these could be read from an external file,
    // but we don't want a runtime dependency. So we hard-code them.
    // This would be bad for translations, but since we can't translate the
    // error messages coming from the schema checkers, we won't be translating
    // these either. So it's not a problem.

    m_Messages[ ALL_OK ] =
        "All OK.";
    m_Messages[ ERROR_GENERIC ] =
        "Error.";
    m_Messages[ ERROR_SCHEMA_NOT_SATISFIED ] =
        "The corresponding schema constraints were not satisfied.";

    m_Messages[ ERROR_EPUB_NOT_VALID_ZIP_ARCHIVE ] =
        "The epub file is not a valid ZIP archive.";
    m_Messages[ ERROR_EPUB_NO_CONTAINER_XML ] =
        "The META-INF/container.xml file was not found.";  

    m_Messages[ ERROR_OCF_CONTAINER_DOESNT_LIST_OPF ] =
        "An OPF file is not listed as a <rootfile>.";
    m_Messages[ ERROR_OCF_CONTAINER_SPECIFIED_OPF_DOESNT_EXIST ] =
        "The specified OPF file \"%1%\" does not exist.";
    m_Messages[ ERROR_EPUB_MIMETYPE_BYTES_INVALID ] =
        "Bytes 30-60 of your epub file are invalid. This means that one or more of the following "
        "rules are not satisfied:\n"
        "   1. There needs to be a \"mimetype\" file in the root folder.\n"
        "   2. Its content needs to be *exactly* the ASCII string \"application/epub+zip\".\n"
        "   3. It needs to be the first file in the epub zip archive.\n"
        "   4. It needs to be uncompressed.";

    m_Messages[ ERROR_XML_NOT_WELL_FORMED ] =
        "XML syntax error.";
    m_Messages[ ERROR_XML_ELEMENT_NOT_PRESENT ] =
        "The <%1%> element is missing.";
    m_Messages[ ERROR_XML_WRONG_ELEMENT_COUNT ] =
        "There needs to be one and only one <%1%> element.";
    m_Messages[ ERROR_XML_CHILD_NOT_RECOGNIZED ] =
        "The <%1%> element is not an allowed child of the <%2%> element.";
    m_Messages[ ERROR_XML_ATTRIBUTE_NOT_RECOGNIZED ] =
        "The \"%1%\" attribute is not an allowed attribute of the <%2%> element.";  
    m_Messages[ ERROR_XML_REQUIRED_ATTRIBUTE_MISSING ] =
        "The required attribute \"%1%\" is missing from the <%2%> element.";
    m_Messages[ ERROR_XML_ID_NOT_UNIQUE ] =
        "An element with ID value \"%1%\" already exists in the document.";
    m_Messages[ ERROR_XML_BAD_ID_VALUE ] =
        "An ID value of \"%1%\" is not a valid value for an ID.";
    m_Messages[ ERROR_XML_SPECIFIES_NEITHER_UTF8_NOR_UTF16 ] =
        "The file declares the use of the \"%1%\" encoding, but only UTF-8 and UTF-16 are allowed."; 
    m_Messages[ ERROR_XML_BYTESTREAM_NEITHER_UTF8_NOR_UTF16 ] =
        "The file declares the use of UTF-8 or UTF-16, but the bytestream of the file's contents "
        "does not match either encoding.";   

    m_Messages[ ERROR_OPF_PACKAGE_NOT_ROOT ] =
        "The <package> element is not the root element.";
    m_Messages[ ERROR_OPF_IDREF_ID_DOES_NOT_EXIST ] =
        "The <itemref> element's \"idref\" attribute is pointing to an <item> with id=\"%1%\" which "
        "does not exist in the <manifest>.";
    m_Messages[ ERROR_OPF_IDREF_NOT_UNIQUE ] =
        "The <itemref> element's \"idref\" attribute has value \"%1%\", but that value is already "
        "in use in a previous <itemref>.";    
    m_Messages[ ERROR_OPF_BAD_SPINE_TOC_VALUE ] =
        "The <spine> element's \"toc\" attribute is pointing to an <item> with id=\"%1%\" which "
        "does not exist in the <manifest>.";
    m_Messages[ ERROR_OPF_PACKAGE_UNIQUE_IDENTIFIER_DOES_NOT_EXIST ] =
        "The <package> element's \"unique-identifier\" attribute is pointing to an <identifier> "
        "with id=\"%1%\" which does not exist in the <metadata>.";
    m_Messages[ ERROR_OPF_BAD_PACKAGE_VERSION ] =
        "The <package> element's \"version\" attribute value needs to be \"%1%\", but is \"%2%\".";
    m_Messages[ ERROR_OPF_BAD_ITEM_LINEAR_VALUE ] =
        "The <item> element's \"linear\" attribute has value \"%1%\", but only \"yes\" and \"no\" "
        "are allowed.";
    m_Messages[ ERROR_OPF_BAD_ITEM_MEDIA_TYPE_VALUE ] =
        "The <item> element's \"media-type\" attribute has value \"%1%\", but the file's "
        "media type is \"%2%\".";    
    m_Messages[ ERROR_OPF_BAD_CREATOR_OR_CONTRIBUTOR_ROLE_VALUE ] =
        "The \"role\" value \"%1%\" is not a registered MARC value. Either use a valid MARC "
        "relator or start your relator with \"oth.\".";
    m_Messages[ ERROR_OPF_BAD_REFERENCE_TYPE_VALUE ] =
        "The <reference> element's \"type\" attribute has value \"%1%\" which is not an "
        "OPF-specified value. Either use a predefined type or start your type with \"other.\".";
    m_Messages[ ERROR_OPF_BAD_DATE_VALUE ] =
        "The <date> element's value is \"%1%\", which is not a valid date format.";    
    m_Messages[ ERROR_OPF_ITEM_HREF_INVALID_URI ] =
        "The <item> element's \"href\" attribute value is \"%1%\", which is not a valid URI.";
    m_Messages[ ERROR_OPF_ITEM_HREF_HAS_FRAGMENT ] =
        "The <item> element's \"href\" attribute value is \"%1%\", which contains "
        "a fragment identifier.";
    m_Messages[ ERROR_OPF_ITEM_HREF_NOT_UNIQUE ] =
        "The <item> element's \"href\" attribute value is \"%1%\" "
        "which is already in use in a previous <item>.";
    m_Messages[ ERROR_OPF_ITEM_REQMOD_WITHOUT_REQNS ] =
        "The <item> element has a \"required-modules\" attribute but doesn't have a "
        "\"required-namespace\" attribute.";
    m_Messages[ ERROR_OPF_ITEM_FILE_DOESNT_EXIST ] =
        "The <item> element's \"href\" attribute points to file \"%1%\" "
        "which does not exist.";      
    m_Messages[ ERROR_OPF_NCX_NOT_PRESENT ] =
        "There is no <item> element in the <manifest> that has the NCX mimetype "
        "(\"application/x-dtbncx+xml\"). The use of an NCX is mandatory in EPUB.";
    m_Messages[ ERROR_OPF_REACHABLE_OPS_DOC_NOT_IN_SPINE ] =
        "This OPS document is reachable but not present in the OPF <spine>. "
        "\"Reachable\" means that a reference of some kind that points to this resource exists in the epub."; 
    m_Messages[ ERROR_OPF_REACHABLE_RESOURCE_NOT_IN_MANIFEST ] =
        "This resource is reachable but not present in the OPF <manifest>. "
        "\"Reachable\" means that a reference of some kind that points to this resource exists in the epub."; 

    m_Messages[ ERROR_NCX_CONTENT_FILE_DOES_NOT_EXIST ] =
        "This <content> element's \"src\" attribute value is \"%1%\", but that file does not exist.";
    m_Messages[ ERROR_NCX_CONTENT_FRAGMENT_DOES_NOT_EXIST ] =
        "This <content> element's \"src\" attribute value is \"%1%\", but an element with an ID the "
        "fragment is referring to does not exist in that file.";

    m_Messages[ ERROR_XHTML_BAD_DTD ] =
        std::string( "The file specifies an incorrect DTD. The correct public ID for the DTD is \"" ) +
         XHTML11_PUBLIC_ID + "\", while the correct system ID is \"" + XHTML11_SYSTEM_ID + "\". "
         "Do note that using a DTD is optional; but if used, it must be correct.";

    m_Messages[ WARNING_GENERIC ] =
        "Warning."; 
    m_Messages[ WARNING_OPF_RESOURCE_IN_MANIFEST_NOT_REACHABLE ] =
        "This resource is present in the OPF <manifest>, but it's not reachable (it's unused).";
}

} // namespace FlightCrew