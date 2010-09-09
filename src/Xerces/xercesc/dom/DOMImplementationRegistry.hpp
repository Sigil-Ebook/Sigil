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
 * $Id: DOMImplementationRegistry.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMIMPLEMENTATIONREGISTRY_HPP)
#define XERCESC_INCLUDE_GUARD_DOMIMPLEMENTATIONREGISTRY_HPP

 /**
  * This class holds the list of registered DOMImplementations.  Implementation
  * or application can register DOMImplementationSource to the registry, and
  * then can query DOMImplementation based on a list of requested features.
  *
  * <p>This provides an application with an implementation independent starting
  * point.
  *
  * @see DOMImplementation
  * @see DOMImplementationList
  * @see DOMImplementationSource
  * @since DOM Level 3
  */

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class DOMImplementation;
class DOMImplementationSource;
class DOMImplementationList;

class CDOM_EXPORT DOMImplementationRegistry
{
public:
    // -----------------------------------------------------------------------
    //  Static DOMImplementationRegistry interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 3 */
    //@{
    /**
     * Return the first registered implementation that has the desired features,
     * or null if none is found.
     *
     * @param features A string that specifies which features are required.
     *                 This is a space separated list in which each feature is
     *                 specified by its name optionally followed by a space
     *                 and a version number.
     *                 This is something like: "XML 1.0 Traversal 2.0"
     * @return An implementation that has the desired features, or
     *   <code>null</code> if this source has none.
     * @since DOM Level 3
     */
    static DOMImplementation* getDOMImplementation(const XMLCh* features);

    /**
     * Return the list of registered implementation that have the desired features.
     *
     * @param features A string that specifies which features are required.
     *                 This is a space separated list in which each feature is
     *                 specified by its name optionally followed by a space
     *                 and a version number.
     *                 This is something like: "XML 1.0 Traversal 2.0"
     * @return A DOMImplementationList object that contains the DOMImplementation
     *         that have the desired features
     * @since DOM Level 3
     */
    static DOMImplementationList* getDOMImplementationList(const XMLCh* features);

    /**
     * Register an implementation.
     *
     * @param source   A DOMImplementation Source object to be added to the registry.
     *                 The registry does NOT adopt the source object.  Users still own it.
     * @since DOM Level 3
     */
    static void addSource(DOMImplementationSource* source);
    //@}

private:
    DOMImplementationRegistry();
};

XERCES_CPP_NAMESPACE_END

#endif
