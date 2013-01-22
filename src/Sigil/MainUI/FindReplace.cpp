/************************************************************************
**
**  Copyright (C) 2011, 2012  John Schember <john@nachtimwald.com>
**  Copyright (C) 2012  Dave Heiland
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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
#include <pcre.h>

#include <QtGui/QKeyEvent>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QCompleter>
#include <QRegularExpression>

#include "MainUI/FindReplace.h"
#include "Misc/SettingsStore.h"
#include "Misc/SleepFunctions.h"
#include "Misc/FindReplaceQLineEdit.h"

static const QString SETTINGS_GROUP = "find_replace";
static const QString REGEX_OPTION_UCP = "(*UCP)";
static const QString REGEX_OPTION_IGNORE_CASE = "(?i)";
static const QString REGEX_OPTION_DOT_ALL = "(?s)";
static const QString REGEX_OPTION_MINIMAL_MATCH = "(?U)";

static const int SHOW_FIND_RESULTS_MESSAGE_DELAY_MS = 20000;

FindReplace::FindReplace(MainWindow &main_window)
    : QWidget(&main_window),
      m_MainWindow(main_window),
      m_RegexOptionDotAll(false),
      m_RegexOptionMinimalMatch(false),
      m_RegexOptionAutoTokenise(false),
      m_OptionWrap(true),
      m_SpellCheck(false),
      m_LookWhereCurrentFile(false),
      m_IsSearchGroupRunning(false)
{
    ui.setupUi(this);
    FindReplaceQLineEdit *find_ledit = new FindReplaceQLineEdit(this);
    ui.cbFind->setLineEdit(find_ledit);
    FindReplaceQLineEdit *replace_ledit = new FindReplaceQLineEdit(this);
    replace_ledit->setTokeniseEnabled(false);
    ui.cbReplace->setLineEdit(replace_ledit);
    QCompleter *fqc = ui.cbFind->completer();
    fqc->setCaseSensitivity(Qt::CaseSensitive);
    fqc->setCompletionMode(QCompleter::PopupCompletion);
    ui.cbFind->setCompleter(fqc);
    QCompleter *rqc = ui.cbReplace->completer();
    rqc->setCaseSensitivity(Qt::CaseSensitive);
    rqc->setCompletionMode(QCompleter::PopupCompletion);
    ui.cbReplace->setCompleter(rqc);
    ExtendUI();
    ConnectSignalsToSlots();
    ShowHideAdvancedOptions();
    ReadSettings();
}


// Destructor
FindReplace::~FindReplace()
{
    WriteSettings();
}


void FindReplace::SetUpFindText()
{
    Searchable *searchable = GetAvailableSearchable();

    if (searchable) {
        QString selected_text = searchable->GetSelectedText();

        if (!selected_text.isEmpty()) {
            if (m_RegexOptionAutoTokenise && GetSearchMode() == FindReplace::SearchMode_Regex) {
                selected_text = TokeniseForRegex(selected_text, false);
            }

            ui.cbFind->setEditText(selected_text);
            // To allow the user to immediately click on Replace, we need to setup the
            // regex match as though the user had clicked on Find.
            searchable->SetUpFindForSelectedText(GetSearchRegex());
        }
    }

    // Find text should be selected by default
    ui.cbFind->lineEdit()->selectAll();
    SetFocus();
}


void FindReplace::SetFocus()
{
    ui.cbFind->lineEdit()->setFocus(Qt::ShortcutFocusReason);
}


bool FindReplace::HasFocus()
{
    return ui.cbFind->lineEdit()->hasFocus();
}


void FindReplace::close()
{
    WriteSettingsVisible(false);
    QWidget::close();
}


void FindReplace::show()
{
    WriteSettingsVisible(true);
    clearMessage();
    QWidget::show();
}


void FindReplace::HideFindReplace()
{
    WriteSettingsVisible(false);
    hide();
}

void FindReplace::AdvancedOptionsClicked()
{
    bool is_currently_visible = ui.chkRegexOptionAutoTokenise->isVisible();
    WriteSettingsAdvancedVisible(!is_currently_visible);
    ShowHideAdvancedOptions();
}

void FindReplace::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        HideFindReplace();
    }
}


void FindReplace::ShowMessage(const QString &message)
{
    QString new_message = message;

    if (m_LookWhereCurrentFile && GetLookWhere() != FindReplace::LookWhere_CurrentFile) {
        new_message.append(" (" % tr("Current File") % ")");
    }

    ui.message->setText(new_message);
    m_timer.start(SHOW_FIND_RESULTS_MESSAGE_DELAY_MS);
    emit ShowMessageRequest(new_message);
}

void FindReplace::SetKeyModifiers()
{
    // Only use with mouse click not menu/shortcuts to avoid modifying actions
    m_LookWhereCurrentFile = QApplication::keyboardModifiers() & Qt::ControlModifier;
}

void FindReplace::ResetKeyModifiers()
{
    m_LookWhereCurrentFile = false;
}

void FindReplace::FindClicked()
{
    SetKeyModifiers();
    Find();
    ResetKeyModifiers();
}

void FindReplace::ReplaceClicked()
{
    SetKeyModifiers();
    Replace();
    ResetKeyModifiers();
}

void FindReplace::ReplaceAllClicked()
{
    SetKeyModifiers();
    ReplaceAll();
    ResetKeyModifiers();
}

void FindReplace::CountClicked()
{
    SetKeyModifiers();
    Count();
    ResetKeyModifiers();
}

void FindReplace::FindWord(QString word)
{
//    SetCodeViewIfNeeded(true);
//    WriteSettings();
//
//    SetSearchMode(FindReplace::SearchMode_Regex);
//    SetLookWhere(FindReplace::LookWhere_AllHTMLFiles);
//    SetSearchDirection(FindReplace::SearchDirection_Down);
//    SetRegexOptionDotAll(true);
//    SetRegexOptionMinimalMatch(true);
//    SetOptionWrap(true);

    word = "\\b" + word + "\\b";
    FindAnyText(word);
//    ui.cbFind->setEditText(word);
//    FindNext();
//
//    ReadSettings();
}

void FindReplace::FindAnyText(QString text)
{
    SetCodeViewIfNeeded(true);
    WriteSettings();

    SetSearchMode(FindReplace::SearchMode_Regex);
    SetLookWhere(FindReplace::LookWhere_AllHTMLFiles);
    SetSearchDirection(FindReplace::SearchDirection_Down);
    SetRegexOptionDotAll(true);
    SetRegexOptionMinimalMatch(true);
    SetOptionWrap(true);

    text = text + "(?![^<>]*>)(?!.*<body[^>]*>)";
    ui.cbFind->setEditText(text);
    FindNext();

    ReadSettings();
}

bool FindReplace::Find()
{
    bool found = false;

    if (GetSearchDirection() == FindReplace::SearchDirection_Up) {
        found = FindPrevious();
    } else {
        found = FindNext();
    }

    return found;
}


bool FindReplace::FindNext()
{
    return FindText(Searchable::Direction_Down);
}


bool FindReplace::FindPrevious()
{
    return FindText(Searchable::Direction_Up);
}


// Counts the number of occurrences of the user's
// term in the document.
int FindReplace::Count()
{
    clearMessage();

    if (!IsValidFindText()) {
        return 0;
    }

    SetCodeViewIfNeeded(true);
    int count = 0;

    if (GetLookWhere() == FindReplace::LookWhere_CurrentFile || m_LookWhereCurrentFile) {
        Searchable *searchable = GetAvailableSearchable();

        if (!searchable) {
            return 0;
        }

        count = searchable->Count(GetSearchRegex(), GetSearchableDirection(), m_OptionWrap);
    } else {
        // If wrap, all files are counted, otherwise only files before/after
        // the current file are counted, and then added to the count of current file.
        count = CountInFiles();
        if (!m_OptionWrap) {
            Searchable *searchable = GetAvailableSearchable();
            if (searchable) {
                count += searchable->Count(GetSearchRegex(), GetSearchableDirection(), m_OptionWrap);
            }
        }
    }

    if (count == 0) {
        CannotFindSearchTerm();
    } else {
        QString message = tr("Matches found: %n", "", count);
        ShowMessage(message);
    }

    UpdatePreviousFindStrings();
    return count;
}


bool FindReplace::Replace()
{
    bool found = false;

    if (GetSearchDirection() == FindReplace::SearchDirection_Up) {
        found = ReplacePrevious();
    } else {
        found = ReplaceNext();
    }

    return found;
}


bool FindReplace::ReplaceNext()
{
    return ReplaceText(Searchable::Direction_Down);
}


bool FindReplace::ReplacePrevious()
{
    return ReplaceText(Searchable::Direction_Up);
}


bool FindReplace::ReplaceCurrent()
{
    bool found = false;

    if (GetSearchDirection() == FindReplace::SearchDirection_Up) {
        found = ReplaceText(Searchable::Direction_Up, true);
    } else {
        found = ReplaceText(Searchable::Direction_Down, true);
    }

    return found;
}


// Replaces the user's search term with the user's
// replacement text in the entire document.
int FindReplace::ReplaceAll()
{
    m_MainWindow.GetCurrentContentTab().SaveTabContent();
    clearMessage();

    if (!IsValidFindText()) {
        return 0;
    }

    SetCodeViewIfNeeded(true);
    int count = 0;

    if (GetLookWhere() == FindReplace::LookWhere_CurrentFile || m_LookWhereCurrentFile) {
        Searchable *searchable = GetAvailableSearchable();

        if (!searchable) {
            return 0;
        }

        count = searchable->ReplaceAll(GetSearchRegex(), ui.cbReplace->lineEdit()->text(), GetSearchableDirection(), m_OptionWrap);
    } else {
        // If wrap, all files are replaced, otherwise only files before/after
        // the current file are updated, and then the current file is done.
        count = ReplaceInAllFiles();
        if (!m_OptionWrap) {
            Searchable *searchable = GetAvailableSearchable();
            if (searchable) {
                count += searchable->ReplaceAll(GetSearchRegex(), ui.cbReplace->lineEdit()->text(), GetSearchableDirection(), m_OptionWrap);
            }
        }
    }

    if (count == 0) {
        ShowMessage(tr("No replacements made"));
    } else {
        QString message = tr("Replacements made: %n", "", count);
        ShowMessage(message);
    }

    if (count > 0) {
        // Signal that the contents have changed and update the view
        m_MainWindow.GetCurrentBook()->SetModified(true);
        m_MainWindow.GetCurrentContentTab().ContentChangedExternally();
    }

    UpdatePreviousFindStrings();
    UpdatePreviousReplaceStrings();
    return count;
}

void FindReplace::FindNextInFile()
{
    m_LookWhereCurrentFile = true;
    FindText(Searchable::Direction_Down);
    m_LookWhereCurrentFile = false;
}

void FindReplace::ReplaceNextInFile()
{
    m_LookWhereCurrentFile = true;
    ReplaceText(Searchable::Direction_Down);
    m_LookWhereCurrentFile = false;
}

void FindReplace::ReplaceAllInFile()
{
    m_LookWhereCurrentFile = true;
    ReplaceAll();
    m_LookWhereCurrentFile = false;
}

void FindReplace::CountInFile()
{
    m_LookWhereCurrentFile = true;
    Count();
    m_LookWhereCurrentFile = false;
}

Searchable::Direction FindReplace::GetSearchableDirection()
{
    Searchable::Direction direction = Searchable::Direction_Down;
    if (GetSearchDirection() == FindReplace::SearchDirection_Up) {
        direction = Searchable::Direction_Up;
    }
    return direction;
}


void FindReplace::clearMessage()
{
    if (!m_IsSearchGroupRunning) {
        ui.message->clear();
        emit ShowMessageRequest("");
    }
}

void FindReplace::expireMessage()
{
    m_timer.stop();
    ui.message->clear();
    emit ShowMessageRequest("");
}

bool FindReplace::FindMisspelledWord()
{
    clearMessage();
    SetCodeViewIfNeeded(true);
    m_SpellCheck = true;

    WriteSettings();
    // Only files, direction, wrap are checked for misspelled searches
    SetLookWhere(FindReplace::LookWhere_AllHTMLFiles);
    SetSearchDirection(FindReplace::SearchDirection_Down);
    SetOptionWrap(true);

    bool found = FindInAllFiles(Searchable::Direction_Down);

    ReadSettings();
    m_SpellCheck = false;

    if (found) {
        clearMessage();
    } else {
        CannotFindSearchTerm();
    }

    return found;
}


// Starts the search for the user's term.
bool FindReplace::FindText(Searchable::Direction direction)
{
    bool found = false;
    clearMessage();

    if (!IsValidFindText()) {
        return found;
    }

    SetCodeViewIfNeeded();

    if (GetLookWhere() == FindReplace::LookWhere_CurrentFile || m_LookWhereCurrentFile) {
        Searchable *searchable = GetAvailableSearchable();

        if (!searchable) {
            return found;
        }

        found = searchable->FindNext(GetSearchRegex(), direction, false, false, m_OptionWrap);
    } else {
        found = FindInAllFiles(direction);
    }

    if (found) {
        clearMessage();
    } else {
        CannotFindSearchTerm();
    }

    UpdatePreviousFindStrings();
    return found;
}


// Replaces the user's search term with the user's
// replacement text if a match is selected. If it's not,
// calls Find in the direction specified so it becomes selected.
bool FindReplace::ReplaceText(Searchable::Direction direction, bool replace_current)
{
    bool found = false;
    clearMessage();

    if (!IsValidFindText()) {
        return found;
    }

    SetCodeViewIfNeeded(true);
    Searchable *searchable = GetAvailableSearchable();

    if (!searchable) {
        return found;
    }

    // If we have the matching text selected, replace it
    // This will not do anything if matching text is not selected.
    found = searchable->ReplaceSelected(GetSearchRegex(), ui.cbReplace->lineEdit()->text(), direction, replace_current);

    // If we are not going to stay put after a simple Replace, then find next match.
    if (!replace_current) {
        // If doing a Replace/Find set the value of found to the result of the Find.
        if (direction == Searchable::Direction_Up) {
            found = FindPrevious();
        } else {
            found = FindNext();
        }
    }

    UpdatePreviousFindStrings();
    UpdatePreviousReplaceStrings();
    // Do not use the return value to tell if a replace was done - only if a complete
    // Find/Replace or ReplaceCurrent was ok.  This allows multiple selections to work as expected.
    return found;
}


void FindReplace::SetCodeViewIfNeeded(bool force)
{
    // We never need to switch to CodeView if only working within scope of a non-html file.
    if (m_LookWhereCurrentFile || GetLookWhere() == FindReplace::LookWhere_CurrentFile) {
        if (GetCurrentResource()->Type() != Resource::HTMLResourceType) {
            return;
        }
    }

    bool has_focus = HasFocus();

    if (force ||
        (!m_LookWhereCurrentFile &&
         (GetLookWhere() == FindReplace::LookWhere_AllHTMLFiles ||
          GetLookWhere() == FindReplace::LookWhere_SelectedHTMLFiles) &&
         (m_MainWindow.GetViewState() == MainWindow::ViewState_BookView))) {
        // Force change to Code View
        m_MainWindow.AnyCodeView();

        if (has_focus) {
            SetFocus();
        }
    }
}


// Displays a message to the user informing him
// that his last search term could not be found.
void FindReplace::CannotFindSearchTerm()
{
    ShowMessage(tr("No matches found"));
}


// Constructs a searching regex from the selected
// options and fields and then returns it.
QString FindReplace::GetSearchRegex()
{
    if (m_SpellCheck) {
        return QString();
    }

    QString search(ui.cbFind->currentText());

    // Search type
    if (GetSearchMode() == FindReplace::SearchMode_Normal || GetSearchMode() == FindReplace::SearchMode_Case_Sensitive) {
        search = QRegularExpression::escape(search);

        if (GetSearchMode() == FindReplace::SearchMode_Normal) {
            search = PrependRegexOptionToSearch(REGEX_OPTION_IGNORE_CASE, search);
        }
    } else {
        if (m_RegexOptionDotAll) {
            search = PrependRegexOptionToSearch(REGEX_OPTION_DOT_ALL, search);
        }

        if (m_RegexOptionMinimalMatch) {
            search = PrependRegexOptionToSearch(REGEX_OPTION_MINIMAL_MATCH, search);
        }
    }

    return search;
}

QString FindReplace::PrependRegexOptionToSearch(const QString &option, const QString &search)
{
    if (search.startsWith(REGEX_OPTION_UCP)) {
        // Special case scenario - this directive must *always* be before any others
        return REGEX_OPTION_UCP % option % search.mid(REGEX_OPTION_UCP.length());
    }

    return option % search;
}

bool FindReplace::IsCurrentFileInHTMLSelection()
{
    bool found = false;
    QList <Resource *> resources = GetHTMLFiles();
    Resource *current_resource = GetCurrentResource();
    HTMLResource *current_html_resource = qobject_cast< HTMLResource *>(current_resource);

    if (current_html_resource) {
        foreach(Resource * resource, resources) {
            if (resource->Filename() == current_html_resource->Filename()) {
                found = true;
                break;
            }
        }
    }

    return found;
}

// Returns all html resources or only those selected in Book Browser
QList <Resource *> FindReplace::GetHTMLFiles()
{
    // For now, this must hold
    Q_ASSERT(GetLookWhere() == FindReplace::LookWhere_AllHTMLFiles || GetLookWhere() == FindReplace::LookWhere_SelectedHTMLFiles);
    QList <Resource *> all_resources;
    QList <Resource *> resources;

    if (GetLookWhere() == FindReplace::LookWhere_AllHTMLFiles) {
        all_resources = m_MainWindow.GetAllHTMLResources();
    } else {
        all_resources = m_MainWindow.GetValidSelectedHTMLResources();
    }

    // If wrapping, or the current resource is not in the HTML files to search
    // (meaning there is no before/after for wrap to use) then just return all files
    Resource *current_resource = GetCurrentResource();
    if (m_OptionWrap || !all_resources.contains(current_resource)) {
        return all_resources;
    }

    // Return only the current file and before/after files
    if (GetSearchDirection() == FindReplace::SearchDirection_Up) {
        foreach (Resource *resource, all_resources) {
            resources.append(resource);
            if (resource == current_resource) {
                break;
            }
        }
    } 
    else {
        bool keep = false;
        foreach (Resource *resource, all_resources) {
            if (resource == current_resource) {
                keep = true;
            }
            if (keep) {
                resources.append(resource);
            }
        }
    }

    return resources;
}

int FindReplace::CountInFiles()
{
    // For now, this must hold
    Q_ASSERT(GetLookWhere() == FindReplace::LookWhere_AllHTMLFiles || GetLookWhere() == FindReplace::LookWhere_SelectedHTMLFiles);
    m_MainWindow.GetCurrentContentTab().SaveTabContent();

    // When not wrapping remove the current resource as it's counted separately
    QList<Resource *>html_files = GetHTMLFiles();
    if (!m_OptionWrap) {
        html_files.removeOne(GetCurrentResource());
    }
    return SearchOperations::CountInFiles(
               GetSearchRegex(),
               html_files,
               SearchOperations::CodeViewSearch);
}


int FindReplace::ReplaceInAllFiles()
{
    // For now, this must hold
    Q_ASSERT(GetLookWhere() == FindReplace::LookWhere_AllHTMLFiles || GetLookWhere() == FindReplace::LookWhere_SelectedHTMLFiles);
    m_MainWindow.GetCurrentContentTab().SaveTabContent();
    // When not wrapping remove the current resource as it's replace separately
    QList<Resource *>html_files = GetHTMLFiles();
    if (!m_OptionWrap) {
        html_files.removeOne(GetCurrentResource());
    }
    int count = SearchOperations::ReplaceInAllFIles(
                    GetSearchRegex(),
                    ui.cbReplace->lineEdit()->text(),
                    html_files,
                    SearchOperations::CodeViewSearch);
    return count;
}


bool FindReplace::FindInAllFiles(Searchable::Direction direction)
{
    Searchable *searchable = 0;
    bool found = false;

    if (IsCurrentFileInHTMLSelection()) {
        searchable = GetAvailableSearchable();

        if (searchable) {
            found = searchable->FindNext(GetSearchRegex(), direction, m_SpellCheck, false, false);
        }
    }

    if (!found) {
        // TODO: make this handle all types of files
        Resource *containing_resource = GetNextContainingHTMLResource(direction);

        if (containing_resource) {
            // Save if editor or F&R has focus
            bool has_focus = HasFocus();
            // Save selected resources since opening tabs changes selection
            QList<Resource *>selected_resources = GetHTMLFiles();
            m_MainWindow.OpenResource(*containing_resource);

            while (!m_MainWindow.GetCurrentContentTab().IsLoadingFinished()) {
                // Make sure Qt processes events, signals and calls slots
                qApp->processEvents();
                SleepFunctions::msleep(100);
            }

            // Restore selection since opening tabs changes selection
            if (GetLookWhere() == FindReplace::LookWhere_SelectedHTMLFiles && !m_SpellCheck) {
                m_MainWindow.SelectResources(selected_resources);
            }

            // Reset focus to F&R if it had it
            if (has_focus) {
                SetFocus();
            }

            searchable = GetAvailableSearchable();

            if (searchable) {
                found = searchable->FindNext(GetSearchRegex(), direction, m_SpellCheck, true, false);
            }
        } else {
            if (searchable) {
                // Check the part of the original file above the cursor
                found = searchable->FindNext(GetSearchRegex(), direction, m_SpellCheck, false, false);
            }
        }
    }

    return found;
}

HTMLResource *FindReplace::GetNextContainingHTMLResource(Searchable::Direction direction)
{
    Resource *current_resource = GetCurrentResource();
    HTMLResource *starting_html_resource = qobject_cast< HTMLResource *> (current_resource);

    QList<Resource *> resources = GetHTMLFiles();

    if (resources.isEmpty()) {
        return NULL;
    }

    if (!starting_html_resource || (GetLookWhere() == FindReplace::LookWhere_SelectedHTMLFiles && !IsCurrentFileInHTMLSelection())) {
        if (direction == Searchable::Direction_Up) {
            starting_html_resource = qobject_cast< HTMLResource *>(resources.first());
        } else {
            starting_html_resource = qobject_cast< HTMLResource *>(resources.last());
        }
    }

    HTMLResource *next_html_resource = starting_html_resource;
    bool passed_starting_html_resource = false;

    while (!passed_starting_html_resource || (next_html_resource != starting_html_resource)) {
        next_html_resource = GetNextHTMLResource(next_html_resource, direction);

        if (next_html_resource == starting_html_resource) {
            passed_starting_html_resource = true ;
        }

        if (next_html_resource) {
            if (ResourceContainsCurrentRegex(next_html_resource)) {
                return next_html_resource;
            }

            // else continue
        } else {
            return NULL;
        }
    }

    return NULL;
}


HTMLResource *FindReplace::GetNextHTMLResource(HTMLResource *current_resource, Searchable::Direction direction)
{
    QList <Resource *> resources = GetHTMLFiles();
    int max_reading_order       = resources.count() - 1;
    int current_reading_order   = 0;
    int next_reading_order      = 0;
    // Find the current resource in the selected/all html entries
    int i = 0;
    foreach(Resource * resource, resources) {
        if (resource->Filename() == current_resource->Filename()) {
            current_reading_order = i;
            break;
        }

        i++;
    }

    // We wrap back (if needed)
    if (direction == Searchable::Direction_Up) {
        next_reading_order = current_reading_order - 1 >= 0 ? current_reading_order - 1 : max_reading_order ;
    } else {
        next_reading_order = current_reading_order + 1 <= max_reading_order ? current_reading_order + 1 : 0;
    }

    if (next_reading_order > max_reading_order || next_reading_order < 0) {
        return NULL;
    } else {
        HTMLResource &html_resource = *qobject_cast< HTMLResource *>(resources[ next_reading_order ]);
        return &html_resource;
    }
}


Resource *FindReplace::GetCurrentResource()
{
    return &m_MainWindow.GetCurrentContentTab().GetLoadedResource();
}


QStringList FindReplace::GetPreviousFindStrings()
{
    QStringList find_strings;

    for (int i = 0; i < qMin(ui.cbFind->count(), ui.cbFind->maxCount()); ++i) {
        if (!find_strings.contains(ui.cbFind->itemText(i))) {
            find_strings.append(ui.cbFind->itemText(i));
        }
    }

    return find_strings;
}


QStringList FindReplace::GetPreviousReplaceStrings()
{
    QStringList replace_strings;

    for (int i = 0; i < qMin(ui.cbReplace->count(), ui.cbReplace->maxCount()); ++i) {
        if (!replace_strings.contains(ui.cbReplace->itemText(i))) {
            replace_strings.append(ui.cbReplace->itemText(i));
        }
    }

    return replace_strings;
}


void FindReplace::UpdatePreviousFindStrings(const QString &text)
{
    QString new_find_string;

    if (!text.isNull()) {
        new_find_string = text;
    } else {
        new_find_string = ui.cbFind->lineEdit()->text();
    }

    int used_at_index = ui.cbFind->findText(new_find_string);

    if (used_at_index != -1) {
        ui.cbFind->removeItem(used_at_index);
    }

    ui.cbFind->insertItem(0, new_find_string);
    // Must not change the current string!
    ui.cbFind->setCurrentIndex(0);
}


void FindReplace::UpdatePreviousReplaceStrings(const QString &text)
{
    QString new_replace_string;

    if (!text.isNull()) {
        new_replace_string = text;
    } else {
        new_replace_string = ui.cbReplace->lineEdit()->text();
    }

    int used_at_index = ui.cbReplace->findText(new_replace_string);

    if (used_at_index != -1) {
        ui.cbReplace->removeItem(used_at_index);
    }

    ui.cbReplace->insertItem(0, new_replace_string);
    // Must not change the current string!
    ui.cbReplace->setCurrentIndex(0);
}

FindReplace::SearchMode FindReplace::GetSearchMode()
{
    int mode = ui.cbSearchMode->itemData(ui.cbSearchMode->currentIndex()).toInt();

    switch (mode) {
        case FindReplace::SearchMode_Regex:
            return static_cast<FindReplace::SearchMode>(mode);
            break;

        case FindReplace::SearchMode_Case_Sensitive:
            return static_cast<FindReplace::SearchMode>(mode);
            break;

        default:
            return FindReplace::SearchMode_Normal;
    }
}

FindReplace::LookWhere FindReplace::GetLookWhere()
{
    int look = ui.cbLookWhere->itemData(ui.cbLookWhere->currentIndex()).toInt();

    switch (look) {
        case FindReplace::LookWhere_AllHTMLFiles:
            return static_cast<FindReplace::LookWhere>(look);
            break;

        case FindReplace::LookWhere_SelectedHTMLFiles:
            return static_cast<FindReplace::LookWhere>(look);
            break;

        default:
            return FindReplace::LookWhere_CurrentFile;
    }
}

FindReplace::SearchDirection FindReplace::GetSearchDirection()
{
    int direction = ui.cbSearchDirection->itemData(ui.cbSearchDirection->currentIndex()).toInt();

    switch (direction) {
        case FindReplace::SearchDirection_Up:
            return static_cast<FindReplace::SearchDirection>(direction);
            break;

        default:
            return FindReplace::SearchDirection_Down;
    }
}


bool FindReplace::IsValidFindText()
{
    return  !ui.cbFind->lineEdit()->text().isEmpty();
}


void FindReplace::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // Find and Replace history
    QStringList find_strings = settings.value("find_strings").toStringList();
    find_strings.removeDuplicates();
    ui.cbFind->clear();
    ui.cbFind->addItems(find_strings);
    QStringList replace_strings = settings.value("replace_strings").toStringList();
    replace_strings.removeDuplicates();
    ui.cbReplace->clear();
    ui.cbReplace->addItems(replace_strings);
    SetSearchMode(settings.value("search_mode", 0).toInt());
    SetLookWhere(settings.value("look_where", 0).toInt());
    SetSearchDirection(settings.value("search_direction", 0).toInt());
    bool regexOptionDotAll = settings.value("regexoptiondotall", false).toBool();
    SetRegexOptionDotAll(regexOptionDotAll);
    bool regexOptionMinimalMatch = settings.value("regexoptionminimalmatch", false).toBool();
    SetRegexOptionMinimalMatch(regexOptionMinimalMatch);
    bool regexOptionAutoTokenise = settings.value("regexoptionautotokenise", false).toBool();
    SetRegexOptionAutoTokenise(regexOptionAutoTokenise);
    bool optionWrap = settings.value("optionwrap", true).toBool();
    SetOptionWrap(optionWrap);
    settings.endGroup();
}

void FindReplace::ShowHide()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    QVariant show_find_replace = settings.value("visible");
    settings.endGroup();

    // Hide the window by default
    if (show_find_replace.isNull() ? false : show_find_replace.toBool()) {
        show();
    } else {
        hide();
    }
}

void FindReplace::ShowHideAdvancedOptions()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    bool show_advanced = settings.value("advanced_visible", true).toBool();
    settings.endGroup();
    ui.optionsl->setVisible(show_advanced);
    ui.chkRegexOptionDotAll->setVisible(show_advanced);
    ui.chkRegexOptionMinimalMatch->setVisible(show_advanced);
    ui.chkRegexOptionAutoTokenise->setVisible(show_advanced);
    ui.chkOptionWrap->setVisible(show_advanced);
    ui.count->setVisible(show_advanced);
    QIcon icon;

    if (show_advanced) {
        icon.addFile(QString::fromUtf8(":/main/chevron-up_16px.png"));
        ui.advancedShowHide->setIcon(icon);
    } else {
        icon.addFile(QString::fromUtf8(":/main/chevron-down_16px.png"));
        ui.advancedShowHide->setIcon(icon);
    }
}

void FindReplace::WriteSettingsVisible(bool visible)
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("visible", visible);
    settings.endGroup();
}

void FindReplace::WriteSettingsAdvancedVisible(bool visible)
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("advanced_visible", visible);
    settings.endGroup();
}

void FindReplace::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("find_strings", GetPreviousFindStrings());
    settings.setValue("replace_strings", GetPreviousReplaceStrings());
    settings.setValue("search_mode", GetSearchMode());
    settings.setValue("look_where", GetLookWhere());
    settings.setValue("search_direction", GetSearchDirection());
    settings.setValue("regexoptiondotall", ui.chkRegexOptionDotAll->isChecked());
    settings.setValue("regexoptionminimalmatch", ui.chkRegexOptionMinimalMatch->isChecked());
    settings.setValue("regexoptionautotokenise", ui.chkRegexOptionAutoTokenise->isChecked());
    settings.setValue("optionwrap", ui.chkOptionWrap->isChecked());
    settings.endGroup();
}


Searchable *FindReplace::GetAvailableSearchable()
{
    Searchable *searchable = m_MainWindow.GetCurrentContentTab().GetSearchableContent();

    if (!searchable) {
        ShowMessage(tr("This tab cannot be searched"));
    }

    return searchable;
}


void FindReplace::SaveSearchAction()
{
    SearchEditorModel::searchEntry *search_entry = new SearchEditorModel::searchEntry();
    search_entry->name = "Unnamed Search";
    search_entry->is_group = false;
    search_entry->find = ui.cbFind->lineEdit()->text(),
                  search_entry->replace = ui.cbReplace->lineEdit()->text(),
                                emit OpenSearchEditorRequest(search_entry);
}

void FindReplace::LoadSearchByName(const QString &name)
{
    LoadSearch(SearchEditorModel::instance()->GetEntryFromName(name));
}

void FindReplace::LoadSearch(SearchEditorModel::searchEntry *search_entry)
{
    if (!search_entry) {
        clearMessage();
        return;
    }

    UpdatePreviousFindStrings(search_entry->find);
    UpdatePreviousReplaceStrings(search_entry->replace);
    // Show a message containing the name that was loaded
    QString message(tr("Unnamed search loaded"));

    if (!search_entry->name.isEmpty()) {
        message = QString("%1: %2 ").arg(tr("Loaded")).arg(search_entry->name.replace('<', "&lt;").replace('>', "&gt;").left(50));
    }

    ShowMessage(message);
}

void FindReplace::FindSearch(QList<SearchEditorModel::searchEntry *> search_entries)
{
    if (search_entries.isEmpty()) {
        ShowMessage(tr("No searches selected"));
        return;
    }

    SetKeyModifiers();
    m_IsSearchGroupRunning = true;
    foreach(SearchEditorModel::searchEntry * search_entry, search_entries) {
        LoadSearch(search_entry);

        if (Find()) {
            break;
        };
    }
    m_IsSearchGroupRunning = false;
    ResetKeyModifiers();
}

void FindReplace::ReplaceCurrentSearch(QList<SearchEditorModel::searchEntry *> search_entries)
{
    if (search_entries.isEmpty()) {
        ShowMessage(tr("No searches selected"));
        return;
    }

    m_IsSearchGroupRunning = true;
    foreach(SearchEditorModel::searchEntry * search_entry, search_entries) {
        LoadSearch(search_entry);

        if (ReplaceCurrent()) {
            break;
        }
    }
    m_IsSearchGroupRunning = false;
}

void FindReplace::ReplaceSearch(QList<SearchEditorModel::searchEntry *> search_entries)
{
    if (search_entries.isEmpty()) {
        ShowMessage(tr("No searches selected"));
        return;
    }

    SetKeyModifiers();
    m_IsSearchGroupRunning = true;
    foreach(SearchEditorModel::searchEntry * search_entry, search_entries) {
        LoadSearch(search_entry);

        if (Replace()) {
            break;
        }
    }
    m_IsSearchGroupRunning = false;
    ResetKeyModifiers();
}

void FindReplace::CountAllSearch(QList<SearchEditorModel::searchEntry *> search_entries)
{
    if (search_entries.isEmpty()) {
        ShowMessage(tr("No searches selected"));
        return;
    }

    SetKeyModifiers();
    m_IsSearchGroupRunning = true;
    int count = 0;
    foreach(SearchEditorModel::searchEntry * search_entry, search_entries) {
        LoadSearch(search_entry);
        count += Count();
    }
    m_IsSearchGroupRunning = false;

    if (count == 0) {
        CannotFindSearchTerm();
    } else {
        QString message = tr("Matches found: %n", "", count);
        ShowMessage(message);
    }

    ResetKeyModifiers();
}

void FindReplace::ReplaceAllSearch(QList<SearchEditorModel::searchEntry *> search_entries)
{
    if (search_entries.isEmpty()) {
        ShowMessage(tr("No searches selected"));
        return;
    }

    SetKeyModifiers();
    m_IsSearchGroupRunning = true;
    int count = 0;
    foreach(SearchEditorModel::searchEntry * search_entry, search_entries) {
        LoadSearch(search_entry);
        count += ReplaceAll();
    }
    m_IsSearchGroupRunning = false;

    if (count == 0) {
        ShowMessage(tr("No replacements made"));
    } else {
        QString message = tr("Replacements made: %n", "", count);
        ShowMessage(message);
    }

    ResetKeyModifiers();
}


void FindReplace::SetSearchMode(int search_mode)
{
    ui.cbSearchMode->setCurrentIndex(0);

    for (int i = 0; i < ui.cbSearchMode->count(); ++i) {
        if (ui.cbSearchMode->itemData(i) == search_mode) {
            ui.cbSearchMode->setCurrentIndex(i);
            break;
        }
    }
}

void FindReplace::SetLookWhere(int look_where)
{
    ui.cbLookWhere->setCurrentIndex(0);

    for (int i = 0; i < ui.cbLookWhere->count(); ++i) {
        if (ui.cbLookWhere->itemData(i)  == look_where) {
            ui.cbLookWhere->setCurrentIndex(i);
            break;
        }
    }
}

void FindReplace::SetSearchDirection(int search_direction)
{
    ui.cbSearchDirection->setCurrentIndex(0);

    for (int i = 0; i < ui.cbSearchDirection->count(); ++i) {
        if (ui.cbSearchDirection->itemData(i) == search_direction) {
            ui.cbSearchDirection->setCurrentIndex(i);
            break;
        }
    }
}

void FindReplace::TokeniseSelection()
{
    if (!IsValidFindText()) {
        return;
    }

    QString text;

    if (ui.cbFind->lineEdit()->hasSelectedText()) {
        // We want to tokenise only the selection
        text = ui.cbFind->lineEdit()->selectedText();
    } else {
        // We will tokenise the whole thing
        text = ui.cbFind->lineEdit()->text();
    }

    QString new_text = TokeniseForRegex(text, true);

    if (new_text != text) {
        if (ui.cbFind->lineEdit()->hasSelectedText()) {
            // We will paste in the new text so the user has the ability to undo.
            ui.cbFind->PasteText(new_text);
        } else {
            // We still want to paste in, but we replacing all the text that is in there
            ui.cbFind->lineEdit()->selectAll();
            ui.cbFind->PasteText(new_text);
        }
    }
}

QString FindReplace::TokeniseForRegex(const QString &text, bool includeNumerics)
{
    QString new_text(text);

    if (!text.contains("\\")) {
        // Going to "assume" that this text has already been escaped by the
        // auto-tokenise logic or the user running tokenise already.
        new_text = QRegularExpression::escape(text);
    }

    // Replace spaces.
    new_text.replace(QRegularExpression("([\\n\\s]{2,})"), "\\s+");

    if (includeNumerics) {
        // Replace numerics.
        new_text.replace(QRegularExpression("(\\d+)"), "\\d+");
    }

    return new_text;
}

void FindReplace::SetRegexOptionDotAll(bool new_state)
{
    m_RegexOptionDotAll = new_state;
    ui.chkRegexOptionDotAll->setChecked(new_state);
}

void FindReplace::SetRegexOptionMinimalMatch(bool new_state)
{
    m_RegexOptionMinimalMatch = new_state;
    ui.chkRegexOptionMinimalMatch->setChecked(new_state);
}

void FindReplace::SetRegexOptionAutoTokenise(bool new_state)
{
    m_RegexOptionAutoTokenise = new_state;
    ui.chkRegexOptionAutoTokenise->setChecked(new_state);
}

void FindReplace::SetOptionWrap(bool new_state)
{
    m_OptionWrap = new_state;
    ui.chkOptionWrap->setChecked(new_state);
}

// The UI is setup based on the capabilities.
void FindReplace::ExtendUI()
{
    // Clear these because we want to add their items based on the
    // capabilities.
    ui.cbSearchMode->clear();
    ui.cbLookWhere->clear();
    ui.cbSearchDirection->clear();
    QString mode_tooltip = "<p>" + tr("What to search for") + ":</p><dl>";
    ui.cbSearchMode->addItem(tr("Normal"), FindReplace::SearchMode_Normal);
    mode_tooltip += "<dt><b>Normal</b><dd>" + tr("Case in-sensitive search of exactly what you type.") + "</dd>";
    ui.cbSearchMode->addItem(tr("Case Sensitive"), FindReplace::SearchMode_Case_Sensitive);
    mode_tooltip += "<dt><b>Case Sensitive</b><dd>" + tr("Case sensitive search of exactly what you type.") + "</dd>";
    ui.cbSearchMode->addItem(tr("Regex"), FindReplace::SearchMode_Regex);
    mode_tooltip += "<dt><b>Regex</b><dd>" + tr("Search for a pattern using Regular Expression syntax.") + "</dd>";
    ui.cbSearchMode->setToolTip(mode_tooltip);
    QString look_tooltip = "<p>" + tr("Where to search") + ":</p><dl>";
    ui.cbLookWhere->addItem(tr("Current File"), FindReplace::LookWhere_CurrentFile);
    look_tooltip += "<dt><b>Current File</b><dd>" + tr("Restrict the find or replace to the opened file.  Hold the Ctrl key down while clicking any search buttons to temporarily restrict the search to the Current File.") + "</dd>";
    ui.cbLookWhere->addItem(tr("All HTML Files"), FindReplace::LookWhere_AllHTMLFiles);
    look_tooltip += "<dt><b>All HTML Files</b><dd>" + tr("Find or replace in all HTML files in Code View.") + "</dd>";
    ui.cbLookWhere->addItem(tr("Selected Files"), FindReplace::LookWhere_SelectedHTMLFiles);
    look_tooltip += "<dt><b>Selected Files</b><dd>" + tr("Restrict the find or replace to the HTML files selected in the Book Browser in Code View.") + "</dd>";
    look_tooltip += "</dl>";
    ui.cbLookWhere->setToolTip(look_tooltip);
    ui.cbSearchDirection->addItem(tr("Up"), FindReplace::SearchDirection_Up);
    ui.cbSearchDirection->addItem(tr("Down"), FindReplace::SearchDirection_Down);
    ui.cbSearchDirection->setToolTip("<p>" + tr("Direction to search") + ":</p>"
                                     "<dl>"
                                     "<dt><b>Up</b><dd>" + tr("Search for the previous match from your current position.") + "</dd>"
                                     "<dt><b>Down</b><dd>" + tr("Search for the next match from your current position.") + "</dd>"
                                     "</dl>");
}

void FindReplace::ConnectSignalsToSlots()
{
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(expireMessage()));
    connect(ui.findNext, SIGNAL(clicked()), this, SLOT(FindClicked()));
    connect(ui.cbFind->lineEdit(), SIGNAL(returnPressed()), this, SLOT(Find()));
    connect(ui.count, SIGNAL(clicked()), this, SLOT(CountClicked()));
    connect(ui.replaceCurrent, SIGNAL(clicked()), this, SLOT(ReplaceCurrent()));
    connect(ui.replaceFind, SIGNAL(clicked()), this, SLOT(ReplaceClicked()));
    connect(ui.cbReplace->lineEdit(), SIGNAL(returnPressed()), this, SLOT(Replace()));
    connect(ui.replaceAll, SIGNAL(clicked()), this, SLOT(ReplaceAllClicked()));
    connect(ui.close, SIGNAL(clicked()), this, SLOT(HideFindReplace()));
    connect(ui.advancedShowHide, SIGNAL(clicked()), this, SLOT(AdvancedOptionsClicked()));
    connect(ui.cbFind, SIGNAL(ClipboardSaveRequest()), this, SIGNAL(ClipboardSaveRequest()));
    connect(ui.cbFind, SIGNAL(ClipboardRestoreRequest()), this, SIGNAL(ClipboardRestoreRequest()));
    connect(ui.cbReplace, SIGNAL(ClipboardSaveRequest()), this, SIGNAL(ClipboardSaveRequest()));
    connect(ui.cbReplace, SIGNAL(ClipboardRestoreRequest()), this, SIGNAL(ClipboardRestoreRequest()));
    connect(ui.chkRegexOptionDotAll, SIGNAL(clicked(bool)), this, SLOT(SetRegexOptionDotAll(bool)));
    connect(ui.chkRegexOptionMinimalMatch, SIGNAL(clicked(bool)), this, SLOT(SetRegexOptionMinimalMatch(bool)));
    connect(ui.chkRegexOptionAutoTokenise, SIGNAL(clicked(bool)), this, SLOT(SetRegexOptionAutoTokenise(bool)));
    connect(ui.chkOptionWrap, SIGNAL(clicked(bool)), this, SLOT(SetOptionWrap(bool)));
}
