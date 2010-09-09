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
 * $Id: DOMDocumentRange.hpp 527149 2007-04-10 14:56:39Z amassari $
*/

#if !defined(XERCESC_INCLUDE_GUARD_DOMDOCUMENTRANGE_HPP)
#define XERCESC_INCLUDE_GUARD_DOMDOCUMENTRANGE_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class DOMRange;


/**
 * <p>See also the <a href='http://www.w3.org/TR/2000/REC-DOM-Level-2-Traversal-Range-20001113'>Document Object Model (DOM) Level 2 Traversal and Range Specification</a>.
 * @since DOM Level 2
 */
class CDOM_EXPORT DOMDocumentRange {

protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{    
    DOMDocumentRange() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMDocumentRange(const DOMDocumentRange &);
    DOMDocumentRange & operator = (const DOMDocumentRange &);
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
    virtual ~DOMDocumentRange() {};
    //@}

    // -----------------------------------------------------------------------
    //  Virtual DOMDocumentRange interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 2 */
    //@{
    /**
	  * To create the range  consisting of boundary-points and offset of the
     * selected contents
     *
     * @return The initial state of the Range such that both the boundary-points
     * are positioned at the beginning of the corresponding DOMDOcument, before
     * any content. The range returned can only be used to select content
     * associated with this document, or with documentFragments and Attrs for
     * which this document is the ownerdocument
     * @since DOM Level 2
	  */
    virtual DOMRange    *createRange() = 0;

    //@}
};


XERCES_CPP_NAMESPACE_END

#endif
