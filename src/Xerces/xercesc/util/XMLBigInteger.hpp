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
 * $Id: XMLBigInteger.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XML_BIGINTEGER_HPP)
#define XERCESC_INCLUDE_GUARD_XML_BIGINTEGER_HPP

#include <xercesc/util/XMemory.hpp>
#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLUTIL_EXPORT XMLBigInteger : public XMemory
{
public:

    /**
     * Constructs a newly allocated <code>XMLBigInteger</code> object that
     * represents the value represented by the string. The string is
     * converted to an int value as if by the <code>valueOf</code> method.
     *
     * @param      strValue   the <code>String</code> to be converted to an
     *                       <code>XMLBigInteger</code>.
     * @param manager    Pointer to the memory manager to be used to
     *                   allocate objects.
     * @exception  NumberFormatException  if the <code>String</code> does not
     *               contain a parsable XMLBigInteger.
     */

    XMLBigInteger
    (
        const XMLCh* const strValue
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    ~XMLBigInteger();

    XMLBigInteger(const XMLBigInteger& toCopy);

    static XMLCh* getCanonicalRepresentation
                        (
                          const XMLCh*         const rawData
                        ,       MemoryManager* const memMgr = XMLPlatformUtils::fgMemoryManager
                        ,       bool                 isNonPositiveInteger = false
                        );

    static void parseBigInteger(const XMLCh* const toConvert
                              , XMLCh* const       retBuffer
                              , int&   signValue
                              , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    static int  compareValues(const XMLBigInteger* const lValue
                             ,const XMLBigInteger* const rValue
                             , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);


    static int  compareValues(const XMLCh*         const lString
                            , const int&                 lSign
                            , const XMLCh*         const rString
                            , const int&                 rSign
                            ,       MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    void        multiply(const unsigned int byteToShift);

    void        divide(const unsigned int byteToShift);

    unsigned int       getTotalDigit() const;
   
    /**
     *  Return a copy of the fMagnitude.
     *  This is similar to toString, except the internal buffer is returned directly
     *  Caller is not required to delete the returned memory.
     */
    inline XMLCh*      getRawData() const;

    /**
     * Compares this object to the specified object.
     * The result is <code>true</code> if and only if the argument is not
     * <code>null</code> and is an <code>XMLBigInteger</code> object that contains
     * the same <code>int</code> value as this object.
     *
     * @param   toCompare   the object to compare with.
     * @return  <code>true</code> if the objects are the same;
     *          <code>false</code> otherwise.
     */
    bool operator==(const XMLBigInteger& toCompare) const;

    /**
     * Returns the signum function of this number (i.e., -1, 0 or 1 as
     * the value of this number is negative, zero or positive).
     */
    int getSign() const;

    int intValue() const;

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------    
    XMLBigInteger& operator=(const XMLBigInteger&);


    void setSign(int);

    /*
     * The number is internally stored in "minimal" sign-fMagnitude format
     * (i.e., no BigIntegers have a leading zero byte in their magnitudes).
     * Zero is represented with a signum of 0 (and a zero-length fMagnitude).
     * Thus, there is exactly one representation for each value.
     */
    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fSign
    //     to represent the sign of the number.
    //
    //  fMagnitude
    //     the buffer holding the number.
    //
    //  fRawData
    //     to preserve the original string used to construct this object,
    //     needed for pattern matching.
    //
    // -----------------------------------------------------------------------

    int         fSign;
    XMLCh*      fMagnitude;  //null terminated
    XMLCh*      fRawData;
    MemoryManager* fMemoryManager;
};

inline int XMLBigInteger::getSign() const
{    
    return fSign;
}

inline unsigned int XMLBigInteger::getTotalDigit() const
{
    return ((getSign() ==0) ? 0 : (unsigned int)XMLString::stringLen(fMagnitude));
}

inline bool XMLBigInteger::operator==(const XMLBigInteger& toCompare) const
{
    return ( compareValues(this, &toCompare, fMemoryManager) ==0 ? true : false);
}

inline void XMLBigInteger::setSign(int newSign)
{
    fSign = newSign;
}

inline XMLCh*  XMLBigInteger::getRawData() const
{
    return fRawData;
}

XERCES_CPP_NAMESPACE_END

#endif
