#ifndef HTML_PRETTY_PRINT
#define HTML_PRETTY_PRINT

#include <QList>

class QString;

class HTMLPrettyPrint
{
public:
    HTMLPrettyPrint(const QString &source);
    ~HTMLPrettyPrint();

    QString prettyPrint();

private:
    typedef enum {
        TOKEN_TYPE_TAG,
        TOKEN_TYPE_OPEN_TAG,
        TOKEN_TYPE_CLOSE_TAG,
        TOKEN_TYPE_SELF_CLOSING_TAG,
        TOKEN_TYPE_COMMENT,
        TOKEN_TYPE_DECL,
        TOKEN_TYPE_XML_DECL,
        TOKEN_TYPE_DOC_TYPE,
        TOKEN_TYPE_TEXT
    } TOKEN_TYPE;

    typedef struct {
        TOKEN_TYPE type;
        size_t start;
        size_t len;
        QString tag;
    } HTMLToken;

    void tokenize();
    QString cleanSegement(const QString &source);
    TOKEN_TYPE tokenType(const QString &source);
    QString tag(const QString &source);

    QString m_source;
    QList<HTMLToken *> m_tokens;
};

#endif

