/************************************************************************
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include <QRawFont>
#include <QFont>

#include "Misc/Utility.h"
#include "ResourceObjects/FontResource.h"

FontResource::FontResource(const QString &mainfolder, const QString &fullfilepath, QObject *parent)
    : Resource(mainfolder, fullfilepath, parent)
{
}


Resource::ResourceType FontResource::Type() const
{
    return Resource::FontResourceType;
}


QString FontResource::GetObfuscationAlgorithm() const
{
    return m_ObfuscationAlgorithm;
}

QString FontResource::GetDescription() const
{
    QRawFont rawfont(GetFullPath(), 16.0);
    QString desc = rawfont.familyName();
    QString weight_name;
    QString style_name;
    if (rawfont.weight() <  QFont::ExtraLight)      weight_name = " Thin";
    else if (rawfont.weight() <  QFont::Light)      weight_name = " ExtraLight";
    else if (rawfont.weight() <  QFont::Normal)     weight_name = " Light";
    else if (rawfont.weight() <  QFont::Medium)     weight_name = "";
    else if (rawfont.weight() <  QFont::DemiBold)   weight_name = " Medium";
    else if (rawfont.weight() <  QFont::Bold)       weight_name = " DemiBold";
    else if (rawfont.weight() <  QFont::ExtraBold)  weight_name = " Bold";
    else if (rawfont.weight() <  QFont::Black)      weight_name = " ExtraBold";
    else if (rawfont.weight() >= QFont::Black)      weight_name = " Black";
    if (desc != "") {
        if (desc.contains(weight_name)) weight_name = "";
        desc = desc + weight_name;
    }

#ifdef Q_OS_WIN32
    if (rawfont.style()      == QFont::StyleItalic)  style_name = " Italic";
    else if (rawfont.style() == QFont::StyleOblique) style_name = " Oblique";
    else style_name = "";
#else
    style_name = " " + rawfont.styleName();
#endif
    if (desc != "") desc = desc + style_name;
    else desc = tr("No reliable font data");
    return desc;
}

void FontResource::SetObfuscationAlgorithm(const QString &algorithm)
{
    m_ObfuscationAlgorithm = algorithm;
}

bool FontResource::LoadFromDisk()
{
    emit Modified();
    return true;
}
