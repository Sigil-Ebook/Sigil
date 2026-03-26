/************************************************************************
 **
 **  Copyright (C) 2026 Kevin B. Hendricks, Stratford Ontario Canada
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
#ifndef FILEDROPZONE_H
#define FILEDROPZONE_H

#include <QWidget>
#include <QLabel>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QMimeData>
#include <QDropEvent>
#include <QUrl>
#include <QStringList>
#include <QString>

class FileDropZone : public QLabel
{
    Q_OBJECT

public:
    FileDropZone(QWidget* parent = 0)
      : QLabel(tr(" To Add: Drop Files Here "), parent)
    {
        setAlignment(Qt::AlignCenter);
       	setStyleSheet("border: 2px dashed #aaa; padding 20px; color: #555;");
	setAcceptDrops(true);
    }

    void dragEnterEvent(QDragEnterEvent* event)
    {
        if (event->mimeData()->hasUrls()) {
	    event->acceptProposedAction();
	    setStyleSheet("border: 2px dashed blue; background: #e1f5fe;");
        } else {
	    event->ignore();
	}
    }

    void dragLeaveEvent(QDragLeaveEvent * event)
    {
       	setStyleSheet("border: 2px dashed #aaa; padding 20px; color: #555;");
    }

    void dropEvent(QDropEvent* event)
    {
        QStringList file_paths;
        foreach(QUrl aurl, event->mimeData()->urls()) {
	    file_paths << aurl.toLocalFile();
        }
	if (!file_paths.isEmpty()) {
            foreach(QString fp, file_paths) {
	        qDebug() << fp;
	    }
	    emit AddDroppedToEpub(file_paths);
	}
       	setStyleSheet("border: 2px dashed #aaa; padding 20px; color: #555;");
    }

signals:

    /**
     * Emitted when files dragged and dropped
     *
     * @param filepaths  The list of file paths
     */
    void AddDroppedToEpub(const QStringList& filepaths);

};

#endif // FILEDROPZONE_H
