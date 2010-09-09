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
 * $Id: SecurityManager.hpp 673960 2008-07-04 08:50:12Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_SECURITYMANAGER_HPP)
#define XERCESC_INCLUDE_GUARD_SECURITYMANAGER_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
  * Allow application to force the parser to behave in a security-conscious
  * way.
  *
  * <p> There are cases in which an XML- or XmL-schema-
  * conformant processor can be presented with documents the
  * processing of which can involve the consumption of
  * prohibitive amounts of system resources.  Applications can
  * attach instances of this class to parsers that they've
  * created, via the
  * http://apache.org/xml/properties/security-manager property.
  * </p>
  *
  * <p> Defaults will be provided for all known security holes.
  * Setter methods will be provided on this class to ensure that
  * an application can customize each limit as it chooses.
  * Components that are vulnerable to any given hole need to be
  * written to act appropriately when an instance of this class
  * has been set on the calling parser.
  * </p>
  */

class XMLUTIL_EXPORT SecurityManager
{
public:

    enum { ENTITY_EXPANSION_LIMIT = 50000};

    /** @name default Constructors */
    //@{
    /** Default constructor */
    SecurityManager()
        : fEntityExpansionLimit((XMLSize_t)ENTITY_EXPANSION_LIMIT)
    {
    }

    /** Destructor */
    virtual ~SecurityManager(){};
    //@}

    /** @name The Security Manager */
    //@{
   /**
    * An application should call this method when it wishes to specify a particular
    * limit to the number of entity expansions the parser will permit in a
    * particular document.  The default behaviour should allow the parser
    * to validate nearly all XML non-malicious XML documents; if an
    * application knows that it is operating in a domain where entities are
    * uncommon, for instance, it may wish to provide a limit lower than the
    * parser's default.
    *
    * @param newLimit  the new entity expansion limit
    *
    */
    virtual void setEntityExpansionLimit(XMLSize_t newLimit)
    {
        fEntityExpansionLimit = newLimit;
    }

   /**
    * Permits the application or a parser component to query the current
    * limit for entity expansions.
    *
    * @return   the current setting of the entity expansion limit
    *
    */
    virtual XMLSize_t getEntityExpansionLimit() const
    {
        return fEntityExpansionLimit;
    }
    //@}

protected:
    XMLSize_t fEntityExpansionLimit;

private:

    /* Unimplemented Constructors and operators */
    /* Copy constructor */
    SecurityManager(const SecurityManager&);

    /** Assignment operator */
    SecurityManager& operator=(const SecurityManager&);
};

XERCES_CPP_NAMESPACE_END

#endif
