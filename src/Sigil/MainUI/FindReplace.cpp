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

#include <QRegExp>
#include <QtGui/QKeyEvent>
#include <QtGui/QLineEdit>
#include <QtGui/QMessageBox>
#include <QtGui/QCompleter>

#include "MainUI/FindReplace.h"
#include "Misc/SettingsStore.h"
#include "Misc/SleepFunctions.h"
#include "Misc/FindReplaceQLineEdit.h"

static const QString SETTINGS_GROUP = "find_replace";
static const int MAXIMUM_SELECTED_TEXT_LIMIT = 500;
static const int MAX_HISTORY_COUNT = 25;

FindReplace::FindReplace( MainWindow &main_window )
    : QWidget( &main_window ),
      m_MainWindow( main_window ),
      m_RegexOptionDotAll( false ),
      m_RegexOptionMinimalMatch( false ),
      m_RegexOptionAutoTokenise( false ),
      m_SpellCheck(false),
      m_LookWhereCurrentFile(false)
{
    ui.setupUi( this );

    FindReplaceQLineEdit *q = new FindReplaceQLineEdit(this);
    ui.cbFind->setLineEdit(q);

    QCompleter *fqc = ui.cbFind->completer();
    fqc->setCaseSensitivity(Qt::CaseSensitive);
    ui.cbFind->setCompleter(fqc);

    QCompleter *rqc = ui.cbReplace->completer();
    rqc->setCaseSensitivity(Qt::CaseSensitive);
    ui.cbReplace->setCompleter(rqc);

    ExtendUI();
    ConnectSignalsToSlots();

    ReadSettings();
}


// Destructor
FindReplace::~FindReplace()
{
    WriteSettings();
}


void FindReplace::SetUpFindText()
{
    Searchable* searchable = GetAvailableSearchable();

    if ( searchable )
    {
        QString selected_text = searchable->GetSelectedText();

        if ( !selected_text.isEmpty() ) 
        {
		    if ( m_RegexOptionAutoTokenise && GetSearchMode() == FindReplace::SearchMode_Regex ) {
			    selected_text = TokeniseSpacesForRegex( selected_text );
		    }
		    // We want to make the text selected in the editor
		    // as the default search text, but only if it's not "too long"
		    if ( selected_text.length() < MAXIMUM_SELECTED_TEXT_LIMIT ) {
                ui.cbFind->setEditText( selected_text );
            }
        }
    }

    // Find text should be selected by default
    ui.cbFind->lineEdit()->selectAll();

    SetFocus();
}


void FindReplace::SetFocus()
{
    ui.cbFind->lineEdit()->setFocus( Qt::ShortcutFocusReason );
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

void FindReplace::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        HideFindReplace();
    }
}


void FindReplace::ShowMessage( const QString &message )
{
    QFont f = ui.message->font();
    f.setBold(true);
    ui.message->setFont(f);

    QString new_message = message;
    if (m_LookWhereCurrentFile && GetLookWhere() != FindReplace::LookWhere_CurrentFile) {
        new_message.append(tr(" (Current File)"));
    }
    ui.message->setText(new_message);
    m_timer.start(15000);

    emit ShowMessageRequest(new_message);
}

void FindReplace::SetLookWhereFromModifier()
{
    // Only use with mouse click not menu/shortcuts to avoid modifying actions
    m_LookWhereCurrentFile = QApplication::keyboardModifiers() & Qt::ControlModifier;
}

void FindReplace::ResetLookWhereFromModifier()
{
    m_LookWhereCurrentFile = false;
}

void FindReplace::FindClicked()
{
    SetLookWhereFromModifier();
    Find();
    ResetLookWhereFromModifier();
}

void FindReplace::ReplaceClicked()
{
    SetLookWhereFromModifier();
    Replace();
    ResetLookWhereFromModifier();
}

void FindReplace::ReplaceAllClicked()
{
    SetLookWhereFromModifier();
    ReplaceAll();
    ResetLookWhereFromModifier();
}

void FindReplace::CountClicked()
{
    SetLookWhereFromModifier();
    Count();
    ResetLookWhereFromModifier();
}

bool FindReplace::Find()
{
    bool found = false;

    if ( GetSearchDirection() == FindReplace::SearchDirection_Up )
    {
        found = FindPrevious();
    }
    else
    {
        found = FindNext();
    }

    return found;
}


