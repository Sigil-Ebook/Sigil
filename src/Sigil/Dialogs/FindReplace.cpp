/************************************************************************
**
**  Copyright (C) 2009, 2010  Strahinja Markovic
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
#include "FindReplace.h"
#include "../ViewEditors/Searchable.h"
#include "../Tabs/TabManager.h"
#include "../Tabs/ContentTab.h"
#include "../Misc/SleepFunctions.h"

static const QString SETTINGS_GROUP = "find_replace";
static const int MAXIMUM_SELECTED_TEXT_LIMIT = 100;

// Constructor;
// the first argument specifies which tab to load first;
// the second argument is the MainWindow that created the dialog;
// the third argument is the widget's parent.
FindReplace::FindReplace( bool find_tab, MainWindow &main_window, QWidget *parent )
    :
    QDialog( parent ),
    m_MainWindow( main_window )
{
    ui.setupUi( this );

    // Telling Qt to delete this window
    // from memory when it is closed
    setAttribute( Qt::WA_DeleteOnClose );

    ExtendUI();
    ConnectSignalsToSlots();

    // Defaults
    ui.rbNormalSearch->setChecked( true );
    ui.rbAllDirection->setChecked( true );

    if ( find_tab )

        ui.twTabs->setCurrentIndex( 0 );

    else

        ui.twTabs->setCurrentIndex( 1 );

    TabChanged();    
    ReadSettings();
    ToggleMoreLess();
    SetUpFindText();
}


// Destructor
FindReplace::~FindReplace()
{
    WriteSettings();
}


// Switches the display between the "more" version with 
// the option controls and the "less" version without them
void FindReplace::ToggleMoreLess()
{
    if ( m_isMore == true )
    {
        // We hide then show the tab widget between
        // update calls to wOptions because this prevents
        // twTabs from doing layout twice and causing flicker.
        // It's a hack, but it works.
        ui.twTabs->hide();
        ui.wOptions->hide();
        ui.twTabs->show();        

        ui.btMore->setText( tr( "&More" ) );

        m_isMore = false;
    }

    else // isMore == false
    {
        // We hide then show the tab widget between
        // update calls to wOptions because this prevents
        // twTabs from doing layout twice and causing flicker.
        // It's a hack, but it works.
        ui.twTabs->hide();
        ui.wOptions->show();
        ui.twTabs->show(); 
        ui.btMore->setText( tr( "Le&ss" ) );

        m_isMore = true;
    }
}


// Gets called whenever the user switches tabs,
// so it moves all the controls to the other tab.
void FindReplace::TabChanged()
{
    // Put the controls on the current tab
    ui.twTabs->currentWidget()->layout()->addWidget( ui.wSearch );
    ui.twTabs->currentWidget()->layout()->addWidget( ui.wOptions );
  
    if ( ui.twTabs->currentIndex() == 0 )
    
        ToFindTab();

    else

        ToReplaceTab();
}


// Starts the search for the user's term.
// Shows a dialog if the term cannot be found.
void FindReplace::FindNext()
{
    if ( ui.cbFind->lineEdit()->text().isEmpty() )

        return;

    Searchable *searchable = GetAvailableSearchable();
    
    if ( !searchable )

        return;

    if ( CurrentLookWhere() == CurrentFile )
    {
        bool found = searchable->FindNext( GetSearchRegex(), GetSearchDirection() );

        if ( !found )

            CannotFindSearchTerm();
    }

    else
    {
        FindInAllFiles( searchable );
    }

    UpdatePreviousFindStrings();
}


// Counts the number of occurrences of the user's
// term in the document. Shows a dialog with the number.
void FindReplace::Count()
{
    if ( ui.cbFind->lineEdit()->text().isEmpty() )

        return;

    Searchable *searchable = GetAvailableSearchable();

    if ( !searchable )

        return;

    int count = CurrentLookWhere() == CurrentFile     ? 
                searchable->Count( GetSearchRegex() ) :
                CountInFiles();

    QString message = ( count < 1 || count > 1 )     ? 
                      tr( "%1 matches were found." ) :
                      tr( "%1 match was found."    );

    QMessageBox::information( 0, tr( "Sigil" ), message.arg( count ) );

    UpdatePreviousFindStrings();
}


// Replaces the user's search term with the user's
// replacement text if a match is selected. If it's not,
// calls FindNext() so it becomes selected.
void FindReplace::Replace()
{
    if ( ui.cbFind->lineEdit()->text().isEmpty() )

        return;

    Searchable *searchable = GetAvailableSearchable();

    if ( !searchable )

        return;

    // If we have the matching text selected, replace it
    searchable->ReplaceSelected( GetSearchRegex(), ui.cbReplace->lineEdit()->text() );

    // Go find the next match
    FindNext(); 

    UpdatePreviousFindStrings();
    UpdatePreviousReplaceStrings();
}


// Replaces the user's search term with the user's
// replacement text in the entire document. Shows a
// dialog telling how many occurrences were replaced.
void FindReplace::ReplaceAll()
{
    if ( ui.cbFind->lineEdit()->text().isEmpty() )

        return;

    Searchable *searchable = GetAvailableSearchable();

    if ( !searchable )

        return;

    int count = CurrentLookWhere() == CurrentFile                                            ? 
                searchable->ReplaceAll( GetSearchRegex(), ui.cbReplace->lineEdit()->text() ) :
                ReplaceInAllFiles();

    QString message = ( count < 1 || count > 1 )                     ? 
                      tr( "The search term was replaced %1 times." ) :
                      tr( "The search term was replaced %1 time."  );

    QMessageBox::information( 0, tr( "Sigil" ), message.arg( count ) );

    UpdatePreviousFindStrings();
    UpdatePreviousReplaceStrings();
}


// Toggles the availability of options depending on
// whether the normal search type is selected.
void FindReplace::ToggleAvailableOptions( bool normal_search_checked )
{
    if ( normal_search_checked )
    {
        ui.cbMinimalMatching->setEnabled( false );
        ui.cbWholeWord->setEnabled( true );
    }

    else
    {
        ui.cbMinimalMatching->setEnabled( true );
        ui.cbWholeWord->setEnabled( false );
    }
}


void FindReplace::LookWhereChanged( int index  )
{
    if ( ui.cbLookWhere->itemData( index ) == FindReplace::AllHTMLFiles &&
         m_MainWindow.GetCurrentContentTab().GetViewState() == ContentTab::ViewState_BookView )
    {
        QMessageBox::critical( this,
                               tr( "Sigil" ),
                               tr( "It is not currently possible to search all the files in Book View mode. "
                                   "Switch to Code View to perform such searches.")
                             );

        // Back to current document search mode
        ui.cbLookWhere->setCurrentIndex( 0 );
    }
}


// Displays a message to the user informing him
// that his last search term could not be found.
void FindReplace::CannotFindSearchTerm()
{
    QMessageBox::information( 0, tr( "Sigil" ), tr( "The search term cannot be found." ) );
}


// Constructs a searching regex from the selected 
// options and fields and then returns it.
QRegExp FindReplace::GetSearchRegex()
{
    QRegExp search( ui.cbFind->lineEdit()->text() );

    // Search type
    if ( ui.rbWildcardSearch->isChecked() )
    {
        search.setPatternSyntax( QRegExp::Wildcard );
    }

    else
    {
        // We need the regex syntax for normal searching
        // too because of the "whole words only" option
        search.setPatternSyntax( QRegExp::RegExp2 );

        if ( ui.rbNormalSearch->isChecked() )

            search.setPattern( QRegExp::escape( ui.cbFind->lineEdit()->text() ) );
    }

    // Whole word searching. The user can select 
    // this option only if the "normal" search type
    // is also selected
    if ( ui.cbWholeWord->isEnabled() && ui.cbWholeWord->isChecked() )
        
        search.setPattern( "\\b" + QRegExp::escape( ui.cbFind->lineEdit()->text() ) + "\\b" );

    // Case sensitivity
    if ( ui.cbMatchCase->isEnabled() && ui.cbMatchCase->isChecked() )

        search.setCaseSensitivity( Qt::CaseSensitive );

    else

        search.setCaseSensitivity( Qt::CaseInsensitive );

    // Regex minimality. The user can select 
    // this option only if the "normal" search type
    // is NOT selected
    if ( ui.cbMinimalMatching->isEnabled() && ui.cbMinimalMatching->isChecked() )

        search.setMinimal( true );

    else

        search.setMinimal( false );

    return search;
}


// Returns the selected search direction.
Searchable::Direction FindReplace::GetSearchDirection()
{
    if ( ui.rbUpDirection->isChecked() )

        return Searchable::Direction_Up;

    else if ( ui.rbDownDirection->isChecked() )

        return Searchable::Direction_Down;

    else

        return Searchable::Direction_All;
}


FindReplace::LookWhere FindReplace::CurrentLookWhere()
{
    return (LookWhere) ui.cbLookWhere->itemData( ui.cbLookWhere->currentIndex() ).toInt();
}


int FindReplace::CountInFiles()
{
    // For now, this must hold
    Q_ASSERT( CurrentLookWhere() == AllHTMLFiles );

    return SearchOperations::CountInFiles( 
            GetSearchRegex(), 
            m_MainWindow.GetCurrentBook()->GetFolderKeeper().GetResourceTypeAsGenericList< HTMLResource >(),
            SearchOperations::CodeViewSearch );
}


int FindReplace::ReplaceInAllFiles()
{
    // For now, this must hold
    Q_ASSERT( CurrentLookWhere() == AllHTMLFiles );

    return SearchOperations::ReplaceInAllFIles( 
            GetSearchRegex(),
            ui.cbReplace->lineEdit()->text(),
            m_MainWindow.GetCurrentBook()->GetFolderKeeper().GetResourceTypeAsGenericList< HTMLResource >(),
            SearchOperations::CodeViewSearch );
}


void FindReplace::FindInAllFiles( Searchable *searchable )
{
    Q_ASSERT( searchable );

    Searchable::Direction search_direction = GetSearchDirection();

    bool ignore_offset = m_LastUsedSearchable != searchable ? true : false;

    bool found = search_direction == Searchable::Direction_All                                       ?
                 searchable->FindNext( GetSearchRegex(), Searchable::Direction_Down, ignore_offset ) :
                 searchable->FindNext( GetSearchRegex(), search_direction, ignore_offset );

    m_LastUsedSearchable = searchable;

    if ( !found )
    {
        // TODO: make this handle all types of files
        Resource *containing_resource = GetNextContainingHTMLResource(); 

        if ( containing_resource )
        {
            m_MainWindow.OpenResource( *containing_resource, ContentTab::ViewState_CodeView );

            while ( !m_MainWindow.GetCurrentContentTab().IsLoadingFinished() )
            {
                // Make sure Qt processes events, signals and calls slots
                qApp->processEvents();
                SleepFunctions::msleep( 100 );
            }

            FindNext();
        }

        else
        {
            CannotFindSearchTerm();
        }
    }   
}


HTMLResource* FindReplace::GetNextContainingHTMLResource()
{
    HTMLResource *starting_html_resource = GetStartingResource< HTMLResource >();
    HTMLResource *next_html_resource     = starting_html_resource;

    while ( true )
    {
        next_html_resource = GetNextHTMLResource( next_html_resource );

        if ( next_html_resource )
        {
            if ( next_html_resource != starting_html_resource )
            {
                if ( ResourceContainsCurrentRegex( next_html_resource ) )
                
                    return next_html_resource;

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


HTMLResource* FindReplace::GetNextHTMLResource( HTMLResource *current_resource )
{
    QSharedPointer< Book > book = m_MainWindow.GetCurrentBook();
    int max_reading_order       = book->GetConstFolderKeeper().GetHighestReadingOrder();
    int current_reading_order   = current_resource->GetReadingOrder();
    int next_reading_order      = 0;

    if ( GetSearchDirection() == Searchable::Direction_Down )
    
        next_reading_order = current_reading_order + 1;

    else if ( GetSearchDirection() == Searchable::Direction_Up )

        next_reading_order = current_reading_order - 1;

    else

        // We wrap back (if needed) for Direction_All
        next_reading_order = current_reading_order + 1 < max_reading_order ? current_reading_order + 1 : 0;

    if ( next_reading_order > max_reading_order || next_reading_order < 0 )

        return NULL;

    else

        return book->GetFolderKeeper().GetResourceTypeList< HTMLResource >( true )[ next_reading_order ];
}


Resource* FindReplace::GetCurrentResource()
{
    return &m_MainWindow.GetCurrentContentTab().GetLoadedResource();
}


// Changes the layout of the controls to the Find tab style
void FindReplace::ToFindTab()
{
    ui.btCount->show();
    ui.btReplace->hide();
    ui.btReplaceAll->hide();

    // We "hide" the replace label and field.
    // We use QStackedWidgets because we want the 
    // replace label and field to take up space 
    // in the layout even when they are not visible.
    // That way the dialog doesn't shift all the controls.
    ui.swReplaceLabelHider->setCurrentIndex( 1 );
    ui.swReplaceFieldHider->setCurrentIndex( 1 );
}


// Changes the layout of the controls to the Replace tab style
void FindReplace::ToReplaceTab()
{
    ui.btCount->hide();
    ui.btReplace->show();
    ui.btReplaceAll->show();

    // We "show" the replace label and field.
    // We use QStackedWidgets because we want the 
    // replace label and field to take up space 
    // in the layout even when they are not visible.
    // That way the dialog doesn't shift all the controls.
    ui.swReplaceLabelHider->setCurrentIndex( 0 );
    ui.swReplaceFieldHider->setCurrentIndex( 0 );
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
        }
    }        

    // Find text should be selected by default
    ui.cbFind->lineEdit()->selectAll();
}


// Reads all the stored dialog settings like
// window position, geometry etc.
void FindReplace::ReadSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    // We flip the stored isMore state because we have to pass through
    // the ToggleMoreLess function to actually set the widgets
    // (and the isMore variable) to the stored state
    m_isMore = ! settings.value( "is_more" ).toBool();

    // The size of the window and it's full screen status
    QByteArray geometry = settings.value( "geometry" ).toByteArray();

    if ( !geometry.isNull() )
    {
        restoreGeometry( geometry );
    }

    else
    {
        // We call this to force the dialog to initially take up
        // as little space as possible. The reason for this is that
        // all the horizontal buttons are initially shown on the form,
        // yet only only a few are used on any tab.
        resize( 0, 0 );
    }

    // Checkbox and radio button values
    ui.cbMatchCase->      setChecked( settings.value( "match_case"       ).toBool() );
    ui.cbMinimalMatching->setChecked( settings.value( "minimal_matching" ).toBool() );
    ui.cbWholeWord->      setChecked( settings.value( "whole_word"       ).toBool() );

    ui.rbNormalSearch->   setChecked( settings.value( "normal_search"    ).toBool() );
    ui.rbWildcardSearch-> setChecked( settings.value( "wildcard_search"  ).toBool() );
    ui.rbRegexSearch->    setChecked( settings.value( "regex_search"     ).toBool() );

    ui.rbUpDirection->    setChecked( settings.value( "up_direction"     ).toBool() );
    ui.rbDownDirection->  setChecked( settings.value( "down_direction"   ).toBool() );
    ui.rbAllDirection->   setChecked( settings.value( "all_direction"    ).toBool() );

    // Input fields
    ui.cbFind->   addItems( settings.value( "find_strings"    ).toStringList() );
    ui.cbReplace->addItems( settings.value( "replace_strings" ).toStringList() );
}

// Writes all the stored dialog settings like
// window position, geometry etc.
void FindReplace::WriteSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    // The size of the window and its full screen status
    settings.setValue( "geometry", saveGeometry() );

    // The window expansion state ("more" or "less")
    settings.setValue( "is_more", m_isMore );

    // Checkbox and radio button values
    settings.setValue( "match_case",       ui.cbMatchCase->      isChecked() );
    settings.setValue( "minimal_matching", ui.cbMinimalMatching->isChecked() );
    settings.setValue( "whole_word",       ui.cbWholeWord->      isChecked() );

    settings.setValue( "normal_search",    ui.rbNormalSearch->   isChecked() );
    settings.setValue( "wildcard_search",  ui.rbWildcardSearch-> isChecked() );
    settings.setValue( "regex_search",     ui.rbRegexSearch->    isChecked() );

    settings.setValue( "up_direction",     ui.rbUpDirection->    isChecked() );
    settings.setValue( "down_direction",   ui.rbDownDirection->  isChecked() );
    settings.setValue( "all_direction",    ui.rbAllDirection->   isChecked() );

    settings.setValue( "find_strings",    GetPreviousFindStrings()    );
    settings.setValue( "replace_strings", GetPreviousReplaceStrings() );
}


void FindReplace::ExtendUI()
{
    // This is necessary. We need to have a default
    // layout on the Replace tab. 
    new QVBoxLayout( ui.ReplaceTab );

    ui.cbLookWhere->addItem( tr( "Current File" ),   FindReplace::CurrentFile  );
    ui.cbLookWhere->addItem( tr( "All HTML Files" ), FindReplace::AllHTMLFiles );

    QCompleter *find_completer = new QCompleter( ui.cbFind );
    find_completer->setCaseSensitivity( Qt::CaseSensitive );
    ui.cbFind->setCompleter( find_completer );

    QCompleter *replace_completer = new QCompleter( ui.cbReplace );
    replace_completer->setCaseSensitivity( Qt::CaseSensitive );
    ui.cbReplace->setCompleter( replace_completer );
}


Searchable* FindReplace::GetAvailableSearchable()
{
    Searchable *searchable = m_MainWindow.GetCurrentContentTab().GetSearchableContent();
    
    if ( !searchable )
    {
        QMessageBox::critical( this,
                               tr( "Sigil" ),
                               tr( "This tab cannot be searched." )
                             );
    }

    return searchable;
}


void FindReplace::ConnectSignalsToSlots()
{
    connect( ui.twTabs,         SIGNAL( currentChanged( int ) ), this, SLOT( TabChanged()                   ) );
    connect( ui.btMore,         SIGNAL( clicked()             ), this, SLOT( ToggleMoreLess()               ) );
    connect( ui.btFindNext,     SIGNAL( clicked()             ), this, SLOT( FindNext()                     ) );
    connect( ui.btCount,        SIGNAL( clicked()             ), this, SLOT( Count()                        ) );
    connect( ui.btReplace,      SIGNAL( clicked()             ), this, SLOT( Replace()                      ) );
    connect( ui.btReplaceAll,   SIGNAL( clicked()             ), this, SLOT( ReplaceAll()                   ) );
    connect( ui.rbNormalSearch, SIGNAL( toggled( bool )       ), this, SLOT( ToggleAvailableOptions( bool ) ) );
    connect( ui.cbLookWhere,    SIGNAL( activated( int )      ), this, SLOT( LookWhereChanged( int )        ) );
}




