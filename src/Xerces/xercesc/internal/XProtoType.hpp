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
 * $Id: XProtoType.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XPROTOTYPE_HPP)
#define XERCESC_INCLUDE_GUARD_XPROTOTYPE_HPP

#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XSerializeEngine;
class XSerializable;

class XMLUTIL_EXPORT XProtoType
{
public:

           void       store(XSerializeEngine& serEng) const;

    static void        load(XSerializeEngine&          serEng
                          , XMLByte*          const    name
                          , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
                          );

    // -------------------------------------------------------------------------------
    //  data
    //
    //  fClassName: 
    //            name of the XSerializable derivatives
    //
    //  fCreateObject:
    //            pointer to the factory method (createObject()) 
    //            of the XSerializable derivatives
    //
    // -------------------------------------------------------------------------------

    XMLByte*          fClassName;

    XSerializable*    (*fCreateObject)(MemoryManager*);

};

#define DECL_XPROTOTYPE(class_name) \
static  XProtoType        class##class_name;                   \
static  XSerializable*    createObject(MemoryManager* manager);

/***
 * For non-abstract class
 ***/
#define IMPL_XPROTOTYPE_TOCREATE(class_name) \
IMPL_XPROTOTYPE_INSTANCE(class_name) \
XSerializable* class_name::createObject(MemoryManager* manager) \
{return new (manager) class_name(manager);}

/***
* For abstract class
 ***/
#define IMPL_XPROTOTYPE_NOCREATE(class_name) \
IMPL_XPROTOTYPE_INSTANCE(class_name) \
XSerializable* class_name::createObject(MemoryManager*) \
{return 0;}


/***
 * Helper Macro 
 ***/
#define XPROTOTYPE_CLASS(class_name) ((XProtoType*)(&class_name::class##class_name))

#define IMPL_XPROTOTYPE_INSTANCE(class_name) \
XProtoType class_name::class##class_name = \
{(XMLByte*) #class_name, class_name::createObject };

XERCES_CPP_NAMESPACE_END

#endif
