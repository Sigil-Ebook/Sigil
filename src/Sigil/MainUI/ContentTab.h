/************************************************************************
**
**  Copyright (C) 2009  Strahinja Markovic
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
#ifndef CONTENTTAB_H
#define CONTENTTAB_H

#include <QWidget>
#include <QString>

class QLayout;
class Searchable;

class ContentTab : public QWidget
{
    Q_OBJECT

public:

    ContentTab( const QString &filepath, QWidget *parent = 0 );

    virtual ~ContentTab() {}

    const QString & GetFilepath();

    void UpdateFilepath( const QString &filepath );

    virtual bool IsModified()           { return false; }

    virtual bool CutEnabled()           { return false; }
    virtual bool CopyEnabled()          { return false; }
    virtual bool PasteEnabled()         { return false; }
    virtual bool BoldChecked()          { return false; }
    virtual bool ItalicChecked()        { return false; }
    virtual bool UnderlineChecked()     { return false; }
    virtual bool StrikethroughChecked() { return false; }
    virtual bool BulletListChecked()    { return false; }
    virtual bool NumberListChecked()    { return false; }

    virtual bool BookViewChecked()      { return false; }
    virtual bool SplitViewChecked()     { return false; }
    virtual bool CodeViewChecked()      { return false; }

    virtual QString GetCaretElementName() { return "";  }

    virtual float GetZoomFactor()       { return 1.0;   }
    virtual void SetZoomFactor( float new_zoom_factor ) { }

    virtual Searchable* GetSearchableContent();

protected:

    QString m_Filepath;

    QLayout &m_Layout;
};

#endif // CONTENTTAB_H