bool FindReplace::FindNext()
{
    return FindText( Searchable::Direction_Down );
}


bool FindReplace::FindPrevious()
{
    return FindText( Searchable::Direction_Up );
}


// Counts the number of occurrences of the user's
// term in the document.
int FindReplace::Count()
{
    clearMessage();

    if ( !IsValidFindText() )
    {
        return 0;
    }

    SetCodeViewIfNeeded( true );

    int count = 0;

    if ( GetLookWhere() == FindReplace::LookWhere_CurrentFile || m_LookWhereCurrentFile)
    {
        Searchable *searchable = GetAvailableSearchable();

        if ( !searchable )
        {
            return 0;
        }

        count = searchable->Count( GetSearchRegex() );
    }
    else
    {
        count = CountInFiles();
    }

    if ( count == 0 )
    {
        CannotFindSearchTerm();
    }
    else
    {
        QString message = tr( "%1 matches found", 0, count );
        ShowMessage( message.arg( count ) );
    }

    UpdatePreviousFindStrings();

    return count;
}


bool FindReplace::Replace()
{
    bool found = false;

    if ( GetSearchDirection() == FindReplace::SearchDirection_Up )
    {
        found = ReplacePrevious();
    }
    else
    {
        found = ReplaceNext();
    }

    return found;
}

bool FindReplace::ReplaceNext()
{
    return ReplaceText( Searchable::Direction_Down );
}


bool FindReplace::ReplacePrevious()
{
    return ReplaceText( Searchable::Direction_Up );
}


// Replaces the user's search term with the user's
// replacement text in the entire document.
int FindReplace::ReplaceAll()
{
    m_MainWindow.GetCurrentContentTab().SaveTabContent();

    clearMessage();

    if ( !IsValidFindText() )
    {
        return 0;
    }

    SetCodeViewIfNeeded( true );

    int count = 0;

    if ( GetLookWhere() == FindReplace::LookWhere_CurrentFile || m_LookWhereCurrentFile)
    {
        Searchable *searchable = GetAvailableSearchable();

        if ( !searchable )
        {
            return 0;
        }
        count = searchable->ReplaceAll( GetSearchRegex(), ui.cbReplace->lineEdit()->text() );
    }
    else
    {
        count = ReplaceInAllFiles();
    }

    if ( count == 0 )
    {
        ShowMessage( tr( "No replacements made" ) );
    }
    else
    {
        QString message = tr( "%1 replacements made", 0, count );
        ShowMessage( message.arg( count ) );
    }

    if ( count > 0 )
    {
        // Signal that the contents have changed and update the view
        m_MainWindow.GetCurrentBook()->SetModified( true );
        m_MainWindow.GetCurrentContentTab().ContentChangedExternally();
    }


    UpdatePreviousFindStrings();
    UpdatePreviousReplaceStrings();

    return count;
}


void FindReplace::clearMessage()
{
    ui.message->clear();
    emit ShowMessageRequest("");
}

void FindReplace::expireMessage()
{
    QFont f = ui.message->font();
    f.setBold(false);
    ui.message->setFont(f);

    m_timer.stop();
    emit ShowMessageRequest("");
}

bool FindReplace::FindMisspelledWord()
{
    clearMessage();

    SetCodeViewIfNeeded(true);

    m_SpellCheck = true;

    bool found = FindInAllFiles(Searchable::Direction_Down);

    m_SpellCheck = false;

    if (found) {
        clearMessage();
    }
    else {
        CannotFindSearchTerm();
    }

    return found;
}


// Starts the search for the user's term.
bool FindReplace::FindText( Searchable::Direction direction )
{
    bool found = false;

    clearMessage();

    if ( !IsValidFindText() )
    {
        return found;
    }

    SetCodeViewIfNeeded();

    if ( GetLookWhere() == FindReplace::LookWhere_CurrentFile || m_LookWhereCurrentFile)
    {
        Searchable *searchable = GetAvailableSearchable();

        if ( !searchable )
        {
            return found;
        }

        found = searchable->FindNext( GetSearchRegex(), direction );

    }
    else
    {
        found = FindInAllFiles( direction );
    }

    if ( found )
    {
        clearMessage();
    }
    else
    {
        CannotFindSearchTerm();
    }

    UpdatePreviousFindStrings();

    return found;
}


