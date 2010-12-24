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

#pragma once
#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace FlightCrew
{
 
extern const std::string MAIN_XML_NAMESPACE;
extern const std::string OPF_XML_NAMESPACE;
extern const std::string DC_XML_NAMESPACE;
extern const std::string CONTAINER_XML_NAMESPACE;
extern const std::string NCX_XML_NAMESPACE;

extern const std::string OEBPS_MIME;  
extern const std::string XHTML_MIME; 
extern const std::string NCX_MIME;    
extern const std::string PNG_MIME;    
extern const std::string GIF_MIME;   
extern const std::string JPEG_MIME;   
extern const std::string SVG_MIME;    
extern const std::string DTBOOK_MIME; 
extern const std::string CSS_MIME;    
extern const std::string XML_MIME;    
extern const std::string XPGT_MIME;   
extern const std::string OTF_MIME;    
extern const std::string TTF_MIME;    
extern const std::string OEB_DOC_MIME;
extern const std::string OEB_CSS_MIME;

extern const std::string UNKNOWN_MIME;

extern const std::string XHTML11_SYSTEM_ID;
extern const std::string XHTML11_PUBLIC_ID;

extern const QName DC_METADATA_QNAME;
extern const QName X_METADATA_QNAME; 
extern const QName TITLE_QNAME;      
extern const QName LANGUAGE_QNAME;   
extern const QName IDENTIFIER_QNAME; 
extern const QName CREATOR_QNAME;    
extern const QName SUBJECT_QNAME;    
extern const QName DESCRIPTION_QNAME;
extern const QName PUBLISHER_QNAME;  
extern const QName CONTRIBUTOR_QNAME;
extern const QName DATE_QNAME;       
extern const QName TYPE_QNAME;       
extern const QName FORMAT_QNAME;     
extern const QName SOURCE_QNAME;     
extern const QName RELATION_QNAME;   
extern const QName COVERAGE_QNAME;   
extern const QName RIGHTS_QNAME;   
extern const QName META_QNAME;   

extern const char*         XHTML11_FLAT_DTD_ID;
extern const unsigned int  XHTML11_FLAT_DTD_LEN;
extern const unsigned char XHTML11_FLAT_DTD[];

extern const char*         OPS201_XSD_NS;
extern const char*         OPS201_XSD_ID; 
extern const unsigned int  OPS201_XSD_LEN;
extern const unsigned char OPS201_XSD[];

extern const char*         OPS_SWITCH_XSD_ID; 
extern const unsigned int  OPS_SWITCH_XSD_LEN;
extern const unsigned char OPS_SWITCH_XSD[];  

extern const char*         SVG11_XSD_ID; 
extern const unsigned int  SVG11_XSD_LEN;
extern const unsigned char SVG11_XSD[];  

extern const char*         XLINK_XSD_ID; 
extern const unsigned int  XLINK_XSD_LEN;
extern const unsigned char XLINK_XSD[]; 

extern const char*         XML_XSD_ID; 
extern const unsigned int  XML_XSD_LEN;
extern const unsigned char XML_XSD[];  

extern const char*         CONTAINER_XSD_NS;
extern const char*         CONTAINER_XSD_ID; 
extern const unsigned int  CONTAINER_XSD_LEN;
extern const unsigned char CONTAINER_XSD[];  

extern const char*         ENCRYPTION_XSD_ID; 
extern const unsigned int  ENCRYPTION_XSD_LEN;
extern const unsigned char ENCRYPTION_XSD[];  

extern const char*         SIGNATURES_XSD_ID; 
extern const unsigned int  SIGNATURES_XSD_LEN;
extern const unsigned char SIGNATURES_XSD[];  

extern const char*         XENC_SCHEMA_XSD_ID; 
extern const unsigned int  XENC_SCHEMA_XSD_LEN;
extern const unsigned char XENC_SCHEMA_XSD[];  

extern const char*         XMLDSIG_CORE_SCHEMA_XSD_ID; 
extern const unsigned int  XMLDSIG_CORE_SCHEMA_XSD_LEN;
extern const unsigned char XMLDSIG_CORE_SCHEMA_XSD[];  

extern const char*         NCX_XSD_NS; 
extern const char*         NCX_XSD_ID; 
extern const unsigned int  NCX_XSD_LEN;
extern const unsigned char NCX_XSD[]; 

extern const char*         NCX_2005_1_DTD_ID; 
extern const unsigned int  NCX_2005_1_DTD_LEN;
extern const unsigned char NCX_2005_1_DTD[];  

}

#endif // CONSTANTS_H
