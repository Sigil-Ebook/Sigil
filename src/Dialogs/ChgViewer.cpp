/************************************************************************
 **
 **  Copyright (C) 2020 Kevin B. Hendricks, Stratford Ontario Canada
 **  Copyright (C) 2020 Doug Massay
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
#include <QString>
#include <QList>
#include <QDialog>
#include <QWidget>
#include <QScrollBar>
#include <QClipboard>
#include <QPlainTextEdit>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QKeySequence>
#include <QKeyEvent>
#include <QApplication>
#include <QDebug>

#include "ViewEditors/TextView.h"
#include "ViewEditors/Navigator.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "Dialogs/ChgViewer.h"

static const QString SETTINGS_GROUP = "change_viewer";

static const QChar _PAD               = QChar(0x2007); // "use a 'figure space' 8199

static const QString _redColor          = "#ffc4c4";
static const QString _darkredColor      = "#ff8282";
static const QString _grayColor         = "#dddddd";
static const QString _greenColor        = "#c9fcd6";
static const QString _darkgreenColor    = "#50c96e";


ChgViewer::ChgViewer(const QList<DiffRecord::DiffRec>& diffinfo, 
		     const QString&file1, const QString& file2, QWidget *parent)
    : QDialog(parent),
      m_view1(new TextView(this)),
      m_view2(new TextView(this)),
      m_lbl1(new QLabel(file1, this)),
      m_lbl2(new QLabel(file2, this)),
      m_nav(new Navigator(this)),
      m_diffinfo(diffinfo)
{
    // handle the layout manually
    m_layout = new QVBoxLayout(this);
    QHBoxLayout *hl = new QHBoxLayout();
    QVBoxLayout *vl1 = new QVBoxLayout();
    vl1->addWidget(m_lbl1);
    vl1->addWidget(m_view1);
    hl->addLayout(vl1);
    QVBoxLayout *vl2 = new QVBoxLayout();
    vl2->addWidget(m_lbl2);
    vl2->addWidget(m_view2);
    hl->addLayout(vl2);
    m_layout->addLayout(hl);
    m_layout->addWidget(m_nav);

    // need fixed width font for diff to show properly
    QFont tf = m_view1->font();
    tf.setFamily("Courier New");
    tf.setStyleHint(QFont::TypeWriter);
    m_view1->setFont(tf);
    m_view2->setFont(tf);

    ReadSettings();
    LoadViewers();
    connectSignalsToSlots();
}

ChgViewer::~ChgViewer()
{
    WriteSettings();
}

void ChgViewer::LoadViewers()
{
    int blockno = 0;
    int leftlineno = 1;
    int rightlineno = 1;
    QString pad = "" + _PAD;
    // codes: 0 = Similar, 1 = RightOnly, 2 = LeftOnly, 3 = Changed
    foreach(DiffRecord::DiffRec diff, m_diffinfo) {
	if (diff.code == "0") { // similar
	    m_view1->insertPlainText(diff.line + "\n");
	    m_view2->insertPlainText(diff.line + "\n");
	} else if (diff.code == "1") { // rightonly
	    m_changelst << blockno;
	    int n = diff.line.length();
	    m_view1->insert_with_background(pad.repeated(n) + "\n", _grayColor);
	    m_view2->insert_with_background(diff.line + "\n", _greenColor);
	} else if (diff.code == "2") { // leftonly
	    m_changelst << blockno;
            int n = diff.line.length();
            m_view1->insert_with_background(diff.line + "\n", _redColor);
	    m_view2->insert_with_background(pad.repeated(n) + "\n", _grayColor);
	} else if (diff.code == "3") { // changed
	    m_changelst << blockno;
            int l1 = diff.line.length();
	    int l2 = diff.newline.length();
            int n = std::max(l1, l2);
            int lc = diff.leftchanges.length();
	    int rc = diff.rightchanges.length();
	    for(int i=0; i < l1; i++) {
		QChar c = diff.line.at(i);
		if ((i < lc) && (diff.leftchanges.at(i) != " ")) {
		    m_view1->insert_with_background(c, _darkredColor);
		} else {
		    m_view1->insert_with_background(c, _redColor);
		}
	    }
	    m_view1->insertPlainText(pad.repeated(n-l1) + "\n");
	    for(int i=0; i < l2; i++) {
		QChar c = diff.newline.at(i);
		if ((i < rc) && (diff.rightchanges.at(i) != " ")) {
		    m_view2->insert_with_background(c, _darkgreenColor);
		} else {
		    m_view2->insert_with_background(c, _greenColor);
		}
	    }
	    m_view2->insertPlainText(pad.repeated(n-l2) + "\n");
	}
	blockno++;
	// map out block to line numbers
	if (diff.code == "2") { // leftonly
	    m_leftno << QString::number(leftlineno);
	    m_rightno << "";
	    leftlineno++;
	} else if (diff.code == "1") { // rightonly
	    m_rightno << QString::number(rightlineno);
	    m_leftno << "";
	    rightlineno++;
	} else { // 
	    m_leftno << QString::number(leftlineno);
	    m_rightno << QString::number(rightlineno);
	    leftlineno++;
	    rightlineno++;
	} 
    }
    m_view1->setBlockMap(m_leftno);
    m_view2->setBlockMap(m_rightno);

    synchronize_viewers();
}

void ChgViewer::cross_link_scrollbars(bool link)
{
    QScrollBar* sb1 = m_view1->GetVerticalScrollBar();
    QScrollBar* sb2 = m_view2->GetVerticalScrollBar();
    if (link) {
	connect(sb1, SIGNAL(valueChanged(int)), sb2, SLOT(setValue(int)));
	connect(sb2, SIGNAL(valueChanged(int)), sb1, SLOT(setValue(int)));
    } else {
	disconnect(sb1, SIGNAL(valueChanged(int)), sb2, SLOT(setValue(int)));
	disconnect(sb2, SIGNAL(valueChanged(int)), sb1, SLOT(setValue(int)));
    }
}

void ChgViewer::slideraction()
{
    bool f1 = m_view1->hasFocus();
    bool f2 = m_view2->hasFocus();
    int v1 = m_view1->GetVerticalScrollBar()->value();
    int v2 = m_view2->GetVerticalScrollBar()->value();
    if (v1 != v2) {
	if (f1) {
	    cross_link_scrollbars(false);
	    m_view2->GetVerticalScrollBar()->setValue(v1);
	    cross_link_scrollbars(true);
	}
	if (f2) {
	    cross_link_scrollbars(false);
	    m_view1->GetVerticalScrollBar()->setValue(v2);
	    cross_link_scrollbars(true);
	}
    }
}

void ChgViewer::synchronize_viewers()
{
    // set cursor to the top and start scroll there
    QTextCursor tc1 = m_view1->textCursor();
    tc1.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor,1);
    m_view1->setTextCursor(tc1);
    QTextCursor tc2 = m_view2->textCursor();
    tc2.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor,1);
    m_view2->setTextCursor(tc2);
    // start scrollbars at the top and cross link them
    m_view1->GetVerticalScrollBar()->setValue(0);
    m_view2->GetVerticalScrollBar()->setValue(0);
    cross_link_scrollbars(true);
}

void ChgViewer::next_change(int dir)
{
    TextView * viewer = nullptr;
    TextView * other = nullptr;
    // determine selected and other viewer
    if (m_nav->use_left_panel()) {
	viewer = m_view1;
	other = m_view2;
    } else {
	viewer = m_view2;
	other = m_view1;
    }
    // get block number of current viewer
    // and look for block number of prev and next changes
    int bnum = viewer->GetCursorBlockNumber();
    int prev = -1;
    int next = -1;
    foreach(int i, m_changelst) {
	if (i < bnum) prev = i;
	if (i > bnum) {
	    next = i;
	    break;
	}
    }
    int nnum = -1;
    if (dir < 0) {
	if (prev != -1) nnum = prev;
    } else {
	if (next != -1) nnum = next;
    }
    // if a next change exists, move both viewers to it
    if (nnum != -1) {
	cross_link_scrollbars(false);
        QTextCursor a = QTextCursor(viewer->document()->findBlockByNumber(nnum));
        int pos = a.position();
	QTextCursor nc = viewer->textCursor();
        nc.setPosition(pos);
        viewer->setTextCursor(nc);
        viewer->centerCursor();
        QTextCursor nd = other->textCursor();
        nd.setPosition(pos);
        other->setTextCursor(nd);
        other->centerCursor();
        cross_link_scrollbars(true);
    }
}

void ChgViewer::do_search(bool reverse)
{
    QString stext = m_nav->get_search_text();
    if (stext.simplified().isEmpty()) return;
    TextView * viewer = nullptr;
    TextView * other = nullptr;
    // determine selected and other viewer
    if (m_nav->use_left_panel()) {
	viewer = m_view1;
	other = m_view2;
    } else {
	viewer = m_view2;
	other = m_view1;
    }
    QTextDocument::FindFlags ff = QTextDocument::FindFlags();
    if (reverse) ff = ff | QTextDocument::FindBackward;

    // search will break scroll bar sync so unlink them first
    cross_link_scrollbars(false);
    bool found = viewer->find(stext, ff);
    if (found) {
	QTextCursor c = viewer->textCursor();
	viewer->centerCursor();
	QTextCursor d = other->textCursor();
	d.setPosition(c.position());
	other->setTextCursor(d);
	other->centerCursor();
    }
    cross_link_scrollbars(true);
}

void ChgViewer::keyPressEvent(QKeyEvent * ev)
{
    if ((ev->key() == Qt::Key_Enter) || (ev->key() == Qt::Key_Return)) return;

    if (ev->key() == Qt::Key_Slash) {
	m_nav->set_focus_on_search();
	return;
    }

    if (ev->matches(QKeySequence::Copy)) {
	QString text = m_view1->GetSelectedText() + m_view2->GetSelectedText();
	if (!text.isEmpty()) {
	    QApplication::clipboard()->setText(text);
	}
	return;
    }

    if (ev->matches(QKeySequence::FindNext)) {
	do_search(false);
        return;
    }
    if (ev->matches(QKeySequence::FindPrevious)) {
	do_search(true);
        return;
    }
    return QDialog::keyPressEvent(ev);
}

void ChgViewer::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }
    settings.endGroup();
}

void ChgViewer::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

int ChgViewer::exec()
{
    return QDialog::exec();
}

// should cover both escape key use and using x to close the runner dialog
void ChgViewer::reject()
{
    QDialog::reject();
}

void ChgViewer::connectSignalsToSlots()
{
    connect(m_nav, SIGNAL(NextChange(int)), this, SLOT(next_change(int)));
    connect(m_nav, SIGNAL(DoSearch(bool)),  this, SLOT(do_search(bool)));
    connect(m_nav, SIGNAL(DoDone()),        this, SLOT(accept()));
    connect(m_view1->GetVerticalScrollBar(), SIGNAL(actionTriggered(int)), this, SLOT(slideraction()));
    connect(m_view2->GetVerticalScrollBar(), SIGNAL(actionTriggered(int)), this, SLOT(slideraction()));
    connect(m_view1, SIGNAL(NextChange(int)), this, SLOT(next_change(int)));
    connect(m_view2, SIGNAL(NextChange(int)), this, SLOT(next_change(int)));
    
    // connect(ui.okButton, SIGNAL(clicked()), this, SLOT(accept()));
}
