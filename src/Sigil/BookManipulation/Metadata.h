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

#pragma once
#ifndef METADATA_H
#define METADATA_Hc

#include <QtCore/QHash>
#include <QtCore/QVariant>
#include <QtCore/QObject>

#include "BookManipulation/XercesHUse.h"

class QMutex;
class QString;

class Metadata : public QObject
{	
    Q_OBJECT

public:

    struct MetaInfo
    {
        // The user-friendly name of the entry
        QString name;

        // The description of the entry
        QString description;
    };

    struct MetaElement
    {
        // The name of the element
        QString name;

        // The value of the element
        QVariant value;

        // The value of a role's file-as, or the event or identifier name
        QString file_as;

        // If entry is a role, value is contributor or creator
        QString role_type;

        // The attributes of the element;
        // the keys are the attribute names,
        // the values are the attribute values
        QHash< QString, QString > attributes;
    };

    static Metadata& Instance();

    const QHash< QString, MetaInfo >& GetRelatorMap();
    const QHash< QString, MetaInfo >& GetBasicMetaMap();

    bool IsRelator( QString code );

    QString GetText( QString text );
    QString GetName( QString code );
    QString GetCode( QString name );

    /**
     * Maps DC and <meta> metadata elements to "internal" MetaElements.
     * Accepts both DublinCore metadata elements like one would find in an 
     * OPF and custom <meta> elements like one would find in an HTML file.
     * 
     * @param element The element to convert.
     * @return The converted MetaElement.
     */
    MetaElement MapToBookMetadata( const xc::DOMElement &element );

private:

    // Maps Dublic Core metadata to internal book meta format
    MetaElement MapToBookMetadata( const MetaElement &meta, bool is_dc_element );

    // Constructor is private because
    // this is a singleton class
    Metadata();

    // Loads miscellaneous field names/values for translation
    void LoadText();

    // Loads the basic metadata and their descriptions from disk
    void LoadBasicMetadata();

    // Loads the relator codes, their full names,
    // and their descriptions from disk
    void LoadRelatorCodes();

    // Converts HTML sourced Dublin Core metadata to OPF style metadata
    MetaElement HtmlToOpfDC( const MetaElement &meta );

    // Converts free form metadata into internal book metadata
    MetaElement FreeFormMetadata( const MetaElement &meta );

    // Converts dc:creator and dc:contributor metadata to book internal metadata
    MetaElement CreateContribMetadata( const MetaElement &meta );

    // Converts dc:date metadata to book internal metadata
    MetaElement DateMetadata( const MetaElement &meta );

    // Converts dc:identifier metadata to book internal metadata
    MetaElement IdentifierMetadata( const MetaElement &meta );


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    static QMutex s_AccessMutex;

    static Metadata *m_Instance;

    // The keys are the untranslated Dublin Core element types
    // and the values are the MetaInfo structures
    // (see http://www.idpf.org/2007/opf/OPF_2.0_final_spec.html#Section2.2 );
    QHash< QString, MetaInfo > m_Basic;

    // The keys are the Dublin Core element user-friendly names
    // the values are the element types for those names
    QHash< QString, QString > m_BasicFullNames;

    // The keys are the MARC relator codes
    // and the values are the MetaInfo structures
    // (see http://www.loc.gov/marc/relators/relaterm.html );
    QHash< QString, MetaInfo > m_Relators;

    // The keys are the full relator names
    // the values are the MARC relator codes
    // (e.g. aut -> Author )
    QHash< QString, QString > m_RelatorFullNames;

    // The keys are special field names
    // the values are the text to display
    // (e.g. date -> Date )
    QHash< QString, QString > m_Text;

};

#endif // METADATA_H

