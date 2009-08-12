/************************************************************************
**
**  Copyright (C) 2009  Strahinja Markovic
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

#include "stdafx.h"
#include "XHTMLHighlighter.h"

// All of our regular expressions
static const QString HTML_ELEMENT_BEGIN     = "<(/\?|/)?(?!!)";
static const QString HTML_ELEMENT_NAME      = "\\s*\\w+\\b\\s*(?!=)";
static const QString HTML_ELEMENT_END       = "(\\?|/)?>";

static const QString HTML_COMMENT_BEGIN     = "<!--";
static const QString HTML_COMMENT_END       = "[^-]*-([^-][^-]*-)*->";

static const QString CSS_BEGIN              = "<\\s*style[^>]*>";
static const QString CSS_END                = "</\\s*style[^>]*>";

static const QString CSS_COMMENT_BEGIN      = "/\\*";
static const QString CSS_COMMENT_END        = "\\*/";

static const QString ATTRIBUTE_VALUE        = "\"[^<\"]*\"|'[^<']*'";
static const QString ATTRIBUTE_NAME         = "\\w+";

// TODO: These should probably be user-selectable in some options menu
static const QColor HTML_COLOR              = Qt::blue;
static const QColor HTML_COMMENT_COLOR      = Qt::darkGreen;
static const QColor CSS_COLOR               = Qt::darkYellow;
static const QColor CSS_COMMENT_COLOR       = Qt::darkGreen;
static const QColor ATTRIBUTE_NAME_COLOR    = Qt::darkRed;
static const QColor ATTRIBUTE_VALUE_COLOR   = Qt::darkCyan;


// Constructor
XHTMLHighlighter::XHTMLHighlighter( QTextDocument *parent )
     : QSyntaxHighlighter( parent )
{
    QTextCharFormat fmtHTML;
    QTextCharFormat fmtHTMLcomment;
    QTextCharFormat fmtCSS;
    QTextCharFormat fmtCSScomment;
    QTextCharFormat fmtAttributeName;
    QTextCharFormat fmtAttributeValue;

    fmtHTML             .setForeground( HTML_COLOR              );
    fmtHTMLcomment      .setForeground( HTML_COMMENT_COLOR      );
    fmtCSS              .setForeground( CSS_COLOR               );
    fmtCSScomment       .setForeground( CSS_COMMENT_COLOR       );
    fmtAttributeName    .setForeground( ATTRIBUTE_NAME_COLOR    );
    fmtAttributeValue   .setForeground( ATTRIBUTE_VALUE_COLOR   );

    HighlightingRule rule;

    rule.pattern    = QRegExp( HTML_ELEMENT_BEGIN );
    rule.format     = fmtHTML;

    hshRules[ "HTML_ELEMENT_BEGIN" ] = rule;

    rule.pattern    = QRegExp( HTML_ELEMENT_END );
    rule.format     = fmtHTML;

    hshRules[ "HTML_ELEMENT_END" ] = rule;

    rule.pattern    = QRegExp( HTML_COMMENT_BEGIN );
    rule.format     = fmtHTMLcomment;

    hshRules[ "HTML_COMMENT_BEGIN" ] = rule;

    rule.pattern    = QRegExp( HTML_COMMENT_END );
    rule.format     = fmtHTMLcomment;

    hshRules[ "HTML_COMMENT_END" ] = rule;

    rule.pattern    = QRegExp( CSS_BEGIN );
    rule.format     = fmtCSS;

    hshRules[ "CSS_BEGIN" ] = rule;

    rule.pattern    = QRegExp( CSS_END );
    rule.format     = fmtCSS;

    hshRules[ "CSS_END" ] = rule;

    rule.pattern    = QRegExp( CSS_COMMENT_BEGIN );
    rule.format     = fmtCSScomment;

    hshRules[ "CSS_COMMENT_BEGIN" ] = rule;

    rule.pattern    = QRegExp( CSS_COMMENT_END );
    rule.format     = fmtCSScomment;

    hshRules[ "CSS_COMMENT_END" ] = rule;

    rule.pattern    = QRegExp( ATTRIBUTE_NAME );
    rule.format     = fmtAttributeName;

    hshRules[ "ATTRIBUTE_NAME" ] = rule;

    rule.pattern    = QRegExp( ATTRIBUTE_VALUE );
    rule.format     = fmtAttributeValue;

    hshRules[ "ATTRIBUTE_VALUE" ] = rule;
}


