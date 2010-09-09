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

#if !defined(XERCESC_INCLUDE_GUARD_DOMCONFIGURATION_HPP)
#define XERCESC_INCLUDE_GUARD_DOMCONFIGURATION_HPP

//------------------------------------------------------------------------------------
//  Includes
//------------------------------------------------------------------------------------

#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/RefVectorOf.hpp>
#include <xercesc/dom/DOMStringList.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
 *   The DOMConfiguration interface represents the configuration of
 *   a document  and  maintains  a  table of recognized parameters.
 *   Using  the   configuration,   it   is   possible   to   change
 *   Document.normalizeDocument  behavior,  such  as replacing
 *   CDATASection   nodes   with   Text  nodes  or
 *   specifying  the  type  of the schema that must be used when the
 *   validation of the Document is requested. DOMConfiguration
 *   objects  are  also used in [DOM Level 3 Load and Save] in
 *   the DOMLSParser and DOMLSSerializer interfaces.
 *
 *   The  DOMConfiguration  distinguish  two  types  of  parameters:
 *   boolean     (boolean    parameters)    and    DOMUserData
 *   (parameters). The names used by the DOMConfiguration object are
 *   defined  throughout  the  DOM Level 3 specifications. Names are
 *   case-insensitive.   To   avoid   possible   conflicts,   as  a
 *   convention,   names   referring   to   boolean  parameters  and
 *   parameters defined outside the DOM specification should be made
 *   unique.  Names  are  recommended  to  follow the XML name
 *   production   rule   but   it   is   not  enforced  by  the  DOM
 *   implementation.  DOM  Level 3 Core Implementations are required
 *   to  recognize  all boolean parameters and parameters defined in
 *   this  specification.  Each boolean parameter state or parameter
 *   value may then be supported or not by the implementation. Refer
 *   to  their  definition  to  know  if  a state or a value must be
 *   supported or not.
 *
 *   Note: Parameters are similar to features and properties used in
 *   SAX2 [SAX].
 *
 *   The following list of parameters defined in the DOM:
 *
 * "error-handler"
 *         [required]
 *         A   DOMErrorHandler   object.   If   an   error  is
 *         encountered in the document, the implementation will call
 *         back  the  DOMErrorHandler  registered  using  this
 *         parameter.
 *         When  called, DOMError.relatedData will contain the
 *         closest   node   to  where  the  error  occured.  If  the
 *         implementation  is unable to determine the node where the
 *         error occurs, DOMError.relatedData will contain the
 *         Document  node.  Mutations  to  the  document  from
 *         within  an  error  handler  will result in implementation
 *         dependent behaviour.
 *
 * "schema-type"
 *         [optional]
 *         A  DOMString  object containing an absolute URI and
 *         representing  the  type  of  the  schema language used to
 *         validate   a  document  against.  Note  that  no  lexical
 *         checking is done on the absolute URI.
 *         If  this  parameter  is  not  set, a default value may be
 *         provided  by  the  implementation,  based  on  the schema
 *         languages  supported  and  on the schema language used at
 *         load time.
 *
 *         Note:   For   XML   Schema  [XML  Schema  Part  1],
 *         applications must use the value
 *         "http://www.w3.org/2001/XMLSchema".     For    XML    DTD
 *         [XML   1.0],   applications   must  use  the  value
 *         "http://www.w3.org/TR/REC-xml".  Other  schema  languages
 *         are  outside  the  scope  of the W3C and therefore should
 *         recommend an absolute URI in order to use this method.
 *
 * "schema-location"
 *         [optional]
 *         A  DOMString  object  containing  a  list  of URIs,
 *         separated   by  white  spaces  (characters  matching  the
 *         nonterminal  production  S  defined  in section 2.3
 *         [XML  1.0]),  that  represents  the schemas against
 *         which  validation  should  occur.  The  types  of schemas
 *         referenced  in  this  list  must match the type specified
 *         with   schema-type,   otherwise   the   behaviour  of  an
 *         implementation  is  undefined.  If the schema type is XML
 *         Schema  [XML  Schema  Part  1], only one of the XML
 *         Schemas in the list can be with no namespace.
 *         If  validation  occurs  against a namespace aware schema,
 *         i.e.  XML  Schema,  and  the  targetNamespace of a schema
 *         (specified    using    this    property)    matches   the
 *         targetNamespace  of  a  schema  occurring in the instance
 *         document,  i.e  in  schemaLocation  attribute, the schema
 *         specified  by  the  user using this property will be used
 *         (i.e.,  in XML Schema the schemaLocation attribute in the
 *         instance  document  or  on  the  import  element  will be
 *         effectively ignored).
 *
 *         Note:  It is illegal to set the schema-location parameter
 *         if  the  schema-type  parameter  value  is not set. It is
 *         strongly  recommended that DOMInputSource.baseURI will be
 *         set,  so  that an implementation can successfully resolve
 *         any external entities referenced.
 *
 *   The  following list of boolean parameters (features) defined in
 *   the DOM:
 *
 * "canonical-form"
 *
 *       true
 *               [optional]
 *               Canonicalize  the  document  according to the rules
 *               specified  in [Canonical XML]. Note that this
 *               is  limited  to what can be represented in the DOM.
 *               In particular, there is no way to specify the order
 *               of the attributes in the DOM.
 *
 *       false
 *               [required] (default)
 *               Do not canonicalize the document.
 *
 * "cdata-sections"
 *
 *       true
 *               [required] (default)
 *               Keep CDATASection nodes in the document.
 *
 *       false
 *               [required]
 *               Transform  CDATASection nodes in the document
 *               into  Text  nodes. The new Text node is
 *               then combined with any adjacent Text node.
 *
 * "comments"
 *
 *       true
 *               [required] (default)
 *               Keep Comment nodes in the document.
 *
 *       false
 *               [required]
 *               Discard Comment nodes in the Document.
 *
 * "datatype-normalization"
 *
 *       true
 *               [required]
 *               Exposed normalized values in the tree.
 *
 *       false
 *               [required] (default)
 *               Do not perform normalization on the tree.
 *
 * "discard-default-content"
 *
 *       true
 *               [required] (default)
 *               Use   whatever   information   available   to   the
 *               implementation (i.e. XML schema, DTD, the specified
 *               flag on Attr nodes, and so on) to decide what
 *               attributes  and content should be discarded or not.
 *               Note that the specified flag on Attr nodes in
 *               itself  is not always reliable, it is only reliable
 *               when  it  is set to false since the only case where
 *               it  can  be  set  to  false is if the attribute was
 *               created  by the implementation. The default content
 *               won't be removed if an implementation does not have
 *               any information available.
 *
 *       false
 *               [required]
 *               Keep all attributes and all content.
 *
 * "entities"
 *
 *       true
 *               [required]
 *               Keep  EntityReference  and Entity nodes
 *               in the document.
 *
 *       false
 *               [required] (default)
 *               Remove  all  EntityReference and Entity
 *               nodes   from   the  document,  putting  the  entity
 *               expansions  directly  in  their  place.  Text
 *               nodes     are     into    "normal"    form.    Only
 *               EntityReference nodes to non-defined entities
 *               are kept in the document.
 *
 * "infoset"
 *
 *       true
 *               [required]
 *               Only  keep  in the document the information defined
 *               in  the  XML Information Set [XML Information
 *               set].
 *               This   forces  the  following  features  to  false:
 *               namespace-declarations,         validate-if-schema,
 *               entities, datatype-normalization, cdata-sections.
 *               This   forces   the  following  features  to  true:
 *               whitespace-in-element-content,            comments,
 *               namespaces.
 *               Other  features  are  not  changed unless explicitly
 *               specified in the description of the features.
 *               Note  that  querying  this  feature with getFeature
 *               returns   true  only  if  the  individual  features
 *               specified above are appropriately set.
 *
 *       false
 *               Setting infoset to false has no effect.
 *
 * "namespaces"
 *
 *       true
 *               [required] (default)
 *               Perform  the  namespace  processing  as  defined in
 *               [XML Namespaces].
 *
 *       false
 *               [optional]
 *               Do not perform the namespace processing.
 *
 * "namespace-declarations"
 *
 *       true
 *               [required] (default)
 *               Include namespace declaration attributes, specified
 *               or  defaulted  from  the  schema or the DTD, in the
 *               document.  See  also  the  section  Declaring
 *               Namespaces in [XML Namespaces].
 *
 *       false
 *               [required]
 *               Discard  all  namespace declaration attributes. The
 *               Namespace   prefixes  are  retained  even  if  this
 *               feature is set to false.
 *
 * "normalize-characters"
 *
 *       true
 *               [optional]
 *               Perform   the   W3C   Text   Normalization  of  the
 *               characters [CharModel] in the document.
 *
 *       false
 *               [required] (default)
 *               Do not perform character normalization.
 *
 * "split-cdata-sections"
 *
 *       true
 *               [required] (default)
 *               Split  CDATA  sections containing the CDATA section
 *               termination  marker  ']]>'. When a CDATA section is
 *               split a warning is issued.
 *
 *       false
 *               [required]
 *               Signal an error if a CDATASection contains an
 *               unrepresentable character.
 *
 * "validate"
 *
 *       true
 *               [optional]
 *               Require  the  validation against a schema (i.e. XML
 *               schema,  DTD,  any  other type or representation of
 *               schema)  of  the document as it is being normalized
 *               as defined by [XML 1.0]. If validation errors
 *               are  found,  or  no  schema  was  found,  the error
 *               handler  is  notified.  Note  also  that normalized
 *               values  will  not  be exposed to the schema in used
 *               unless the feature datatype-normalization is true.
 *
 *               Note:  validate-if-schema and validate are mutually
 *               exclusive, setting one of them to true will set the
 *               other one to false.
 *
 *       false
 *               [required] (default)
 *               Only  XML  1.0  non-validating  processing  must be
 *               done.  Note  that  validation might still happen if
 *               validate-if-schema is true.
 *
 * "validate-if-schema"
 *
 *       true
 *               [optional]
 *               Enable  validation  only  if  a declaration for the
 *               document  element  can  be  found (independently of
 *               where  it  is  found,  i.e. XML schema, DTD, or any
 *               other   type   or  representation  of  schema).  If
 *               validation  errors  are found, the error handler is
 *               notified. Note also that normalized values will not
 *               be exposed to the schema in used unless the feature
 *               datatype-normalization is true.
 *
 *               Note:  validate-if-schema and validate are mutually
 *               exclusive, setting one of them to true will set the
 *               other one to false.
 *
 *       false
 *               [required] (default)
 *               No  validation  should be performed if the document
 *               has  a  schema.  Note  that  validation  must still
 *               happen if validate is true.
 *
 * "element-content-whitespace"
 *
 *       true
 *               [required] (default)
 *               Keep all white spaces in the document.
 *
 *       false
 *               [optional]
 *               Discard   white  space  in  element  content  while
 *               normalizing.  The implementation is expected to use
 *               the isWhitespaceInElementContent flag on Text
 *               nodes to determine if a text node should be written
 *               out or not.
 *
 *   The  resolutions  of  entities  is done using Document.baseURI.
 *   However,  when  the  features "LS-Load" or "LS-Save" defined in
 *   [DOM  Level  3  Load  and  Save] are supported by the DOM
 *   implementation,  the  parameter  "entity-resolver"  can also be
 *   used  on  DOMConfiguration  objects  attached to Document
 *   nodes. If this parameter is set,
 *   Document.normalizeDocument   will   invoke   the   entity
 *   resolver instead of using Document.baseURI.
 */
