/************************************************************************
 **
 **  Copyright (C) 2019 Kevin B. Hendricks Stratford, ON, Canada 
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

#ifndef TEXT_DOCUMENT
#define TEXT_DOCUMENT

#include <QString>
#include <QTextDocument>

class TextDocument : public QTextDocument
{
    Q_OBJECT

    public:

  TextDocument(QObject *parent = 0);
  ~TextDocument() {} ;

  // QTextDocument toPlainText() is broken in that it replaces a valid
  // unicode character (the non-breaking space) with a normal space
  // for no good reason, which is horrible for those coding.

  // In Qt 5.9.0 someone at Qt woke up and added toRawText() (Thank You!)
  // but we need a way to do it for all Qt 5 versions

  // Also in QTextDocument toPlainText is NOT marked virtual making proper
  // base and derived class aware overriding impossible
  
  // So introduce a new routine to do what toPlainText() should have

  QString toText();

};

#endif
