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
 * $Id: XMLAbstractDoubleFloat.hpp 605828 2007-12-20 08:05:47Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XML_ABSTRACT_DOUBLE_FLOAT_HPP)
#define XERCESC_INCLUDE_GUARD_XML_ABSTRACT_DOUBLE_FLOAT_HPP


#include <xercesc/util/XMLNumber.hpp>
#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/***
 * 3.2.5.1 Lexical representation
 *
 *   double values have a lexical representation consisting of a mantissa followed,
 *   optionally, by the character "E" or "e", followed by an exponent.
 *
 *   The exponent must be an integer.
 *   The mantissa must be a decimal number.
 *   The representations for exponent and mantissa must follow the lexical rules
 *   for integer and decimal.
 *
 *   If the "E" or "e" and the following exponent are omitted,
 *   an exponent value of 0 is assumed.
***/

/***
 * 3.2.4.1 Lexical representation
 *
 *   float values have a lexical representation consisting of a mantissa followed,
 *   optionally, by the character "E" or "e", followed by an exponent.
 *
 *   The exponent must be an integer.
 *   The mantissa must be a decimal number.
 *   The representations for exponent and mantissa must follow the lexical rules
 *   for integer and decimal.
 *
 *   If the "E" or "e" and the following exponent are omitted,
 *   an exponent value of 0 is assumed.
***/

class XMLUTIL_EXPORT XMLAbstractDoubleFloat : public XMLNumber
{
public:

    enum LiteralType
    {
        NegINF,
        PosINF,
        NaN,
        SpecialTypeNum,
        Normal
    };

    virtual ~XMLAbstractDoubleFloat();

    static XMLCh* getCanonicalRepresentation
                        (
                          const XMLCh*         const rawData
                        ,       MemoryManager* const memMgr = XMLPlatformUtils::fgMemoryManager
                        );
    
    virtual XMLCh*        getRawData() const;

    virtual const XMLCh*  getFormattedString() const;

    virtual int           getSign() const;

    MemoryManager*        getMemoryManager() const;

    inline  bool          isDataConverted()  const;

    inline  bool          isDataOverflowed()  const;

    inline  double        getValue() const;

    inline  LiteralType   getType() const;

    /***
     *
     * The decimal point delimiter for the schema double/float type is
     * defined to be a period and is not locale-specific. So, it must
     * be replaced with the local-specific delimiter before converting
     * from string to double/float.
     *
     ***/
    static void            normalizeDecimalPoint(char* const toNormal);

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(XMLAbstractDoubleFloat)

protected:

    //
    // To be used by derived class exclusively
    //
    XMLAbstractDoubleFloat(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    void                  init(const XMLCh* const strValue);

    /**
	 * Compares this object to the specified object.
	 * The result is <code>true</code> if and only if the argument is not
	 * <code>null</code> and is an <code>XMLAbstractDoubleFloat</code> object that contains
	 * the same <code>int</code> value as this object.
	 *
	 * @param   lValue the object to compare with.
	 * @param   rValue the object to compare against.
     * @param manager The MemoryManager to use to allocate objects
	 * @return  <code>true</code> if the objects are the same;
	 *          <code>false</code> otherwise.
	 */

    static int            compareValues(const XMLAbstractDoubleFloat* const lValue
                                      , const XMLAbstractDoubleFloat* const rValue
                                      , MemoryManager* const manager);

    //
    // to be overridden by derived class
    //
    virtual void          checkBoundary(char* const strValue) = 0;

    void
    convert(char* const strValue);

private:
    //
    // Unimplemented
    //
    // copy ctor
    // assignment ctor
    //
    XMLAbstractDoubleFloat(const XMLAbstractDoubleFloat& toCopy);
    XMLAbstractDoubleFloat& operator=(const XMLAbstractDoubleFloat& toAssign);

	void                  normalizeZero(XMLCh* const);

    inline bool           isSpecialValue() const;

    static int            compareSpecial(const XMLAbstractDoubleFloat* const specialValue                                       
                                       , MemoryManager* const manager);

    void                  formatString();

protected:
    double                  fValue;
    LiteralType             fType;
    bool                    fDataConverted;
    bool                    fDataOverflowed;

private:
    int                     fSign;
    XMLCh*                  fRawData;

    //
    // If the original string is not lexcially the same as the five
    // special value notations, and the value is converted to
    // special value due underlying platform restriction on data
    // representation, then this string is constructed and
    // takes the form "original_string (special_value_notation)", 
    // otherwise it is empty.
    //
    XMLCh*                  fFormattedString;
    MemoryManager*          fMemoryManager;

};

inline bool XMLAbstractDoubleFloat::isSpecialValue() const
{
    return (fType < SpecialTypeNum);
}

inline MemoryManager* XMLAbstractDoubleFloat::getMemoryManager() const
{
    return fMemoryManager;
}

inline bool XMLAbstractDoubleFloat::isDataConverted() const
{
    return fDataConverted;
}

inline bool XMLAbstractDoubleFloat::isDataOverflowed() const
{
    return fDataOverflowed;
}

inline double XMLAbstractDoubleFloat::getValue() const
{
    return fValue;
}

inline  XMLAbstractDoubleFloat::LiteralType   XMLAbstractDoubleFloat::getType() const
{
    return fType;
}

XERCES_CPP_NAMESPACE_END

#endif
