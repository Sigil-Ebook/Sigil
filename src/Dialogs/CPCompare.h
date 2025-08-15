/************************************************************************
 **
 **  Copyright (C) 2020-2025 Kevin B. Hendricks, Stratford Ontario Canada
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

#ifndef CPCOMPARE_H
#define CPCOMPARE_H

#include <QString>
#include <QStringList>
#include <QWidget>
#include <QDialog>

class QToolButton;
class QVBoxLayout;
class ListSelector;
class ViewImage;
class QEventLoop;

class CPCompare : public QDialog

{
    Q_OBJECT

public:

    CPCompare(const QString& bookroot, 
              const QString& cpdir, 
              const QStringList& dlist,
              const QStringList& alist,
              const QStringList& mlist,
              QWidget* parent);

    ~CPCompare();

public slots:
    int exec();
    void reject();
    void accept();
    void handle_del_request();
    void handle_add_request();
    void handle_mod_request();

    // protected:
    //void keyPressEvent(QKeyEvent * ev);

private:
    void ReadSettings();
    void WriteSettings();
    void connectSignalsToSlots();
    void handle_cleanup();

    QString       m_bookroot;
    QString       m_cpdir;
    QToolButton*  m_bp;
    ListSelector* m_dlist;
    ListSelector* m_alist;
    ListSelector* m_mlist;
    QVBoxLayout*  m_layout;
    // create own exec that creates its QEventLoop on the heap, and
    // create an unused QWebEngineView and do a setUrl() on it.
    // horrible hack to deal with QTBUG-138687, QTBUG-135002, QTBUG-139109
    // this forces any reparenting for QWebEngine to happen early before exec
    // and before any non webengine widget is dynamically created to prevent
    // parent child loss from reparenting impacting QDialog modality
    QEventLoop*   m_loop;
    ViewImage*    m_vi;
};
#endif
