/***************************************************************************
**
**  Copyright (C) 2015-2022 Kevin B. Hendricks, Stratford, Ontario, Canada
**  Copyright (C) 2011-2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012      Dave Heiland
**  Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#define PCRE2_CODE_UNIT_WIDTH 16
#include <pcre2.h>

#include <QString>
#include <QAction>
#include <QMenu>
// #include <QToolButton>
#include <QToolButton>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMessageBox>
#include <QCompleter>
#include <QRegularExpression>
#include <QDebug>

#include "Dialogs/DryRunReplace.h"
#include "Dialogs/ReplacementChooser.h"
#include "Tabs/TextTab.h"
#include "Tabs/FlowTab.h"
#include "MainUI/FindReplace.h"
#include "Misc/SettingsStore.h"
#include "Misc/FindReplaceQLineEdit.h"
#include "PCRE2/PCREErrors.h"
#include "ResourceObjects/Resource.h"
#include "ResourceObjects/TextResource.h"

#define DBG if(0)

static const QString SETTINGS_GROUP = "find_replace";
static const QString REGEX_OPTION_UCP = "(*UCP)";
static const QString REGEX_OPTION_IGNORE_CASE = "(?i)";
static const QString REGEX_OPTION_DOT_ALL = "(?s)";
static const QString REGEX_OPTION_MINIMAL_MATCH = "(?U)";
static const QString REGEX_OPTION_TEXT_ONLY = "<[^<>]*>(*SKIP)(*F)|";

static const int SHOW_FIND_RESULTS_MESSAGE_DELAY_MS = 20000;

// mappings from LookWhere, Search, and Direction enums to Controls code
// Must be kept in sync with those enums
static const QStringList TGTS = QStringList() << "CF" << "AH" << "SH" << "TH" << "AC" << "SC" << "TC" << "OP" << "NX";
static const QStringList MDS = QStringList() << "NL" << "CS" << "RX";
static const QStringList DRS = QStringList() << "DN" << "UP";

static const QString INVALID = QString(QChar(9940));
static const QString VALID = QString("");


FindReplace::FindReplace(MainWindow *main_window)
    : QWidget(main_window),
      m_MainWindow(main_window),
      m_RegexOptionDotAll(false),
      m_RegexOptionMinimalMatch(false),
      m_RegexOptionAutoTokenise(false),
      m_OptionWrap(true),
      m_RegexOptionTextOnly(false),
      m_SpellCheck(false),
      m_LookWhereCurrentFile(false),
      m_IsSearchGroupRunning(false),
      m_StartingResource(nullptr),
      m_StartingPos(-1),
      m_InRemainder(false),
      m_RestartPerformed(false),
      m_SearchRunning(false),
      m_DryRunRunning(false),
      m_ShiftUsed(false),
      m_DotAllCheckAction(nullptr),
      m_MinimalMatchCheckAction(nullptr),
      m_AutoTokeniseCheckAction(nullptr),
      m_menu(nullptr)
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
    // ShowHideAdvancedOptions();
    ShowHideMarkedText(false);
    ReadSettings();
    m_PreviousSearch.clear();
}


// Destructor
FindReplace::~FindReplace()
{
    WriteSettings();
    if (m_DotAllCheckAction) {
        delete m_DotAllCheckAction;
        m_DotAllCheckAction = nullptr;
    }
    if (m_MinimalMatchCheckAction) {
        delete m_MinimalMatchCheckAction;
        m_MinimalMatchCheckAction = nullptr;
    }
    if (m_AutoTokeniseCheckAction) {
        delete m_AutoTokeniseCheckAction;
        m_AutoTokeniseCheckAction = nullptr;
    }
    if (m_menu) {
        delete m_menu;
        m_menu = nullptr;
    }
}

void FindReplace::SetPreviousSearch()
{
    m_PreviousSearch.clear();
    m_PreviousSearch << ui.cbFind->lineEdit()->text();
    m_PreviousSearch << TGTS.at(GetLookWhere());
    m_PreviousSearch << DRS.at(GetSearchDirection());
}

bool FindReplace::IsNewSearch()
{
    if (m_PreviousSearch.count() != 3) return true;
    if (m_PreviousSearch.at(0) != ui.cbFind->lineEdit()->text()) return true;
    if (m_PreviousSearch.at(1) != TGTS.at(GetLookWhere())) return true;
    if (m_PreviousSearch.at(2) != DRS.at(GetSearchDirection())) return true;
    return false;
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


QString FindReplace::GetControls()
{
    QStringList  controls;
    controls << MDS.at(GetSearchMode());
    if (m_RegexOptionDotAll) controls << "DA";
    if (m_RegexOptionMinimalMatch) controls << "MM";
    if (m_RegexOptionAutoTokenise) controls << "AT";
    if (m_OptionWrap) controls << "WR";
    if (m_RegexOptionTextOnly) controls << "TO";
    controls << DRS.at(GetSearchDirection());
    controls << TGTS.at(GetLookWhere());
    return controls.join(' ');
}


bool FindReplace::isSearchXML()
{
    if (isWhereHTML() || isWhereOPF() || isWhereNCX()) return true;
    if (isWhereCSS()) return false;
    if (isWhereCF() || m_LookWhereCurrentFile) {
        Resource * current_resource = GetCurrentResource();
        QString mt = current_resource->GetMediaType();
        if (mt.endsWith("+xml")) return true;
    }
    return false;
}


bool FindReplace::isWhereHTML()
{
    if ((GetLookWhere() == FindReplace::LookWhere_AllHTMLFiles) ||
        (GetLookWhere() == FindReplace::LookWhere_SelectedHTMLFiles) ||
        (GetLookWhere() == FindReplace::LookWhere_TabbedHTMLFiles)) return true;
    return false;
}


bool FindReplace::isWhereCSS()
{
    if ((GetLookWhere() == FindReplace::LookWhere_AllCSSFiles) ||
        (GetLookWhere() == FindReplace::LookWhere_SelectedCSSFiles) ||
        (GetLookWhere() == FindReplace::LookWhere_TabbedCSSFiles)) return true;
    return false;
}


bool FindReplace::isWhereSelected()
{
    if ((GetLookWhere() == FindReplace::LookWhere_SelectedHTMLFiles) ||
        (GetLookWhere() == FindReplace::LookWhere_TabbedHTMLFiles) ||
        (GetLookWhere() == FindReplace::LookWhere_SelectedCSSFiles) ||
        (GetLookWhere() == FindReplace::LookWhere_TabbedCSSFiles) ||
        (GetLookWhere() == FindReplace::LookWhere_OPFFile) ||
        (GetLookWhere() == FindReplace::LookWhere_NCXFile)) return true;
    return false;
}


bool FindReplace::isWhereAll()
{
    if ((GetLookWhere() == FindReplace::LookWhere_AllHTMLFiles) ||
        (GetLookWhere() == FindReplace::LookWhere_AllCSSFiles)) return true;
    return false;
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


void FindReplace::ShowHideMarkedText(bool marked)
{
    if (marked) {
        ui.cbLookWhere->hide();
        ui.MarkedTextIndicator->show();
    } else {
        ui.cbLookWhere->show();
        ui.MarkedTextIndicator->hide();
    }
}

bool FindReplace::IsMarkedText()
{
    return !ui.MarkedTextIndicator->isHidden();
}


void FindReplace::HideFindReplace()
{
    WriteSettingsVisible(false);
    hide();
}

void FindReplace::DoRestart()
{
    m_PreviousSearch.clear();
    ShowMessage(tr("Search will restart"));
}

void FindReplace::RestartClicked()
{
    m_PreviousSearch.clear();
    m_RestartPerformed = true;
    ShowMessage(tr("Search will restart"));
}

#if 0
void FindReplace::AdvancedOptionsClicked()
{
    bool is_currently_visible = ui.tbRegexOptions->isVisible();
    WriteSettingsAdvancedVisible(!is_currently_visible);
    ShowHideAdvancedOptions();
}
#endif

void FindReplace::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        HideFindReplace();
    }
}


void FindReplace::ShowMessage(const QString &message)
{
    QString new_message = message;

    if (m_LookWhereCurrentFile && !isWhereCF()) {
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
    m_ShiftUsed = QApplication::keyboardModifiers() & Qt::ShiftModifier;
}

void FindReplace::ResetKeyModifiers()
{
    m_LookWhereCurrentFile = false;
    m_ShiftUsed = false;
}

void FindReplace::FindClicked()
{
    SetKeyModifiers();
    Find();
    ResetKeyModifiers();
}

void FindReplace::ReplaceClicked()
{
    // This is really ReplaceFind";
    SetKeyModifiers();
    Replace();
    ResetKeyModifiers();
}

void FindReplace::ReplaceAllClicked()
{
    SetKeyModifiers();
    if (m_ShiftUsed) {
        ChooseReplacements();
    } else {
        ReplaceAll();
    }
    ResetKeyModifiers();
}

void FindReplace::CountClicked()
{
    SetKeyModifiers();
    if (m_ShiftUsed) {
        PerformDryRunReplace();
    } else {
        Count();
    }
    ResetKeyModifiers();
}


bool FindReplace::FindAnyText(QString text, bool escape)
{
    // bool had_focus = HasFocus();
    SetCodeViewIfNeeded();
    WriteSettings();

    SetSearchMode(FindReplace::SearchMode_Regex);
    SetLookWhere(FindReplace::LookWhere_AllHTMLFiles);
    SetSearchDirection(FindReplace::SearchDirection_Down);
    SetRegexOptionDotAll(true);
    SetRegexOptionTextOnly(false);
    SetRegexOptionMinimalMatch(true);
    // SetOptionWrap(true);

    QString search_text;
    if (escape) {
        search_text = QRegularExpression::escape(text);
    } else {
        search_text = text + "(?![^<>]*>)(?!.*<body[^>]*>)";
    }
    ui.cbFind->setEditText(search_text);
    bool found = Find();
    ReadSettings();
    // Show the search term in case it's needed
    ui.cbFind->setEditText(search_text);
    // RestoreFRFocusIfNeeded(had_focus, true);
    return found;
}

void FindReplace::FindAnyTextInTags(QString text)
{
    // bool had_focus = HasFocus();
    SetCodeViewIfNeeded();
    WriteSettings();

    SetSearchMode(FindReplace::SearchMode_Regex);
    SetLookWhere(FindReplace::LookWhere_AllHTMLFiles);
    SetSearchDirection(FindReplace::SearchDirection_Down);
    SetRegexOptionDotAll(true);
    SetRegexOptionMinimalMatch(true);
    // SetOptionWrap(true);
    SetRegexOptionTextOnly(false);
    text = text + "(?=[^<]*>)(?!(?:[^<\"]*\"[^<\"]*\")+\\s*/?>)";
    ui.cbFind->setEditText(text);
    Find();

    ReadSettings();
    // RestoreFRFocusIfNeeded(had_focus, true);
}