// Replaces the user's search term with the user's
// replacement text if a match is selected. If it's not,
// calls Find in the direction specified so it becomes selected.
bool FindReplace::ReplaceText( Searchable::Direction direction )
{
    bool found = false;

    clearMessage();

    if ( !IsValidFindText() )
    {
        return found;
    }

    SetCodeViewIfNeeded( true );

    Searchable *searchable = GetAvailableSearchable();

    if ( searchable )
    {
        // If we have the matching text selected, replace it
        // This will not do anything if matching text is not selected.
        found = searchable->ReplaceSelected( GetSearchRegex(), ui.cbReplace->lineEdit()->text(), direction );
    }

    if ( direction == Searchable::Direction_Up)
    {
        if (FindPrevious()) {
            found = true;
        };
    }
    else
    {
        if (FindNext()) {
            found = true;
        }
    }

    UpdatePreviousFindStrings();
    UpdatePreviousReplaceStrings();

    return found;
}


void FindReplace::SetCodeViewIfNeeded( bool force )
{
    bool has_focus = HasFocus();

    if ( force ||
            ( !m_LookWhereCurrentFile && 
              ( GetLookWhere() == FindReplace::LookWhere_AllHTMLFiles || 
                GetLookWhere() == FindReplace::LookWhere_SelectedHTMLFiles ) &&
              ( m_MainWindow.GetViewState() == MainWindow::ViewState_BookView ||
                m_MainWindow.GetViewState() == MainWindow::ViewState_PreviewView ) ) )
    {
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
    ShowMessage( tr( "No matches found" ) );
}


// Constructs a searching regex from the selected
// options and fields and then returns it.
QString FindReplace::GetSearchRegex()
{
    if (m_SpellCheck) {
        return QString();
    }

    QString search( ui.cbFind->currentText() );

    // Search type
    if ( GetSearchMode() == FindReplace::SearchMode_Normal || GetSearchMode() == FindReplace::SearchMode_Case_Sensitive )
    {
        search = QRegExp::escape(search);

        if ( GetSearchMode() == FindReplace::SearchMode_Normal ) {
            search = "(?i)" + search;
        }
    }
    else 
	{
        if ( m_RegexOptionDotAll ) {
			search = "(?s)" + search;
		}
        if ( m_RegexOptionMinimalMatch ) {
			search = "(?U)" + search;
		}
	}

    return search;
}

bool FindReplace::IsCurrentFileInHTMLSelection()
{
    bool found = false;

    QList <Resource *> resources = GetHTMLFiles();
    Resource *current_resource = GetCurrentResource();
    HTMLResource *current_html_resource = qobject_cast< HTMLResource *>( current_resource );

    if ( current_html_resource )
    {
        foreach ( Resource *resource, resources )
        {
            if ( resource->Filename() == current_html_resource->Filename() )
            {
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
    Q_ASSERT( GetLookWhere() == FindReplace::LookWhere_AllHTMLFiles || GetLookWhere() == FindReplace::LookWhere_SelectedHTMLFiles );
    QList <Resource *> resources;

    if ( GetLookWhere() == FindReplace::LookWhere_AllHTMLFiles )
    {
        resources = m_MainWindow.GetAllHTMLResources();
    }
    else
    {
        resources = m_MainWindow.GetValidSelectedHTMLResources();
    }

    return resources;
}

int FindReplace::CountInFiles()
{
    // For now, this must hold
    Q_ASSERT( GetLookWhere() == FindReplace::LookWhere_AllHTMLFiles || GetLookWhere() == FindReplace::LookWhere_SelectedHTMLFiles );

    m_MainWindow.GetCurrentContentTab().SaveTabContent();

    return SearchOperations::CountInFiles(
            GetSearchRegex(),
            GetHTMLFiles(),
            SearchOperations::CodeViewSearch );
}


int FindReplace::ReplaceInAllFiles()
{
    // For now, this must hold
    Q_ASSERT( GetLookWhere() == FindReplace::LookWhere_AllHTMLFiles || GetLookWhere() == FindReplace::LookWhere_SelectedHTMLFiles );

    m_MainWindow.GetCurrentContentTab().SaveTabContent();

    int count = SearchOperations::ReplaceInAllFIles(
            GetSearchRegex(),
            ui.cbReplace->lineEdit()->text(),
            GetHTMLFiles(),
            SearchOperations::CodeViewSearch );

    return count;
}


bool FindReplace::FindInAllFiles( Searchable::Direction direction )
{
    Searchable *searchable = 0;

    bool found = false;
    if ( IsCurrentFileInHTMLSelection() )
    {
        searchable = GetAvailableSearchable();
        if ( searchable )
        {
            found = searchable->FindNext( GetSearchRegex(), direction, m_SpellCheck, false, false);
        }
    }

    if ( !found )
    {
        // TODO: make this handle all types of files
        Resource *containing_resource = GetNextContainingHTMLResource( direction );

        if ( containing_resource )
        {
            // Save if editor or F&R has focus
            bool has_focus = HasFocus();

            // Save selected resources since opening tabs changes selection
            QList<Resource *>selected_resources = GetHTMLFiles();

            m_MainWindow.OpenResource( *containing_resource);

            while ( !m_MainWindow.GetCurrentContentTab().IsLoadingFinished() )
            {
                // Make sure Qt processes events, signals and calls slots
                qApp->processEvents();
                SleepFunctions::msleep( 100 );
            }

            // Restore selection since opening tabs changes selection 
            if ( GetLookWhere() == FindReplace::LookWhere_SelectedHTMLFiles )
            {
                m_MainWindow.SelectResources(selected_resources);
            }

            // Reset focus to F&R if it had it
            if (has_focus) {
                SetFocus();
            }

            searchable = GetAvailableSearchable();
            if ( searchable )
            {
                found = searchable->FindNext( GetSearchRegex(), direction, m_SpellCheck, true, false );
            }
        }
        else
        {
            if ( searchable )
            {
                // Check the part of the original file above the cursor
                found = searchable->FindNext( GetSearchRegex(), direction, m_SpellCheck, false, false );
            }
        }
    }

    return found;
}

HTMLResource* FindReplace::GetNextContainingHTMLResource( Searchable::Direction direction )
{
    Resource* current_resource = GetCurrentResource();
    HTMLResource *starting_html_resource = qobject_cast< HTMLResource *> ( current_resource );

    if ( !starting_html_resource || ( GetLookWhere() == FindReplace::LookWhere_SelectedHTMLFiles && !IsCurrentFileInHTMLSelection() ) )
    {
        QList<Resource *> resources = GetHTMLFiles();
        if ( resources.isEmpty() )
        {
            return NULL;
        }
        if ( direction == Searchable::Direction_Up )
        {
            starting_html_resource = qobject_cast< HTMLResource *>( resources.first() );
        }
        else
        {
            starting_html_resource = qobject_cast< HTMLResource *>( resources.last() );
        }
    }

    HTMLResource *next_html_resource = starting_html_resource;

    bool passed_starting_html_resource = false;

    while ( !passed_starting_html_resource || ( next_html_resource != starting_html_resource ) )
    {
        next_html_resource = GetNextHTMLResource( next_html_resource, direction );
        if ( next_html_resource == starting_html_resource )
        {
             passed_starting_html_resource = true ;
        }

        if ( next_html_resource )
        {
            if ( ResourceContainsCurrentRegex( next_html_resource ) )
            {
                return next_html_resource;
            }
            // else continue
        }
        else
            return NULL;
    }
    return NULL;
}


HTMLResource* FindReplace::GetNextHTMLResource( HTMLResource *current_resource, Searchable::Direction direction )
{
    QList <Resource *> resources = GetHTMLFiles();

    int max_reading_order       = resources.count() - 1;
    int current_reading_order   = 0;
    int next_reading_order      = 0;

    // Find the current resource in the selected/all html entries
    int i = 0;
    foreach ( Resource *resource, resources )
    {
        if ( resource->Filename() == current_resource->Filename() )
        {
            current_reading_order = i;
            break;
        }
        i++;
    }

    // We wrap back (if needed)
    if ( direction == Searchable::Direction_Up )
    {
        next_reading_order = current_reading_order - 1 >= 0 ? current_reading_order - 1 : max_reading_order ;
    }
    else
    {
        next_reading_order = current_reading_order + 1 <= max_reading_order ? current_reading_order + 1 : 0;
    }

    if ( next_reading_order > max_reading_order || next_reading_order < 0 )
    {
        return NULL;
    }
    else
    {
        HTMLResource &html_resource = *qobject_cast< HTMLResource *>( resources[ next_reading_order ] );
        return &html_resource;
    }
}


Resource* FindReplace::GetCurrentResource()
{
    return &m_MainWindow.GetCurrentContentTab().GetLoadedResource();
}


QStringList FindReplace::GetPreviousFindStrings()
{
    QStringList find_strings;

    for ( int i = 0; i < qMin( ui.cbFind->count(), MAX_HISTORY_COUNT ); ++i )
    {
        if ( !find_strings.contains( ui.cbFind->itemText( i ) ) )
        {
            find_strings.append( ui.cbFind->itemText( i ) );
        }
    }

    return find_strings;
}


QStringList FindReplace::GetPreviousReplaceStrings()
{
    QStringList replace_strings;

    for ( int i = 0; i < qMin( ui.cbReplace->count(), MAX_HISTORY_COUNT ); ++i )
    {
        if ( !replace_strings.contains( ui.cbReplace->itemText( i ) ) )
        {
            replace_strings.append( ui.cbReplace->itemText( i ) );
        }
    }

    return replace_strings;
}


void FindReplace::UpdatePreviousFindStrings(const QString &text)
{
    QString new_find_string;
    if (!text.isNull()) {
        new_find_string = text;
    }
    else {
        new_find_string = ui.cbFind->lineEdit()->text();
    }

    int used_at_index = ui.cbFind->findText( new_find_string );

    if ( used_at_index != -1 )
    {
        ui.cbFind->removeItem( used_at_index );
    }

    ui.cbFind->insertItem( 0, new_find_string );

    // Must not change the current string!
    ui.cbFind->setCurrentIndex( 0 );
}


void FindReplace::UpdatePreviousReplaceStrings(const QString &text)
{
    QString new_replace_string;
    if (!text.isNull()) {
        new_replace_string = text;
    }
    else {
        new_replace_string = ui.cbReplace->lineEdit()->text();
    }
    int used_at_index = ui.cbReplace->findText( new_replace_string );

    if ( used_at_index != -1 )
    {
        ui.cbReplace->removeItem( used_at_index );
    }

    ui.cbReplace->insertItem( 0, new_replace_string );

    // Must not change the current string!
    ui.cbReplace->setCurrentIndex( 0 );
}

FindReplace::SearchMode FindReplace::GetSearchMode()
{
    int mode = ui.cbSearchMode->itemData( ui.cbSearchMode->currentIndex() ).toInt();
    switch ( mode )
    {
    case FindReplace::SearchMode_Regex:
        return static_cast<FindReplace::SearchMode>( mode );
        break;
    case FindReplace::SearchMode_Case_Sensitive:
        return static_cast<FindReplace::SearchMode>( mode );
        break;
    default:
        return FindReplace::SearchMode_Normal;
    }
}

FindReplace::LookWhere FindReplace::GetLookWhere()
{
    int look = ui.cbLookWhere->itemData( ui.cbLookWhere->currentIndex() ).toInt();
    switch ( look )
    {
    case FindReplace::LookWhere_AllHTMLFiles:
        return static_cast<FindReplace::LookWhere>( look );
        break;
    case FindReplace::LookWhere_SelectedHTMLFiles:
        return static_cast<FindReplace::LookWhere>( look );
        break;
    default:
        return FindReplace::LookWhere_CurrentFile;
    }
}

FindReplace::SearchDirection FindReplace::GetSearchDirection()
{
    int direction = ui.cbSearchDirection->itemData( ui.cbSearchDirection->currentIndex() ).toInt();
    switch ( direction )
    {
    case FindReplace::SearchDirection_Up:
        return static_cast<FindReplace::SearchDirection>( direction );
        break;
    default:
        return FindReplace::SearchDirection_Down;
    }
}


bool FindReplace::IsValidFindText()
{
    return  !ui.cbFind->lineEdit()->text().isEmpty();
}                                                     

// Reads all the stored settings
void FindReplace::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup( SETTINGS_GROUP );

    // Find and Replace history
    QStringList find_strings = settings.value( "find_strings" ).toStringList();
    find_strings.removeDuplicates();
    ui.cbFind->clear();
    ui.cbFind->addItems( find_strings );

    QStringList replace_strings = settings.value( "replace_strings" ).toStringList();
    replace_strings.removeDuplicates();
    ui.cbReplace->clear();
    ui.cbReplace->addItems( replace_strings );

    SetSearchMode( settings.value( "search_mode", 0 ).toInt() );
    SetLookWhere( settings.value( "look_where", 0 ).toInt() );
    SetSearchDirection( settings.value( "search_direction", 0 ).toInt() );

    settings.endGroup();
}

void FindReplace::ShowHide()
{
    SettingsStore settings;
    settings.beginGroup( SETTINGS_GROUP );

    QVariant show_find_replace = settings.value( "visible" );

    settings.endGroup();

    // Hide the window by default
    if (show_find_replace.isNull() ? false : show_find_replace.toBool()) {
        show();
    }
    else {
        hide();
    }

}

void FindReplace::WriteSettingsVisible(bool visible)
{
    SettingsStore *settings = new SettingsStore();
    settings->beginGroup( SETTINGS_GROUP );

    settings->setValue( "visible", visible);

    settings->endGroup();
}

void FindReplace::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup( SETTINGS_GROUP );

    settings.setValue( "find_strings", GetPreviousFindStrings() );
    settings.setValue( "replace_strings", GetPreviousReplaceStrings() );

    settings.setValue( "search_mode", GetSearchMode() );
    settings.setValue( "look_where", GetLookWhere() );
    settings.setValue( "search_direction", GetSearchDirection() );
    settings.endGroup();
}


Searchable* FindReplace::GetAvailableSearchable()
{
    Searchable *searchable = m_MainWindow.GetCurrentContentTab().GetSearchableContent();

    if ( !searchable )
    {
        ShowMessage( tr( "This tab cannot be searched" ) );
    }

    return searchable;
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

void FindReplace::SaveSearchAction()
{
    SearchEditorModel::searchEntry *search_entry = new SearchEditorModel::searchEntry();
    search_entry->name = "Unnamed Search";
    search_entry->is_group = false;
    search_entry->find = ui.cbFind->lineEdit()->text(),
    search_entry->replace = ui.cbReplace->lineEdit()->text(),

    emit OpenSearchEditorRequest(search_entry);
}

void FindReplace::LoadSearchByName(QString name)
{
    LoadSearch(SearchEditorModel::instance()->GetEntryFromName(name));
}

void FindReplace::SetSearchMode(int search_mode)
{
    ui.cbSearchMode->setCurrentIndex(0);
    for ( int i = 0; i < ui.cbSearchMode->count(); ++i )
    {
        if ( ui.cbSearchMode->itemData( i ) == search_mode )
        {
            ui.cbSearchMode->setCurrentIndex( i );
            break;
        }
    }
}

void FindReplace::SetLookWhere(int look_where)
{
    ui.cbLookWhere->setCurrentIndex( 0 );
    for ( int i = 0; i < ui.cbLookWhere->count(); ++i )
    {
        if ( ui.cbLookWhere->itemData( i )  == look_where )
        {
            ui.cbLookWhere->setCurrentIndex( i );
            break;
        }
    }
}

void FindReplace::SetSearchDirection(int search_direction)
{
    ui.cbSearchDirection->setCurrentIndex( 0 );
    for ( int i = 0; i < ui.cbSearchDirection->count(); ++i )
    {
        if ( ui.cbSearchDirection->itemData( i ) == search_direction )
        {
            ui.cbSearchDirection->setCurrentIndex( i );
            break;
        }
    }
}

void FindReplace::LoadSearch(SearchEditorModel::searchEntry *search_entry)
{
    clearMessage();

    if (!search_entry) {
        return;
    }
  
    UpdatePreviousFindStrings(search_entry->find);
    UpdatePreviousReplaceStrings(search_entry->replace);

    // Default for all saved searches is Regex, All HTML Files, Down.
    SetSearchMode(FindReplace::SearchMode_Regex);
    SetLookWhere(FindReplace::LookWhere_AllHTMLFiles);;
    SetSearchDirection(FindReplace::SearchDirection_Down);

    ui.cbLookWhere->setCurrentIndex(1);
    ui.cbSearchDirection->setCurrentIndex(1);

    // Show a message containing the name that was loaded
    QString message = "";
    message = tr("Unnamed search loaded");
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

    SetLookWhereFromModifier();

    foreach (SearchEditorModel::searchEntry* search_entry, search_entries) {
        LoadSearch(search_entry);
        if (Find()) {
            break;
        };
    }

    ResetLookWhereFromModifier();
}

void FindReplace::ReplaceSearch(QList<SearchEditorModel::searchEntry *> search_entries)
{
    if (search_entries.isEmpty()) {
        ShowMessage(tr("No searches selected"));
        return;
    }

    SetLookWhereFromModifier();

    foreach (SearchEditorModel::searchEntry* search_entry, search_entries) {
        LoadSearch(search_entry);
        if (Replace()) {
            break;
        }
    }

    ResetLookWhereFromModifier();
}

void FindReplace::CountAllSearch(QList<SearchEditorModel::searchEntry *> search_entries)
{
    if (search_entries.isEmpty()) {
        ShowMessage(tr("No searches selected"));
        return;
    }

    SetLookWhereFromModifier();

    int count = 0;
    foreach (SearchEditorModel::searchEntry* search_entry, search_entries) {
        LoadSearch(search_entry);
        count += Count();
    }

    if (count == 0) {
        CannotFindSearchTerm();
    }
    else {
        QString message = tr( "%1 matches found", 0, count);
        ShowMessage(message.arg(count));
    }

    ResetLookWhereFromModifier();
}

void FindReplace::ReplaceAllSearch(QList<SearchEditorModel::searchEntry *> search_entries)
{
    if (search_entries.isEmpty()) {
        ShowMessage(tr("No searches selected"));
        return;
    }

    SetLookWhereFromModifier();

    int count = 0;
    foreach (SearchEditorModel::searchEntry* search_entry, search_entries) {
        LoadSearch(search_entry);
        count += ReplaceAll();
    }

    if (count == 0) {
        ShowMessage(tr( "No replacements made" ));
    }
    else {
        QString message = tr("%1 replacements made", 0, count);
        ShowMessage(message.arg(count));
    }

    ResetLookWhereFromModifier();
}

void FindReplace::TokeniseSelection()
{
    if ( !IsValidFindText() )
    {
        return;
    }
    QString text;
    if (ui.cbFind->lineEdit()->hasSelectedText()) {
        // We want to tokenise only the selection
        text = ui.cbFind->lineEdit()->selectedText();
    }
    else {
        // We will tokenise the whole thing
        text = ui.cbFind->lineEdit()->text();
    }

	QString new_text = TokeniseSpacesForRegex( text );
	new_text = TokeniseNumericsForRegex( new_text );

    if (ui.cbFind->lineEdit()->hasSelectedText()) {
        int selectionStart = ui.cbFind->lineEdit()->selectionStart();
        int selectionLength = ui.cbFind->lineEdit()->selectedText().length();
        text = ui.cbFind->lineEdit()->text();
        QString all_text = text.left(selectionStart) + new_text + text.right(text.length() - selectionLength - selectionStart);
	    ui.cbFind->setEditText( all_text );
        ui.cbFind->lineEdit()->setSelection(selectionStart, new_text.length());
    }
    else {
	    ui.cbFind->setEditText( new_text );
    }
}

QString FindReplace::TokeniseSpacesForRegex(const QString &text)
{
    QRegExp replace_spaces("([\\n\\s]{2,})");
    QString new_text = text;
    return new_text.replace(replace_spaces, "\\s+");
}

QString FindReplace::TokeniseNumericsForRegex(const QString &text)
{
    QRegExp replace_numerics("(\\d+)");
    QString new_text = text;
    return new_text.replace(replace_numerics, "\\d+");
}

void FindReplace::SetRegexOptionDotAll(bool new_state)
{
    m_RegexOptionDotAll = new_state;
}

void FindReplace::SetRegexOptionMinimalMatch(bool new_state)
{
    m_RegexOptionMinimalMatch = new_state;
}

void FindReplace::SetRegexOptionAutoTokenise(bool new_state)
{
    m_RegexOptionAutoTokenise = new_state;
}

void FindReplace::ConnectSignalsToSlots()
{
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(expireMessage()));
    connect(ui.findNext, SIGNAL(clicked()), this, SLOT(FindClicked()));
    connect(ui.cbFind->lineEdit(), SIGNAL(returnPressed()), this, SLOT(Find()));
    connect(ui.count, SIGNAL(clicked()), this, SLOT(CountClicked()));
    connect(ui.replaceNext, SIGNAL(clicked()), this, SLOT(ReplaceClicked()));
    connect(ui.cbReplace->lineEdit(), SIGNAL(returnPressed()), this, SLOT(Replace()));
    connect(ui.replaceAll, SIGNAL(clicked()), this, SLOT(ReplaceAllClicked()));
    connect(ui.close, SIGNAL(clicked()), this, SLOT(HideFindReplace()));
}
