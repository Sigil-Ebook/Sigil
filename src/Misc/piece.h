    QString prettyprint(QString indent_chars="  ");

private:

    std::string prettyprint(GumboNode* node, int lvl, const std::string indent_chars);
    std::string prettyprint_contents(GumboNode* node, int lvl, const std::string indent_chars);