bool FindReplace::DoFindNext()
{
    if (m_SearchRunning) return false;
    m_SearchRunning = true;
    SetSearchDirection(FindReplace::SearchDirection_Down);
    bool found = Find();
    m_SearchRunning = false;
    return found;
}

bool FindReplace::DoFindPrevious()
{
    if (m_SearchRunning) return	false;
    m_SearchRunning = true;
    SetSearchDirection(FindReplace::SearchDirection_Up);
    bool found = Find();
    m_SearchRunning = false;
    return found;
}

bool FindReplace::DoReplaceNext()
{
    if (m_SearchRunning) return	false;
    m_SearchRunning = true;
    SetSearchDirection(FindReplace::SearchDirection_Down);
    bool found = Replace();
    m_SearchRunning = false;
    return found;
    
}

bool FindReplace::DoReplacePrevious()
{
    if (m_SearchRunning) return	false;
    m_SearchRunning = true;
    SetSearchDirection(FindReplace::SearchDirection_Up);
    bool found = Replace();
    m_SearchRunning = false;
    return found;
}

bool FindReplace::Find()
{
    DBG qDebug() << "Find";
    if (IsNewSearch()) {
        DBG qDebug() << " .. new search";
        SetStartingResource(true);
        SetPreviousSearch();
    }
    
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

    // count changes nothing and should not change
    // current file and position, nor update previous search

    if (IsNewSearch()) {
        DBG qDebug() << " .. new search";
        SetStartingResource(true);
        SetPreviousSearch();
    }
    
    if (!IsValidFindText()) {
        return 0;
    }

    // bool had_focus = HasFocus();
    SetCodeViewIfNeeded();
    int count = 0;

    if (isWhereCF() || m_LookWhereCurrentFile || IsMarkedText()) {
        Searchable *searchable = GetAvailableSearchable();

        if (!searchable) {
            return 0;
        }

        count = searchable->Count(GetSearchRegex(), GetSearchableDirection(), m_OptionWrap, IsMarkedText());
    } else {
        count = CountInFiles();
        // If wrap, all files are counted, otherwise only files before/after
        // the current file are counted, and then added to the count of current file.
        // if (!m_OptionWrap) {
        //    Searchable *searchable = GetAvailableSearchable();
        //    if (searchable) {
        //       count += searchable->Count(GetSearchRegex(), GetSearchableDirection(), m_OptionWrap);
        //    }
        // }
    }

    if (count == 0) {
        CannotFindSearchTerm();
    } else if (count > 0) {
        QString message = tr("Matches found: %n", "", count);
        ShowMessage(message);
    }

    UpdatePreviousFindStrings();
    // RestoreFRFocusIfNeeded(had_focus, true);
    return count;
}


bool FindReplace::Replace()
{
    DBG qDebug() << "In Replace";
    if (IsNewSearch()) {
        DBG qDebug() << " .. new search";
        SetStartingResource(true);
        SetPreviousSearch();
    }

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
    DBG qDebug() << "In ReplaceCurrent";

    // isNewSearch should always return false here
    // as search must have already found something to replace

    bool found = false;

    if (GetSearchDirection() == FindReplace::SearchDirection_Up) {
        found = ReplaceText(Searchable::Direction_Up, true);
    } else {
        found = ReplaceText(Searchable::Direction_Down, true);
    }

    return found;
}


// Builds a Find All table
void FindReplace::PerformDryRunReplace()
{
    if (m_DryRunRunning) return;
    m_DryRunRunning = true;

    m_MainWindow->GetCurrentContentTab()->SaveTabContent();

    if (IsNewSearch()) {
        SetStartingResource(true);
        SetPreviousSearch();
    }

    if (!IsValidFindText()) return;

    DryRunReplace*  dr = new DryRunReplace(this);
    connect(dr, &QWidget::destroyed, this, &FindReplace::DryRunComplete);
    dr->CreateTable();
    // do this non-modally
    dr->show();
    dr->raise();
    dr->activateWindow();
}

