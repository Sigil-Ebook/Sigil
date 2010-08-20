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
#ifndef GUIDESEMANTICS_H
#define GUIDESEMANTICS_H

#include <QMutex>
#include <QHash>

// This needs to be here even though it's in stdafx.h
// because this file is included in HTMLResource.h, and
// that file gets a moc_*.cpp file that does not use stdafx.h
#include <boost/tuple/tuple.hpp>
using boost::tuple;


/**
 * Singleton storing information about <guide> semantic information.
 */
class GuideSemantics
{

public:

    /**
    * Represents all the semantic types
    * for the <guide> element.
    *
    * @see http://www.idpf.org/2007/opf/OPF_2.0_final_spec.html#Section2.6
    */
    enum GuideSemanticType
    {
        NoType = -1,
        Cover,
        TitlePage,
        TableOfContents,
        Index,
        Glossary, 	
        Acknowledgments, 	
        Bibliography,
        Colophon,
        CopyrightPage, 	
        Dedication,
        Epigraph,
        Foreword,	
        ListOfIllustrations,
        ListOfTables,
        Notes,
        Preface, 	
        Text
    };

    /**
     * Returns a reference to the instance of the singleton.
     *
     * @return The reference.
     */
    static GuideSemantics& Instance();

    /**
     * Returns a reference to m_GuideTypeMapping
     * 
     * @return The reference.
     */
    const QHash< int, tuple< QString, QString > >& GetGuideTypeMapping();

    /**
     * Maps a reference type string ("loi", "cover" etc.)
     * to a value in the GuideSemanticType enum.
     *
     * @param reference_type The type string.
     * @return The corresponding value in the enum.
     */
    GuideSemanticType MapReferenceTypeToGuideEnum( const QString &reference_type );

private:

    /**
     * Constructor. Private because this is a singleton class.
     */
    GuideSemantics();

    /**
     * Creates m_GuideTypeMapping.
     */
    void CreateGuideMapping();

    /**
     * Creates m_ReferenceTypeToGuideEnum.
     */
    void CreateReferenceTypeToGuideEnum();


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * The mutex guarding the ceation of the singleton instance.
     */
    static QMutex s_AccessMutex;

    /**
     * A pointer to the singleton instance.
     */
    static GuideSemantics *m_Instance;

    /**
     * A mapping between GuideSemanticType
     * and the reference type and default title.
     */
    QHash< int, tuple< QString, QString > > m_GuideTypeMapping;

    /**
     * A mapping of a reference type string ("loi", "cover" etc.)
     * to a value in the GuideSemanticType enum.
     */
    QHash< QString, int > m_ReferenceTypeToGuideEnum;
};


#endif // GUIDESEMANTICS_H