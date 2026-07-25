#ifndef HEADER_CSSUTILS
#define HEADER_CSSUTILS 

#include <QChar>
#include <QString>
#include <QVector>
#include <QFile>
#include <QFileInfo>

class CSSUtils
{

public:

    // Returns the lowercase version of a string
    static QString strtolower(QString istring);

    // Apparently faster replacement for tolower
    static QChar chartolower(const QChar c);

    // Returns the uppercase version of a string
    static QString strtoupper(QString istring);
    static QChar chartoupper(const QChar c);

    // Converts a hexadecimal number (string) to a decimal number
    static unsigned int hexdec(QString istring);

    // Checks if a charcter is escaped
    static bool escaped(const QString &istring, int pos);

    // Returns a QChar of a QString at pos but checks the QString-length before
    static QChar s_at(const QString &istring, int pos);

    // Implodes a vector of QString  e (string join with e as seprator))
    static QString implode(const QString e, const QVector<QString> s);

    // Explodes a QString into a vector of QStrings, splitting at s
    static QVector<QString> explode(const QString e, QString s, const bool check);

    // Builds a compact value QString, inserting spaces only where necessary
    static QString build_value(const QVector<QString> subvalues);

    // isspace() and isdigit() do not work correctly with UTF-8 QStrings
    static bool ctype_space(const QChar c);
    static bool ctype_digit(const QChar c);
    static bool ctype_xdigit(QChar c);
    static bool ctype_alpha(QChar c);

    // trims  whitespace at the specific or both ends of a QString
    static const QString trim(const QString istring);
    static const QString rtrim(const QString istring);
    static const QString rtrim(const QString istring, const QString chars);
    static const QString ltrim(const QString istring);

    // creates proper indent level
    static const QString indent(int lvl, const QString &base);

    // make std::string find_* equivalents for QString
    static int find_first_of(const QString &tgt, const QString& stopchars, int p = 0);
    static int find_first_not_of(const QString &tgt, const QString& skipchars, int p = 0);
    static int find_last_not_of(const QString &tgt, const QString& skipchars);
};
#endif // HEADER_CSSUTILS