class CDOM_EXPORT DOMConfiguration
{
protected:
    //-----------------------------------------------------------------------------------
    //  Constructor
    //-----------------------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMConfiguration() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMConfiguration(const DOMConfiguration &);
    DOMConfiguration & operator = (const DOMConfiguration &);
    //@}

public:

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    
    /** Set the value of a parameter. 
     * @param name The name of the parameter to set.
     * @param value The new value or null if the user wishes to unset the 
     * parameter. While the type of the value parameter is defined as 
     * <code>DOMUserData</code>, the object type must match the type defined
     * by the definition of the parameter. For example, if the parameter is 
     * "error-handler", the value must be of type <code>DOMErrorHandler</code>
     * @exception DOMException (NOT_SUPPORTED_ERR) Raised when the 
     * parameter name is recognized but the requested value cannot be set.
     * @exception DOMException (NOT_FOUND_ERR) Raised when the 
     * parameter name is not recognized.
     * @since DOM level 3
     **/
    virtual void setParameter(const XMLCh* name, const void* value) = 0;
    virtual void setParameter(const XMLCh* name, bool value) = 0;

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    /** Return the value of a parameter if known. 
     * @param name The name of the parameter.
     * @return The current object associated with the specified parameter or 
     * null if no object has been associated or if the parameter is not 
     * supported.
     * @exception DOMException (NOT_FOUND_ERR) Raised when the i
     * boolean parameter 
     * name is not recognized.
     * @since DOM level 3
     **/    
    virtual const void* getParameter(const XMLCh* name) const = 0;

                                        
    // -----------------------------------------------------------------------
    //  Query methods
    // -----------------------------------------------------------------------

    /** Check if setting a parameter to a specific value is supported. 
     * @param name The name of the parameter to check.
     * @param value An object. if null, the returned value is true.
     * @return true if the parameter could be successfully set to the specified 
     * value, or false if the parameter is not recognized or the requested value 
     * is not supported. This does not change the current value of the parameter 
     * itself.
     * @since DOM level 3
     **/
    virtual bool canSetParameter(const XMLCh* name, const void* value) const = 0;
    virtual bool canSetParameter(const XMLCh* name, bool value) const = 0;

    /**
     * The list of the parameters supported by this DOMConfiguration object and 
     * for which at least one value can be set by the application. 
     * Note that this list can also contain parameter names defined outside this specification.
     *
     * @return The list of parameters that can be used with setParameter/getParameter
     * @since DOM level 3
     **/
    virtual const DOMStringList* getParameterNames() const = 0;

    // -----------------------------------------------------------------------
    //  All constructors are hidden, just the destructor is available
    // -----------------------------------------------------------------------
    /** @name Destructor */
    //@{
    /**
     * Destructor
     *
     */
    virtual ~DOMConfiguration() {};
    //@}
};
	
XERCES_CPP_NAMESPACE_END

#endif 

/**
 * End of file DOMConfiguration.hpp
 */
