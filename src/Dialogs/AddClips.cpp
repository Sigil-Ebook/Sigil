/************************************************************************
**
**  Copyright (C) 2025 Kevin B. Hendricks, Stratford, ON, Canada
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

#include <QDate>
#include <QModelIndex>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "Misc/AriaRoles.h"
#include "Misc/AriaClips.h"
#include "Dialogs/AddRoles.h"
#include "Dialogs/AddClips.h"

static const QString SETTINGS_GROUP = "add_clips";

static const QStringList UPDATE_ONLY_NUMBER = QStringList() << "fn_ref" << "fn_backlink" << "endnote_ref" <<
                                                                   "endnote_backlink" << "pagebreak_span";

static const QStringList UPDATE_NUMBER_AND_FILL = QStringList() << "fn_aside" << "fn_div" << "fn_p" << "endnote_li";


AddClips::AddClips(const QString& selected_text, const QString& book_lang, QWidget *parent)
    :
    QDialog(parent),
    m_selected_text(selected_text),
    m_book_lang(book_lang)
{
    ui.setupUi(this);

    connect(ui.lwProperties, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(UpdateDescription(QListWidgetItem *)));
    connect(this, SIGNAL(accepted()), this, SLOT(WriteSettings()));
    connect(ui.lwProperties, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(accept()));

    // Fill the dialog with sorted translated role names
    QStringList names;
    foreach (QString code, AriaClips::instance()->GetAllCodes()) {
        QString name = AriaClips::instance()->GetName(code);
        name = name + " (" + code + ")";
        m_Name2Code[name]=code;
        names.append(name);
    }
    names = Utility::LocaleAwareSort(names);

    foreach(QString name, names) {
        ui.lwProperties->addItem(name);
    }
    ReadSettings();
}

void AddClips::UpdateDescription(QListWidgetItem *current)
{
    QString text;
    QString code = m_Name2Code.value(current->text(), QString());
    if (!code.isEmpty()) {
        text = AriaClips::instance()->GetDescriptionByCode(code);
        text = AriaClips::instance()->TranslatePlaceholders(text, m_book_lang);
    }
    if (!text.isEmpty()) {
        ui.lbDescription->setText(text);
    }
}

QString AddClips::GetSelectedClip()
{
    QString clip = "";
    if (!m_SelectedEntries.isEmpty()) {
        QString code = m_SelectedEntries.at(0);
        clip = AriaClips::instance()->GetDescriptionByCode(code);
        
        if (!m_selected_text.isEmpty()) {
            if (UPDATE_ONLY_NUMBER.contains(code) || UPDATE_NUMBER_AND_FILL.contains(code)) {
                // extract a number from text selected in CV to replace _N_
                QRegularExpression findLeadingNumber("^\\s*\\[?(\\d+)\\]?\\.?\\s*");
                QRegularExpressionMatch amtch = findLeadingNumber.match(m_selected_text, 0);
                QString anum;
                if (amtch.hasMatch()) {
                    anum = amtch.captured(1);
                    // size_t alen  = amtch.capturedLength(0);
                    // m_selected_text = m_selected_text.remove(0, alen);
                }
                if (!anum.isEmpty()) {
                    clip.replace("_N_", anum);
                    if (UPDATE_NUMBER_AND_FILL.contains(code)) {
                        clip.replace("\\1", m_selected_text);
                    }
                }
            }
        }
        if ((code == "section") || (code == "aside")) {
            // allow user to select roles and epub types
            AddRoles addmeaning(code, this);
            if (addmeaning.exec() == QDialog::Accepted) {
                QStringList rolecodes = addmeaning.GetSelectedEntries();
                if (!rolecodes.isEmpty()) {
                    QString rcode = rolecodes.at(0);
                    QString etype = AriaRoles::instance()->EpubTypeMapping(rcode);
                    QString atts_added = " ";
                    if (etype == rcode) {
                        // this is an epub:type only attribute
                        atts_added += "epub:type=\"" + etype + "\"";
                    } else {
                        if (!etype.isEmpty()) {
                            atts_added += "epub:type=\"" + etype + "\" ";
                        }
                        atts_added += "role=\"" + rcode + "\"";
                    }
                    // inject those attributes before the first ">" in the clip
                    int p = clip.indexOf('>');
                    if (p > -1) {
                        clip.insert(p,atts_added);
                    }
                }
            }
        }
        clip = AriaClips::instance()->TranslatePlaceholders(clip, m_book_lang);
    }
    return clip;
}

void AddClips::SaveSelection()
{
    m_SelectedEntries.clear();
    foreach(QListWidgetItem * item, ui.lwProperties->selectedItems()) {
        QString code = m_Name2Code.value(item->text(), QString() );
        m_SelectedEntries.append(code);
    }
}


void AddClips::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    QByteArray geometry = settings.value("geometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }

    QByteArray splitter_position = settings.value("splitter").toByteArray();

    if (!splitter_position.isNull()) {
        ui.splitter->restoreState(splitter_position);
    }

    settings.endGroup();
}


void AddClips::WriteSettings()
{
    SaveSelection();
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    // The position of the splitter handle
    settings.setValue("splitter", ui.splitter->saveState());
    settings.endGroup();
}
