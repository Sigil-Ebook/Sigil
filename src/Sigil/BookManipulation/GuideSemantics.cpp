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
#include "GuideSemantics.h"

QMutex GuideSemantics::s_AccessMutex;
GuideSemantics* GuideSemantics::m_Instance = NULL;

GuideSemantics& GuideSemantics::Instance()
{
    // We use a static local variable
    // to hold our singleton instance; using a pointer member
    // variable creates problems with object destruction;

    QMutexLocker locker( &s_AccessMutex );

    if ( !m_Instance )
    {
        static GuideSemantics guide_semantics;
        m_Instance = &guide_semantics;
    }

    return *m_Instance;
}


const QHash< int, tuple< QString, QString > >& GuideSemantics::GetGuideTypeMapping()
{
    return m_GuideTypeMapping;
}


GuideSemantics::GuideSemanticType GuideSemantics::MapReferenceTypeToGuideEnum( const QString &reference_type )
{
    return (GuideSemanticType) m_ReferenceTypeToGuideEnum.value( reference_type, NoType );
}


GuideSemantics::GuideSemantics()
{
    CreateGuideMapping();
    CreateReferenceTypeToGuideEnum();
}


void GuideSemantics::CreateGuideMapping()
{
    m_GuideTypeMapping[ GuideSemantics::Cover ]           
        = make_tuple( QString( "cover" ),           QString( "Cover" ) );

    m_GuideTypeMapping[ GuideSemantics::TitlePage ]       
        = make_tuple( QString( "title-page" ),      QString( "Title Page" ) );

    m_GuideTypeMapping[ GuideSemantics::TableOfContents ] 
        = make_tuple( QString( "toc" ),             QString( "Table Of Contents" ) );

    m_GuideTypeMapping[ GuideSemantics::Index ] 
        = make_tuple( QString( "index" ),           QString( "Index" ) );

    m_GuideTypeMapping[ GuideSemantics::Glossary ] 
        = make_tuple( QString( "glossary" ),        QString( "Glossary" ) );

    m_GuideTypeMapping[ GuideSemantics::Acknowledgments ] 
        = make_tuple( QString( "acknowledgements" ), QString( "Acknowledgements" ) );

    m_GuideTypeMapping[ GuideSemantics::Bibliography ] 
        = make_tuple( QString( "bibliography" ),    QString( "Bibliography" ) );

    m_GuideTypeMapping[ GuideSemantics::Colophon ] 
        = make_tuple( QString( "colophon" ),        QString( "Colophon" ) );

    m_GuideTypeMapping[ GuideSemantics::CopyrightPage ] 
        = make_tuple( QString( "copyright-page" ),  QString( "Copyright Page" ) );

    m_GuideTypeMapping[ GuideSemantics::Dedication ] 
        = make_tuple( QString( "dedication" ),      QString( "Dedication" ) );

    m_GuideTypeMapping[ GuideSemantics::Epigraph ] 
        = make_tuple( QString( "epigraph" ),        QString( "Epigraph" ) );

    m_GuideTypeMapping[ GuideSemantics::Foreword ] 
        = make_tuple( QString( "foreword" ),        QString( "Foreword" ) );

    m_GuideTypeMapping[ GuideSemantics::ListOfIllustrations ] 
        = make_tuple( QString( "loi" ),             QString( "List Of Illustrations" ) );

    m_GuideTypeMapping[ GuideSemantics::ListOfTables ] 
        = make_tuple( QString( "lot" ),             QString( "List Of Tables" ) );

    m_GuideTypeMapping[ GuideSemantics::Notes ] 
        = make_tuple( QString( "notes" ),           QString( "Notes" ) );

    m_GuideTypeMapping[ GuideSemantics::Preface ] 
        = make_tuple( QString( "preface" ),         QString( "Preface" ) );

    m_GuideTypeMapping[ GuideSemantics::Text ] 
        = make_tuple( QString( "text" ),            QString( "Text" ) );
}

void GuideSemantics::CreateReferenceTypeToGuideEnum()
{
    m_ReferenceTypeToGuideEnum[ "cover"           ] = Cover;
    m_ReferenceTypeToGuideEnum[ "title-page"      ] = TitlePage;
    m_ReferenceTypeToGuideEnum[ "toc"             ] = TableOfContents;
    m_ReferenceTypeToGuideEnum[ "index"           ] = Index;
    m_ReferenceTypeToGuideEnum[ "glossary"        ] = Glossary;
    m_ReferenceTypeToGuideEnum[ "acknowledgments" ] = Acknowledgments;
    m_ReferenceTypeToGuideEnum[ "bibliography"    ] = Bibliography;
    m_ReferenceTypeToGuideEnum[ "colophon"        ] = Colophon;
    m_ReferenceTypeToGuideEnum[ "copyright-page"  ] = CopyrightPage;
    m_ReferenceTypeToGuideEnum[ "dedication"      ] = Dedication;
    m_ReferenceTypeToGuideEnum[ "epigraph"        ] = Epigraph;
    m_ReferenceTypeToGuideEnum[ "foreword"        ] = Foreword;
    m_ReferenceTypeToGuideEnum[ "loi"             ] = ListOfIllustrations;
    m_ReferenceTypeToGuideEnum[ "lot"             ] = ListOfTables;
    m_ReferenceTypeToGuideEnum[ "notes"           ] = Notes;
    m_ReferenceTypeToGuideEnum[ "preface"         ] = Preface;
    m_ReferenceTypeToGuideEnum[ "text"            ] = Text;
}
