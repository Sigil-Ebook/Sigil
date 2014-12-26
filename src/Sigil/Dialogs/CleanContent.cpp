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

CleanContent::CleanContent(MainWindow &main_window)
    :
    QDialog(&main_window),
    m_MainWindow(main_window),
    m_Book(NULL)
{
    ui.setupUi(this);

    ui.cbLookWhere->clear();
    ui.cbLookWhere->addItem(tr("Current File"), CleanContent::LookWhere_CurrentFile);
    ui.cbLookWhere->addItem(tr("All HTML Files"), CleanContent::LookWhere_AllHTMLFiles);
    ui.cbLookWhere->addItem(tr("Selected Files"), CleanContent::LookWhere_SelectedHTMLFiles);

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
    params.page_number_remove_empty_paragraphs = ui.checkBoxRemoveEmptyParagraphs->isChecked();

    params.join_paragraphs = ui.checkBoxJoinParagraphs->isChecked();
    params.join_paragraphs_only_not_formatted = ui.checkBoxJoinNotFormatted->isChecked();

    return params;
}

void CleanContent::showEvent(QShowEvent *event)
{
    ui.message->setText("");
}

void CleanContent::Execute()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    ui.message->setText("");

    m_MainWindow.GetCurrentContentTab().SaveTabContent();

    QList<HTMLResource *> html_resources = GetHTMLFiles();

    ChangesCount changes_count =
            CleanContentUpdates::CleanContentInAllFiles(html_resources, GetParams());

    if (changes_count.number_of_changes > 0) {
        m_MainWindow.GetCurrentBook()->SetModified(true);
        m_MainWindow.GetCurrentContentTab().ContentChangedExternally();
        m_MainWindow.AnyCodeView();
    }

    activateWindow();

    ui.message->setText(QString(tr("Done %1 change(s) in %2 file(s).")
                                .arg(changes_count.number_of_changes)
                                .arg(changes_count.number_of_files)));

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

void CleanContent::SetLookWhere(int look_where)
{
    ui.cbLookWhere->setCurrentIndex(0);

    for (int i = 0; i < ui.cbLookWhere->count(); ++i) {
        if (ui.cbLookWhere->itemData(i)  == look_where) {
            ui.cbLookWhere->setCurrentIndex(i);
            break;
        }
    }
}

CleanContent::LookWhere CleanContent::GetLookWhere()
{
    int look = ui.cbLookWhere->itemData(ui.cbLookWhere->currentIndex()).toInt();

    switch (look) {
        case CleanContent::LookWhere_AllHTMLFiles:
            return static_cast<CleanContent::LookWhere>(look);
            break;

        case CleanContent::LookWhere_SelectedHTMLFiles:
            return static_cast<CleanContent::LookWhere>(look);
            break;

        default:
            return CleanContent::LookWhere_CurrentFile;
    }
}

QList <HTMLResource *> CleanContent::GetHTMLFiles()
{
    QList<HTMLResource *> html_resources;
    QList<Resource *> resources;

    CleanContent::LookWhere look_where = GetLookWhere();
    if (look_where == CleanContent::LookWhere_AllHTMLFiles) {
        resources = m_MainWindow.GetAllHTMLResources();
    } else if (look_where == CleanContent::LookWhere_SelectedHTMLFiles) {
        resources = m_MainWindow.GetValidSelectedHTMLResources();
    } else {
        resources.append(&m_MainWindow.GetCurrentContentTab().GetLoadedResource());
    }

    foreach (Resource * resource, resources) {
        HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
        if (html_resource) {
            html_resources.append(html_resource);
        }
    }

    return html_resources;
}
