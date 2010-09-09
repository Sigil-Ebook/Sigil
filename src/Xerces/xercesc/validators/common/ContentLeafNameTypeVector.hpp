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
 * $Id: ContentLeafNameTypeVector.hpp 676911 2008-07-15 13:27:32Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_CONTENTLEAFNAMETYPEVECTOR_HPP)
#define XERCESC_INCLUDE_GUARD_CONTENTLEAFNAMETYPEVECTOR_HPP

#include <xercesc/validators/common/ContentSpecNode.hpp>
#include <xercesc/framework/MemoryManager.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLPARSER_EXPORT ContentLeafNameTypeVector : public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Class specific types
    // -----------------------------------------------------------------------


    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    ContentLeafNameTypeVector
    (
        MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    ContentLeafNameTypeVector
    (
        QName** const                     qName
      , ContentSpecNode::NodeTypes* const types
      , const XMLSize_t                   count
      , MemoryManager* const              manager = XMLPlatformUtils::fgMemoryManager
    );

    ~ContentLeafNameTypeVector();

    ContentLeafNameTypeVector(const ContentLeafNameTypeVector&);

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    QName* getLeafNameAt(const XMLSize_t pos) const;

    ContentSpecNode::NodeTypes getLeafTypeAt(const XMLSize_t pos) const;
    XMLSize_t getLeafCount() const;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void setValues
    (
        QName** const                      qName
      , ContentSpecNode::NodeTypes* const  types
      , const XMLSize_t                    count
    );

    // -----------------------------------------------------------------------
    //  Miscellaneous
    // -----------------------------------------------------------------------

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    ContentLeafNameTypeVector& operator=(const ContentLeafNameTypeVector&);

    // -----------------------------------------------------------------------
    //  helper methods
    // -----------------------------------------------------------------------
    void cleanUp();
    void init(const XMLSize_t size);

    // -----------------------------------------------------------------------
    //  Private Data Members
    //
    // -----------------------------------------------------------------------
    MemoryManager*                fMemoryManager;
    QName**                       fLeafNames;
    ContentSpecNode::NodeTypes   *fLeafTypes;
    XMLSize_t                     fLeafCount;
};

inline void ContentLeafNameTypeVector::cleanUp()
{
	fMemoryManager->deallocate(fLeafNames); //delete [] fLeafNames;
	fMemoryManager->deallocate(fLeafTypes); //delete [] fLeafTypes;
}

inline void ContentLeafNameTypeVector::init(const XMLSize_t size)
{
    fLeafNames = (QName**) fMemoryManager->allocate(size * sizeof(QName*));//new QName*[size];
    fLeafTypes = (ContentSpecNode::NodeTypes *) fMemoryManager->allocate
    (
        size * sizeof(ContentSpecNode::NodeTypes)
    ); //new ContentSpecNode::NodeTypes [size];
    fLeafCount = size;
}

XERCES_CPP_NAMESPACE_END

#endif
