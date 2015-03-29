#ifndef _PYOBJECTPTR_H
#define _PYOBJECTPTR_H

/*
 *  Copyright (C) 2015 Kevin B. Hendricks Stratford, ON, Canada 
 *  Much Simplified Version adapted from the PythonQt project.
 *  Original license and info provided below:
 * 
 *  Copyright (C) 2010 MeVis Medical Solutions AG All Rights Reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  Further, this software is distributed without any warranty that it is
 *  free of the rightful claim of any third person regarding infringement
 *  or the like.  Any license provided herein, whether implied or
 *  otherwise, applies only to this software file.  Patent licenses, if
 *  any, provided herein do not apply to combinations of this program with
 *  other software, or any other product whatsoever.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Contact information: MeVis Medical Solutions AG, Universitaetsallee 29,
 *  28359 Bremen, Germany or:
 *
 *  http://www.mevis.de
 *
 * file    PythonQtObjectPtr.h
 * author  Florian Link
 * date    2006-05
 */

#include <Python.h>
#include <QVariant>
#include <QMetaType>

//! a smart pointer that stores a PyObject pointer and that handles reference counting automatically
class PyObjectPtr
{
public:
  PyObjectPtr():_object(NULL) {}

  PyObjectPtr(const PyObjectPtr &p)
  :_object(NULL) {
    setObject(p.object());
  }

  // If the given variant holds a PythonQtObjectPtr, extract the value from it 
  // and hold onto the reference. This results in an increment of the reference count.
  PyObjectPtr(const QVariant& variant):_object(NULL) {
      fromVariant(variant);
  }

  PyObjectPtr(PyObject* o);
  
  ~PyObjectPtr();
  
  // If the given variant holds a PyObjectPtr, extract the value from it and hold 
  // onto the reference. This results in an increment of the reference count.
  bool fromVariant(const QVariant& variant);

  PyObjectPtr &operator=(const PyObjectPtr &p) {
    setObject(p.object());
    return *this;
  }

  PyObjectPtr &operator=(PyObject* o) {
    setObject(o);
    return *this;
  }

  PyObjectPtr &operator=(const QVariant& variant) {
      fromVariant(variant);
      return *this;
  }

  bool operator==( const PyObjectPtr &p ) const {
    return object() == p.object();
  }

  bool operator!= ( const PyObjectPtr& p ) const {
    return !( *this == p );
  }

  bool operator==( PyObject* p ) const {
    return object() == p;
  }

  bool operator!= ( PyObject* p ) const {
    return object() != p;
  }

  bool isNull() const { return !object(); }

  PyObject* operator->() const { return object(); }

  PyObject& operator*() const { return *( object() ); }

  operator PyObject*() const { return object(); }

  //! sets the object and passes the ownership (stealing the reference, in Python slang)
  void setNewRef(PyObject* o);
  
  PyObject* object() const {
    return _object;
  }

protected:

  void setObject(PyObject* o);
  
private:
  PyObject* _object;
};

// register it to the meta type system
Q_DECLARE_METATYPE(PyObjectPtr)

#endif
