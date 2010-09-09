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
 * $Id: DOMCDATASection.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMCDATASECTION_HPP)
#define XERCESC_INCLUDE_GUARD_DOMCDATASECTION_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMText.hpp>

XERCES_CPP_NAMESPACE_BEGIN


/**
 * CDATA sections are used to escape blocks of text containing characters that
 * would otherwise be regarded as markup. The only delimiter that is
 * recognized in a CDATA section is the "]]&gt;" string that ends the CDATA
 * section. CDATA sections cannot be nested. Their primary purpose is for
 * including material such as XML fragments, without needing to escape all
 * the delimiters.
 * <p>The <code>data</code> attribute of the <code>DOMText</code> node holds
 * the text that is contained by the CDATA section. Note that this may
 * contain characters that need to be escaped outside of CDATA sections and
 * that, depending on the character encoding ("charset") chosen for
 * serialization, it may be impossible to write out some characters as part
 * of a CDATA section.
 * <p>The <code>DOMCDATASection</code> interface inherits from the
 * <code>DOMCharacterData</code> interface through the <code>DOMText</code>
 * interface. Adjacent <code>DOMCDATASection</code> nodes are not merged by use
 * of the <code>normalize</code> method of the <code>DOMNode</code> interface.
 * Because no markup is recognized within a <code>DOMCDATASection</code>,
 * character numeric references cannot be used as an escape mechanism when
 * serializing. Therefore, action needs to be taken when serializing a
 * <code>DOMCDATASection</code> with a character encoding where some of the
 * contained characters cannot be represented. Failure to do so would not
 * produce well-formed XML.One potential solution in the serialization
 * process is to end the CDATA section before the character, output the
 * character using a character reference or entity reference, and open a new
 * CDATA section for any further characters in the text node. Note, however,
 * that some code conversion libraries at the time of writing do not return
 * an error or exception when a character is missing from the encoding,
 * making the task of ensuring that data is not corrupted on serialization
 * more difficult.
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Core-20001113'>Document Object Model (DOM) Level 2 Core Specification</a>.
 *
 * @since DOM Level 1
 */
class CDOM_EXPORT DOMCDATASection: public DOMText {
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{    
    DOMCDATASection() {}
    DOMCDATASection(const DOMCDATASection &other) : DOMText(other) {}
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented operators */
    //@{
    DOMCDATASection & operator = (const DOMCDATASection &);
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
    virtual ~DOMCDATASection() {};
    //@}

};

XERCES_CPP_NAMESPACE_END

#endif