// Allows you to delete unwanted replacements and apply those remaining
void FindReplace::ChooseReplacements()
{
    m_MainWindow->GetCurrentContentTab()->SaveTabContent();
    ShowMessage(tr("Choose Replacements"));

    if (IsNewSearch()) {
        SetStartingResource(true);
        SetPreviousSearch();
    }

    if (!IsValidFindText()) return;

    // must be modal to prevent crashes and nonsense
    ReplacementChooser rc(this);
    rc.CreateTable();
    rc.exec();

    clearMessage();

    int count = rc.GetReplacementCount();
    if (count == 0) {
        ShowMessage(tr("No replacements made"));
    } else if (count > 0) {
        QString message = tr("Replacements made: %n", "", count);
        ShowMessage(message);
    }

    if (count > 0) {
        // Signal that the contents have changed and update the view
        m_MainWindow->GetCurrentBook()->SetModified(true);
        m_MainWindow->GetCurrentContentTab()->ContentChangedExternally();
    }

    UpdatePreviousFindStrings();
    UpdatePreviousReplaceStrings();
}


// Replaces the user's search term with the user's
// replacement text in the entire document.
int FindReplace::ReplaceAll()
{
    m_MainWindow->GetCurrentContentTab()->SaveTabContent();
    clearMessage();

    if (IsNewSearch()) {
        DBG qDebug() << " .. new search";
        SetStartingResource(true);
        SetPreviousSearch();
    }

    if (!IsValidFindText()) {
        return 0;
    }

    // bool had_focus = HasFocus();
    SetCodeViewIfNeeded();
    int count = 0;

    if (isWhereCF() || m_LookWhereCurrentFile || IsMarkedText()) {
        Searchable *searchable = GetAvailableSearchable();

        if (!searchable) {
            return 0;
        }

        count = searchable->ReplaceAll(GetSearchRegex(), ui.cbReplace->lineEdit()->text(), GetSearchableDirection(), m_OptionWrap, IsMarkedText());
    } else {
        count = ReplaceInAllFiles();
        // If wrap, all files are replaced, otherwise only files before/after
        // the current file are updated, and then the current file is done.
        // if (!m_OptionWrap) {
        //     Searchable *searchable = GetAvailableSearchable();
        //     if (searchable) {
        //         count += searchable->ReplaceAll(GetSearchRegex(), ui.cbReplace->lineEdit()->text(), GetSearchableDirection(), m_OptionWrap);
        //     }
        // }
    }

    if (count == 0) {
        ShowMessage(tr("No replacements made"));
    } else if (count > 0) {
        QString message = tr("Replacements made: %n", "", count);
        ShowMessage(message);
    }

    if (count > 0) {
        // Signal that the contents have changed and update the view
        m_MainWindow->GetCurrentBook()->SetModified(true);
        m_MainWindow->GetCurrentContentTab()->ContentChangedExternally();
    }

    UpdatePreviousFindStrings();
    UpdatePreviousReplaceStrings();
    // RestoreFRFocusIfNeeded(had_focus, true);
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
    // bool had_focus = HasFocus();
    SetCodeViewIfNeeded();
    m_SpellCheck = true;

    WriteSettings();
    // Only files, direction, wrap are checked for misspelled searches
    SetLookWhere(FindReplace::LookWhere_AllHTMLFiles);
    SetSearchDirection(FindReplace::SearchDirection_Down);
    // SetOptionWrap(true);

    bool found = FindInAllFiles(Searchable::Direction_Down);

    ReadSettings();
    m_SpellCheck = false;

    if (found) {
        clearMessage();
    } else {
        CannotFindSearchTerm();
    }

    // RestoreFRFocusIfNeeded(had_focus, true);
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

    // bool had_focus = HasFocus();
    SetCodeViewIfNeeded();

    if (isWhereCF() || m_LookWhereCurrentFile || IsMarkedText()) {
        Searchable *searchable = GetAvailableSearchable();

        if (!searchable) {
            return found;
        }

        found = searchable->FindNext(GetSearchRegex(), direction, false, false, m_OptionWrap, IsMarkedText());
    } else {
        found = FindInAllFiles(direction);
    }

    if (found) {
        clearMessage();
    } else {
        CannotFindSearchTerm();
    }

    UpdatePreviousFindStrings();
    // RestoreFRFocusIfNeeded(had_focus);
    return found;
}


// Replaces the user's search term with the user's
// replacement text if a match is selected. If it's not,
// calls Find in the direction specified so it becomes selected.
bool FindReplace::ReplaceText(Searchable::Direction direction, bool replace_current)
{
    DBG qDebug() << "In ReplaceText with replace_current: " << replace_current;
    bool found = false;
    clearMessage();

    if (!IsValidFindText()) {
        return found;
    }

    // bool had_focus = HasFocus();
    SetCodeViewIfNeeded();
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
    // RestoreFRFocusIfNeeded(had_focus, true);
    // Do not use the return value to tell if a replace was done - only if a complete
    // Find/Replace or ReplaceCurrent was ok.  This allows multiple selections to work as expected.
    return found;
}

void FindReplace::SetCodeViewIfNeeded()
{
    bool has_focus = HasFocus();
    if (has_focus) {
        // give the current tab CodeView Tab the focus
        ui.cbFind->lineEdit()->clearFocus();
        ContentTab * current_tab = m_MainWindow->GetCurrentContentTab();
        if (current_tab) current_tab->setFocus();
    }
}

#if 0
// This originally was the code at the end of SetCodeViewIfNeeded(force)
// But it made no sense cause it did not really accomplish anything
// where it was.  Even splitting it out into a routine to be done
// at the end did not help so I am just leaving it here in case
// this decision comes back to bite me!
void FindReplace::RestoreFRFocusIfNeeded(bool had_focus, bool force)
{
    if (force ||
        (!m_LookWhereCurrentFile && (isWhereHTML() || isWhereCSS() || isWhereOPF() || isWhereNCX())))
    {
        if (had_focus) {
            SetFocus();
        }
    }
}
#endif

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

    QString text = ui.cbFind->lineEdit()->text();
    // Convert &#x2029; to match line separator used by plainText.
    text.replace(QRegularExpression("\\R"), "\n");

    QString search(text);

    // Search type
    if (GetSearchMode() == FindReplace::SearchMode_Normal || GetSearchMode() == FindReplace::SearchMode_Case_Sensitive) {
        search = QRegularExpression::escape(search);
        if (m_RegexOptionTextOnly && isSearchXML()) {
            // must be immediately before the user search
            search = PrependRegexOptionToSearch(REGEX_OPTION_TEXT_ONLY, search);
        }
        if (GetSearchMode() == FindReplace::SearchMode_Normal) {
            search = PrependRegexOptionToSearch(REGEX_OPTION_IGNORE_CASE, search);
        }
    } else {
        // must be immediately before the user search
        if (m_RegexOptionTextOnly && isSearchXML()) {
            search = PrependRegexOptionToSearch(REGEX_OPTION_TEXT_ONLY, search);
        }
        if (m_RegexOptionDotAll) {
            search = PrependRegexOptionToSearch(REGEX_OPTION_DOT_ALL, search);
        }
        if (m_RegexOptionMinimalMatch) {
            search = PrependRegexOptionToSearch(REGEX_OPTION_MINIMAL_MATCH, search);
        }
    }
    // qDebug() << "GetSearchRegex returns: " << search;
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

QString FindReplace::GetReplace()
{
    return ui.cbReplace->lineEdit()->text();
}

QList<Resource*> FindReplace::GetAllResourcesToSearch()
{
    QList<Resource *> resources;

    if (isWhereCF() || m_LookWhereCurrentFile) {
        resources << GetCurrentResource();
    } else {
        resources = GetFilesToSearch(true);
    }
    return resources;
}

