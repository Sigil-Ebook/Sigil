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
 * $Id: ContentSpecNode.cpp 933155 2010-04-12 09:07:02Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/validators/common/ContentSpecNode.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  ContentSpecNode: Copy Constructor
//
//  Note: this copy constructor has dependency on various get*() methods
//        and shall be placed after those method's declaration.
//        aka inline function compilation error on AIX 4.2, xlC 3 r ev.1
// ---------------------------------------------------------------------------

ContentSpecNode::ContentSpecNode(const ContentSpecNode& toCopy) :
    XSerializable(toCopy)
    , XMemory(toCopy)
    , fMemoryManager(toCopy.fMemoryManager)
    , fElement(0)
    , fElementDecl(toCopy.fElementDecl)
    , fFirst(0)
    , fSecond(0)
    , fType(toCopy.fType)
    , fAdoptFirst(true)
    , fAdoptSecond(true)
    , fMinOccurs(toCopy.fMinOccurs)
    , fMaxOccurs(toCopy.fMaxOccurs)
{
    const QName* tempElement = toCopy.getElement();
    if (tempElement)
        fElement = new (fMemoryManager) QName(*tempElement);

    const ContentSpecNode *tmp = toCopy.getFirst();
    if (tmp)
        fFirst = new (fMemoryManager) ContentSpecNode(*tmp);

    tmp = toCopy.getSecond();
    if (tmp)
        fSecond = new (fMemoryManager) ContentSpecNode(*tmp);
}

// ---------------------------------------------------------------------------
//  Local methods
// ---------------------------------------------------------------------------
static void formatNode( const   ContentSpecNode* const      curNode
                        , const ContentSpecNode::NodeTypes  parentType
                        ,       XMLBuffer&                  bufToFill)
{
    if (!curNode)
        return;

    const ContentSpecNode* first = curNode->getFirst();
    const ContentSpecNode* second = curNode->getSecond();
    const ContentSpecNode::NodeTypes curType = curNode->getType();

    // Get the type of the first node
    const ContentSpecNode::NodeTypes firstType = first ?
                                                 first->getType() :
                                                 ContentSpecNode::Leaf;

    // Calculate the parens flag for the rep nodes
    bool doRepParens = false;
    if (((firstType != ContentSpecNode::Leaf)
            && (parentType != ContentSpecNode::UnknownType))
    ||  ((firstType == ContentSpecNode::Leaf)
            && (parentType == ContentSpecNode::UnknownType)))
    {
        doRepParens = true;
    }

    // Now handle our type
    switch(curType & 0x0f)
    {
        case ContentSpecNode::Leaf :
            if (curNode->getElement()->getURI() == XMLElementDecl::fgPCDataElemId)
                bufToFill.append(XMLElementDecl::fgPCDataElemName);
            else
            {
                bufToFill.append(curNode->getElement()->getRawName());
                // show the + and * modifiers also when we have a non-infinite number of repetitions
                if(curNode->getMinOccurs()==0 && (curNode->getMaxOccurs()==-1 || curNode->getMaxOccurs()>1))
                    bufToFill.append(chAsterisk);
                else if(curNode->getMinOccurs()==0 && curNode->getMaxOccurs()==1)
                    bufToFill.append(chQuestion);
                else if(curNode->getMinOccurs()==1 && (curNode->getMaxOccurs()==-1 || curNode->getMaxOccurs()>1))
                    bufToFill.append(chPlus);
            }
            break;

        case ContentSpecNode::ZeroOrOne :
            if (doRepParens)
                bufToFill.append(chOpenParen);
            formatNode(first, curType, bufToFill);
            if (doRepParens)
                bufToFill.append(chCloseParen);
            bufToFill.append(chQuestion);
            break;

        case ContentSpecNode::ZeroOrMore :
            if (doRepParens)
                bufToFill.append(chOpenParen);
            formatNode(first, curType, bufToFill);
            if (doRepParens)
                bufToFill.append(chCloseParen);
            bufToFill.append(chAsterisk);
            break;

        case ContentSpecNode::OneOrMore :
            if (doRepParens)
                bufToFill.append(chOpenParen);
            formatNode(first, curType, bufToFill);
            if (doRepParens)
                bufToFill.append(chCloseParen);
            bufToFill.append(chPlus);
            break;

        case ContentSpecNode::Choice :
            if ((parentType & 0x0f) != (curType & 0x0f))
                bufToFill.append(chOpenParen);
            formatNode(first, curType, bufToFill);
            if(second!=NULL)
            {
                bufToFill.append(chPipe);
                formatNode(second, curType, bufToFill);
            }
            if ((parentType & 0x0f) != (curType & 0x0f))
                bufToFill.append(chCloseParen);
            break;

        case ContentSpecNode::Sequence :
            if ((parentType & 0x0f) != (curType & 0x0f))
                bufToFill.append(chOpenParen);
            formatNode(first, curType, bufToFill);
            if(second!=NULL)
            {
                bufToFill.append(chComma);
                formatNode(second, curType, bufToFill);
            }
            if ((parentType & 0x0f) != (curType & 0x0f))
                bufToFill.append(chCloseParen);
            break;

        case ContentSpecNode::All :
            if ((parentType & 0x0f) != (curType & 0x0f))
			{
                bufToFill.append(chLatin_A);
                bufToFill.append(chLatin_l);
                bufToFill.append(chLatin_l);
                bufToFill.append(chOpenParen);
			}
            formatNode(first, curType, bufToFill);
            bufToFill.append(chComma);
            formatNode(second, curType, bufToFill);
            if ((parentType & 0x0f) != (curType & 0x0f))
                bufToFill.append(chCloseParen);
            break;
    }
}


