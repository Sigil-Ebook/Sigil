/************************************************************************
**
**  Copyright (C) 2011  John Schember <john@nachtimwald.com>
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

#include <stdafx.h>
#include "pcre.h"
#include "FindReplace.h"
#include "Misc/SleepFunctions.h"

static const QString SETTINGS_GROUP = "find_replace";
static const int MAXIMUM_SELECTED_TEXT_LIMIT = 100;

FindReplace::FindReplace( MainWindow &main_window )
    : QWidget( &main_window ),
      m_MainWindow( main_window )
{
    ui.setupUi( this );

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

        // We want to make the text selected in the editor
        // as the default search text, but only if it's not "too long"
        if ( !selected_text.isEmpty() &&
             selected_text.length() < MAXIMUM_SELECTED_TEXT_LIMIT )
        {
            ui.cbFind->lineEdit()->setText( selected_text );
            ui.cbFind->insertItem( 0, selected_text );
        }
    }

    // Find text should be selected by default
    ui.cbFind->lineEdit()->selectAll();
    ui.cbFind->lineEdit()->setFocus( Qt::ShortcutFocusReason );
}


void FindReplace::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hide();
    }
}


void FindReplace::ShowMessage( const QString &message )
{
    ui.message->setText(message);
    m_timer.start(5000);
}


void FindReplace::FindNext()
{
    FindText( Searchable::Direction_Down );
}


void FindReplace::FindPrevious()
{
    FindText( Searchable::Direction_Up );
}


// Counts the number of occurrences of the user's
// term in the document.
void FindReplace::Count()
{
    if ( ui.cbFind->lineEdit()->text().isEmpty() )
    {
        return;
    }

    Searchable *searchable = GetAvailableSearchable();

    if ( !searchable )
    {
        return;
    }

    int count = 0;

    if ( GetLookWhere() == LookWhere_CurrentFile || !CheckBookWideSearchingAllowed() )
    {
        count = searchable->Count( GetSearchRegex() );
    }
    else
    {
        count = CountInFiles();
    }

    QString message = tr( "%n match(es) were found.", "", count );
    ShowMessage( message.arg( count ) );

    UpdatePreviousFindStrings();
}


void FindReplace::ReplaceNext()
{
    ReplaceText( Searchable::Direction_Down );
}


void FindReplace::ReplacePrevious()
{
    ReplaceText( Searchable::Direction_Up );
}


// Replaces the user's search term with the user's
// replacement text in the entire document. Shows a
// dialog telling how many occurrences were replaced.
void FindReplace::ReplaceAll()
{
    if ( ui.cbFind->lineEdit()->text().isEmpty() )
    {
        return;
    }

    Searchable *searchable = GetAvailableSearchable();

    if ( !searchable )
    {
        return;
    }
    int count = 0;

    if ( GetLookWhere() == LookWhere_CurrentFile || !CheckBookWideSearchingAllowed() )
    {
        count = searchable->ReplaceAll( GetSearchRegex(), ui.cbReplace->lineEdit()->text() );
    }
    else
    {
        count = ReplaceInAllFiles();
    }

    QString message = tr( "The search term was replaced %n time(s).", "", count );

    if ( count > 0 )
    {
        // Signal that the contents have changed and update the view
        m_MainWindow.GetCurrentBook()->SetModified( true );
        m_MainWindow.GetCurrentContentTab().ContentChangedExternally();
    }

    ShowMessage( message.arg( count ) );

    UpdatePreviousFindStrings();
    UpdatePreviousReplaceStrings();
}


void FindReplace::clearMessage()
{
    m_timer.stop();
    ui.message->clear();
}

// Starts the search for the user's term.
void FindReplace::FindText( Searchable::Direction direction )
{
    if ( ui.cbFind->lineEdit()->text().isEmpty() )
    {
        return;
    }

    Searchable *searchable = GetAvailableSearchable();

    if ( !searchable )
    {
        return;
    }

    if ( GetLookWhere() == LookWhere_CurrentFile || !CheckBookWideSearchingAllowed() )
    {
        bool found = searchable->FindNext( GetSearchRegex(), direction );

        if ( found )
        {
            clearMessage();
        }
        else
        {
            CannotFindSearchTerm();
        }
    }
    else if ( CheckBookWideSearchingAllowed() )
    {
        FindInAllFiles( searchable, direction );
    }
    else
    {
        return;
    }

    UpdatePreviousFindStrings();
}


// Replaces the user's search term with the user's
// replacement text if a match is selected. If it's not,
// calls Find in the direction specified so it becomes selected.
void FindReplace::ReplaceText( Searchable::Direction direction )
{
    if ( ui.cbFind->lineEdit()->text().isEmpty() )
    {
        return;
    }

    Searchable *searchable = GetAvailableSearchable();

    if ( !searchable )
    {
        return;
    }

    // If we have the matching text selected, replace it
    // This will not do anything if matching text is not selected.
    searchable->ReplaceSelected( GetSearchRegex(), ui.cbReplace->lineEdit()->text() );

    // Go find the next match
    if ( direction == Searchable::Direction_Down )
    {
        FindNext();
    }
    else if ( direction == Searchable::Direction_Up )
    {
        FindPrevious();
    }

    UpdatePreviousFindStrings();
    UpdatePreviousReplaceStrings();
}


bool FindReplace::CheckBookWideSearchingAllowed()
{
    if ( GetLookWhere() == FindReplace::LookWhere_AllHTMLFiles &&
         m_MainWindow.GetCurrentContentTab().GetViewState() == ContentTab::ViewState_BookView )
    {
        return false;
    }

    return true;
}


// Displays a message to the user informing him
// that his last search term could not be found.
void FindReplace::CannotFindSearchTerm()
{
    ShowMessage( tr( "The search term cannot be found." ) );
}


// Constructs a searching regex from the selected
// options and fields and then returns it.
QString FindReplace::GetSearchRegex()
{
    QString search( ui.cbFind->lineEdit()->text() );

    // Search type
    if ( GetSearchMode() == SearchMode_Normal || GetSearchMode() == SearchMode_Case_Sensitive )
    {
        search = QRegExp::escape(search);

        if ( GetSearchMode() == SearchMode_Normal ) {
            search = "(?i)" + search;
        }
    }

    return search;
}



int FindReplace::CountInFiles()
{
    // For now, this must hold
    Q_ASSERT( GetLookWhere() == LookWhere_AllHTMLFiles );

    return SearchOperations::CountInFiles(
            GetSearchRegex(),
            m_MainWindow.GetCurrentBook()->GetFolderKeeper().GetResourceTypeAsGenericList< HTMLResource >(),
            SearchOperations::CodeViewSearch );
}


int FindReplace::ReplaceInAllFiles()
{
    // For now, this must hold
    Q_ASSERT( GetLookWhere() == LookWhere_AllHTMLFiles );

    int count = SearchOperations::ReplaceInAllFIles(
            GetSearchRegex(),
            ui.cbReplace->lineEdit()->text(),
            m_MainWindow.GetCurrentBook()->GetFolderKeeper().GetResourceTypeAsGenericList< HTMLResource >(),
            SearchOperations::CodeViewSearch );

    // Update the content displayed in the current tab.
    m_MainWindow.GetCurrentContentTab().LoadTabContent();

    return count;
}


void FindReplace::FindInAllFiles( Searchable *searchable, Searchable::Direction direction )
{
    Q_ASSERT( searchable );

    bool found = searchable->FindNext( GetSearchRegex(), direction );

    if ( !found )
    {
        // TODO: make this handle all types of files
        Resource *containing_resource = GetNextContainingHTMLResource( direction );

        if ( containing_resource )
        {
            m_MainWindow.OpenResource( *containing_resource, ContentTab::ViewState_CodeView );

            while ( !m_MainWindow.GetCurrentContentTab().IsLoadingFinished() )
            {
                // Make sure Qt processes events, signals and calls slots
                qApp->processEvents();
                SleepFunctions::msleep( 100 );
            }

            searchable = GetAvailableSearchable();
            searchable->ClearSearchCache();
            searchable->FindNext( GetSearchRegex(), direction, true );
        }
        else
        {
            CannotFindSearchTerm();
        }
    }
}


HTMLResource* FindReplace::GetNextContainingHTMLResource( Searchable::Direction direction )
{
    HTMLResource *starting_html_resource = GetStartingResource< HTMLResource >();
    HTMLResource *next_html_resource = starting_html_resource;

    while ( true )
    {
        next_html_resource = GetNextHTMLResource( next_html_resource, direction );

        if ( next_html_resource )
        {
            if ( next_html_resource != starting_html_resource )
            {
                if ( ResourceContainsCurrentRegex( next_html_resource ) )
                {
                    return next_html_resource;
                }
                // else continue
            }
            else
            {
                return NULL;
            }
        }
        else
        {
            return NULL;
        }
    }
}


HTMLResource* FindReplace::GetNextHTMLResource( HTMLResource *current_resource, Searchable::Direction direction )
{
    QSharedPointer< Book > book = m_MainWindow.GetCurrentBook();
    int max_reading_order       = book->GetFolderKeeper().GetHighestReadingOrder();
    int current_reading_order   = book->GetOPF().GetReadingOrder( *current_resource );
    int next_reading_order      = 0;

    // We wrap back (if needed) for Direction_All
    // Direction_Down: find in next file.
    if ( direction == Searchable::Direction_Down )
    {
        next_reading_order = current_reading_order + 1 <= max_reading_order ? current_reading_order + 1 : 0;
    }
    // Direction Up: find in previous file.
    else
    {
        next_reading_order = current_reading_order - 1 >= 0 ? current_reading_order - 1 : max_reading_order;
    }

    if ( next_reading_order > max_reading_order || next_reading_order < 0 )
    {
       return NULL;
    }
    else
    {
        return book->GetFolderKeeper().GetResourceTypeList< HTMLResource >( true )[ next_reading_order ];
    }
}


Resource* FindReplace::GetCurrentResource()
{
    return &m_MainWindow.GetCurrentContentTab().GetLoadedResource();
}


QStringList FindReplace::GetPreviousFindStrings()
{
    QStringList find_strings;

    for ( int i = 0; i < ui.cbFind->count(); ++i )
    {
        find_strings.append( ui.cbFind->itemText( i ) );
    }

    return find_strings;
}


QStringList FindReplace::GetPreviousReplaceStrings()
{
    QStringList replace_strings;

    for ( int i = 0; i < ui.cbReplace->count(); ++i )
    {
        replace_strings.append( ui.cbReplace->itemText( i ) );
    }

    return replace_strings;
}


void FindReplace::UpdatePreviousFindStrings()
{
    QString new_find_string = ui.cbFind->lineEdit()->text();
    int used_at_index = ui.cbFind->findText( new_find_string );

    if ( used_at_index != -1 )
    {
        ui.cbFind->removeItem( used_at_index );
    }

    ui.cbFind->insertItem( 0, new_find_string );

    // Must not change the current string!
    ui.cbFind->setCurrentIndex( 0 );
}


void FindReplace::UpdatePreviousReplaceStrings()
{
    QString new_replace_string = ui.cbReplace->lineEdit()->text();
    int used_at_index = ui.cbReplace->findText( new_replace_string );

    if ( used_at_index != -1 )
    {
        ui.cbReplace->removeItem( used_at_index );
    }

    ui.cbReplace->insertItem( 0, new_replace_string );

    // Must not change the current string!
    ui.cbReplace->setCurrentIndex( 0 );
}


FindReplace::LookWhere FindReplace::GetLookWhere()
{
    int look = ui.cbLookWhere->itemData( ui.cbLookWhere->currentIndex() ).toInt();
    switch ( look )
    {
    case FindReplace::LookWhere_AllHTMLFiles:
        return static_cast<FindReplace::LookWhere>( look );
        break;
    default:
        return FindReplace::LookWhere_CurrentFile;
    }
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


// Reads all the stored dialog settings like
// window position, geometry etc.
void FindReplace::ReadSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    // Input fields
    ui.cbFind->addItems( settings.value( "find_strings" ).toStringList() );
    ui.cbReplace->addItems( settings.value( "replace_strings" ).toStringList() );

    ui.cbSearchMode->setCurrentIndex( settings.value( "search_mode", 0 ).toInt() );
    ui.cbLookWhere->setCurrentIndex( settings.value( "look_where", 0 ).toInt() );
}


// Writes all the stored dialog settings like
// window position, geometry etc.
void FindReplace::WriteSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    settings.setValue( "find_strings", GetPreviousFindStrings() );
    settings.setValue( "replace_strings", GetPreviousReplaceStrings() );

    settings.setValue( "search_mode", GetSearchMode() );
    settings.setValue( "look_where", GetLookWhere() );
}


Searchable* FindReplace::GetAvailableSearchable()
{
    Searchable *searchable = m_MainWindow.GetCurrentContentTab().GetSearchableContent();

    if ( !searchable )
    {
        ShowMessage( tr( "This tab cannot be searched." ) );
    }

    return searchable;
}


void FindReplace::ExtendUI()
{
    ui.cbSearchMode->addItem( tr( "Normal" ), FindReplace::SearchMode_Normal );
    ui.cbSearchMode->addItem( tr( "Case Sensitive" ), FindReplace::SearchMode_Case_Sensitive );
    ui.cbSearchMode->addItem( tr( "Regex" ), FindReplace::SearchMode_Regex );

    ui.cbLookWhere->addItem( tr( "Current File" ), FindReplace::LookWhere_CurrentFile );
    ui.cbLookWhere->addItem( tr( "All HTML Files" ), FindReplace::LookWhere_AllHTMLFiles );
}


void FindReplace::ConnectSignalsToSlots()
{
    connect( &m_timer, SIGNAL( timeout() ), this, SLOT( clearMessage() ) );
    connect( ui.findNext, SIGNAL( clicked() ), this, SLOT( FindNext() ) );
    connect( ui.cbFind->lineEdit(), SIGNAL( returnPressed() ), this, SLOT( FindNext() ) );
    connect( ui.findPrevious, SIGNAL( clicked() ), this, SLOT( FindPrevious() ) );
    connect( ui.count, SIGNAL( clicked() ), this, SLOT( Count() ) );
    connect( ui.replaceNext, SIGNAL( clicked() ), this, SLOT( ReplaceNext() ) );
    connect( ui.replacePrevious, SIGNAL( clicked() ), this, SLOT( ReplacePrevious() ) );
    connect( ui.cbReplace->lineEdit(), SIGNAL( returnPressed() ), this, SLOT( ReplaceNext() ));
    connect( ui.replaceAll, SIGNAL( clicked() ), this, SLOT( ReplaceAll() ) );
    connect( ui.close, SIGNAL( clicked() ), this, SLOT( hide() ) );
}
