/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * $Id: RegularExpression.hpp 822158 2009-10-06 07:52:59Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_REGULAREXPRESSION_HPP)
#define XERCESC_INCLUDE_GUARD_REGULAREXPRESSION_HPP

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/RefArrayVectorOf.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/regx/Op.hpp>
#include <xercesc/util/regx/TokenFactory.hpp>
#include <xercesc/util/regx/BMPattern.hpp>
#include <xercesc/util/regx/OpFactory.hpp>
#include <xercesc/util/regx/RegxUtil.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Forward Declaration
// ---------------------------------------------------------------------------
class RangeToken;
class Match;
class RegxParser;

/**
 * The RegularExpression class represents a parsed executable regular expression.
 * This class is thread safe. Two similar regular expression syntaxes are
 * supported:
 *
 * <ol>
 * <li><a href="http://www.w3.org/TR/xpath-functions/#regex-syntax">The XPath 2.0 / XQuery regular expression syntax.</a>
 * <li><a href="http://www.w3.org/TR/xmlschema-2/#regexs">The XML Schema regular expression syntax.</a></li>
 * </ol>
 * 
 * XPath 2.0 regular expression syntax is used unless the "X" option is specified during construction.
 *
 * Options can be specified during construction to change the way that the regular expression is handled.
 * Options are specified by a string consisting of any number of the following characters:
 *
 * <table border='1'>
 * <tr>
 * <th>Character</th>
 * <th>Meaning</th>
 * </tr>
 * <tr>
 * <td valign='top' rowspan='1' colspan='1'>i</td>
 * <td valign='top' rowspan='1' colspan='1'><a href="http://www.w3.org/TR/xpath-functions/#flags">
 * Ignore case</a> when matching the regular expression.</td>
 * </tr>
 * <tr>
 * <td valign='top' rowspan='1' colspan='1'>m</td>
 * <td valign='top' rowspan='1' colspan='1'><a href="http://www.w3.org/TR/xpath-functions/#flags">
 * Multi-line mode</a>. The meta characters "^" and "$" will match the beginning and end of lines.</td>
 * </tr>
 * <tr>
 * <td valign='top' rowspan='1' colspan='1'>s</td>
 * <td valign='top' rowspan='1' colspan='1'><a href="http://www.w3.org/TR/xpath-functions/#flags">
 * Single-line mode</a>. The meta character "." will match a newline character.</td>
 * </tr>
 * <tr>
 * <td valign='top' rowspan='1' colspan='1'>x</td>
 * <td valign='top' rowspan='1' colspan='1'>Allow extended comments.</td>
 * </tr>
 * <tr>
 * <td valign='top' rowspan='1' colspan='1'>F</td>
 * <td valign='top' rowspan='1' colspan='1'>Prohibit the fixed string optimization.</td>
 * </tr>
 * <tr>
 * <td valign='top' rowspan='1' colspan='1'>H</td>
 * <td valign='top' rowspan='1' colspan='1'>Prohibit the head character optimization.</td>
 * </tr>
 * <tr>
 * <td valign='top' rowspan='1' colspan='1'>X</td>
 * <td valign='top' rowspan='1' colspan='1'>Parse the regular expression according to the
 * <a href="http://www.w3.org/TR/xmlschema-2/#regexs">XML Schema regular expression syntax</a>.</td>
 * </tr>
 * </table>
 */
