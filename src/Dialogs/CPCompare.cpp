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
#include <QKeySequence>
#include <QKeyEvent>
#include <QLabel>
#include <QToolButton>
#include <QListWidget>
#include <QApplication>
#include <QtConcurrent>
#include <QFuture>
#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>

#include "Dialogs/ListSelector.h"
#include "Dialogs/SourceViewer.h"
#include "Dialogs/ViewImage.h"
#include "Dialogs/ViewAV.h"
#include "Dialogs/ViewFont.h"
#include "Dialogs/ChgViewer.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "EmbedPython/DiffRec.h"
#include "EmbedPython/PythonRoutines.h"

#include "Dialogs/CPCompare.h"

static const QString SETTINGS_GROUP = "checkpoint_compare";

static const QStringList TEXT_EXTENSIONS = QStringList() << "css" << "htm" << "html" <<
                                                            "js" << "ncx" << "opf" << "pls" << "smil" <<
                                                            "svg" << "ttml" << "txt" << "vtt" << "xhtml" <<
                                                            "xml" << "xpgt";

static const QStringList IMAGE_EXTENSIONS = QStringList() << "bm" << "bmp" << "gif" << "jpeg" << "jpg" <<
                                                             "png" << "tif" << "tiff" << "webp";


static const QStringList AUDIO_EXTENSIONS = QStringList() << "aac"   << "m4a"  << "mp3" << "mpeg" << 
                                                             "mpg" << "oga" << "ogg";

static const QStringList VIDEO_EXTENSIONS = QStringList() << "m4v"   << "mp4"  << "mov" << "ogv"  << 
                                                             "webm";

static const QStringList FONT_EXTENSIONS = QStringList() << "ttf"   << "ttc"  << "otf" << "woff" << "woff2";

CPCompare::CPCompare(const QString& bookroot,
                     const QString& cpdir,
                     const QStringList& dlist,
                     const QStringList& alist,
                     const QStringList& mlist,
                     QWidget * parent)
    : QDialog(parent),
      m_bookroot(bookroot),
      m_cpdir(cpdir),
      m_bp(new QToolButton(this)),
      m_layout(new QVBoxLayout(this))
{
    m_dlist = new ListSelector(tr("Files Only in Checkpoint"), tr("View"), dlist, this);
    m_alist = new ListSelector(tr("Files Only in Current ePub"), tr("View"), alist, this);
    m_mlist = new ListSelector(tr("Modified since Checkpoint"), tr("View"), mlist, this);
    setWindowTitle(tr("Results of Comparison"));
    m_bp->setText(tr("Done"));
    m_bp->setToolButtonStyle(Qt::ToolButtonTextOnly);
    QHBoxLayout *hl = new QHBoxLayout();
    hl->addWidget(m_dlist);
    hl->addWidget(m_alist);
    hl->addWidget(m_mlist);
    m_layout->addLayout(hl);
    QHBoxLayout* hl2 = new QHBoxLayout();
    hl2->addStretch(0);
    hl2->addWidget(m_bp);
    m_layout->addLayout(hl2);
    ReadSettings();
    connectSignalsToSlots();
}

void CPCompare::handle_del_request()
{
    // only exists in checkpoint
    QStringList pathlist = m_dlist->get_selections();
    foreach(QString apath, pathlist) {
        QString filepath = m_cpdir + "/" + apath;
        QFileInfo fi(filepath);
        QString ext = fi.suffix().toLower();
        if (TEXT_EXTENSIONS.contains(ext)) {
            QString data = Utility::ReadUnicodeTextFile(filepath);
            SourceViewer* sv = new SourceViewer(apath, data, this);
            sv->show();
            sv->raise();
        } else if (IMAGE_EXTENSIONS.contains(ext)) {
            ViewImage * vi = new ViewImage(this, true);
            vi->ShowImage(filepath);
            vi->show();
            vi->raise();
        } else if (AUDIO_EXTENSIONS.contains(ext) || VIDEO_EXTENSIONS.contains(ext)) {
            ViewAV * av = new ViewAV(this);
            av->ShowAV(filepath);
            av->show();
            av->raise();
        } else if (FONT_EXTENSIONS.contains(ext)) {
            ViewFont * vf = new ViewFont(this);
            vf->ShowFont(filepath);
            vf->show();
            vf->raise();
        } else {
            qDebug() << "attempted to show a binary file " << apath;
        }
    }
}

