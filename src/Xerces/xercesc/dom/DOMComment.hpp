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
 * $Id: DOMComment.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMCOMMENT_HPP)
#define XERCESC_INCLUDE_GUARD_DOMCOMMENT_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMCharacterData.hpp>

XERCES_CPP_NAMESPACE_BEGIN


/**
 * This interface inherits from <code>CharacterData</code> and represents the
 * content of a comment, i.e., all the characters between the starting '
 * <code>&lt;!--</code>' and ending '<code>--&gt;</code>'.
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Core-20001113'>Document Object Model (DOM) Level 2 Core Specification</a>.
 *
 * @since DOM Level 1
 */
class CDOM_EXPORT DOMComment: public DOMCharacterData {
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{    
    DOMComment() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMComment(const DOMComment &);
    DOMComment & operator = (const DOMComment &);
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
    virtual ~DOMComment() {};
    //@}
};

XERCES_CPP_NAMESPACE_END

#endif

