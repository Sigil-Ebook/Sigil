/************************************************************************
 **
 **  Copyright (C) 2020-2021 Kevin B. Hendricks, Stratford Ontario Canada
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
#include <QScrollBar>
#include <QTextBlock>
#include <QKeySequence>
#include <QKeyEvent>
#include <QApplication>
#include <QDebug>

#include "Widgets/TextView.h"
#include "Widgets/Navigator2.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "MainUI/MainApplication.h"
#include "Dialogs/SourceViewer.h"

static const QString SETTINGS_GROUP = "source_viewer";

static const QStringList XML_EXTENSIONS = QStringList() << "htm" << "html" << "xhtml" <<
                                                           "ncx" << "opf" << "pls" << "smil" <<
                                                           "svg" << "ttml" << "xml" << "xpgt";

static const QStringList CSS_EXTENSIONS = QStringList() << "css";

SourceViewer::SourceViewer(const QString&file1, const QString& data, QWidget *parent)
    : QDialog(parent),
      m_view(new TextView(this)),
      m_lbl(new QLabel(file1, this)),
      m_nav(new Navigator2(this)),
      m_data(data),
      m_layout(new QVBoxLayout(this))
{
    setAttribute(Qt::WA_DeleteOnClose,true);
    QString ext = file1.split(".").last().toLower();
    // handle the layout manually
    m_layout->addWidget(m_lbl);
    m_layout->addWidget(m_view);
    m_layout->addWidget(m_nav);

    // need fixed width font for diff to show properly
    QFont tf = m_view->font();
    tf.setFamily("Courier New");
    tf.setStyleHint(QFont::TypeWriter);
    m_view->setFont(tf);

    ReadSettings();
    LoadViewer();
    m_hightype = TextView::Highlight_NONE;
    if (XML_EXTENSIONS.contains(ext)) {
        m_hightype = TextView::Highlight_XHTML;
        m_view->DoHighlightDocument(m_hightype);
    }
    if (CSS_EXTENSIONS.contains(ext)) {
        m_hightype = TextView::Highlight_CSS;
        m_view->DoHighlightDocument(m_hightype);
    }
    connectSignalsToSlots();
}

SourceViewer::~SourceViewer()
{
    WriteSettings();
}


// This is needed on macOS to force the 
// syntax highlighting to start from scratch
// when dark to light mode is switched dynamically
void SourceViewer::ReloadViewer()
{
    // This will force the TextView to delete its current XHTMLHighlighter and 
    // if needed install a freshly created one.  This is all because of a bug 
    // in QSyntaxHighlighting that can not detect color format changes alone 
    // if the exact same ranges were previously formatted
    m_view->Refresh(m_hightype);
}

void SourceViewer::LoadViewer()
{
    int blockno = 0;
    int lineno = 1;
    QStringList recs = m_data.split("\n");
    foreach(QString rec, recs) {
        m_view->insertPlainText(rec + "\n");
        m_blockmap << QString::number(lineno);
        blockno++;
        lineno++;
    }
    m_view->setBlockMap(m_blockmap);

    // set cursor to the top and start scroll there
    QTextCursor tc = m_view->textCursor();
    tc.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor,1);
    m_view->setTextCursor(tc);
    m_view->GetVerticalScrollBar()->setValue(0);
}

void SourceViewer::next_page(int dir)
{
    int d = 1;
    if (dir < 0) d = -1;
    int amount = m_view->GetVerticalScrollBar()->pageStep();
    int value = m_view->GetVerticalScrollBar()->value();
    if (amount != 0) {
        m_view->GetVerticalScrollBar()->setValue(value + (d * amount));
    }
}

void SourceViewer::do_search(bool reverse)
{
    QString stext = m_nav->get_search_text();
    if (stext.simplified().isEmpty()) return;
    QTextDocument::FindFlags ff = QTextDocument::FindFlags();
    if (reverse) ff = ff | QTextDocument::FindBackward;
    bool found = m_view->find(stext, ff);
    if (found) {
        QTextCursor c = m_view->textCursor();
        m_view->centerCursor();
    }
}

void SourceViewer::keyPressEvent(QKeyEvent * ev)
{
    if ((ev->key() == Qt::Key_Enter) || (ev->key() == Qt::Key_Return)) return;

    if (ev->key() == Qt::Key_Slash) {
        m_nav->set_focus_on_search();
        return;
    }

    if (ev->matches(QKeySequence::Copy)) {
        QString text = m_view->GetSelectedText();
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

void SourceViewer::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }
    settings.endGroup();
}

void SourceViewer::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

int SourceViewer::exec()
{
    return QDialog::exec();
}

// should cover both escape key use and using x to close the runner dialog
void SourceViewer::reject()
{
    QDialog::reject();
}

void SourceViewer::connectSignalsToSlots()
{
#ifdef Q_OS_MAC
    MainApplication *mainApplication = qobject_cast<MainApplication *>(qApp);
    connect(mainApplication, SIGNAL(applicationPaletteChanged()), this, SLOT(ReloadViewer()));
#endif
    connect(m_nav, SIGNAL(NextPage(int)), this, SLOT(next_page(int)));
    connect(m_nav, SIGNAL(DoSearch(bool)),  this, SLOT(do_search(bool)));
    connect(m_nav, SIGNAL(DoDone()),        this, SLOT(accept()));
}
