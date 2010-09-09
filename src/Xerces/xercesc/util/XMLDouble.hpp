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
 * $Id: XMLDouble.hpp 673155 2008-07-01 17:55:39Z dbertoni $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XML_DOUBLE_HPP)
#define XERCESC_INCLUDE_GUARD_XML_DOUBLE_HPP

#include <xercesc/util/XMLAbstractDoubleFloat.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLUTIL_EXPORT XMLDouble : public XMLAbstractDoubleFloat
{
public:

	/**
	 * Constructs a newly allocated <code>XMLDouble</code> object that
	 * represents the value represented by the string.
	 *
	 * @param      strValue the <code>String</code> to be converted to an
	 *                 <code>XMLDouble</code>.
         * @param manager    Pointer to the memory manager to be used to
         *                   allocate objects.
	 * @exception  NumberFormatException  if the <code>String</code> does not
	 *               contain a parsable XMLDouble.
	 */

    XMLDouble(const XMLCh* const strValue,
              MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    ~XMLDouble();

	/**
	 * Compares this object to the specified object.
	 * The result is <code>true</code> if and only if the argument is not
	 * <code>null</code> and is an <code>XMLDouble</code> object that contains
	 * the same <code>int</code> value as this object.
	 *
	 * @param   lValue the object to compare with.
	 * @param   rValue the object to compare against.
	 * @return  <code>true</code> if the objects are the same;
	 *          <code>false</code> otherwise.
	 */

    inline static int            compareValues(const XMLDouble* const lValue
                                             , const XMLDouble* const rValue);

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(XMLDouble)

    XMLDouble(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

protected:

    virtual void          checkBoundary(char* const strValue);

private:
    //
    // Unimplemented
    //
    // copy ctor
    // assignment ctor
    //
     XMLDouble(const XMLDouble& toCopy);
     XMLDouble& operator=(const XMLDouble& toAssign);

};

inline int XMLDouble::compareValues(const XMLDouble* const lValue
                                  , const XMLDouble* const rValue)
{
    return XMLAbstractDoubleFloat::compareValues((const XMLAbstractDoubleFloat*) lValue,
                                                 (const XMLAbstractDoubleFloat*) rValue 
                                                 , ((XMLAbstractDoubleFloat*)lValue)->getMemoryManager());
}

XERCES_CPP_NAMESPACE_END

#endif

