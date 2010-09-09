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
 * $Id: DOMXPathNSResolver.hpp 698579 2008-09-24 14:13:08Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMXPATHNSRESOLVER_HPP)
#define XERCESC_INCLUDE_GUARD_DOMXPATHNSRESOLVER_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN
/**
 * The <code>DOMXPathNSResolver</code> interface permit prefix strings
 * in the expression to be properly bound to namespaceURI strings.
 * <code>DOMXPathEvaluator</code> can construct an implementation of
 * <code>DOMXPathNSResolver</code> from a node, or the interface may be
 * implemented by any application.
 * @since DOM Level 3
 */
class CDOM_EXPORT DOMXPathNSResolver
{

protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMXPathNSResolver() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMXPathNSResolver(const DOMXPathNSResolver &);
    DOMXPathNSResolver& operator = (const  DOMXPathNSResolver&);
    //@}

public:
    // -----------------------------------------------------------------------
    //  All constructors are hidden, just the destructor is available
    // -----------------------------------------------------------------------
    /** @name Destructor */
    //@{
    /**
     * Destructor
     *
     */
    virtual ~DOMXPathNSResolver() {};
    //@}

    // -----------------------------------------------------------------------
    // Virtual DOMDocument interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 3 */
    //@{

    /** Look up the namespace URI associated to the given namespace prefix.
     *
     * @param prefix of type XMLCh - The prefix to look for. An empty or
     * null string denotes the default namespace.
     * @return the associated namespace URI or null if none is found.
     */
    virtual const XMLCh*          lookupNamespaceURI(const XMLCh* prefix) const = 0;
    //@}


    // -----------------------------------------------------------------------
    // Non-standard extension
    // -----------------------------------------------------------------------
    /** @name Non-standard extension */
    //@{

    /**
     * Non-standard extension
     *
     * XPath2 implementations require a reverse lookup in the static context.
     * Look up the prefix associated with the namespace URI
     * @param URI of type XMLCh - The namespace to look for.
     * @return the associated prefix which can be an empty string if this
     * is a default namespace or null if none is found.
     */
    virtual const XMLCh*          lookupPrefix(const XMLCh* URI) const = 0;

    /**
     * Non-standard extension
     *
     * Associate the given namespace prefix to the namespace URI.
     * @param prefix of type XMLCh - The namespace prefix to bind. An empty
     * or null string denotes the default namespace.
     * @param uri of type XMLCh - The associated namespace URI. If this
     * argument is null or an empty string then the existing binding for this
     * prefix is removed.
     */
    virtual void addNamespaceBinding(const XMLCh* prefix, const XMLCh* uri) = 0;

    /**
     * Called to indicate that this object (and its associated children) is no longer in use
     * and that the implementation may relinquish any resources associated with it and
     * its associated children.
     *
     * Access to a released object will lead to unexpected result.
     */
    virtual void release() = 0;

    //@}
};

XERCES_CPP_NAMESPACE_END

#endif
