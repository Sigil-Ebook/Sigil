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
#ifndef OPFRESOURCE_H
#define OPFRESOURCE_H

#include "XMLResource.h"
#include "BookManipulation/GuideSemantics.h"

// Needed because the moc_* version of this file doesn't include stdafx.h
#include <boost/shared_ptr.hpp>


class OPFResource : public XMLResource 
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param fullfilepath The full path to the file that this
     *                     resource is representing.
     * @param parent The object's parent.
     */
    OPFResource( const QString &fullfilepath, QObject *parent = NULL );

    // inherited

    virtual bool RenameTo( const QString &new_filename );
    
    virtual ResourceType Type() const;

    GuideSemantics::GuideSemanticType GetGuideSemanticTypeForResource( const Resource &resource );

    QString GetCoverPageOEBPSPath();

public slots:

    void AddResource( const Resource &resource );

    void RemoveResource( const Resource &resource );

    void AddGuideSemanticType( const Resource &resource, GuideSemantics::GuideSemanticType new_type );

private:

    void AppendToSpine( const QString &id, xc::DOMDocument &document );

    void RemoveFromSpine( const QString &id, xc::DOMDocument &document );

    boost::shared_ptr< xc::DOMDocument > GetDocument();

    xc::DOMElement& GetManifestElement( const xc::DOMDocument &document );
    
    xc::DOMElement& GetSpineElement( const xc::DOMDocument &document );

    xc::DOMElement& GetGuideElement( xc::DOMDocument &document );

    // CAN BE NULL! NULL means no reference for resource
    xc::DOMElement* GetGuideReferenceForResource( 
        const Resource &resource, 
        xc::DOMDocument &document );

    void RemoveGuideReferenceForResource( 
        const Resource &resource, 
        xc::DOMDocument &document );

    GuideSemantics::GuideSemanticType GetGuideSemanticTypeForResource( 
        const Resource &resource, 
        xc::DOMDocument &document );

    void SetGuideSemanticTypeForResource(
        GuideSemantics::GuideSemanticType type,
        const Resource &resource, 
        xc::DOMDocument &document );

    void RemoveDuplicateGuideTypes(
        GuideSemantics::GuideSemanticType new_type, 
        xc::DOMDocument &document );

    void FillWithDefaultText();

    QString GetResourceMimetype( const Resource &resource );

    /**
     * Initializes m_Mimetypes.
     */
    void CreateMimetypes();

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * A mapping between file extensions
     * and appropriate MIME types.
     */
    QHash< QString, QString > m_Mimetypes;

};

#endif // OPFRESOURCE_H
