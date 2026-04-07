/************************************************************************
 **
 **  qCSSDenester.h
 **  Used in Sigil's built in Qt based css parser
 **
 **  Copyright (C) 2026  Kevin B. Hendricks, Stratford, Ontario, Canada
 **  Co-authored with Google AI
 **
 **  License: LGPL v2.1 or later
 **
 **  Extracted and modified from code suggestions from Google's Browser AI
 **      and refined by repeated prompts
 **
 ** You should have received a copy of the GNU Lesser General Public License 2.1
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include <QString>
#include <QChar>
#include <QVector>
#include <QMap>
#include <QTextStream>
#include <QRegularExpression>

#include "Parsers/qCSSUtils.h"

struct ParseError {
    QString message;
    int line;
    QString context;
};

class CSSDenester {
private:
    QVector<ParseError> errors;

    void logError(const QString& msg, int line, const QString& buffer) {
        // Capture a small snippet of the buffer to help identify the error
        QString snippet = buffer.mid(0, 20) + (buffer.length() > 20 ? "..." : "");
        errors.push_back({msg, line, CSSUtils::trim(snippet)});
    }

public:
    QString denest(const QString& input) {
        errors.clear();
        QString output;
        QTextStream out_stream(&output);
        QVector<QVector<QString>> selectorStack;
        QVector<QString> atRuleStack;
        QString buffer;
        int  currentLine = 1;

        for (int i = 0; i < input.size(); ++i) {
            QChar c = input.at(i);
            if (c == '\n') currentLine++;

            // 1. Skip Comments and track internal lines
            if (c == '/' && i + 1 < input.size() && input.at(i + 1) == '*') {
                int end = input.indexOf("*/", i + 2);
                if (end == -1) {
                    logError("Unclosed comment", currentLine, "/*...");
                    break;
                }
                QString commentBody = input.mid(i, end - i);
                currentLine += std::count(commentBody.begin(), commentBody.end(), '\n');
                i = end + 1;
                continue;
            }

            // 2. Handle Strings (prevents braces inside quotes from breaking logic)
            if (c == '"' || c == '\'') {
                QChar quote = c;
                buffer += c;
                while (++i < input.size() && input.at(i) != quote) {
                    if (input.at(i) == '\\') buffer += input.at(i++);
                    buffer += input.at(i);
                    if (input.at(i) == '\n') currentLine++;
                }
                if (i >= input.size()) logError("Unclosed string literal", currentLine, buffer);
                else buffer += quote;
                continue;
            }

            // 3. Structural Parsing
            if (c == '{') {
                QString context = CSSUtils::trim(buffer);
                if (context.isEmpty()) {
                    logError("Empty block selector", currentLine, "{");
                } else if (context.startsWith('@')) {
                    atRuleStack.push_back(context);
                } else {
                    selectorStack.push_back(CSSUtils::explode("," , context, false));
                }
                buffer.clear();
            } else if (c == '}') {
                if (selectorStack.isEmpty() && atRuleStack.isEmpty()) {
                    logError("Unexpected closing brace", currentLine, "}");
                } else {
                    if (!selectorStack.isEmpty()) selectorStack.pop_back();
                    else atRuleStack.pop_back();
                }
                buffer.clear();
            } else if (c == ';') {
                if (!buffer.isEmpty()) writeRule(out_stream, buffer, selectorStack, atRuleStack);
                buffer.clear();
            } else {
                buffer += c;
            }
        }

        if (!selectorStack.isEmpty() || !atRuleStack.isEmpty()) {
            logError("Missing closing brace(s)", currentLine, "End of File");
        }

        return output;
    }

    void writeRule(QTextStream& out, const QString& prop,
                   const QVector<QVector<QString>>& sStack,
                   const QVector<QString>& aStack) {

        QString property = CSSUtils::trim(prop);
        if (property.isEmpty()) return;

        // Apply At-Rule wrappers (e.g., @media)
        for (const auto& rule : aStack) out << rule << " { ";

        // Flatten Selector Hierarchy (Cartesian Product for comma-separated selectors)
        QVector<QString> paths = {""};
        for (const auto& level : sStack) {
            QVector<QString> next;
            for (const auto& p : paths) {
                for (const auto& c : level) {
                    QString combined;
                    int ampPos = c.indexOf('&');
                    if (ampPos != -1) {
                        combined = c;
                        combined.replace(ampPos, 1, p);
                    } else {
                        combined = p.isEmpty() ? c : p + " " + c;
                    }
                    next.push_back(CSSUtils::trim(combined));
                }
            }
            paths = next;
        }

        for (int i = 0; i < paths.size(); ++i) {
            out << paths[i] << (i == paths.size() - 1 ? "" : ", ");
        }

        out << " { " << property << "; } ";
        for (int i = 0; i < aStack.size(); ++i) out << "}";
        out << "\n";
    }

    int get_denest_error_count()
    {
        return errors.size();
    }

    QVector<QString> get_denest_errors()
    {
        QVector<QString> denest_errors;
        for (const auto& e : errors) {
            QString errormsg = e.message + " line:" + QString::number(e.line) + " near: " + e.context;
            denest_errors.push_back(errormsg);
        }
        return denest_errors;
    }
    
};
