/************************************************************************
**
**  Copyright (C) 2025 Kevin B. Hendricks Stratford, ON Canada 
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
#ifndef METAEDITORTREEVIEW_H
#define METAEDITORTREEVIEW_H

#include <QAbstractItemView>
#include <QTreeView>
#include <QKeyEvent>

class MetaEditorTreeView : public QTreeView
{
    Q_OBJECT

public:
  
  MetaEditorTreeView(QWidget * parent = 0);
  ~MetaEditorTreeView() { };
  
protected:
  void keyPressEvent(QKeyEvent* event) override;

};

#endif // METAEDITORTREEVIEW_H
