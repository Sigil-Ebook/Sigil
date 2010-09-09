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
 * $Id: DOMLSSerializerImpl.hpp 695856 2008-09-16 12:52:32Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMLSSERIALIZERMPL_HPP)
#define XERCESC_INCLUDE_GUARD_DOMLSSERIALIZERMPL_HPP

#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMLSSerializer.hpp>
#include <xercesc/util/XMLDOMMsg.hpp>
#include <xercesc/util/RefHashTableOf.hpp>
#include <xercesc/util/RefVectorOf.hpp>
#include <xercesc/framework/XMLFormatter.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class DOMStringListImpl;

class CDOM_EXPORT DOMLSSerializerImpl : public XMemory,
                                        public DOMLSSerializer,
                                        public DOMConfiguration
{

public:

    /** @name Constructor and Destructor */
    //@{

    /**
     * Constructor.
     */
    DOMLSSerializerImpl(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    /**
     * Destructor.
     */
    ~DOMLSSerializerImpl();
    //@}

    /** @name Implementation of DOMLSSerializer interface */
    //@{

    virtual DOMConfiguration*       getDomConfig();

    virtual void                    setNewLine(const XMLCh* const newLine);
    virtual const XMLCh*            getNewLine() const;

    virtual void                    setFilter(DOMLSSerializerFilter *filter);
    virtual DOMLSSerializerFilter*  getFilter() const;

    virtual bool                    write(const DOMNode*        nodeToWrite,
                                          DOMLSOutput* const    destination);
    virtual bool                    writeToURI(const DOMNode*   nodeToWrite,
                                               const XMLCh*     uri);
    /**
	  *  The caller is responsible for the release of the returned string
	  */
    virtual XMLCh*                  writeToString(const DOMNode* nodeToWrite, MemoryManager* manager = NULL);
    virtual void                    release();

    //@}

    /** @name Implementation of DOMConfiguration interface */
    //@{
    virtual void setParameter(const XMLCh* name, const void* value);
    virtual void setParameter(const XMLCh* name, bool value);
    virtual const void* getParameter(const XMLCh* name) const;
    virtual bool canSetParameter(const XMLCh* name, const void* value) const;
    virtual bool canSetParameter(const XMLCh* name, bool value) const;
    virtual const DOMStringList* getParameterNames() const;
    //@}

private:

    /** unimplemented copy ctor and assignment operator */
    DOMLSSerializerImpl(const DOMLSSerializerImpl&);
    DOMLSSerializerImpl & operator = (const DOMLSSerializerImpl&);

protected:
    /** helper **/
    void                          processNode(const DOMNode* const);

    void                          procCdataSection(const XMLCh*   const nodeValue
                                                 , const DOMNode* const nodeToWrite);

    void                          procUnrepCharInCdataSection(const XMLCh*   const nodeValue
                                                            , const DOMNode* const nodeToWrite);

protected:
    /**
     * Overidden by derived classes to extend the abilities of the standard writer
     * always returns false in the default implementation
     * @return true if the method deals with nodeToWrite
     */
    virtual bool                          customNodeSerialize(const DOMNode* const nodeToWrite, int level);

    DOMNodeFilter::FilterAction   checkFilter(const DOMNode* const) const;

    bool                          checkFeature(const XMLCh* const featName
                                             , bool               state
                                             , int&               featureId) const;

    bool                          reportError(const DOMNode* const    errorNode
                                            , DOMError::ErrorSeverity errorType
                                            , const XMLCh*   const    errorMsg);
    bool                          reportError(const DOMNode* const    errorNode
                                            , DOMError::ErrorSeverity errorType
                                            , XMLDOMMsg::Codes        toEmit);

    bool                          canSetFeature(const int featureId
                                              , bool      val)     const;
    void                          setFeature(const int featureId
                                           , bool      val);
    bool                          getFeature(const int featureId) const;

    void                          printNewLine();
    void                          setURCharRef();
    bool                          isDefaultNamespacePrefixDeclared() const;
    bool                          isNamespaceBindingActive(const XMLCh* prefix, const XMLCh* uri) const;


    void printIndent(unsigned int level);
    //does the actual work for processNode while keeping track of the level
    void processNode(const DOMNode* const nodeToWrite, int level);

    void processBOM();

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fFeatures
    //
    //  fNewLine
    //      own it
    //
    //  fErrorHandler
    //      don't own it
    //
    //  fFilter
    //      don't own it
    //
    //  fDocumentVersion
    //      The XML Version of the document to be serialized.
    //
    //  fSupportedParameters
    //      A list of the parameters that can be set, including the ones
    //      specific of Xerces
    //
    //  fEncodingUsed (session var)
    //      the actual encoding used in write(),
    //      it does not own any data(memory).
    //
    //  fNewLineUsed (session var)
    //      the actual "end of line" sequence used in write(),
    //      it does not own any data(memory).
    //
    //  fFormatter (session var)
    //      the formatter used in write()
    //
    //  fErrorCount
    //      the count of error encountered in the serialization,
    //      which neither the error handler, nor the serializer itself,
    //      treat as fatal. And the serializer will return true/false
    //      based on this value.
    //
    //  fCurrentLine
    //      the current line. Used to track the line number the current
    //      node begins on
    //
    // -----------------------------------------------------------------------

    int                           fFeatures;
    XMLCh                        *fNewLine;
    DOMErrorHandler              *fErrorHandler;
    DOMLSSerializerFilter        *fFilter;
    const XMLCh                  *fDocumentVersion;
    DOMStringListImpl            *fSupportedParameters;

    //session vars
    const XMLCh                  *fEncodingUsed;
    const XMLCh                  *fNewLineUsed;
    XMLFormatter                 *fFormatter;
    int                           fErrorCount;
    int                           fCurrentLine;
    bool                          fLineFeedInTextNodePrinted;
    unsigned int                  fLastWhiteSpaceInTextNode;

    RefVectorOf< RefHashTableOf<XMLCh> >* fNamespaceStack;
    MemoryManager*               fMemoryManager;
};

inline DOMConfiguration* DOMLSSerializerImpl::getDomConfig()
{
    return this;
}

inline void DOMLSSerializerImpl::setFeature(const int featureId
                                    , bool      val)
{
    (val)? fFeatures |= (1<<featureId) : fFeatures &= ~(1<<featureId);
}

inline bool DOMLSSerializerImpl::getFeature(const int featureId) const
{
    return ((fFeatures & ( 1<<featureId )) != 0) ? true : false;
}

inline void DOMLSSerializerImpl::setURCharRef()
{
    fFormatter->setUnRepFlags(XMLFormatter::UnRep_CharRef);
}

XERCES_CPP_NAMESPACE_END

#endif
