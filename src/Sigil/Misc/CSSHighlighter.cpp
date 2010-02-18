/************************************************************************
**
**  Copyright (C) 2009  Nokia Corporation, Strahinja Markovic
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
#include "CSSHighlighter.h"

// TODO: This code was written by Nokia for Qt Designer, and it's horribly ugly.
// I've tried to clean it up, but it's hopeless. Write a new CSS parser to replace this.

static enum State 
{ 
    Selector, 
    Property, 
    Value, 
    Pseudo, 
    Pseudo1, 
    Pseudo2, 
    Quote, 
    MaybeComment, 
    Comment, 
    MaybeCommentEnd 
};

static enum Token
{ 
    ALNUM,
    LBRACE, 
    RBRACE, 
    COLON, 
    SEMICOLON, 
    COMMA, 
    QUOTE, 
    SLASH, 
    STAR 
};

// Transitions for the DFA
static const int transitions[ 10 ][ 9 ] = 
{
    { Selector, Property, Selector, Pseudo,  Property, Selector, Quote,   MaybeComment, Selector        }, // Selector
    { Property, Property, Selector, Value,   Property, Property, Quote,   MaybeComment, Property        }, // Property
    { Value,    Property, Selector, Value,   Property, Value,    Quote,   MaybeComment, Value           }, // Value
    { Pseudo1,  Property, Selector, Pseudo2, Selector, Selector, Quote,   MaybeComment, Pseudo          }, // Pseudo
    { Pseudo1,  Property, Selector, Pseudo,  Selector, Selector, Quote,   MaybeComment, Pseudo1         }, // Pseudo1
    { Pseudo2,  Property, Selector, Pseudo,  Selector, Selector, Quote,   MaybeComment, Pseudo2         }, // Pseudo2
    { Quote,    Quote,    Quote,    Quote,   Quote,    Quote,    -1,      Quote,        Quote           }, // Quote
    { -1,       -1,       -1,       -1,     -1,        -1,       -1,      -1,           Comment         }, // MaybeComment
    { Comment,  Comment,  Comment,  Comment, Comment,  Comment,  Comment, Comment,      MaybeCommentEnd }, // Comment
    { Comment,  Comment,  Comment,  Comment, Comment,  Comment,  Comment, -1,           MaybeCommentEnd }  // MaybeCommentEnd
};


CSSHighlighter::CSSHighlighter( QObject *parent )
    : QSyntaxHighlighter( parent )
{
}

void CSSHighlighter::highlightBlock( const QString& text )
{
    int lastIndex = 0;
    bool lastWasSlash = false;
    int state = previousBlockState();
    int save_state = 0;

    if ( state == -1 )
    {
        // As long as the text is empty, leave the state undetermined
        if ( text.isEmpty() ) 
        {
            setCurrentBlockState( -1 );
            return;
        }

        // The initial state is based on the presence of a ":" and the absence of a "{".
        // This is because Qt style sheets support both a full stylesheet as well as
        // an inline form with just properties.
        state = save_state = ( text.indexOf( QLatin1Char( ':' ) ) > -1 &&
                               text.indexOf( QLatin1Char( '{' ) ) == -1 ) ? Property : Selector;
    } 
    
    else 
    {
        save_state = state >> 16;
        state &= 0x00ff;
    }

    if ( state == MaybeCommentEnd )
    
        state = Comment;     
    
    else if ( state == MaybeComment )

        state = save_state;
    

    for ( int i = 0; i < text.length(); i++ ) 
    {
        int token = ALNUM;
        const char character = text.at( i ).toAscii();

        if ( state == Quote ) 
        {
            if ( character == '\\' ) 
            {
                lastWasSlash = true;
            } 
            
            else
            {
                if ( character == '\"' && !lastWasSlash )
                {
                    token = QUOTE;
                }

                lastWasSlash = false;
            }
        } 
        
        else 
        {
            token = character == '{'  ? LBRACE    :
                    character == '}'  ? RBRACE    :
                    character == ':'  ? COLON     :
                    character == ';'  ? SEMICOLON :
                    character == ','  ? COMMA     :
                    character == '\"' ? QUOTE     :
                    character == '/'  ? SLASH     :
                    character == '*'  ? STAR      :
                                        ALNUM;            
        }

        int new_state = transitions[ state ][ token ];

        if ( new_state != state ) 
        {
            bool include_token = new_state == MaybeCommentEnd || 
                                (state == MaybeCommentEnd && new_state!= Comment) ||
                                state == Quote;

            highlight( text, lastIndex, i - lastIndex + include_token, state );

            if ( new_state == Comment) 

                lastIndex = i - 1; // include the slash and star
            else

                lastIndex = i + ( ( token == ALNUM || new_state == Quote ) ? 0 : 1 );            
        }

        if ( new_state == -1 )
        {
            state = save_state;
        } 
        
        else if ( state <= Pseudo2 )
        {
            save_state = state;
            state = new_state;
        } 
        
        else 
        {
            state = new_state;
        }
    }

    highlight( text, lastIndex, text.length() - lastIndex, state );
    setCurrentBlockState( state + ( save_state << 16 ) );
}


void CSSHighlighter::highlight( const QString &text, int start, int length, int state )
{
    if ( start >= text.length() || length <= 0 )

        return;

    QTextCharFormat format;

    switch ( state ) 
    {
        case Selector:
            setFormat( start, length, Qt::darkRed );
            break;

        case Property:
            setFormat( start, length, Qt::darkBlue );
            break;

        case Value:
            setFormat( start, length, Qt::black );
            break;

        case Pseudo1:
            setFormat( start, length, Qt::darkRed );
            break;

        case Pseudo2:
            setFormat( start, length, Qt::darkRed );
            break;

        case Quote:
            setFormat( start, length, Qt::darkMagenta );
            break;

        case Comment:
        case MaybeCommentEnd:
            format.setForeground( Qt::darkGreen );
            setFormat( start, length, format );
            break;

        default:
            break;
    }
}
