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
 * $Id: XMLSchemaDescriptionImpl.cpp 471747 2006-11-06 14:31:56Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/schema/XMLSchemaDescriptionImpl.hpp>
#include <xercesc/util/QName.hpp>

#include <xercesc/internal/XTemplateSerializer.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XMLSchemaDescriptionImpl: constructor and destructor
// ---------------------------------------------------------------------------
XMLSchemaDescriptionImpl::XMLSchemaDescriptionImpl(const XMLCh* const   targetNamespace
                                                 , MemoryManager* const memMgr)
:XMLSchemaDescription(memMgr)
,fContextType(CONTEXT_UNKNOWN)
,fNamespace(0)
,fLocationHints(0)
,fTriggeringComponent(0)
,fEnclosingElementName(0)
,fAttributes(0) 
{
    if (targetNamespace)
        fNamespace = XMLString::replicate(targetNamespace, memMgr);


    fLocationHints =  new (memMgr) RefArrayVectorOf<XMLCh>(4, true, memMgr);

    /***
        fAttributes
    ***/
}

XMLSchemaDescriptionImpl::~XMLSchemaDescriptionImpl()
{
    if (fNamespace)
        XMLGrammarDescription::getMemoryManager()->deallocate((void*)fNamespace);

    if (fLocationHints)
        delete fLocationHints;

    if (fTriggeringComponent)
        delete fTriggeringComponent;

    if (fEnclosingElementName)
        delete fEnclosingElementName;

}

const XMLCh* XMLSchemaDescriptionImpl::getGrammarKey() const
{ 
    return getTargetNamespace(); 
}
    
XMLSchemaDescription::ContextType XMLSchemaDescriptionImpl::getContextType() const
{
    return fContextType; 
}

const XMLCh* XMLSchemaDescriptionImpl::getTargetNamespace() const
{ 
    return fNamespace; 
}

const RefArrayVectorOf<XMLCh>* XMLSchemaDescriptionImpl::getLocationHints() const
{ 
    return fLocationHints; 
}

const QName* XMLSchemaDescriptionImpl::getTriggeringComponent() const                      
{ 
    return fTriggeringComponent; 
}

const QName* XMLSchemaDescriptionImpl::getEnclosingElementName() const
{ 
    return fEnclosingElementName; 
}

const XMLAttDef* XMLSchemaDescriptionImpl::getAttributes() const                      
{ 
    return fAttributes; 
}
         
void XMLSchemaDescriptionImpl::setContextType(ContextType type)
{ 
    fContextType = type; 
}

void XMLSchemaDescriptionImpl::setTargetNamespace(const XMLCh* const newNamespace)
{  
    if (fNamespace) {
        XMLGrammarDescription::getMemoryManager()->deallocate((void*)fNamespace);
        fNamespace = 0;
    }
    
    fNamespace = XMLString::replicate(newNamespace, XMLGrammarDescription::getMemoryManager()); 
}

void XMLSchemaDescriptionImpl::setLocationHints(const XMLCh* const hint)
{    
    fLocationHints->addElement(XMLString::replicate(hint, XMLGrammarDescription::getMemoryManager())); 
}

void XMLSchemaDescriptionImpl::setTriggeringComponent(QName* const trigComponent)
{ 
    if ( fTriggeringComponent) {
        delete fTriggeringComponent;
        fTriggeringComponent = 0;
    }
    
    fTriggeringComponent = new (trigComponent->getMemoryManager()) QName(*trigComponent); 

}

void XMLSchemaDescriptionImpl::setEnclosingElementName(QName* const encElement)
{ 
    if (fEnclosingElementName) {
        delete fEnclosingElementName;
        fEnclosingElementName = 0; 
    }

    fEnclosingElementName = new (encElement->getMemoryManager()) QName(*encElement); 

}

void XMLSchemaDescriptionImpl::setAttributes(XMLAttDef* const attDefs)
{ 
    // XMLAttDef is part of the grammar that this description refers to
    // so we reference to it instead of adopting/owning/cloning.
    fAttributes = attDefs; 
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(XMLSchemaDescriptionImpl)

void XMLSchemaDescriptionImpl::serialize(XSerializeEngine& serEng)
{
    XMLSchemaDescription::serialize(serEng);

    if (serEng.isStoring())
    {
        serEng<<(int)fContextType;       
        serEng.writeString(fNamespace);

        /***
         * 
         * Serialize RefArrayVectorOf<XMLCh>*               fLocationHints;
         *
         ***/
        XTemplateSerializer::storeObject(fLocationHints, serEng);

        QName* tempQName = (QName*)fTriggeringComponent;
        serEng<<tempQName;
        tempQName = (QName*)fEnclosingElementName;
        serEng<<tempQName;

        XMLAttDef* tempAttDef = (XMLAttDef*)fAttributes;
        serEng<<tempAttDef;
    }

    else
    {
        int i;
        serEng>>i;

        fContextType = (ContextType)i;       

        //the original fNamespace which came from the ctor needs deallocated
        if (fNamespace)
        {
            XMLGrammarDescription::getMemoryManager()->deallocate((void*)fNamespace);
        }
        serEng.readString((XMLCh*&)fNamespace);

        /***
         *
         *  Deserialize RefArrayVectorOf<XMLCh>    fLocationHints     
         *
         ***/
        XTemplateSerializer::loadObject(&fLocationHints, 4, true, serEng);

        QName* tempQName;
        serEng>>tempQName;
        fTriggeringComponent = tempQName;

        serEng>>tempQName;
        fEnclosingElementName = tempQName;

        XMLAttDef* tempAttDef;
        serEng>>tempAttDef;
        fAttributes=tempAttDef;

    }

}

XMLSchemaDescriptionImpl::XMLSchemaDescriptionImpl(MemoryManager* const memMgr)
:XMLSchemaDescription(memMgr)
,fContextType(CONTEXT_UNKNOWN)
,fNamespace(0)
,fLocationHints(0)
,fTriggeringComponent(0)
,fEnclosingElementName(0)
,fAttributes(0) 
{
}

XERCES_CPP_NAMESPACE_END
