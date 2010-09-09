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
 * $Id: Match.hpp 678879 2008-07-22 20:05:05Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_MATCH_HPP)
#define XERCESC_INCLUDE_GUARD_MATCH_HPP

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/ArrayIndexOutOfBoundsException.hpp>
#include <xercesc/util/RuntimeException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
  * An instance of this class has ranges captured in matching
  */
  class XMLUTIL_EXPORT Match : public XMemory
{
public:

    // -----------------------------------------------------------------------
    //  Public Constructors and Destructor
    // -----------------------------------------------------------------------
    Match(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    
    /**
    * Copy constructor
    */
    Match(const Match& toCopy);
    Match& operator=(const Match& toAssign);

    virtual ~Match();

    // -----------------------------------------------------------------------
    // Getter functions
    // -----------------------------------------------------------------------
    int getNoGroups() const;
    int getStartPos(int index) const;
    int getEndPos(int index) const;

    // -----------------------------------------------------------------------
    // Setter functions
    // -----------------------------------------------------------------------
    void setNoGroups(const int n);
    void setStartPos(const int index, const int value);
    void setEndPos(const int index, const int value);

private:
    // -----------------------------------------------------------------------
    // Initialize/Clean up methods
    // -----------------------------------------------------------------------
    void initialize(const Match& toCopy);
    void cleanUp();

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fNoGroups
    //  Represents no of regular expression groups
    //        
    //  fStartPositions
    //  Array of start positions in the target text matched to specific
    //        regular expression group
    //
    //    fEndPositions
    //        Array of end positions in the target text matched to specific
    //        regular expression group
    //
    //    fPositionsSize
    //        Actual size of Start/EndPositions array.
    // -----------------------------------------------------------------------
    int fNoGroups;
    int fPositionsSize;
    int* fStartPositions;
    int* fEndPositions;
    MemoryManager* fMemoryManager;
};

/**
  * Inline Methods
  */

// ---------------------------------------------------------------------------
//  Match: getter methods
// ---------------------------------------------------------------------------
inline int Match::getNoGroups() const {

    if (fNoGroups < 0)
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_Result_Not_Set, fMemoryManager);

    return fNoGroups;
}

inline int Match::getStartPos(int index) const {

    if (!fStartPositions)
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_Result_Not_Set, fMemoryManager);

    if (index < 0 || fNoGroups <= index)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Array_BadIndex, fMemoryManager);

    return fStartPositions[index];
}

inline int Match::getEndPos(int index) const {

    if (!fEndPositions)
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_Result_Not_Set, fMemoryManager);

    if (index < 0 || fNoGroups <= index)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Array_BadIndex, fMemoryManager);

    return fEndPositions[index];
}

// ---------------------------------------------------------------------------
//  Match: setter methods
// ---------------------------------------------------------------------------
inline void Match::setStartPos(const int index, const int value) {

    if (!fStartPositions)
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_Result_Not_Set, fMemoryManager);

    if (index < 0 || fNoGroups <= index)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Array_BadIndex, fMemoryManager);

    fStartPositions[index] = value;
}

inline void Match::setEndPos(const int index, const int value) {

    if (!fEndPositions)
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_Result_Not_Set, fMemoryManager);

    if (index < 0 || fNoGroups <= index)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Array_BadIndex, fMemoryManager);

    fEndPositions[index] = value;
}

XERCES_CPP_NAMESPACE_END

#endif
