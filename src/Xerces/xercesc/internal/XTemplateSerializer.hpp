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
 * $Id: XTemplateSerializer.hpp 678409 2008-07-21 13:08:10Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XTEMPLATE_SERIALIZER_HPP)
#define XERCESC_INCLUDE_GUARD_XTEMPLATE_SERIALIZER_HPP

#include <xercesc/util/ValueVectorOf.hpp>
#include <xercesc/util/RefArrayVectorOf.hpp>
#include <xercesc/util/RefVectorOf.hpp>
#include <xercesc/util/RefHashTableOf.hpp>
#include <xercesc/util/RefHash2KeysTableOf.hpp>
#include <xercesc/util/RefHash3KeysIdPool.hpp>
#include <xercesc/util/NameIdPool.hpp>

#include <xercesc/framework/XMLNotationDecl.hpp>
#include <xercesc/framework/XMLRefInfo.hpp>
#include <xercesc/util/XMLNumber.hpp>
#include <xercesc/validators/common/ContentSpecNode.hpp>
#include <xercesc/validators/DTD/DTDAttDef.hpp>
#include <xercesc/validators/DTD/DTDElementDecl.hpp>
#include <xercesc/validators/DTD/DTDEntityDecl.hpp>
#include <xercesc/validators/schema/SchemaAttDef.hpp>
#include <xercesc/validators/schema/SchemaElementDecl.hpp>
#include <xercesc/validators/schema/XercesGroupInfo.hpp>
#include <xercesc/validators/schema/XercesAttGroupInfo.hpp>
#include <xercesc/validators/schema/SchemaGrammar.hpp>
#include <xercesc/validators/schema/identity/IC_Field.hpp>
#include <xercesc/validators/schema/identity/IdentityConstraint.hpp>
#include <xercesc/validators/schema/identity/XercesXPath.hpp>
#include <xercesc/framework/psvi/XSAnnotation.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLUTIL_EXPORT XTemplateSerializer
{
public:

    /**********************************************************
     *
     * ValueVectorOf
     *
     *   SchemaElementDecl*
     *   unsigned int
     *
     ***********************************************************/
    static void           storeObject(ValueVectorOf<SchemaElementDecl*>* const tempObjToWrite
                                    , XSerializeEngine&                        serEng);

    static void           loadObject(ValueVectorOf<SchemaElementDecl*>**       tempObjToRead
                                   , int                                       initSize
                                   , bool                                      toCallDestructor
                                   , XSerializeEngine&                         serEng);

    static void           storeObject(ValueVectorOf<unsigned int>* const tempObjToWrite
                                    , XSerializeEngine&                  serEng);

    static void           loadObject(ValueVectorOf<unsigned int>**       tempObjToRead
                                   , int                                 initSize
                                   , bool                                toCallDestructor
                                   , XSerializeEngine&                   serEng);

    /**********************************************************
     *
     * RefArrayVectorOf
     *
     *   XMLCh
     *
     ***********************************************************/
    static void           storeObject(RefArrayVectorOf<XMLCh>* const tempObjToWrite
                                    , XSerializeEngine&              serEng);

    static void           loadObject(RefArrayVectorOf<XMLCh>**       tempObjToRead
                                   , int                             initSize
                                   , bool                            toAdopt
                                   , XSerializeEngine&               serEng);

    /**********************************************************
     *
     * RefVectorOf
     *
     *   SchemaAttDef
     *   SchemaElementDecl
     *   ContentSpecNode
     *   IC_Field
     *   DatatypeValidator
     *   IdentityConstraint
     *   XMLNumber
     *   XercesLocationPath
     *   XercesStep
     *
     ***********************************************************/
    static void           storeObject(RefVectorOf<SchemaAttDef>* const tempObjToWrite
                                    , XSerializeEngine&                serEng);

    static void           loadObject(RefVectorOf<SchemaAttDef>**       tempObjToRead
                                   , int                               initSize
                                   , bool                              toAdopt
                                   , XSerializeEngine&                 serEng);

    static void           storeObject(RefVectorOf<SchemaElementDecl>* const tempObjToWrite
                                    , XSerializeEngine&                     serEng);

    static void           loadObject(RefVectorOf<SchemaElementDecl>**       tempObjToRead
                                   , int                                    initSize
                                   , bool                                   toAdopt
                                   , XSerializeEngine&                      serEng);

    static void           storeObject(RefVectorOf<ContentSpecNode>* const tempObjToWrite
                                    , XSerializeEngine&                   serEng);

    static void           loadObject(RefVectorOf<ContentSpecNode>**       tempObjToRead
                                   , int                                  initSize
                                   , bool                                 toAdopt
                                   , XSerializeEngine&                    serEng);

    static void           storeObject(RefVectorOf<IC_Field>* const tempObjToWrite
                                    , XSerializeEngine&            serEng);

    static void           loadObject(RefVectorOf<IC_Field>**       tempObjToRead
                                   , int                           initSize
                                   , bool                          toAdopt
                                   , XSerializeEngine&             serEng);

    static void           storeObject(RefVectorOf<DatatypeValidator>* const tempObjToWrite
                                    , XSerializeEngine&                     serEng);

    static void           loadObject(RefVectorOf<DatatypeValidator>**       tempObjToRead
                                   , int                                    initSize
                                   , bool                                   toAdopt
                                   , XSerializeEngine&                      serEng);

    static void           storeObject(RefVectorOf<IdentityConstraint>* const tempObjToWrite
                                    , XSerializeEngine&                      serEng);

    static void           loadObject(RefVectorOf<IdentityConstraint>**       tempObjToRead
                                   , int                                     initSize
                                   , bool                                    toAdopt
                                   , XSerializeEngine&                       serEng);

    static void           storeObject(RefVectorOf<XMLNumber>* const tempObjToWrite
                                    , XSerializeEngine&             serEng);

    static void           loadObject(RefVectorOf<XMLNumber>**       tempObjToRead
                                   , int                            initSize
                                   , bool                           toAdopt
                                   , XMLNumber::NumberType          numType
                                   , XSerializeEngine&              serEng);

    static void           storeObject(RefVectorOf<XercesLocationPath>* const tempObjToWrite
                                    , XSerializeEngine&                      serEng);

    static void           loadObject(RefVectorOf<XercesLocationPath>**       tempObjToRead
                                   , int                                     initSize
                                   , bool                                    toAdopt
                                   , XSerializeEngine&                       serEng);

    static void           storeObject(RefVectorOf<XercesStep>* const tempObjToWrite
                                    , XSerializeEngine&              serEng);

    static void           loadObject(RefVectorOf<XercesStep>**       tempObjToRead
                                   , int                             initSize
                                   , bool                            toAdopt
                                   , XSerializeEngine&               serEng);

    /**********************************************************
     *
     * RefHashTableOf
     *
     *   KVStringPair
     *   XMLAttDef
     *   DTDAttDef
     *   ComplexTypeInfo
     *   XercesGroupInfo
     *   XercesAttGroupInfo
     *   XMLRefInfo
     *   DatatypeValidator
     *   Grammar
     *   XSAnnotation
     *
     ***********************************************************/
    static void           storeObject(RefHashTableOf<KVStringPair>* const tempObjToWrite
                                    , XSerializeEngine&                   serEng);

    static void           loadObject(RefHashTableOf<KVStringPair>**       tempObjToRead
                                   , int                                  initSize
                                   , bool                                 toAdopt
                                   , XSerializeEngine&                    serEng);

    static void           storeObject(RefHashTableOf<XMLAttDef>* const tempObjToWrite
                                    , XSerializeEngine&                serEng);

    static void           loadObject(RefHashTableOf<XMLAttDef>**       tempObjToRead
                                   , int                               initSize
                                   , bool                              toAdopt
                                   , XSerializeEngine&                 serEng);

    static void           storeObject(RefHashTableOf<DTDAttDef>* const tempObjToWrite
                                    , XSerializeEngine&                serEng);

    static void           loadObject(RefHashTableOf<DTDAttDef>**       tempObjToRead
                                   , int                               initSize
                                   , bool                              toAdopt
                                   , XSerializeEngine&                 serEng);

    static void           storeObject(RefHashTableOf<ComplexTypeInfo>* const tempObjToWrite
                                    , XSerializeEngine&                      serEng);

    static void           loadObject(RefHashTableOf<ComplexTypeInfo>**       tempObjToRead
                                   , int                                     initSize
                                   , bool                                    toAdopt
                                   , XSerializeEngine&                       serEng);

    static void           storeObject(RefHashTableOf<XercesGroupInfo>* const tempObjToWrite
                                    , XSerializeEngine&                      serEng);

    static void           loadObject(RefHashTableOf<XercesGroupInfo>**       tempObjToRead
                                   , int                                     initSize
                                   , bool                                    toAdopt
                                   , XSerializeEngine&                       serEng);

    static void           storeObject(RefHashTableOf<XercesAttGroupInfo>* const tempObjToWrite
                                    , XSerializeEngine&                         serEng);

    static void           loadObject(RefHashTableOf<XercesAttGroupInfo>**       tempObjToRead
                                   , int                                        initSize
                                   , bool                                       toAdopt
                                   , XSerializeEngine&                          serEng);

    static void           storeObject(RefHashTableOf<XMLRefInfo>* const tempObjToWrite
                                    , XSerializeEngine&                 serEng);

    static void           loadObject(RefHashTableOf<XMLRefInfo>**       tempObjToRead
                                   , int                                initSize
                                   , bool                               toAdopt
                                   , XSerializeEngine&                  serEng);

    static void           storeObject(RefHashTableOf<DatatypeValidator>* const tempObjToWrite
                                    , XSerializeEngine&                        serEng);

    static void           loadObject(RefHashTableOf<DatatypeValidator>**       tempObjToRead
                                   , int                                       initSize
                                   , bool                                      toAdopt
                                   , XSerializeEngine&                         serEng);

    static void           storeObject(RefHashTableOf<Grammar>* const tempObjToWrite
                                    , XSerializeEngine&              serEng);

    static void           loadObject(RefHashTableOf<Grammar>**       tempObjToRead
                                   , int                             initSize
                                   , bool                            toAdopt
                                   , XSerializeEngine&               serEng);

    static void           storeObject(RefHashTableOf<XSAnnotation, PtrHasher>* const tempObjToWrite
                                    , XSerializeEngine&                   serEng);

    static void           loadObject(RefHashTableOf<XSAnnotation, PtrHasher>**  tempObjToRead
                                   , int                             initSize
                                   , bool                            toAdopt
                                   , XSerializeEngine&               serEng);

    /**********************************************************
     *
     * RefHash2KeysTableOf
     *
     *   SchemaAttDef
     *   ElemVector
     *
     ***********************************************************/
    static void           storeObject(RefHash2KeysTableOf<SchemaAttDef>* const tempObjToWrite
                                    , XSerializeEngine&                        serEng);

    static void           loadObject(RefHash2KeysTableOf<SchemaAttDef>**       tempObjToRead
                                   , int                                       initSize
                                   , bool                                      toAdopt
                                   , XSerializeEngine&                         serEng);

    static void           storeObject(RefHash2KeysTableOf<ElemVector>* const tempObjToWrite
                                    , XSerializeEngine&                      serEng);

    static void           loadObject(RefHash2KeysTableOf<ElemVector>**       tempObjToRead
                                   , int                                     initSize
                                   , bool                                    toAdopt
                                   , XSerializeEngine&                       serEng);

    /**********************************************************
     *
     * RefHash3KeysIdPool
     *
     *   SchemaElementDecl
     *
     ***********************************************************/
    static void           storeObject(RefHash3KeysIdPool<SchemaElementDecl>* const tempObjToWrite
                                    , XSerializeEngine&                            serEng);

    static void           loadObject(RefHash3KeysIdPool<SchemaElementDecl>**       tempObjToRead
                                   , int                                           initSize
                                   , bool                                          toAdopt
                                   , int                                           initSize2
                                   , XSerializeEngine&                             serEng);

    /**********************************************************
     *
     * NameIdPool
     *
     *   DTDElementDecl
     *   DTDEntityDecl
     *   XMLNotationDecl
     *
     ***********************************************************/
    static void           storeObject(NameIdPool<DTDElementDecl>* const tempObjToWrite
                                    , XSerializeEngine&                 serEng);

    static void           loadObject(NameIdPool<DTDElementDecl>**       tempObjToRead
                                   , int                                initSize
                                   , int                                initSize2
                                   , XSerializeEngine&                  serEng);

    static void           storeObject(NameIdPool<DTDEntityDecl>* const tempObjToWrite
                                    , XSerializeEngine&                serEng);

    static void           loadObject(NameIdPool<DTDEntityDecl>**       tempObjToRead
                                   , int                               initSize
                                   , int                               initSize2
                                   , XSerializeEngine&                 serEng);

    static void           storeObject(NameIdPool<XMLNotationDecl>* const tempObjToWrite
                                    , XSerializeEngine&                  serEng);

    static void           loadObject(NameIdPool<XMLNotationDecl>**      tempObjToRead
                                   , int                                initSize
                                   , int                                initSize2
                                   , XSerializeEngine&                  serEng);

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
	~XTemplateSerializer();
    XTemplateSerializer();
    XTemplateSerializer(const XTemplateSerializer&);
	XTemplateSerializer& operator=(const XTemplateSerializer&);

};

XERCES_CPP_NAMESPACE_END

#endif