void FindReplace::EmitOpenFileRequest(const QString& bookpath, int line, int pos)
{
    emit FROpenFileRequest(bookpath, line, pos);
}

bool FindReplace::IsCurrentFileInSelection()
{
    bool found = false;
    QList <Resource *> resources = GetFilesToSearch();
    if (resources.isEmpty()) return false;

    Resource *current_resource = GetCurrentResource();

    if (current_resource) {
        foreach(Resource * resource, resources) {
            if (resource && resource->GetRelativePath() == current_resource->GetRelativePath()) {
                found = true;
                break;
            }
        }
    }
    DBG qDebug() << "IsCurrentFileInSection: " << found;

    return found;
}


// Returns all resources according to LookWhere setting
QList <Resource *> FindReplace::GetFilesToSearch(bool force_all)
{
    QList <Resource *> all_resources;
    QList <Resource *> resources;

    if (GetLookWhere() == FindReplace::LookWhere_AllHTMLFiles) {
        all_resources = m_MainWindow->GetAllHTMLResources();

    } else if (GetLookWhere() == FindReplace::LookWhere_SelectedHTMLFiles) {
        all_resources = m_MainWindow->GetValidSelectedHTMLResources();

    } else if (GetLookWhere() == FindReplace::LookWhere_TabbedHTMLFiles) {
        all_resources = m_MainWindow->GetTabbedHTMLResources();

    } else if (GetLookWhere() == FindReplace::LookWhere_AllCSSFiles) {
        all_resources = m_MainWindow->GetAllCSSResources();

    } else if (GetLookWhere() == FindReplace::LookWhere_SelectedCSSFiles) {
        all_resources = m_MainWindow->GetValidSelectedCSSResources();

    } else if (GetLookWhere() == FindReplace::LookWhere_TabbedCSSFiles) {
        all_resources = m_MainWindow->GetTabbedCSSResources();

    } else if (GetLookWhere() == FindReplace::LookWhere_OPFFile) {
        all_resources = m_MainWindow->GetOPFResource();

    } else if (GetLookWhere() == FindReplace::LookWhere_NCXFile) {
        all_resources = m_MainWindow->GetNCXResource();
    }

    // special case an empty list or force all
    if (force_all || all_resources.isEmpty()) return all_resources;

    // If starting a new search, or if current resource is the starting resource or
    // or if the current resource is not in the files to search, or if the starting
    // resource is no longer part of all resources that means
    // there is no before/after for search to use) then just return all files
    Resource *current_resource = GetCurrentResource();
    DBG qDebug() << "F2S current: " << current_resource->GetRelativePath();
    if (!m_StartingResource) {
        DBG qDebug() << "F2S starting:  NULL";
    } else {
        DBG qDebug() << "F2S starting: " << m_StartingResource->GetRelativePath();
    }

    if (!m_StartingResource ||
        !all_resources.contains(current_resource) ||
        !all_resources.contains(m_StartingResource) ||
        (m_StartingResource == current_resource)) {
        DBG qDebug() << "F2S returning all resources";
        return all_resources;
    }

    // Otherwise build the list of resources yet to be searched
    int c = -1;
    int s = -1;
    // walk the list once recording the indexes of the current and starting resources
    for (int j=0; j < all_resources.count(); j++) {
        if (all_resources.at(j) == current_resource) {
            c = j;
        }
        if (all_resources.at(j) == m_StartingResource) {
            s = j;
        }
    }

    // For Down that means removing all of the resources from
    // m_StartingResource up to but not including the current resource,
    // while handling the wrap around case where is the current is less than starting
    if (GetSearchDirection() == FindReplace::SearchDirection_Down) {
        bool skip_resource = c < s;
        foreach (Resource *resource, all_resources) {
            if (resource == m_StartingResource) skip_resource = true;
            if (resource == current_resource) skip_resource = false;
            if (! skip_resource) resources.append(resource);
        }
    // For up that means removing all of the resources before and including
    // with the m_StartingResource up to but not including
    // the current resource while properly handling the wrap around case.
    } else {
        bool skip_resource = s < c;
        foreach (Resource *resource, all_resources) {
            if (! skip_resource) resources.append(resource);
            // if the last resource was the starting resource disable skipping for the next 
            if (resource == m_StartingResource) skip_resource = false;
            // if the last resource was the current resource enable skipping for the next
            if (resource == current_resource) skip_resource = true;
        }
    }
    // for (int j=0; j < resources.count(); j++) {
    //     qDebug() << "F2S returning: " << resources.at(j)->GetRelativePath();
    // }
    return resources;
}


int FindReplace::CountInFiles()
{
    m_MainWindow->GetCurrentContentTab()->SaveTabContent();

    QList<Resource *>search_files = GetFilesToSearch(true);
    if (search_files.isEmpty()) return 0;

    // When not wrapping remove the current resource as it's counted separately
    // if (!m_OptionWrap) {
    //     search_files.removeOne(GetCurrentResource());
    // }
    return SearchOperations::CountInFiles(
               GetSearchRegex(),
               search_files);
}

int FindReplace::ReplaceInAllFiles()
{
    m_MainWindow->GetCurrentContentTab()->SaveTabContent();
    QList<Resource *>search_files = GetFilesToSearch(true);
    if (search_files.isEmpty()) return 0;

    int count = SearchOperations::ReplaceInAllFIles(
                    GetSearchRegex(),
                    ui.cbReplace->lineEdit()->text(),
                    search_files);
    return count;
}


bool FindReplace::FindInAllFiles(Searchable::Direction direction)
{
    Searchable *searchable = 0;
    bool found = false;
    Resource * current_resource = GetCurrentResource();

    // special case when doing search in the starting resource
    if (current_resource == m_StartingResource) {
        DBG qDebug() << " FIAF in starting resource";
        if (!m_InRemainder) {
            searchable = GetAvailableSearchable();
            if (searchable) {
                found = searchable->FindNext(GetSearchRegex(), direction, m_SpellCheck, false, false);
            }
            if (!found) m_InRemainder = true;
        }
        if (m_InRemainder && (m_StartingPos != -1)) {
            DBG qDebug() << " FIAF in Remainder";
            searchable = GetAvailableSearchable();
            if (searchable) {
                found = searchable->FindNext(GetSearchRegex(), direction, m_SpellCheck, false, false, false, m_StartingPos);
            }
            DBG qDebug() << " FIAF in Remainder: " << m_StartingPos << found;

        }
    } else if (IsCurrentFileInSelection()) {
        searchable = GetAvailableSearchable();
        if (searchable) {
            found = searchable->FindNext(GetSearchRegex(), direction, m_SpellCheck, false, false);
        }
    }

    if (!found) {
        Resource *containing_resource = GetNextContainingResource(direction);
        DBG qDebug() << " .. FindInAllFiles GetNextContainingResource" << containing_resource;

        if (containing_resource) {
            DBG qDebug() << " .. which is " << containing_resource->GetRelativePath();

            // Save if editor or F&R has focus
            bool has_focus = HasFocus();
            // Save selected resources since opening tabs changes selection
            QList<Resource *>selected_resources = m_MainWindow->GetBookBrowserSelectedResources();

            m_MainWindow->OpenResourceAndWaitUntilLoaded(containing_resource);
            // give the new tab initial focus
            ContentTab * current_tab = m_MainWindow->GetCurrentContentTab();
            if (current_tab) current_tab->setFocus();

            // Restore selection since opening tabs changes selection
            if (isWhereSelected() && !m_SpellCheck) {
                m_MainWindow->SelectResources(selected_resources);
            }

            // Reset focus to F&R if it had it
            if (has_focus) {
                SetFocus();
            }

            searchable = GetAvailableSearchable();

            if (searchable) {
                found = searchable->FindNext(GetSearchRegex(), direction, m_SpellCheck, true, false);
            }
        }

        // } else {
        //     if (searchable) {
        //         // Check the part of the original file above the cursor
        //         found = searchable->FindNext(GetSearchRegex(), direction, m_SpellCheck, false, false);
        //     }
        // }
    }

    return found;
}


