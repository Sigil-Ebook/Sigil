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
 * $Id: XMLCanRepGroup.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLCANREPGROUP_HPP)
#define XERCESC_INCLUDE_GUARD_XMLCANREPGROUP_HPP

#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class VALIDATORS_EXPORT XMLCanRepGroup : public XMemory
{
public:

    enum CanRepGroup {
        Boolean,
        DoubleFloat,
        DateTime,
        Time,
        Decimal,
        Decimal_Derived_signed,
        Decimal_Derived_unsigned,
        Decimal_Derived_npi,
        String
    };

    ~XMLCanRepGroup();

    XMLCanRepGroup(CanRepGroup val);

    inline CanRepGroup    getGroup() const;

private:

    CanRepGroup    fData;

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLCanRepGroup(const XMLCanRepGroup&);
	XMLCanRepGroup& operator=(const XMLCanRepGroup&);

};

inline XMLCanRepGroup::CanRepGroup XMLCanRepGroup::getGroup() const
{
    return fData;
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file XMLCanRepGroup.hpp
  */