// ---------------------------------------------------------------------------
//  ContentSpecNode: Miscellaneous
// ---------------------------------------------------------------------------
void ContentSpecNode::formatSpec(XMLBuffer&      bufToFill) const
{
    // Clean out the buffer first
    bufToFill.reset();

    if (fType == ContentSpecNode::Leaf)
        bufToFill.append(chOpenParen);
    formatNode
    (
        this
        , UnknownType
        , bufToFill
    );
    if (fType == ContentSpecNode::Leaf)
        bufToFill.append(chCloseParen);
}

int ContentSpecNode::getMinTotalRange() const {

    int min = fMinOccurs;

    if ((fType & 0x0f) == ContentSpecNode::Sequence
        || fType == ContentSpecNode::All
        || (fType & 0x0f) == ContentSpecNode::Choice) {

        int minFirst = fFirst->getMinTotalRange();

        if (fSecond) {

            int minSecond = fSecond->getMinTotalRange();

            if ((fType & 0x0f) == ContentSpecNode::Choice) {
                min = min * ((minFirst < minSecond)? minFirst : minSecond);
            }
            else {
                min = min * (minFirst + minSecond);
            }
        }
        else
            min = min * minFirst;
    }

    return min;
}

int ContentSpecNode::getMaxTotalRange() const {

    int max = fMaxOccurs;

    if (max == SchemaSymbols::XSD_UNBOUNDED) {
         return SchemaSymbols::XSD_UNBOUNDED;
    }

    if ((fType & 0x0f) == ContentSpecNode::Sequence
        || fType == ContentSpecNode::All
        || (fType & 0x0f) == ContentSpecNode::Choice) {

        int maxFirst = fFirst->getMaxTotalRange();

        if (maxFirst == SchemaSymbols::XSD_UNBOUNDED) {
             return SchemaSymbols::XSD_UNBOUNDED;
        }

        if (fSecond) {

            int maxSecond = fSecond->getMaxTotalRange();

            if (maxSecond == SchemaSymbols::XSD_UNBOUNDED) {
                return SchemaSymbols::XSD_UNBOUNDED;
            }
            else {

                if ((fType & 0x0f) == ContentSpecNode::Choice) {
                    max = max * (maxFirst > maxSecond) ? maxFirst : maxSecond;
                }
                else {
                    max = max * (maxFirst + maxSecond);
                }
            }
        }
        else {
            max = max * maxFirst;
        }
    }

    return max;
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(ContentSpecNode)

void ContentSpecNode::serialize(XSerializeEngine& serEng)
{
    /***
     *  Since fElement, fFirst, fSecond are NOT created by the default 
     *  constructor, we need to create them dynamically.
     ***/

    if (serEng.isStoring())
    {
        serEng<<fElement;
        XMLElementDecl::storeElementDecl(serEng, fElementDecl);
        serEng<<fFirst;
        serEng<<fSecond;

        serEng<<(int)fType;
        serEng<<fAdoptFirst;
        serEng<<fAdoptSecond;
        serEng<<fMinOccurs;
        serEng<<fMaxOccurs;
    }
    else
    {
        serEng>>fElement;
        fElementDecl = XMLElementDecl::loadElementDecl(serEng);
        serEng>>fFirst;
        serEng>>fSecond;

        int type;
        serEng>>type;
        fType = (NodeTypes)type;

        serEng>>fAdoptFirst;
        serEng>>fAdoptSecond;
        serEng>>fMinOccurs;
        serEng>>fMaxOccurs;
    }

}

XERCES_CPP_NAMESPACE_END

