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

#include <QtCore/QObject>
#include <QtCore/QMutexLocker>
#include <QtCore/QString>

#include "GuideSemantics.h"

QMutex GuideSemantics::s_AccessMutex;
GuideSemantics *GuideSemantics::m_Instance = NULL;

GuideSemantics &GuideSemantics::Instance()
{
    // We use a static local variable
    // to hold our singleton instance; using a pointer member
    // variable creates problems with object destruction;
    QMutexLocker locker(&s_AccessMutex);

    if (!m_Instance) {
        static GuideSemantics guide_semantics;
        m_Instance = &guide_semantics;
    }

    return *m_Instance;
}


const QHash<int, std::tuple<QString, QString>> &GuideSemantics::GetGuideTypeMapping()
{
    return m_GuideTypeMapping;
}

QString GuideSemantics::GetGuideName(GuideSemantics::GuideSemanticType type)
{
    if (m_GuideTypeMapping.contains(type)) {
        QString abbrev;
        QString name;
        std::tie(abbrev, name) = m_GuideTypeMapping[type];
        return name;
    }

    return "";
}

GuideSemantics::GuideSemanticType GuideSemantics::MapReferenceTypeToGuideEnum(const QString &reference_type)
{
    return (GuideSemanticType) m_ReferenceTypeToGuideEnum.value(reference_type, NoType);
}


GuideSemantics::GuideSemantics()
{
    CreateGuideMapping();
    CreateReferenceTypeToGuideEnum();
}


void GuideSemantics::CreateGuideMapping()
{
    m_GuideTypeMapping[ GuideSemantics::Cover ]
        = std::make_tuple(QString("cover"),           QObject::tr("Cover"));
    m_GuideTypeMapping[ GuideSemantics::TitlePage ]
        = std::make_tuple(QString("title-page"),      QObject::tr("Title Page"));
    m_GuideTypeMapping[ GuideSemantics::TableOfContents ]
        = std::make_tuple(QString("toc"),             QObject::tr("Table Of Contents"));
    m_GuideTypeMapping[ GuideSemantics::Index ]
        = std::make_tuple(QString("index"),           QObject::tr("Index"));
    m_GuideTypeMapping[ GuideSemantics::Glossary ]
        = std::make_tuple(QString("glossary"),        QObject::tr("Glossary"));
    m_GuideTypeMapping[ GuideSemantics::Acknowledgements ]
        = std::make_tuple(QString("acknowledgements"), QObject::tr("Acknowledgements"));
    m_GuideTypeMapping[ GuideSemantics::Bibliography ]
        = std::make_tuple(QString("bibliography"),    QObject::tr("Bibliography"));
    m_GuideTypeMapping[ GuideSemantics::Colophon ]
        = std::make_tuple(QString("colophon"),        QObject::tr("Colophon"));
    m_GuideTypeMapping[ GuideSemantics::CopyrightPage ]
        = std::make_tuple(QString("copyright-page"),  QObject::tr("Copyright Page"));
    m_GuideTypeMapping[ GuideSemantics::Dedication ]
        = std::make_tuple(QString("dedication"),      QObject::tr("Dedication"));
    m_GuideTypeMapping[ GuideSemantics::Epigraph ]
        = std::make_tuple(QString("epigraph"),        QObject::tr("Epigraph"));
    m_GuideTypeMapping[ GuideSemantics::Foreword ]
        = std::make_tuple(QString("foreword"),        QObject::tr("Foreword"));
    m_GuideTypeMapping[ GuideSemantics::ListOfIllustrations ]
        = std::make_tuple(QString("loi"),             QObject::tr("List Of Illustrations"));
    m_GuideTypeMapping[ GuideSemantics::ListOfTables ]
        = std::make_tuple(QString("lot"),             QObject::tr("List Of Tables"));
    m_GuideTypeMapping[ GuideSemantics::Notes ]
        = std::make_tuple(QString("notes"),           QObject::tr("Notes"));
    m_GuideTypeMapping[ GuideSemantics::Preface ]
        = std::make_tuple(QString("preface"),         QObject::tr("Preface"));
    m_GuideTypeMapping[ GuideSemantics::Text ]
        = std::make_tuple(QString("text"),            QObject::tr("Text"));
}

void GuideSemantics::CreateReferenceTypeToGuideEnum()
{
    m_ReferenceTypeToGuideEnum[ "cover"            ] = Cover;
    m_ReferenceTypeToGuideEnum[ "title-page"       ] = TitlePage;
    m_ReferenceTypeToGuideEnum[ "toc"              ] = TableOfContents;
    m_ReferenceTypeToGuideEnum[ "index"            ] = Index;
    m_ReferenceTypeToGuideEnum[ "glossary"         ] = Glossary;
    m_ReferenceTypeToGuideEnum[ "acknowledgements" ] = Acknowledgements;
    m_ReferenceTypeToGuideEnum[ "bibliography"     ] = Bibliography;
    m_ReferenceTypeToGuideEnum[ "colophon"         ] = Colophon;
    m_ReferenceTypeToGuideEnum[ "copyright-page"   ] = CopyrightPage;
    m_ReferenceTypeToGuideEnum[ "dedication"       ] = Dedication;
    m_ReferenceTypeToGuideEnum[ "epigraph"         ] = Epigraph;
    m_ReferenceTypeToGuideEnum[ "foreword"         ] = Foreword;
    m_ReferenceTypeToGuideEnum[ "loi"              ] = ListOfIllustrations;
    m_ReferenceTypeToGuideEnum[ "lot"              ] = ListOfTables;
    m_ReferenceTypeToGuideEnum[ "notes"            ] = Notes;
    m_ReferenceTypeToGuideEnum[ "preface"          ] = Preface;
    m_ReferenceTypeToGuideEnum[ "text"             ] = Text;
}