// Overrides the function from QSyntaxHighlighter;
// gets called by QTextEditor whenever
// a block (line of text) needs to be repainted
void XHTMLHighlighter::highlightBlock( const QString& text )
{
    // By default, all block states are -1;
    // in our implementation regular text is state == 1
    if ( previousBlockState() == -1 )
    {
        setCurrentBlockState( State_Text );
    }

    // Propagate previous state; needed for state tracking
    else
    {
        setCurrentBlockState( previousBlockState() );
    }
        
    // The order of these operations is important
    // because some states format text over previous states!
    HighlightLine( text, State_CSS );
    HighlightLine( text, State_HTML );
    HighlightLine( text, State_CSSComment );
    HighlightLine( text, State_HTMLComment );	
}


// Returns the regex that matches the left bracket of a state
QRegExp XHTMLHighlighter::GetLeftBracketRegEx( int state ) const
{
    QRegExp empty;
    
    switch ( state )
    {
        case State_HTML:

            return hshRules[ "HTML_ELEMENT_BEGIN" ].pattern;

        case State_HTMLComment:

            return hshRules[ "HTML_COMMENT_BEGIN" ].pattern;

        case State_CSS:

            return hshRules[ "CSS_BEGIN" ].pattern;

        case State_CSSComment:

            return hshRules[ "CSS_COMMENT_BEGIN" ].pattern;

        default:

            return empty;
    }
}


// Returns the regex that matches the right bracket of a state
QRegExp XHTMLHighlighter::GetRightBracketRegEx( int state ) const
{
    QRegExp empty;
    
    switch ( state )
    {
        case State_HTML:

            return hshRules[ "HTML_ELEMENT_END" ].pattern;

        case State_HTMLComment:

            return hshRules[ "HTML_COMMENT_END" ].pattern;

        case State_CSS:

            return hshRules[ "CSS_END" ].pattern;

        case State_CSSComment:

            return hshRules[ "CSS_COMMENT_END" ].pattern;

        default:

            return empty;
    }
}


// Sets the requested state for the current text block
void XHTMLHighlighter::SetState( int state )
{
    int current_state = currentBlockState();

    // Add the current state to the list
    current_state = current_state | state;

    setCurrentBlockState( current_state );
}


// Clears the requested state for the current text block
void XHTMLHighlighter::ClearState( int state )
{
    int current_state = currentBlockState();

    // Remove the current state from the list
    current_state = current_state & ~state;

    setCurrentBlockState( current_state );
}


// Checks if the requested state is set
// for the current text block
bool XHTMLHighlighter::CheckState( int state )
{
    int current_state = currentBlockState();

    // Check if our state is in the list
    if ( (current_state & state) != 0 )

        return true;

    else

        return false;
}


