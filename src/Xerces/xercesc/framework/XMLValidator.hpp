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
 * $Id: XMLValidator.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLVALIDATOR_HPP)
#define XERCESC_INCLUDE_GUARD_XMLVALIDATOR_HPP

#include <xercesc/framework/XMLAttr.hpp>
#include <xercesc/framework/XMLValidityCodes.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class ReaderMgr;
class XMLBufferMgr;
class XMLElementDecl;
class XMLScanner;
class Grammar;


/**
 *  This abstract class provides the interface for all validators. This is
 *  the simple amount of API that all validators must honor, in order for
 *  the scanner to use them to do validation. All validators will actually
 *  contain much more functionality than is accessible via this common API,
 *  but that functionality requires that you know what type of validator you
 *  are dealing with.
 *
 *  Basically, at this level, the primary concern is to be able to query
 *  core information about elements and attributes. Adding decls to the
 *  validator requires that you go through the derived interface because they
 *  all have their own decl types. At this level, we can return information
 *  via the base decl classes, from which each validator derives its own
 *  decl classes.
 */
class XMLPARSER_EXPORT XMLValidator : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Constructors are hidden, just the virtual destructor is exposed
    // -----------------------------------------------------------------------

    /** @name Destructor */
    //@{

    /**
     *  The derived class should clean up its allocated data, then this class
     *  will do the same for data allocated at this level.
     */
    virtual ~XMLValidator()
    {
    }
    //@}


    // -----------------------------------------------------------------------
    //  The virtual validator interface
    // -----------------------------------------------------------------------

    /** @name Virtual validator interface */
    //@{

    /**
      * The derived class should look up its declaration of the passed element
      * from its element pool. It should then use the content model description
      * contained in that element declaration to validate that the passed list
      * of child elements are valid for that content model. The count can be
      * zero, indicating no child elements.
      *
      * Note that whitespace and text content are not validated here. Those are
      * handled by the scanner. So only element ids are provided here.
      *
      * @param  elemDecl    The element whose content is to be checked.
      *
      * @param  children    An array of element QName which represent the elements
      *                     found within the parent element, i.e. the content
      *                     to be validated.
      *
      * @param  childCount  The number of elements in the childIds array. It can
      *                     be zero if the element had none.
      *
      * @param  indexFailingChild  On return, it will contain the index of the 
      *                            children failing validation, if the retun value
      *                            is false
      *
      */
    virtual bool checkContent
    (
        XMLElementDecl* const   elemDecl
        , QName** const         children
        , XMLSize_t             childCount
        , XMLSize_t*            indexFailingChild
    ) = 0;

    /**
      * The derived class should fault in the passed XMLAttr value. It should
      * use the passeed attribute definition (which is passed via the base
      * type so it must often be downcast to the appropriate type for the
      * derived validator class), to fill in the passed attribute. This is done
      * as a performance enhancement since the derived class has more direct
      * access to the information.
      */
    virtual void faultInAttr
    (
                XMLAttr&    toFill
        , const XMLAttDef&  attDef
    )   const = 0;

    /**
      * This method is called by the scanner after a Grammar is scanned.
      */
    virtual void preContentValidation(bool reuseGrammar,
                                      bool validateDefAttr = false) = 0;

    /**
      * This method is called by the scanner after the parse has completed. It
      * gives the validator a chance to check certain things that can only be
      * checked after the whole document has been parsed, such as referential
      * integrity of ID/IDREF pairs and so forth. The validator should just
      * issue errors for any problems it finds.
      */
    virtual void postParseValidation() = 0;

    /**
      * This method is called by the scanner before a new document is about
      * to start. It gives the validator a change to reset itself in preparation
      * for another validation pass.
      */
    virtual void reset() = 0;

    /**
      * The derived class should return a boolean that indicates whether it
      * requires namespace processing or not. Some do and some allow it to be
      * optional. This flag is used to control whether the client code's
      * requests to disable namespace processing can be honored or not.
      */
    virtual bool requiresNamespaces() const = 0;

    /**
      * The derived class should apply any rules to the passed attribute value
      * that are above and beyond those defined by XML 1.0. The scanner itself
      * will impose XML 1.0 rules, based on the type of the attribute. This
      * will generally be used to check things such as range checks and other
      * datatype related validation.
      *
      * If the value breaks any rules as defined by the derived class, it
      * should just issue errors as usual.
      */
    virtual void validateAttrValue
    (
        const   XMLAttDef*                  attDef
        , const XMLCh* const                attrValue
        , bool                              preValidation = false
        , const XMLElementDecl*             elemDecl = 0
    ) = 0;

    /**
      * The derived class should apply any rules to the passed element decl
      * that are above and beyond those defined by XML 1.0.
      *
      * If the value breaks any rules as defined by the derived class, it
      * should just issue errors as usual.
      */
    virtual void validateElement
    (
        const   XMLElementDecl*             elemDef
    ) = 0;

    /**
      * Retrieve the Grammar used
      */
    virtual Grammar* getGrammar() const =0;

    /**
      * Set the Grammar
      */
    virtual void setGrammar(Grammar* aGrammar) =0;


    //@}

    // -----------------------------------------------------------------------
    //  Virtual DTD handler interface.
    // -----------------------------------------------------------------------

    /** @name Virtual DTD handler interface */
    //@{

    /**
      * This method allows the scanner to ask the validator if it handles
      * DTDs or not.
      */
    virtual bool handlesDTD() const = 0;

    // -----------------------------------------------------------------------
    //  Virtual Schema handler interface.
    // -----------------------------------------------------------------------

    /** @name Virtual Schema handler interface */

    /**
      * This method allows the scanner to ask the validator if it handles
      * Schema or not.
      */
    virtual bool handlesSchema() const = 0;

    //@}

    // -----------------------------------------------------------------------
    //  Setter methods
    //
    //  setScannerInfo() is called by the scanner to tell the validator
    //  about the stuff it needs to have access to.
    // -----------------------------------------------------------------------

    /** @name Setter methods */
    //@{

    /**
      * @param  owningScanner   This is a pointer to the scanner to which the
      *                         validator belongs. The validator will often
      *                         need to query state data from the scanner.
      *
      * @param  readerMgr       This is a pointer to the reader manager that is
      *                         being used by the scanner.
      *
      * @param  bufMgr          This is the buffer manager of the scanner. This
      *                         is provided as a convenience so that the validator
      *                         doesn't have to create its own buffer manager
      *                         during the parse process.
      */
    void setScannerInfo
    (
        XMLScanner* const           owningScanner
        , ReaderMgr* const          readerMgr
        , XMLBufferMgr* const       bufMgr
    );

    /**
      * This method is called to set an error reporter on the validator via
      * which it will report any errors it sees during parsing or validation.
      * This is generally called by the owning scanner.
      *
      * @param  errorReporter   A pointer to the error reporter to use. This
      *                         is not adopted, just referenced so the caller
      *                         remains responsible for its cleanup, if any.
      */
    void setErrorReporter
    (
        XMLErrorReporter* const errorReporter
    );

    //@}


    // -----------------------------------------------------------------------
    //  Error emitter methods
    // -----------------------------------------------------------------------

    /** @name Error emittor methods */
    //@{

    /**
     *  This call is a convenience by which validators can emit errors. Most
     *  of the grunt work of loading the text, getting the current source
     *  location, ect... is handled here.
     *
     *  If the loaded text has replacement parameters, then text strings can be
     *  passed. These will be used to replace the tokens {0}, {1}, {2}, and {3}
     *  in the order passed. So text1 will replace {0}, text2 will replace {1},
     *  and so forth.
     *
     *  textX   Up to four replacement parameters. They can be provided
     *          as either XMLCh strings, or local code page strings which
     *          will be transcoded internally.
     *
     *  @param toEmit   The error code to emit. it must be one of the defined
     *                  validator error codes.
     *
     */
    void emitError(const XMLValid::Codes toEmit);
    void emitError
    (
        const   XMLValid::Codes toEmit
        , const XMLCh* const    text1
        , const XMLCh* const    text2 = 0
        , const XMLCh* const    text3 = 0
        , const XMLCh* const    text4 = 0
    );
    void emitError
    (
        const   XMLValid::Codes toEmit
        , const char* const     text1
        , const char* const     text2 = 0
        , const char* const     text3 = 0
        , const char* const     text4 = 0
    );
    void emitError
    (
        const   XMLValid::Codes toEmit
        , const XMLExcepts::Codes   originalErrorCode
        , const XMLCh* const        text1 = 0
        , const XMLCh* const        text2 = 0
        , const XMLCh* const        text3 = 0
        , const XMLCh* const        text4 = 0

    );

    //@}