void CPCompare::handle_add_request()
{
    // only exists in current epub
    QStringList pathlist = m_alist->get_selections();
    foreach(QString apath, pathlist) {
        QString filepath = m_bookroot + "/" + apath;
        QFileInfo fi(filepath);
        QString ext = fi.suffix().toLower();
        if (TEXT_EXTENSIONS.contains(ext)) {
            QString data = Utility::ReadUnicodeTextFile(filepath);
            SourceViewer* sv = new SourceViewer(apath, data, this);
            sv->show();
            sv->raise();
        } else if (IMAGE_EXTENSIONS.contains(ext)) {
            ViewImage * vi = new ViewImage(this, true);
            vi->ShowImage(filepath);
            vi->show();
            vi->raise();
        } else if (AUDIO_EXTENSIONS.contains(ext) || VIDEO_EXTENSIONS.contains(ext)) {
            ViewAV * av = new ViewAV(this);
            av->ShowAV(filepath);
            av->show();
            av->raise();
        } else if (FONT_EXTENSIONS.contains(ext)) {
            ViewFont * vf = new ViewFont(this);
            vf->ShowFont(filepath);
            vf->show();
            vf->raise();
        } else {
            qDebug() << "attempted to show a binary file " << apath;
        }
    }
}

void CPCompare::handle_mod_request()
{
    QStringList pathlist = m_mlist->get_selections();
    PythonRoutines pr;
    foreach(QString apath, pathlist) {
        QString leftpath = m_cpdir + "/" + apath;
        QString rightpath = m_bookroot + "/" + apath;
        QFileInfo fi(rightpath);
        QFileInfo lfi(leftpath);
        QString ext = fi.suffix().toLower();
        if (TEXT_EXTENSIONS.contains(ext)) {

            QApplication::setOverrideCursor(Qt::WaitCursor);
            QFuture<QList<DiffRecord::DiffRec>> bfuture =
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                QtConcurrent::run(&pr, &PythonRoutines::GenerateParsedNDiffInPython, leftpath, rightpath);
#else
                QtConcurrent::run(&PythonRoutines::GenerateParsedNDiffInPython, &pr, leftpath, rightpath);
#endif
            bfuture.waitForFinished();
            QList<DiffRecord::DiffRec> diffinfo = bfuture.result();
            QApplication::restoreOverrideCursor();

            ChgViewer* cv = new ChgViewer(diffinfo, tr("Checkpoint:") + " " + apath, tr("Current:") + " " + apath, this);
            cv->show();
            cv->raise();
        } else {
            QMessageBox * msgbox = new QMessageBox(this);
            msgbox->setIcon(QMessageBox::Information);
            msgbox->setWindowTitle(tr("Results of Comparison"));
            msgbox->setStandardButtons(QMessageBox::Ok);
            QString amsg = tr("These binary files differ in content:") + "\n";
            amsg += tr("Checkpoint:") + " " + apath + " " + QString::number(lfi.size()) + tr("bytes") + "\n";
            amsg += tr("Current:") + " " + apath + " " + QString::number(fi.size()) + tr("bytes") + "\n";
            msgbox->setText(amsg);
            msgbox->show();
            msgbox->raise();
        }
    }
}

void CPCompare::handle_cleanup()
{
}

CPCompare::~CPCompare()
{
    WriteSettings();
}

void CPCompare::ReadSettings()
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

void CPCompare::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

int CPCompare::exec()
{
    return QDialog::exec();
}

// should cover both escape key use and using x to close the runner dialog
void CPCompare::reject()
{
    handle_cleanup();
    QDialog::reject();
}

// should cover both escape key use and using x to close the runner dialog
void CPCompare::accept()
{
    handle_cleanup();
    QDialog::accept();
}

void CPCompare::connectSignalsToSlots()
{
    connect(m_bp,  SIGNAL(clicked()), this, SLOT(accept()));
    connect(m_dlist, SIGNAL(ViewRequest()), this, SLOT(handle_del_request()));
    connect(m_alist, SIGNAL(ViewRequest()), this, SLOT(handle_add_request()));
    connect(m_mlist, SIGNAL(ViewRequest()), this, SLOT(handle_mod_request()));
}
