/************************************************************************
**
**  Copyright (C) 2023 Kevin B. Hendricks, Stratford, Ontario Canada
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

#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtWidgets/QLayout>
#include <QGuiApplication>
#include <QApplication>
#include "MainUI/MainWindow.h"
#include "Misc/Utility.h"
#include "sigil_constants.h"
#include "Widgets/PdfView.h"
#include "Tabs/PdfTab.h"

PdfTab::PdfTab(Resource *resource, QWidget *parent)
    : ContentTab(resource, parent),
      m_pdf(new PdfView(this))
{
    m_Layout->addWidget(m_pdf);
    ConnectSignalsToSlots();
    ShowPdf();
}

void PdfTab::ShowPdf()
{
    m_pdf->ShowPdf(m_Resource->GetFullPath());
}

void PdfTab::RefreshContent()
{
    m_pdf->ReloadViewer();
}

void PdfTab::ConnectSignalsToSlots()
{
    connect(m_Resource, SIGNAL(ResourceUpdatedOnDisk()), this, SLOT(RefreshContent()));
    connect(m_Resource, SIGNAL(Deleted(const Resource*)), this, SLOT(Close()));
}