Resource *FindReplace::GetNextContainingResource(Searchable::Direction direction)
{
    Resource *current_resource = GetCurrentResource();
    Resource *starting_resource = NULL;
    bool need_to_check_assigned_starting_resource = false;
    
    // if CurrentFile is the same type as LookWhere, set it as the starting resource
    if (isWhereHTML() && (current_resource->Type() == Resource::HTMLResourceType)) {
        starting_resource = current_resource;
    } else if (isWhereCSS() && (current_resource->Type() == Resource::CSSResourceType)) {
        starting_resource = current_resource;
    } else if (isWhereOPF() && (current_resource->Type() == Resource::OPFResourceType)) {
        starting_resource = current_resource;
    } else if (isWhereNCX() && (current_resource->Type() == Resource::NCXResourceType)) {
        starting_resource = current_resource;
    }

    QList<Resource *> resources = GetFilesToSearch();
    if (resources.isEmpty()) return NULL;

    // If no starting resource we will need to set one first and then search it.
    // Otherwise,  this resource was part of the current selection (or earlier) and has already been
    // searched in FindInAllFiles

    if (!starting_resource || (isWhereSelected() && !IsCurrentFileInSelection())) {
        need_to_check_assigned_starting_resource = true;
        if (direction == Searchable::Direction_Up) {
            starting_resource = resources.first();
        } else {
            starting_resource = resources.last();
        }
    }

    Resource *next_resource = starting_resource;

    if (need_to_check_assigned_starting_resource) {
        DBG qDebug() << "Trying newly assigned first eesource: " << next_resource->GetRelativePath();
        if (next_resource) {
            if (ResourceContainsCurrentRegex(next_resource)) {
                DBG qDebug() << "Found it";
                return next_resource;
            }
    }

    // handle list of resources to search of size 1 as special case
    if (resources.count() == 1) {
            /* already searched it so done */
            return NULL;
        }
    }
    
    // this will only work if the resource list has at least 2 elements
    // as it relies on list order to know if already searched or not
    // since it keeps no state itself
    bool passed_starting_resource = false;

    while (!passed_starting_resource || (next_resource != starting_resource)) {
        
        next_resource = GetNextResource(next_resource, direction);

        if (next_resource == starting_resource) {
            return NULL;
        }

        if (next_resource) {
            DBG qDebug() << "Trying Next Resource: " << next_resource->GetRelativePath();
            if (ResourceContainsCurrentRegex(next_resource)) {
                DBG qDebug() << "Found it";
                return next_resource;
            } else {
                DBG qDebug() << "resource did not contain current regex";
            }

        // else continue
        } else {
            return NULL;
        }

    }

    return NULL;
}


Resource *FindReplace::GetNextResource(Resource *current_resource, Searchable::Direction direction)
{
    QList <Resource *> resources = GetFilesToSearch();
    int max_reading_order       = resources.count() - 1;
    int current_reading_order   = 0;
    int next_reading_order      = 0;

    if (resources.isEmpty()) return NULL;

    // Find the current resource in the tabbed/selected/all resource entries
    int i = 0;
    if (current_resource) {
        foreach(Resource * resource, resources) {
            if (resource && (resource->GetRelativePath() == current_resource->GetRelativePath())) {
                current_reading_order = i;
                break;
            }

            i++;
        }
    }

    // We wrap back (if needed)
    if (direction == Searchable::Direction_Up) {
        next_reading_order = current_reading_order - 1 >= 0 ? current_reading_order - 1 : max_reading_order ;
    } else {
        next_reading_order = current_reading_order + 1 <= max_reading_order ? current_reading_order + 1 : 0;
    }

    if (next_reading_order > max_reading_order || next_reading_order < 0) {
        DBG qDebug() << "GetNextResource returns NULL";
        return NULL;
    } else {
        Resource* nextres = resources[ next_reading_order ];
        DBG qDebug() << "GetNextResource returns: " <<  nextres->GetRelativePath();
        return nextres;
    }
}


Resource *FindReplace::GetCurrentResource()
{
    return m_MainWindow->GetCurrentContentTab()->GetLoadedResource();
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


void FindReplace::UpdateSearchControls(const QString &text)
{
    if (text.isEmpty()) return;

    // Search Mode
    if (text.contains("NL")) {
        SetSearchMode(FindReplace::SearchMode_Normal);
    } else if (text.contains("RX")) {
        SetSearchMode(FindReplace::SearchMode_Regex);
    } else if (text.contains("CS")) {
        SetSearchMode(FindReplace::SearchMode_Case_Sensitive);
    }

    // Search LookWhere
    if (text.contains("CF")) {
        SetLookWhere(FindReplace::LookWhere_CurrentFile);
    } else if (text.contains("AH")) {
        SetLookWhere(FindReplace::LookWhere_AllHTMLFiles);
    } else if (text.contains("SH")) {
        SetLookWhere(FindReplace::LookWhere_SelectedHTMLFiles);
    } else if (text.contains("TH")) {
        SetLookWhere(FindReplace::LookWhere_TabbedHTMLFiles);
    } else if (text.contains("AC")) {
        SetLookWhere(FindReplace::LookWhere_AllCSSFiles);
    } else if (text.contains("SC")) {
        SetLookWhere(FindReplace::LookWhere_SelectedCSSFiles);
    } else if (text.contains("TC")) {
        SetLookWhere(FindReplace::LookWhere_TabbedCSSFiles);
    } else if (text.contains("OP")) {
        SetLookWhere(FindReplace::LookWhere_OPFFile);
    } else if (text.contains("NX")) {
        SetLookWhere(FindReplace::LookWhere_NCXFile);
    }

    // Search Direction
    if (text.contains("UP")) {
        SetSearchDirection(FindReplace::SearchDirection_Up);
    } else if (text.contains("DN")) {
        SetSearchDirection(FindReplace::SearchDirection_Down);
    }

    // Search Flags
    SetOptionWrap(text.contains("WR"));
    SetRegexOptionTextOnly(text.contains("TO"));
    SetRegexOptionDotAll(text.contains("DA"));
    SetRegexOptionMinimalMatch(text.contains("MM"));
    SetRegexOptionAutoTokenise(text.contains("AT"));
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
        case FindReplace::LookWhere_SelectedHTMLFiles:
        case FindReplace::LookWhere_TabbedHTMLFiles:
        case FindReplace::LookWhere_AllCSSFiles:
        case FindReplace::LookWhere_SelectedCSSFiles:
        case FindReplace::LookWhere_TabbedCSSFiles:
        case FindReplace::LookWhere_OPFFile:
        case FindReplace::LookWhere_NCXFile:
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

    // Set F&R Buttons to text-only if requested
    bool frButtonsTextOnly = settings.value("frbuttonstextonly", false).toBool();
    if (frButtonsTextOnly) {
        SetFRButtonsTextOnly();
    }

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
    bool regexOptionTextOnly = settings.value("regexoptiontextonly", false).toBool();
    SetRegexOptionTextOnly(regexOptionTextOnly);
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

#if 0
void FindReplace::ShowHideAdvancedOptions()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    bool show_advanced = settings.value("advanced_visible", true).toBool();
    settings.endGroup();
    ui.optionsl->setVisible(show_advanced);
    ui.space0->setVisible(show_advanced);
    ui.space1->setVisible(show_advanced);
    ui.space2->setVisible(show_advanced);
    ui.tbRegexOptions->setVisible(show_advanced);
    ui.chkOptionWrap->setVisible(show_advanced);
    ui.chkOptionTextOnly->setVisible(show_advanced);
    ui.replaceFind->setVisible(show_advanced);
    ui.count->setVisible(show_advanced);
    ui.revalid->setVisible(show_advanced);
    QIcon icon;

    if (show_advanced) {
        icon.addFile(QString::fromUtf8(":/main/chevron-up.svg"));
        ui.advancedShowHide->setIcon(icon);
    } else {
        icon.addFile(QString::fromUtf8(":/main/chevron-down.svg"));
        ui.advancedShowHide->setIcon(icon);
    }
}
#endif

void FindReplace::WriteSettingsVisible(bool visible)
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("visible", visible);
    settings.endGroup();
}