protected :
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    XMLValidator
    (
        XMLErrorReporter* const errReporter = 0
    );


    // -----------------------------------------------------------------------
    //  Protected getters
    // -----------------------------------------------------------------------
    const XMLBufferMgr* getBufMgr() const;
    XMLBufferMgr* getBufMgr();
    const ReaderMgr* getReaderMgr() const;
    ReaderMgr* getReaderMgr();
    const XMLScanner* getScanner() const;
    XMLScanner* getScanner();


private :
    // -----------------------------------------------------------------------
    //  Unimplemented Constructors and Operators
    // -----------------------------------------------------------------------
    XMLValidator(const XMLValidator&);
    XMLValidator& operator=(const XMLValidator&);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fErrorReporter
    //      The error reporter we are to use, if any.
    //
    // -----------------------------------------------------------------------
    XMLBufferMgr*       fBufMgr;
    XMLErrorReporter*   fErrorReporter;
    ReaderMgr*          fReaderMgr;
    XMLScanner*         fScanner;
};


// -----------------------------------------------------------------------
//  Setter methods
// -----------------------------------------------------------------------
inline void
XMLValidator::setScannerInfo(XMLScanner* const      owningScanner
                            , ReaderMgr* const      readerMgr
                            , XMLBufferMgr* const   bufMgr)
{
    // We don't own any of these, we just reference them
    fScanner = owningScanner;
    fReaderMgr = readerMgr;
    fBufMgr = bufMgr;
}

inline void
XMLValidator::setErrorReporter(XMLErrorReporter* const errorReporter)
{
    fErrorReporter = errorReporter;
}


// ---------------------------------------------------------------------------
//  XMLValidator: Protected getter
// ---------------------------------------------------------------------------
inline const XMLBufferMgr* XMLValidator::getBufMgr() const
{
    return fBufMgr;
}

inline XMLBufferMgr* XMLValidator::getBufMgr()
{
    return fBufMgr;
}

inline const ReaderMgr* XMLValidator::getReaderMgr() const
{
    return fReaderMgr;
}

inline ReaderMgr* XMLValidator::getReaderMgr()
{
    return fReaderMgr;
}

inline const XMLScanner* XMLValidator::getScanner() const
{
    return fScanner;
}

inline XMLScanner* XMLValidator::getScanner()
{
    return fScanner;
}

XERCES_CPP_NAMESPACE_END

#endif
