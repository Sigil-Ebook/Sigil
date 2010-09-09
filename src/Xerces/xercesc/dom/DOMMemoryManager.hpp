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

#if !defined(XERCESC_INCLUDE_GUARD_DOMMEMORYMANAGER_HPP)
#define XERCESC_INCLUDE_GUARD_DOMMEMORYMANAGER_HPP

//------------------------------------------------------------------------------------
//  Includes
//------------------------------------------------------------------------------------

XERCES_CPP_NAMESPACE_BEGIN

/**
  * The <code>DOMMemoryManager</code> interface exposes the memory allocation-related
  * functionalities of a <code>DOMDocument</code>
  */

class CDOM_EXPORT DOMMemoryManager
{
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMMemoryManager() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMMemoryManager(const DOMMemoryManager &);
    DOMMemoryManager & operator = (const DOMMemoryManager &);
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
    virtual ~DOMMemoryManager() {};
    //@}

    // -----------------------------------------------------------------------
    //  data types
    // -----------------------------------------------------------------------
    enum NodeObjectType {
        ATTR_OBJECT                   = 0,
        ATTR_NS_OBJECT                = 1,
        CDATA_SECTION_OBJECT          = 2,
        COMMENT_OBJECT                = 3,
        DOCUMENT_FRAGMENT_OBJECT      = 4,
        DOCUMENT_TYPE_OBJECT          = 5,
        ELEMENT_OBJECT                = 6,
        ELEMENT_NS_OBJECT             = 7,
        ENTITY_OBJECT                 = 8,
        ENTITY_REFERENCE_OBJECT       = 9,
        NOTATION_OBJECT               = 10,
        PROCESSING_INSTRUCTION_OBJECT = 11,
        TEXT_OBJECT                   = 12
    };

    //@{
    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    /**
     * Returns the size of the chunks of memory allocated by the memory manager
     *
     * @return the dimension of the chunks of memory allocated by the memory manager
     */
    virtual XMLSize_t getMemoryAllocationBlockSize() const = 0;

    //@}

    //@{
    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    /**
     * Set the size of the chunks of memory allocated by the memory manager
     *
     * @param size the new size of the chunks; it must be greater than 4KB
     */
    virtual void setMemoryAllocationBlockSize(XMLSize_t size) = 0;
    //@}

    //@{
    // -----------------------------------------------------------------------
    //  Operations
    // -----------------------------------------------------------------------
    /**
     * Allocate a memory block of the requested size from the managed pool
     *
     * @param amount the size of the new memory block
     *
     * @return the pointer to the newly allocated block
     */
    virtual void* allocate(XMLSize_t amount) = 0;

    /**
     * Allocate a memory block of the requested size from the managed pool of DOM objects
     *
     * @param amount the size of the new memory block
     * @param type   the type of the DOM object that will be stored in the block
     *
     * @return the pointer to the newly allocated block
     */
    virtual void* allocate(XMLSize_t amount, DOMMemoryManager::NodeObjectType type) = 0;

    /**
     * Release a DOM object and place its memory back in the pool
     *
     * @param object the pointer to the DOM node
     * @param type   the type of the DOM object 
     */
    virtual void release(DOMNode* object, DOMMemoryManager::NodeObjectType type) = 0;

    /**
     * Allocate a memory block from the mnaged pool and copy the provided string
     *
     * @param src the string to be copied
     *
     * @return the pointer to the newly allocated block
     */    
    virtual XMLCh* cloneString(const XMLCh *src) = 0;
    //@}

};

XERCES_CPP_NAMESPACE_END

#endif

/**
 * End of file DOMMemoryManager.hpp
 */