#if 0
void FindReplace::WriteSettingsAdvancedVisible(bool visible)
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("advanced_visible", visible);
    settings.endGroup();
}
#endif

void FindReplace::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    settings.setValue("find_strings", GetPreviousFindStrings());
    settings.setValue("replace_strings", GetPreviousReplaceStrings());
    settings.setValue("search_mode", GetSearchMode());
    settings.setValue("look_where", GetLookWhere());
    settings.setValue("search_direction", GetSearchDirection());
    settings.setValue("regexoptiondotall", m_DotAllCheckAction->isChecked());
    settings.setValue("regexoptionminimalmatch", m_MinimalMatchCheckAction->isChecked());
    settings.setValue("regexoptionautotokenise", m_AutoTokeniseCheckAction->isChecked());
    settings.setValue("optionwrap", ui.chkOptionWrap->isChecked());
    settings.setValue("regexoptiontextonly", ui.chkOptionTextOnly->isChecked());
    settings.endGroup();
}


Searchable *FindReplace::GetAvailableSearchable()
{
    Searchable *searchable = m_MainWindow->GetCurrentContentTab()->GetSearchableContent();

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
    search_entry->find = ui.cbFind->lineEdit()->text();
    search_entry->replace = ui.cbReplace->lineEdit()->text();
    search_entry->controls = GetControls();
    emit OpenSearchEditorRequest(search_entry);
}


void FindReplace::LoadSearchByName(const QString &name)
{
    // callers to SearchEditorModel's GetEntryFromName receive a searchEntry pointer 
    // created by a call to new and must take ownership and so must clean up after themselves
    SearchEditorModel::searchEntry * search_entry = SearchEditorModel::instance()->GetEntryFromName(name);
    if (search_entry) {
        LoadSearch(search_entry);
        delete search_entry;
    }
}

// LoadSearch is NOT the owner of any passed in search entry pointers
void FindReplace::LoadSearch(SearchEditorModel::searchEntry *search_entry)
{
    if (!search_entry) {
        clearMessage();
        return;
    }

    UpdatePreviousFindStrings(search_entry->find);
    UpdatePreviousReplaceStrings(search_entry->replace);
    UpdateSearchControls(search_entry->controls);

    // Show a message containing the name that was loaded
    QString message(tr("Unnamed search loaded"));

    if (!search_entry->name.isEmpty()) {
        message = QString("%1: %2 ").arg(tr("Loaded")).arg(search_entry->name.replace('<', "&lt;").replace('>', "&gt;").left(50));
    }
    ShowMessage(message);
}

void FindReplace::SetStartingResource(bool update_position)
{
    bool manual_restart = m_RestartPerformed;
    m_RestartPerformed = false;

    if (isWhereCF() || m_LookWhereCurrentFile || IsMarkedText()) return;

    Resource * current_resource = GetCurrentResource();

    m_StartingResource = nullptr;
    QList<Resource*> resources = GetFilesToSearch(true);

    if (resources.isEmpty()) return;

    if (GetSearchDirection() == FindReplace::SearchDirection_Down) {
        if (resources.contains(current_resource)) {
            m_StartingResource = current_resource;
        } else {
            m_StartingResource = resources.first();
        }
    } else {
        if (resources.contains(current_resource)) {
            m_StartingResource = current_resource;
        } else {
            m_StartingResource = resources.last();
        }
    }
    DBG qDebug() << "Setting m_StartingResource: " << m_StartingResource->GetRelativePath();

    // All new searches running from the Saved Search Dialog should start
    // new searches at the top (or bottom) of the current file if it is in the set.
    // Also, when the user hits Restart, the next search should start at the top (bottom)
    
    if (m_IsSearchGroupRunning || manual_restart ) {
        if ( m_StartingResource == current_resource ) {
            int pos = 0;
            if (GetSearchDirection() == FindReplace::SearchDirection_Up) {
                TextResource* text_resource = qobject_cast<TextResource*>(current_resource);
                if (text_resource) pos = text_resource->GetText().length();
            }
            // first try flowtab (html) then all other text tabs
            FlowTab* flow_tab = qobject_cast<FlowTab*>(m_MainWindow->GetCurrentContentTab());
            TextTab* text_tab = qobject_cast<TextTab*>(m_MainWindow->GetCurrentContentTab());
            if (flow_tab) {
                flow_tab->ScrollToPosition(pos);
            } else if (text_tab) {
                text_tab->ScrollToPosition(pos);
            }
        }
    }

    m_StartingPos = -1;
    m_InRemainder = false;
    if (m_StartingResource == current_resource) {
        m_StartingPos = m_MainWindow->GetCurrentContentTab()->GetCursorPosition();
    }
}

// These are *Search methods are invoked by the SearchEditor
void FindReplace::FindSearch()
{
    // these entries are owned by the Search Editor who will clean up as needed
    QList<SearchEditorModel::searchEntry*> search_entries = m_MainWindow->SearchEditorGetCurrentEntries();

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
        } else {
            m_MainWindow->SearchEditorRecordEntryAsCompleted(search_entry);
        }
    }
    m_IsSearchGroupRunning = false;
    ResetKeyModifiers();
}

void FindReplace::ReplaceCurrentSearch()
{
    // these entries are owned by the Search Editor who will clean up as needed
    QList<SearchEditorModel::searchEntry*> search_entries = m_MainWindow->SearchEditorGetCurrentEntries();

    if (search_entries.isEmpty()) {
        ShowMessage(tr("No searches selected"));
        return;
    }

    m_IsSearchGroupRunning = true;
    
#if 0
    foreach(SearchEditorModel::searchEntry * search_entry, search_entries) {
        LoadSearch(search_entry);
        if (ReplaceCurrent()) {
            break;
        } else {
            m_MainWindow->SearchEditorRecordEntryAsCompleted(search_entry);
        }
    }
#else
    SearchEditorModel::searchEntry * search_entry = search_entries.first();
    LoadSearch(search_entry);
    ReplaceCurrent();
#endif
    m_IsSearchGroupRunning = false;
}

void FindReplace::ReplaceSearch()
{
    // these entries are owned by the Search Editor who will clean up as needed
    QList<SearchEditorModel::searchEntry*> search_entries = m_MainWindow->SearchEditorGetCurrentEntries();

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
        } else {
            m_MainWindow->SearchEditorRecordEntryAsCompleted(search_entry);
        }
    }
    m_IsSearchGroupRunning = false;
    ResetKeyModifiers();
}

