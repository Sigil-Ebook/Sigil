/************************************************************************
 **
 **  Copyright (C) 2021  Kevin B. Hendricks, Stratford, Ontario, Canada
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
 ** Extracted and modified from:
 ** CSSTidy (https://github.com/csstidy-c/csstidy)
 **
 ** CSSTidy Portions Copyright:
 **   Florian Schmitz <floele@gmail.com>
 **   Thierry Charbonnel
 **   Will Mitchell <aethon@gmail.com>
 **   Brett Zamir <brettz9@yahoo.com>
 **   sined_ <sined_@users.sourceforge.net>
 **   Dmitry Leskov <git@dmitryleskov.com>
 **   Kevin Coyner <kcoyner@debian.org>
 **   Tuukka Pasanen <pasanen.tuukka@gmail.com>
 **   Frank W. Bergmann <csstidy-c@tuxad.com>
 **   Frank Dana <ferdnyc@gmail.com>
 **
 ** CSSTidy us Available under the LGPL 2.1
 ** You should have received a copy of the GNU Lesser General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/
 
#ifndef HEADER_CSSPARSER
#define HEADER_CSSPARSER 

#include <assert.h>
#include <QChar>
#include <QString>
#include <QVector>
#include <QMap>

class CSSParser 
{ 

public:

    enum parse_status
    {
        PIS, PIP, PIV, PINSTR, PIC, PAT
    };

    enum message_type
    {
        Information,Warning,Error
    };

    enum token_type
    {
        CHARSET, IMPORT, NAMESP, AT_START, AT_END, SEL_START, SEL_END, PROPERTY, VALUE, COMMENT, CSS_END
    };

    struct token
    {
        token_type type;
        int pos;
        int line;
        QString data;
    };

    struct message
    {
        QString m;
        message_type t;
    };

    CSSParser();

    // valid levels are "CSS1.0", "CSS2.0", "CSS2.1", "CSS3.0"
    void set_level(QString level = "CSS3.0");

    void parse_css(QString css_input);

    void reset_parser();        

    // serialize the current list of csstokens back to css
    QString serialize_css(bool tostdout = true);
        
    // access charset, namespace and imports without having to walk csstokens
    QString get_charset();
    QVector<QString> get_import();
    QString get_namespace();

    // walk the csstokens list, token by token
    // set start_ptr to the position you woud like to start at in the list
    // leaving it as -1 will simply start at 0 and increment
    // last token is a dummy token with type set to CSS_END 
    token get_next_token(int start_ptr = -1);

    // covert token type enum value to a descriptive string
    QString get_type_name(token_type t);

    // this routine allows external modifications to the csstokens
    // to be brought back into CSSParser in order to serialize them
    // with serialize_css
    void set_csstokens(const QVector<token> &ntokens);

    // get errors, warnings, and information
    QVector<QString> get_parse_errors();
    QVector<QString> get_parse_warnings();
    QVector<QString> get_parse_info();

    // utility routine to properly split group selectors into individual selectors
    static QStringList splitGroupSelector(const QString &sel);

private:

    void parseInAtBlock(QString& css_input, int& i, parse_status& astatus, parse_status& afrom);

    void parseInSelector(QString& css_input, int& i, parse_status& astatus, parse_status& afrom,
                         bool& invalid_at, QChar& str_char, int str_size);

    void parseInProperty(QString& css_input, int& i, parse_status& astatus, parse_status& afrom,
                         bool& invalid_at);

    void parseInValue(QString& css_input, int& i, parse_status& astatus, parse_status& afrom,
                      bool& invalid_at, QChar& str_char, bool& pn, int str_size );

    void parseInComment(QString& css_input, int& i, parse_status& astatus, parse_status& afrom,
                        QString& cur_comment);

    void parseInString(QString& css_input, int& i, parse_status& astatus, parse_status& afrom,
                       QChar&  str_char, bool& str_in_str);


    // filter the logs by message type
    QVector<QString> get_logs(message_type t);

    // Pparses unicode notations
    QString unicode(QString& istring, int& i);
        
    // checks if the chat in istring at i is a token
    bool is_token(QString& istring, const int i);
                        
    void add_token(const token_type ttype, const QString data);
        
    // Add a message to the message log
    void log(const QString msg, const message_type type, int iline = 0);

    // records token position information
    void record_position(parse_status old_status, parse_status new_status,
                         QString &css_input, int i, bool force=false);
        
    int _seeknocomment(const int key, const int move);

    void explode_selectors();

    static bool property_is_next(QString istring, int pos);

    // private member variables
    QVector<QString> token_type_names;
    QMap<QString, parse_status>  at_rules;
    QVector<QString> csstemplate;
    QString css_level;
    QString tokens;
    int token_ptr;

    QMap<int, QVector<message> > logs;
    int  line;
    int  spos;
    int  sline;
    int  selector_nest_level;

    QString charset;
    QString namesp;
    QVector<QString> import;
    QVector<token> csstokens;
    QString cur_selector;
    QString cur_at;
    QString cur_property;
    QString cur_function;
    QString cur_sub_value;
    QString cur_value; 
    QString cur_string;
    QVector<int>  sel_separate;
    QVector<QString> cur_sub_value_arr;
    QVector<QString> cur_function_arr;

};
        
#endif // HEADER_CSSPARSER