class XMLUTIL_EXPORT RegularExpression : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Public Constructors and Destructor
    // -----------------------------------------------------------------------

    /** @name Constructors and destructor */
    //@{

    /** Parses the given regular expression.
      *
      * @param pattern the regular expression in the local code page
      * @param manager the memory manager to use
      */
    RegularExpression
    (
        const char* const pattern
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Parses the given regular expression using the options specified.
      *
      * @param pattern the regular expression in the local code page
      * @param options the options string in the local code page
      * @param manager the memory manager to use
      */
    RegularExpression
    (
        const char* const pattern
        , const char* const options
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Parses the given regular expression.
      *
      * @param pattern the regular expression
      * @param manager the memory manager to use
      */
    RegularExpression
    (
        const XMLCh* const pattern
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    /** Parses the given regular expression using the options specified.
      *
      * @param pattern the regular expression
      * @param options the options string
      * @param manager the memory manager to use
      */
    RegularExpression
    (
        const XMLCh* const pattern
        , const XMLCh* const options
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    virtual ~RegularExpression();

    //@}

    // -----------------------------------------------------------------------
    //  Public Constants
    // -----------------------------------------------------------------------
    static const unsigned int   IGNORE_CASE;
    static const unsigned int   SINGLE_LINE;
    static const unsigned int   MULTIPLE_LINE;
    static const unsigned int   EXTENDED_COMMENT;
    static const unsigned int   PROHIBIT_HEAD_CHARACTER_OPTIMIZATION;
    static const unsigned int   PROHIBIT_FIXED_STRING_OPTIMIZATION;
    static const unsigned int   XMLSCHEMA_MODE;
    typedef enum
    {
        wordTypeIgnore = 0,
        wordTypeLetter = 1,
        wordTypeOther = 2
    } wordType;

    // -----------------------------------------------------------------------
    //  Public Helper methods
    // -----------------------------------------------------------------------

    /** @name Public helper methods */
    //@{

    static int getOptionValue(const XMLCh ch);
    static bool isSet(const int options, const int flag);

    //@}

    // -----------------------------------------------------------------------
    //  Matching methods
    // -----------------------------------------------------------------------

    /** @name Matching methods */
    //@{

    /** Tries to match the given null terminated string against the regular expression, returning
      * true if successful.
      *
      * @param matchString the string to match in the local code page
      * @param manager     the memory manager to use
      *
      * @return Whether the string matched the regular expression or not.
      */
    bool matches(const char* const matchString,
                 MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) const;

    /** Tries to match the given string between the specified start and end offsets
      * against the regular expression, returning true if successful.
      *
      * @param matchString the string to match in the local code page
      * @param start       the offset of the start of the string
      * @param end         the offset of the end of the string
      * @param manager     the memory manager to use
      *
      * @return Whether the string matched the regular expression or not.
      */
    bool matches(const char* const matchString, const XMLSize_t start, const XMLSize_t end,
                 MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) const;

    /** Tries to match the given null terminated string against the regular expression, returning
      * true if successful.
      *
      * @param matchString the string to match in the local code page
      * @param pMatch      a Match object, which will be populated with the offsets for the
      * regular expression match and sub-matches.
      * @param manager     the memory manager to use
      *
      * @return Whether the string matched the regular expression or not.
      */
    bool matches(const char* const matchString, Match* const pMatch,
                 MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) const;

    /** Tries to match the given string between the specified start and end offsets
      * against the regular expression, returning true if successful.
      *
      * @param matchString the string to match in the local code page
      * @param start       the offset of the start of the string
      * @param end         the offset of the end of the string
      * @param pMatch      a Match object, which will be populated with the offsets for the
      * regular expression match and sub-matches.
      * @param manager     the memory manager to use
      *
      * @return Whether the string matched the regular expression or not.
      */
    bool matches(const char* const matchString, const XMLSize_t start, const XMLSize_t end,
                 Match* const pMatch, MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) const;

    /** Tries to match the given null terminated string against the regular expression, returning
      * true if successful.
      *
      * @param matchString the string to match
      * @param manager     the memory manager to use
      *
      * @return Whether the string matched the regular expression or not.
      */
    bool matches(const XMLCh* const matchString,
                 MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) const;

    /** Tries to match the given string between the specified start and end offsets
      * against the regular expression, returning true if successful.
      *
      * @param matchString the string to match
      * @param start       the offset of the start of the string
      * @param end         the offset of the end of the string
      * @param manager     the memory manager to use
      *
      * @return Whether the string matched the regular expression or not.
      */
    bool matches(const XMLCh* const matchString, const XMLSize_t start, const XMLSize_t end,
                 MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) const;

    /** Tries to match the given null terminated string against the regular expression, returning
      * true if successful.
      *
      * @param matchString the string to match
      * @param pMatch      a Match object, which will be populated with the offsets for the
      * regular expression match and sub-matches.
      * @param manager     the memory manager to use
      *
      * @return Whether the string matched the regular expression or not.
      */
    bool matches(const XMLCh* const matchString, Match* const pMatch,
                 MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) const;

    /** Tries to match the given string between the specified start and end offsets
      * against the regular expression, returning true if successful.
      *
      * @param matchString the string to match
      * @param start       the offset of the start of the string
      * @param end         the offset of the end of the string
      * @param pMatch      a Match object, which will be populated with the offsets for the
      * regular expression match and sub-matches.
      * @param manager     the memory manager to use
      *
      * @return Whether the string matched the regular expression or not.
      */
    bool matches(const XMLCh* const matchString, const XMLSize_t start, const XMLSize_t end,
                 Match* const pMatch, MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) const;

    /** Tries to match the given string between the specified start and end offsets
      * against the regular expression. The subEx vector is populated with the details
      * for every non-overlapping occurrence of a match in the string.
      *
      * @param matchString the string to match
      * @param start       the offset of the start of the string
      * @param end         the offset of the end of the string
      * @param subEx       a RefVectorOf Match objects, populated with the offsets for the
      * regular expression match and sub-matches.
      * @param manager     the memory manager to use
      */
    void allMatches(const XMLCh* const matchString, const XMLSize_t start, const XMLSize_t end,
                    RefVectorOf<Match> *subEx, MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) const;

    //@}

    // -----------------------------------------------------------------------
    //  Tokenize methods
    // -----------------------------------------------------------------------
    // Note: The caller owns the string vector that is returned, and is responsible
    //       for deleting it.

    /** @name Tokenize methods */
    //@{

    /** Tokenizes the null terminated string according to the regular expression, returning
      * the parts of the string that do not match the regular expression.
      *
      * @param matchString the string to match in the local code page
      * @param manager     the memory manager to use
      *
      * @return A RefArrayVectorOf sub-strings that do not match the regular expression allocated using the
      * given MemoryManager. The caller owns the string vector that is returned, and is responsible for
      * deleting it.
      */
    RefArrayVectorOf<XMLCh> *tokenize(const char* const matchString,
                                      MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) const;

    /** Tokenizes the string between the specified start and end offsets according to the regular
      * expression, returning the parts of the string that do not match the regular expression.
      *
      * @param matchString the string to match in the local code page
      * @param start       the offset of the start of the string
      * @param end         the offset of the end of the string
      * @param manager     the memory manager to use
      *
      * @return A RefArrayVectorOf sub-strings that do not match the regular expression allocated using the
      * given MemoryManager. The caller owns the string vector that is returned, and is responsible for
      * deleting it.
      */
    RefArrayVectorOf<XMLCh> *tokenize(const char* const matchString, const XMLSize_t start, const XMLSize_t end,
                                      MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) const;

    /** Tokenizes the null terminated string according to the regular expression, returning
      * the parts of the string that do not match the regular expression.
      *
      * @param matchString the string to match
      * @param manager     the memory manager to use
      *
      * @return A RefArrayVectorOf sub-strings that do not match the regular expression allocated using the
      * given MemoryManager. The caller owns the string vector that is returned, and is responsible for
      * deleting it.
      */
    RefArrayVectorOf<XMLCh> *tokenize(const XMLCh* const matchString,
                                      MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) const;

    /** Tokenizes the string between the specified start and end offsets according to the regular
      * expression, returning the parts of the string that do not match the regular expression.
      *
      * @param matchString the string to match
      * @param start       the offset of the start of the string
      * @param end         the offset of the end of the string
      * @param manager     the memory manager to use
      *
      * @return A RefArrayVectorOf sub-strings that do not match the regular expression allocated using the
      * given MemoryManager. The caller owns the string vector that is returned, and is responsible for
      * deleting it.
      */
    RefArrayVectorOf<XMLCh> *tokenize(const XMLCh* const matchString, const XMLSize_t start, const XMLSize_t end,
                                      MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) const;

    //@}

    // -----------------------------------------------------------------------
    //  Replace methods
    // -----------------------------------------------------------------------
    // Note: The caller owns the XMLCh* that is returned, and is responsible for
    //       deleting it.

    /** @name Replace methods */
    //@{

    /** Performs a search and replace on the given null terminated string, replacing
      * any substring that matches the regular expression with a string derived from
      * the <a href="http://www.w3.org/TR/xpath-functions/#func-replace">replacement string</a>.
      *
      * @param matchString   the string to match in the local code page
      * @param replaceString the string to replace in the local code page
      * @param manager       the memory manager to use
      *
      * @return The resulting string allocated using the given MemoryManager. The caller owns the string
      * that is returned, and is responsible for deleting it.
      */
    XMLCh *replace(const char* const matchString, const char* const replaceString,
                   MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) const;

    /** Performs a search and replace on the given string between the specified start and end offsets, replacing
      * any substring that matches the regular expression with a string derived from
      * the <a href="http://www.w3.org/TR/xpath-functions/#func-replace">replacement string</a>.
      *
      * @param matchString   the string to match in the local code page
      * @param replaceString the string to replace in the local code page
      * @param start         the offset of the start of the string
      * @param end           the offset of the end of the string
      * @param manager       the memory manager to use
      *
      * @return The resulting string allocated using the given MemoryManager. The caller owns the string
      * that is returned, and is responsible for deleting it.
      */
    XMLCh *replace(const char* const matchString, const char* const replaceString,
                   const XMLSize_t start, const XMLSize_t end,
                   MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) const;

    /** Performs a search and replace on the given null terminated string, replacing
      * any substring that matches the regular expression with a string derived from
      * the <a href="http://www.w3.org/TR/xpath-functions/#func-replace">replacement string</a>.
      *
      * @param matchString   the string to match
      * @param replaceString the string to replace
      * @param manager       the memory manager to use
      *
      * @return The resulting string allocated using the given MemoryManager. The caller owns the string
      * that is returned, and is responsible for deleting it.
      */
    XMLCh *replace(const XMLCh* const matchString, const XMLCh* const replaceString,
                   MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) const;

    /** Performs a search and replace on the given string between the specified start and end offsets, replacing
      * any substring that matches the regular expression with a string derived from
      * the <a href="http://www.w3.org/TR/xpath-functions/#func-replace">replacement string</a>.
      *
      * @param matchString   the string to match
      * @param replaceString the string to replace
      * @param start         the offset of the start of the string
      * @param end           the offset of the end of the string
      * @param manager       the memory manager to use
      *
      * @return The resulting string allocated using the given MemoryManager. The caller owns the string
      * that is returned, and is responsible for deleting it.
      */
    XMLCh *replace(const XMLCh* const matchString, const XMLCh* const replaceString,
                   const XMLSize_t start, const XMLSize_t end,
                   MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) const;

    //@}

    // -----------------------------------------------------------------------
    //  Static initialize and cleanup methods
    // -----------------------------------------------------------------------

    /** @name Static initilize and cleanup methods */
    //@{

    static void
    staticInitialize(MemoryManager*  memoryManager);

    static void
    staticCleanup();

    //@}

protected:
    virtual RegxParser* getRegexParser(const int options, MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    // -----------------------------------------------------------------------
    //  Cleanup methods
    // -----------------------------------------------------------------------
    void cleanUp();

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void setPattern(const XMLCh* const pattern, const XMLCh* const options=0);

    // -----------------------------------------------------------------------
    //  Protected data types
    // -----------------------------------------------------------------------
    class XMLUTIL_EXPORT Context : public XMemory
    {
        public :
            Context(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
            Context(Context* src);
            ~Context();

            Context& operator= (const Context& other);
            inline const XMLCh* getString() const { return fString; }
            void reset(const XMLCh* const string, const XMLSize_t stringLen,
                       const XMLSize_t start, const XMLSize_t limit, const int noClosures,
                       const unsigned int options);
            bool nextCh(XMLInt32& ch, XMLSize_t& offset);

            bool           fAdoptMatch;
            XMLSize_t      fStart;
            XMLSize_t      fLimit;
            XMLSize_t      fLength;    // fLimit - fStart
            int            fSize;
            XMLSize_t      fStringMaxLen;
            int*           fOffsets;
            Match*         fMatch;
            const XMLCh*   fString;
            unsigned int   fOptions;
            MemoryManager* fMemoryManager;
    };

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    RegularExpression(const RegularExpression&);
    RegularExpression& operator=(const RegularExpression&);

    // -----------------------------------------------------------------------
    //  Protected Helper methods
    // -----------------------------------------------------------------------
    void prepare();
    int parseOptions(const XMLCh* const options);

    /**
      *    Matching helpers
      */
    int match(Context* const context, const Op* const operations, XMLSize_t offset) const;
    bool matchIgnoreCase(const XMLInt32 ch1, const XMLInt32 ch2) const;

    /**
      *    Helper methods used by match(Context* ...)
      */
    bool matchChar(Context* const context, const XMLInt32 ch, XMLSize_t& offset,
                   const bool ignoreCase) const;
    bool matchDot(Context* const context, XMLSize_t& offset) const;
    bool matchRange(Context* const context, const Op* const op,
                    XMLSize_t& offset, const bool ignoreCase) const;
    bool matchAnchor(Context* const context, const XMLInt32 ch,
                     const XMLSize_t offset) const;
    bool matchBackReference(Context* const context, const XMLInt32 ch,
                            XMLSize_t& offset, const bool ignoreCase) const;
    bool matchString(Context* const context, const XMLCh* const literal,
                     XMLSize_t& offset, const bool ignoreCase) const;
    int  matchUnion(Context* const context, const Op* const op, XMLSize_t offset) const;
    int matchCapture(Context* const context, const Op* const op, XMLSize_t offset) const;

    /**
     *    Replace helpers
     */
    void subInExp(const XMLCh* const repString,
                  const XMLCh* const origString,
                  const Match* subEx,
                  XMLBuffer &result,
                  MemoryManager* const manager) const;
    /**
     *    Converts a token tree into an operation tree
     */
    void compile(const Token* const token);
    Op*  compile(const Token* const token, Op* const next,
                 const bool reverse);
    /**
      *    Helper methods used by compile
      */
    Op* compileUnion(const Token* const token, Op* const next,
                     const bool reverse);
    Op* compileParenthesis(const Token* const token, Op* const next,
                           const bool reverse);
    Op* compileConcat(const Token* const token, Op* const next,
                      const bool reverse);
    Op* compileClosure(const Token* const token, Op* const next,
                       const bool reverse, const Token::tokType tkType);

    bool doTokenOverlap(const Op* op, Token* token);

    // -----------------------------------------------------------------------
    //  Protected data members
    // -----------------------------------------------------------------------
    bool               fHasBackReferences;
    bool               fFixedStringOnly;
    int                fNoGroups;
    XMLSize_t          fMinLength;
    unsigned int       fNoClosures;
    unsigned int       fOptions;
    const BMPattern*   fBMPattern;
    XMLCh*             fPattern;
    XMLCh*             fFixedString;
    const Op*          fOperations;
    Token*             fTokenTree;
    RangeToken*        fFirstChar;
    static RangeToken* fWordRange;
    OpFactory          fOpFactory;
    TokenFactory*      fTokenFactory;
    MemoryManager*     fMemoryManager;
};



  // -----------------------------------------------------------------------
  //  RegularExpression: Static initialize and cleanup methods
  // -----------------------------------------------------------------------
  inline void RegularExpression::staticCleanup()
  {
      fWordRange = 0;
  }

  // ---------------------------------------------------------------------------
  //  RegularExpression: Cleanup methods
  // ---------------------------------------------------------------------------
  inline void RegularExpression::cleanUp() {

      fMemoryManager->deallocate(fPattern);//delete [] fPattern;
      fMemoryManager->deallocate(fFixedString);//delete [] fFixedString;
      delete fBMPattern;
      delete fTokenFactory;
  }

  // ---------------------------------------------------------------------------
  //  RegularExpression: Helper methods
  // ---------------------------------------------------------------------------
  inline bool RegularExpression::isSet(const int options, const int flag) {

      return (options & flag) == flag;
  }


  inline Op* RegularExpression::compileUnion(const Token* const token,
                                             Op* const next,
                                             const bool reverse) {

      XMLSize_t tokSize = token->size();
      UnionOp* uniOp = fOpFactory.createUnionOp(tokSize);

      for (XMLSize_t i=0; i<tokSize; i++) {

          uniOp->addElement(compile(token->getChild(i), next, reverse));
      }

      return uniOp;
  }


  inline Op* RegularExpression::compileParenthesis(const Token* const token,
                                                   Op* const next,
                                                   const bool reverse) {

      if (token->getNoParen() == 0)
          return compile(token->getChild(0), next, reverse);

      Op* captureOp    = 0;

      if (reverse) {

          captureOp = fOpFactory.createCaptureOp(token->getNoParen(), next);
          captureOp = compile(token->getChild(0), captureOp, reverse);

          return fOpFactory.createCaptureOp(-token->getNoParen(), captureOp);
      }

      captureOp = fOpFactory.createCaptureOp(-token->getNoParen(), next);
      captureOp = compile(token->getChild(0), captureOp, reverse);

      return fOpFactory.createCaptureOp(token->getNoParen(), captureOp);
  }

  inline Op* RegularExpression::compileConcat(const Token* const token,
                                              Op*  const next,
                                              const bool reverse) {

      Op* ret = next;
      XMLSize_t tokSize = token->size();

      if (!reverse) {

          for (XMLSize_t i= tokSize; i>0; i--) {
              ret = compile(token->getChild(i-1), ret, false);
          }
      }
      else {

          for (XMLSize_t i= 0; i< tokSize; i++) {
              ret = compile(token->getChild(i), ret, true);
          }
      }

      return ret;
  }

  inline Op* RegularExpression::compileClosure(const Token* const token,
                                               Op* const next,
                                               const bool reverse,
                                               const Token::tokType tkType) {

      Op*    ret      = 0;
      Token* childTok = token->getChild(0);
      int    min      = token->getMin();
      int    max      = token->getMax();

      if (min >= 0 && min == max) {

          ret = next;
          for (int i=0; i< min; i++) {
              ret = compile(childTok, ret, reverse);
          }

          return ret;
      }

      if (min > 0 && max > 0)
          max -= min;

      if (max > 0) {

          ret = next;
          for (int i=0; i<max; i++) {

              ChildOp* childOp = fOpFactory.createQuestionOp(
                  tkType == Token::T_NONGREEDYCLOSURE);

              childOp->setNextOp(next);
              childOp->setChild(compile(childTok, ret, reverse));
              ret = childOp;
          }
      }
      else {

          ChildOp* childOp = 0;

          if (tkType == Token::T_NONGREEDYCLOSURE) {
              childOp = fOpFactory.createNonGreedyClosureOp();
          }
          else {

              if (childTok->getMinLength() == 0)
                  childOp = fOpFactory.createClosureOp(fNoClosures++);
              else
                  childOp = fOpFactory.createClosureOp(-1);
          }

          childOp->setNextOp(next);
          if(next==NULL || !doTokenOverlap(next, childTok))
          {
              childOp->setOpType(tkType == Token::T_NONGREEDYCLOSURE?Op::O_FINITE_NONGREEDYCLOSURE:Op::O_FINITE_CLOSURE);
              childOp->setChild(compile(childTok, NULL, reverse));
          }
          else
          {
              childOp->setChild(compile(childTok, childOp, reverse));
          }
          ret = childOp;
      }

      if (min > 0) {

          for (int i=0; i< min; i++) {
              ret = compile(childTok, ret, reverse);
          }
      }

      return ret;
  }

XERCES_CPP_NAMESPACE_END

#endif
/**
  * End of file RegularExpression.hpp
  */