void FindReplace::CountAllSearch()
{
    // these entries are owned by the Search Editor who will clean up as needed
    QList<SearchEditorModel::searchEntry*> search_entries = m_MainWindow->SearchEditorGetCurrentEntries();

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
    } else if (count > 0) {
        QString message = tr("Matches found: %n", "", count);
        ShowMessage(message);
    }
    ResetKeyModifiers();
}


void FindReplace::CountsReportCount(SearchEditorModel::searchEntry* entry, int& count)
{
    if (entry) {
        SetKeyModifiers();
        m_IsSearchGroupRunning = true;
        LoadSearch(entry);
        count = Count();
        m_IsSearchGroupRunning = false;
    } else {
        qDebug() << "why am I here";
        count = -1;
    }
}


void FindReplace::ReplaceAllSearch()
{
    // these entries are owned by the Search Editor who will clean up as needed
    QList<SearchEditorModel::searchEntry*> search_entries = m_MainWindow->SearchEditorGetCurrentEntries();

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
        m_MainWindow->SearchEditorRecordEntryAsCompleted(search_entry);
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

void FindReplace::ClearHistory()
{
    QMessageBox::StandardButton button_pressed;
    button_pressed = QMessageBox::warning(this,
            tr("Sigil"),
            tr("Are you sure you want to clear your Find and Replace current values and history?"),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel
            );

    if (button_pressed == QMessageBox::Yes) {
        ui.cbFind->clear();
        ui.cbReplace->clear();
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

    // Convert any form of newline or tabs to multiple spaces
    new_text.replace(QRegularExpression("\\R"), "  ");
    new_text.replace("\\t", "  ");

    // If the text does not contain a backslash we "assume" it has not been
    // tokenised already so we need to escape it
    if (!new_text.contains("\\")) {
        new_text = QRegularExpression::escape(new_text);
    }

    // Restore some characters for readability
    new_text.replace("\\ ", " ");
    new_text.replace("\\<", "<");
    new_text.replace("\\>", ">");
    new_text.replace("\\/", "/");
    new_text.replace("\\;", ";");
    new_text.replace("\\:", ":");
    new_text.replace("\\&", "&");
    new_text.replace("\\=", "=");

    // Replace multiple spaces
    new_text.replace(QRegularExpression("(\\s{2,})"), "\\s+");

    if (includeNumerics) {
        // Replace numerics.
        new_text.replace(QRegularExpression("(\\d+)"), "\\d+");
    }

    return new_text;
}

void FindReplace::SetRegexOptionTextOnly(bool new_state)
{
    m_RegexOptionTextOnly = new_state;
    ui.chkOptionTextOnly->setChecked(new_state);
}

void FindReplace::SetRegexOptionDotAll(bool new_state)
{
    m_RegexOptionDotAll = new_state;
    m_DotAllCheckAction->setChecked(new_state);
}

void FindReplace::SetRegexOptionMinimalMatch(bool new_state)
{
    m_RegexOptionMinimalMatch = new_state;
    m_MinimalMatchCheckAction->setChecked(new_state);
}

void FindReplace::SetRegexOptionAutoTokenise(bool new_state)
{
    m_RegexOptionAutoTokenise = new_state;
    m_AutoTokeniseCheckAction->setChecked(new_state);
}

void FindReplace::SetOptionWrap(bool new_state)
{
    m_OptionWrap = new_state;
    ui.chkOptionWrap->setChecked(new_state);
}

void FindReplace::SetFRButtonsTextOnly() {
    // Set F&R Buttons to text-only
    QList<QToolButton *> all_toolbuttons = findChildren<QToolButton *>();
    foreach(QToolButton * toolbutton, all_toolbuttons) {
        // tbRegexOptions QToolButton is always text-only
        // Close button is always icon-only
        if (toolbutton->objectName() != "tbRegexOptions" && toolbutton->objectName() != "close") {
            toolbutton->setToolButtonStyle(Qt::ToolButtonTextOnly);
        }
    }
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
    mode_tooltip += "<dt><b>" + tr("Normal") + "</b><dd>" + tr("Case in-sensitive search of exactly what you type.") + "</dd>";

    ui.cbSearchMode->addItem(tr("Case Sensitive"), FindReplace::SearchMode_Case_Sensitive);
    mode_tooltip += "<dt><b>" + tr("Case Sensitive") + "</b><dd>" + tr("Case sensitive search of exactly what you type.") + "</dd>";

    ui.cbSearchMode->addItem(tr("Regex"), FindReplace::SearchMode_Regex);
    mode_tooltip += "<dt><b>" + tr("Regex") + "</b><dd>" + tr("Search for a pattern using Regular Expression syntax.") + "</dd>";

    ui.cbSearchMode->setToolTip(mode_tooltip);

    QString look_tooltip = "<p>" + tr("Where to search") + ":</p><dl>";

    ui.cbLookWhere->addItem(tr("Current File"), FindReplace::LookWhere_CurrentFile);
    look_tooltip += "<dt><b>" + tr("Current File") + "</b><dd>" + tr("Restrict the find or replace to the opened file.  Hold the Ctrl key down while clicking any search buttons to temporarily restrict the search to the Current File.") + "</dd>";

    ui.cbLookWhere->addItem(tr("All HTML Files"), FindReplace::LookWhere_AllHTMLFiles);
    look_tooltip += "<dt><b>" + tr("All HTML Files") + "</b><dd>" + tr("Find or replace in all HTML files in Code View.") + "</dd>";

    ui.cbLookWhere->addItem(tr("Selected HTML Files"), FindReplace::LookWhere_SelectedHTMLFiles);
    look_tooltip += "<dt><b>" + tr("Selected HTML Files") + "</b><dd>" + tr("Restrict the find or replace to the HTML files selected in the Book Browser in Code View.") + "</dd>";

    ui.cbLookWhere->addItem(tr("Tabbed HTML Files"), FindReplace::LookWhere_TabbedHTMLFiles);
    look_tooltip += "<dt><b>" + tr("Current File") + "</b><dd>" + tr("Restrict the find or replace to the HTML files open in Tabs.") + "</dd>";

    ui.cbLookWhere->addItem(tr("All CSS Files"), FindReplace::LookWhere_AllCSSFiles);
    look_tooltip += "<dt><b>" + tr("All CSS Files") + "</b><dd>" + tr("Find or replace in all CSS files in Code View.") + "</dd>";

    ui.cbLookWhere->addItem(tr("Selected CSS Files"), FindReplace::LookWhere_SelectedCSSFiles);
    look_tooltip += "<dt><b>" + tr("Selected CSS Files") + "</b><dd>" + tr("Restrict the find or replace to the CSS files selected in the Book Browser in Code View.") + "</dd>";

    ui.cbLookWhere->addItem(tr("Tabbed CSS Files"), FindReplace::LookWhere_TabbedCSSFiles);
    look_tooltip += "<dt><b>" + tr("Tabbed CSS Files") + "</b><dd>" + tr("Restrict the find or replace to the CSS files open in Tabs.") + "</dd>";

    ui.cbLookWhere->addItem(tr("OPF File"), FindReplace::LookWhere_OPFFile);
    look_tooltip += "<dt><b>" + tr("OPF File") + "</b><dd>" + tr("Restrict the find or replace to the OPF file.") + "</dd>";

    ui.cbLookWhere->addItem(tr("NCX File"), FindReplace::LookWhere_NCXFile);
    look_tooltip += "<dt><b>" + tr("NCX File") + "</b><dd>" + tr("Restrict the find or replace to the NCX file.") + "</dd>";

    look_tooltip += "</dl>";
    look_tooltip += "<p>" + tr("To restrict search to selected text, use Search&rarr;Mark Selected Text.") + "</p>";
    ui.cbLookWhere->setToolTip(look_tooltip);

    // Special Marked Text indicator.
    QString mark_tooltip = "<p>" + tr("Where to search") + ":</p><dl>";
    ui.MarkedTextIndicator->addItem(tr("Marked Text"));
    mark_tooltip += "<dt><b>" + tr("Marked Text") + "</b><dd>" + tr("Restrict the find or replace to the text marked by Search&rarr;Mark Selected Text.  Cleared if you use Undo, enter text, or change views or tabs.") + "</dd>";
    mark_tooltip += "</dl>";
    ui.MarkedTextIndicator->setToolTip(mark_tooltip);

    ui.cbSearchDirection->addItem(tr("Up"), FindReplace::SearchDirection_Up);
    ui.cbSearchDirection->addItem(tr("Down"), FindReplace::SearchDirection_Down);
    ui.cbSearchDirection->setToolTip("<p>" + tr("Direction to search") + ":</p>"
                                     "<dl>"
                                     "<dt><b>" + tr("Up") + "</b><dd>" + tr("Search for the previous match from your current position.") + "</dd>"
                                     "<dt><b>" + tr("Down") + "</b><dd>" + tr("Search for the next match from your current position.") + "</dd>"
                                     "</dl>");

    // setup up regex options toolbutton and menu
    QMenu * m_menu = new QMenu();
    
    m_DotAllCheckAction = new QAction(tr("Dot All"), m_menu);
    m_DotAllCheckAction->setCheckable(true);
    m_DotAllCheckAction->setEnabled(true);
    m_DotAllCheckAction->setToolTip(tr("For Regex searches, prefix your search with (?s)."));
    m_DotAllCheckAction->setChecked(m_RegexOptionDotAll);
    m_menu->addAction(m_DotAllCheckAction);

    m_MinimalMatchCheckAction = new QAction(tr("Minimal Match"), m_menu);
    m_MinimalMatchCheckAction->setCheckable(true);
    m_MinimalMatchCheckAction->setEnabled(true);
    m_MinimalMatchCheckAction->setToolTip(tr("For Regex searches, prefix your search with (?U)."));
    m_MinimalMatchCheckAction->setChecked(m_RegexOptionMinimalMatch);
    m_menu->addAction(m_MinimalMatchCheckAction);

    m_AutoTokeniseCheckAction = new QAction(tr("Auto Tokenise"), m_menu);
    m_AutoTokeniseCheckAction->setCheckable(true);
    m_AutoTokeniseCheckAction->setEnabled(true);
    m_AutoTokeniseCheckAction->setToolTip(tr("For Regex searches, tokenise/escape selection when opening Find."));
    m_AutoTokeniseCheckAction->setChecked(m_RegexOptionAutoTokenise);
    m_menu->addAction(m_AutoTokeniseCheckAction);

    // the toolbutton will NOT take ownership of the menu as per the Qt docs
    // so clean up manually
    ui.tbRegexOptions->setMenu(m_menu);
    ui.tbRegexOptions->setPopupMode(QToolButton::InstantPopup);
    ui.tbRegexOptions->setArrowType(Qt::NoArrow);
    ui.tbRegexOptions->setToolButtonStyle(Qt::ToolButtonTextOnly);
    ui.tbRegexOptions->setStyleSheet("QToolButton::menu-indicator { image: none; }");
}


void FindReplace::ValidateRegex()
{
    if (GetSearchMode() == FindReplace::SearchMode_Regex) {
        QString rawtext = ui.cbFind->lineEdit()->text();
        QString text = GetSearchRegex();
        // searches have prepended regex pieces for minimal match and dotall that users do not see
        int offset_correction = text.length() - rawtext.length();
        SPCRE rex(text);
        QString emsg;
        if (!rex.isValid()) {
            emsg = tr("Invalid Regex:") + " " + PCREErrors::instance()->GetError(rex.getError(),"");
            emsg = emsg + "\n" + tr("offset:") + " " + QString::number(rex.getErrPos() - offset_correction); 
            ui.cbFind->setToolTip(emsg);
            ui.revalid->setText(INVALID); 
        } else {
            ui.cbFind->setToolTip(tr("Valid Regex"));
            ui.revalid->setText(VALID);
        }
        return;
    }
    ui.cbFind->setToolTip("");
    ui.revalid->setText("");
}


void FindReplace::ConnectSignalsToSlots()
{
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(expireMessage()));
    connect(ui.findNext, SIGNAL(clicked()), this, SLOT(FindClicked()));
    connect(ui.cbFind->lineEdit(), SIGNAL(returnPressed()), this, SLOT(Find()));
    connect(ui.count, SIGNAL(clicked()), this, SLOT(CountClicked()));
    connect(ui.restart, SIGNAL(clicked()), this, SLOT(RestartClicked()));
    connect(ui.replaceCurrent, SIGNAL(clicked()), this, SLOT(ReplaceCurrent()));
    connect(ui.replaceFind, SIGNAL(clicked()), this, SLOT(ReplaceClicked()));
    connect(ui.cbReplace->lineEdit(), SIGNAL(returnPressed()), this, SLOT(Replace()));
    connect(ui.replaceAll, SIGNAL(clicked()), this, SLOT(ReplaceAllClicked()));
    connect(ui.close, SIGNAL(clicked()), this, SLOT(HideFindReplace()));
    // connect(ui.advancedShowHide, SIGNAL(clicked()), this, SLOT(AdvancedOptionsClicked()));
    connect(ui.cbFind, SIGNAL(ClipboardSaveRequest()), this, SIGNAL(ClipboardSaveRequest()));
    connect(ui.cbFind, SIGNAL(ClipboardRestoreRequest()), this, SIGNAL(ClipboardRestoreRequest()));
    connect(ui.cbReplace, SIGNAL(ClipboardSaveRequest()), this, SIGNAL(ClipboardSaveRequest()));
    connect(ui.cbReplace, SIGNAL(ClipboardRestoreRequest()), this, SIGNAL(ClipboardRestoreRequest()));
    connect(m_DotAllCheckAction, SIGNAL(triggered(bool)), this, SLOT(SetRegexOptionDotAll(bool)));
    connect(m_MinimalMatchCheckAction, SIGNAL(triggered(bool)), this, SLOT(SetRegexOptionMinimalMatch(bool)));
    connect(m_AutoTokeniseCheckAction, SIGNAL(triggered(bool)), this, SLOT(SetRegexOptionAutoTokenise(bool)));
    connect(ui.chkOptionWrap, SIGNAL(clicked(bool)), this, SLOT(SetOptionWrap(bool)));
    connect(ui.chkOptionTextOnly, SIGNAL(clicked(bool)), this, SLOT(SetRegexOptionTextOnly(bool)));
    connect(ui.cbFind, SIGNAL(editTextChanged(const QString&)), this, SLOT(ValidateRegex()));
    connect(ui.cbFind, SIGNAL(currentTextChanged(const QString&)), this, SLOT(ValidateRegex()));
    connect(ui.cbSearchMode, SIGNAL(currentTextChanged(const QString&)), this, SLOT(ValidateRegex()));
    connect(m_DotAllCheckAction, SIGNAL(triggered(bool)), this, SLOT(ValidateRegex()));
    connect(m_MinimalMatchCheckAction, SIGNAL(triggered(bool)), this, SLOT(ValidateRegex()));
    connect(m_AutoTokeniseCheckAction, SIGNAL(triggered(bool)), this, SLOT(ValidateRegex()));
}