// Formats the inside of a node;
// "text" is the textblock/line of text;
// "state" describes the node;
// "index" is the index to start formatting from;
// "length" is the length of chars to format.
void XHTMLHighlighter::FormatBody( const QString& text, int state, int index, int length )
{
    if ( state == State_HTML )
    {
        // First paint everything the color of the brackets
        setFormat( index, length, hshRules[ "HTML_ELEMENT_BEGIN" ].format );

        QRegExp name    = hshRules[ "ATTRIBUTE_NAME" ].pattern;
        QRegExp value   = hshRules[ "ATTRIBUTE_VALUE" ].pattern;

        // Used to move over the line
        int main_index  = index;

        // We skip over the left bracket (if it's present)
        QRegExp bracket( HTML_ELEMENT_BEGIN );

        if ( text.indexOf( bracket, main_index ) == main_index )
        {
            main_index += bracket.matchedLength();
        }

        // We skip over the element name (if it's present) 
        // because we want it to be the same color as the brackets
        QRegExp elem_name( HTML_ELEMENT_NAME );

        if ( text.indexOf( elem_name, main_index ) == main_index )
        {
            main_index += elem_name.matchedLength();
        }

        while( true )
        {
            // Get the indexes of the attribute names and values
            int name_index  = text.indexOf( name, main_index );
            int value_index = text.indexOf( value, main_index );
            
            // If we can't find the names and values or we found them 
            // outside of the area we are formatting, we exit
            if ( ( ( name_index  != -1 ) && ( name_index  < index + length ) ) ||
                 ( ( value_index != -1 ) && ( value_index < index + length ) ) )
                
            {
                // ... otherwise format the found sections
                setFormat( name_index,  name.matchedLength(),   hshRules[ "ATTRIBUTE_NAME" ].format );
                setFormat( value_index, value.matchedLength(),  hshRules[ "ATTRIBUTE_VALUE" ].format );
            }
            
            else
            
                break;

            // Update the main index with the regex that matched "further down the line"
            if ( name_index + name.matchedLength() > value_index + value.matchedLength() )

                main_index = name_index + name.matchedLength();

            else

                main_index = value_index + value.matchedLength();
        }
    }

    else if ( state == State_HTMLComment )
    {
        setFormat( index, length, hshRules[ "HTML_COMMENT_BEGIN" ].format );
    }

    else if ( state == State_CSS )
    {
        setFormat( index, length, hshRules[ "CSS_BEGIN" ].format );
    }

    else if ( state == State_CSSComment )
    {
        setFormat( index, length, hshRules[ "CSS_COMMENT_BEGIN" ].format );
    }
}


// Highlights the current line according to the state requested;
// check to see if the node of type "state" is present;
// if it is, the node is formatted
void XHTMLHighlighter::HighlightLine( const QString& text, int state )
{
    QRegExp left_bracket_regex  = GetLeftBracketRegEx( state );
    QRegExp right_bracket_regex = GetRightBracketRegEx( state );

    int left_bracket_index      = -1;
    int right_bracket_index     = -1;

    int main_index = 0;

    // We loop over the line several times
    // because we could have several nodes on it
    while( main_index < text.length() )
    {
        left_bracket_index  = text.indexOf( left_bracket_regex, main_index );
        right_bracket_index = text.indexOf( right_bracket_regex, main_index );

        // If we are not starting our state and our state is
        // not already set, we don't format; see the four cases explanation below
        if ( ( left_bracket_index == -1 ) && ( CheckState( state ) == false ) )

            return;

        // Every node/state has a left "bracket", a right "bracket" and the inside body.
        // This example uses HTML tags, but the principle is the same for every node/state.
        // There are four possible cases:
        // (1)  <......>    (both brackets on the same line; state starts and stops here)
        // (2)  <.......    (only the left bracket; next line continues state)
        // (3)  .......>    (only the right bracket; current line ends state)
        // (4)  ........    (no brackets; a line between (2) and (3))

        // We also check the state because we don't want to start a new node
        // if the current node of the same type hasn't finished
        if ( ( left_bracket_index != -1 ) && ( CheckState( state ) == false ) )
        {
            main_index = left_bracket_index + left_bracket_regex.matchedLength();

            // (1)
            if ( right_bracket_index != -1 )
            {
                main_index  = right_bracket_index + right_bracket_regex.matchedLength();

                int length  = right_bracket_index - left_bracket_index + right_bracket_regex.matchedLength();

                FormatBody( text, state, left_bracket_index, length );

                // There's no point in setting the state here because the state
                // starts and ends on this line.
            }
            
            // (2)
            else
            {
                int length = text.length() - left_bracket_index;

                FormatBody( text, state, left_bracket_index, length );

                main_index += length;

                // Set the current state so the next line can continue
                // with the formatting.
                SetState( state );
            }
        }

        else
        {
            // (3)
            if ( right_bracket_index != -1 )
            {
                main_index  = right_bracket_index + right_bracket_regex.matchedLength();

                int length  = right_bracket_index + right_bracket_regex.matchedLength();

                FormatBody( text, state, 0, length );

                // Clear the current state because our state has just ended.
                ClearState( state );
            }

            // (4)
            else
            {
                int length = text.length();

                FormatBody( text, state, 0, length );

                main_index += length;
            }
        }
    }
}



