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
 * $Id: DOMErrorImpl.hpp 676853 2008-07-15 09:58:05Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMERRORIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_DOMERRORIMPL_HPP

#include <xercesc/dom/DOMError.hpp>
#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_BEGIN


/**
  * Introduced in DOM Level 3
  * Implementation of a DOMError interface.
  *
  * @see DOMError#DOMError
  */

class CDOM_EXPORT DOMErrorImpl : public DOMError
{
public:
    /** @name Constructors and Destructor */
    //@{

    /** Constructors */
    DOMErrorImpl(const ErrorSeverity severity);

    DOMErrorImpl
    (
        const ErrorSeverity severity
        , const XMLCh* const message
        , DOMLocator* const location
    );

    DOMErrorImpl
    (
        const ErrorSeverity severity
        , const XMLCh* type
        , const XMLCh* message
        , void* relatedData
    );

    /** Desctructor */
    virtual ~DOMErrorImpl();

    //@}

    // DOMError interface
    virtual ErrorSeverity getSeverity() const;
    virtual const XMLCh* getMessage() const;
    virtual DOMLocator* getLocation() const;
    virtual void* getRelatedException() const;
    virtual const XMLCh* getType() const;
    virtual void* getRelatedData() const;

    // Setters
    void setSeverity(const ErrorSeverity severity);
    void setMessage(const XMLCh* const message);
    void setLocation(DOMLocator* const location);
    void setAdoptLocation(const bool value);
    void setRelatedException(void* exc) const;
    void setType(const XMLCh* type);
    void setRelatedData(void* relatedData);

private:
    /* Unimplemented constructors and operators */

    /* Copy constructor */
    DOMErrorImpl(const DOMErrorImpl&);

    /* Assignment operator */
    DOMErrorImpl& operator=(const DOMErrorImpl&);

protected:
    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fAdoptLocation
    //      Indicates whether we own the DOMLocator object or not.
    //
    //  fSeverity
    //      The type of the error.
    //
    //  fMessage
    //      The error message.
    //
    //  fLocation
    //      The location info of the error.
    //
    //  fType
    //      The type of the error.
    //
    //  fRelatedData
    //      The data related to this error.
    //
    // -----------------------------------------------------------------------
    bool          fAdoptLocation;
    ErrorSeverity fSeverity;
    const XMLCh*  fMessage;
    DOMLocator*   fLocation;
    const XMLCh*  fType;
    void*         fRelatedData;
};

// ---------------------------------------------------------------------------
//  DOMErrorImpl: Getter methods
// ---------------------------------------------------------------------------
inline DOMError::ErrorSeverity DOMErrorImpl::getSeverity() const
{
    return fSeverity;
}

inline const XMLCh* DOMErrorImpl::getMessage() const
{
    return fMessage;
}

inline DOMLocator* DOMErrorImpl::getLocation() const
{
    return fLocation;
}

inline void* DOMErrorImpl::getRelatedException() const
{
    return 0;
}

inline const XMLCh* DOMErrorImpl::getType() const
{
    return fType;
}

inline void* DOMErrorImpl::getRelatedData() const
{
    return fRelatedData;
}

// ---------------------------------------------------------------------------
//  DOMErrorImpl: Setter methods
// ---------------------------------------------------------------------------
inline void DOMErrorImpl::setSeverity(const ErrorSeverity severity)
{
    fSeverity = severity;
}

inline void DOMErrorImpl::setMessage(const XMLCh* const message)
{
    fMessage = message;
}

inline void DOMErrorImpl::setAdoptLocation(const bool value)
{
    fAdoptLocation = value;
}

inline void DOMErrorImpl::setType(const XMLCh* type)
{
    fType = type;
}

inline void DOMErrorImpl::setRelatedData(void* relatedData)
{
    fRelatedData = relatedData;
}


XERCES_CPP_NAMESPACE_END

#endif
