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
 * $Id: Match.cpp 678879 2008-07-22 20:05:05Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/regx/Match.hpp>
#include <xercesc/framework/MemoryManager.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Match: Constructors and Destructors
// ---------------------------------------------------------------------------
Match::Match(MemoryManager* const manager) :
    fNoGroups(0)
    , fPositionsSize(0)
    , fStartPositions(0)
    , fEndPositions(0)
    , fMemoryManager(manager)
{

}

Match::Match(const Match& toCopy) :
    XMemory(toCopy) 
    , fNoGroups(0)
    , fPositionsSize(0)
    , fStartPositions(0)
    , fEndPositions(0)
    , fMemoryManager(0)
{
  initialize(toCopy);
}

Match& Match::operator=(const Match& toAssign) {
  
  initialize(toAssign);
  return *this;
}


Match::~Match() {

    cleanUp();
}

// ---------------------------------------------------------------------------
//  Match: Setter Methods
// ---------------------------------------------------------------------------
void Match::setNoGroups(const int n) {

    if (fNoGroups <= 0 || fPositionsSize < n) {

        cleanUp();
        fPositionsSize = n;
        fStartPositions = (int*) fMemoryManager->allocate(n * sizeof(int));//new int[n];
        fEndPositions = (int*) fMemoryManager->allocate(n * sizeof(int));//new int[n];
    }

    fNoGroups = n;

    for (int i=0; i< fPositionsSize; i++) {

        fStartPositions[i] = -1;
        fEndPositions[i] = -1;
    }
}

// ---------------------------------------------------------------------------
//  Match: private helpers methods
// ---------------------------------------------------------------------------
void Match::initialize(const Match &toCopy){

  //do not copy over value of fPositionSize as it is irrelevant to the 
  //state of the Match

  fMemoryManager = toCopy.fMemoryManager;
  int toCopySize = toCopy.getNoGroups();
  setNoGroups(toCopySize);

  for (int i=0; i<toCopySize; i++){
    setStartPos(i, toCopy.getStartPos(i));
    setEndPos(i, toCopy.getEndPos(i));
  }           

}

void Match::cleanUp() {

    fMemoryManager->deallocate(fStartPositions);//delete [] fStartPositions;
    fMemoryManager->deallocate(fEndPositions);//delete [] fEndPositions;

    fStartPositions = 0;
    fEndPositions = 0;
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file Match.cpp
  */
