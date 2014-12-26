/************************************************************************
**
**  Copyright (C) 2014 Marek Gibek
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

#include "Dialogs/CleanContent.h"
#include "MainUI/FindReplace.h"

CleanContent::CleanContent(MainWindow &main_window)
    :
    QDialog(&main_window),
    m_MainWindow(main_window),
    m_Book(NULL)
{
    ui.setupUi(this);

    ui.cbLookWhere->clear();
    ui.cbLookWhere->addItem(tr("Current File"), FindReplace::LookWhere_CurrentFile);
    ui.cbLookWhere->addItem(tr("All HTML Files"), FindReplace::LookWhere_AllHTMLFiles);
    ui.cbLookWhere->addItem(tr("Selected Files"), FindReplace::LookWhere_SelectedHTMLFiles);

    ConnectSignalsSlots();
}

void CleanContent::SetBook(QSharedPointer <Book> book)
{
    m_Book = book;
}

void CleanContent::ForceClose()
{
    close();
}

CleanContentParams CleanContent::GetParams()
{
    CleanContentParams params;

    params.remove_page_numbers = ui.checkBoxRemovePageNumbers->isChecked();
    params.page_number_format = ui.lineEditPageNumberFormat->text();

    params.remove_empty_paragraphs = ui.checkBoxRemoveEmptyParagraphs->isChecked();

    params.join_paragraphs = ui.checkBoxJoinParagraphs->isChecked();

    return params;
}

void CleanContent::Execute()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    m_MainWindow.GetCurrentContentTab().SaveTabContent();

    QList<HTMLResource *> html_resources;
    QList<Resource *> resources = m_MainWindow.GetAllHTMLResources();
    foreach(Resource * resource, resources) {
        HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
        if (html_resource) {
            html_resources.append(html_resource);
        }
    }

    CleanContentUpdates::CleanContentInAllFiles(html_resources, GetParams());

    m_MainWindow.GetCurrentBook()->SetModified(true);
    m_MainWindow.GetCurrentContentTab().ContentChangedExternally();
    m_MainWindow.AnyCodeView();

    activateWindow();

    //m_MainWindow.ShowMessageOnStatusBar(tr("Cleaning content done."));

    QApplication::restoreOverrideCursor();
}

void CleanContent::Save()
{

}

void CleanContent::ConnectSignalsSlots()
{
    connect(ui.btnExecute, SIGNAL(clicked()), this, SLOT(Execute()));
    connect(ui.btnSave, SIGNAL(clicked()), this, SLOT(Save()));
}
