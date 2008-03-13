/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *	   $Id$
 *
 *****************************************************************************/

#ifndef __PYTHONSTRUCT_H
#define __PYTHONSTRUCT_H

#include "compat.h"
#include "Python.h"
#include "bindings.h"
#include "Callback.h"
#include "logger.h"
#include "SharedPtr.h"

namespace Opde 
{
	namespace Python {
		
		
		/** Templatized struct binder (no field writing supported). Exposes Type T as a python object with attributes. 
		* @note Every descendant needs to do at least three things: Initialize the msName and fill the msNameToAttr using method field,
		* and call the static init method created for the filling operation from somewhere.
		*/
		template<typename T> class PythonStruct {
			public:
				static PyObject* create(const T& t) {
					Object* object;
					object = PyObject_New(Object, &msType);
					
					if (object != NULL) {
						object->mInstance = new T(t);
					}
					
					return (PyObject *)object;
				};

			protected:
                static void dealloc(PyObject* self) {
					Object* o = reinterpret_cast<Object*>(self);
					
					delete o->mInstance;
					
					PyObject_Del(self);
				}
				
				static PyObject* getattr(PyObject* self, char* attrname) {
					PyObject* result = NULL;
					Object* object = python_cast<Object*>(self, &msType);
			
					typename NameToAttr::iterator it = msNameToAttr.find(attrname);
					
					if (it != msNameToAttr.end()) {
						result = (*it->second)(object->mInstance);
					} else
						PyErr_Format(PyExc_AttributeError, "Invalid attribute name encountered: %s", attrname);
				
					return result;
				}
			   
                static PyObject* repr(PyObject *self) {
                    return PyString_FromFormat("<Struct:%s at %p>", msName, self);
                }
				
				struct FieldDefBase {
					virtual PyObject* operator()(const T* var) const = 0;
					
					virtual char* getType() const = 0;
					
					virtual ~FieldDefBase() {};
				};
				
				template<typename F> struct FieldDef : public FieldDefBase {
				    typedef F T::*MemberPointer;
   					typedef TypeInfo<F> MemberType;
				    
					MemberPointer field;
					MemberType type;
					
					FieldDef(MemberPointer f) : field(f), type() {};
					
					virtual PyObject* operator()(const T* var) const {
						return type.toPyObject(var->*field);
					}
					
					virtual char* getType() const {
					    return type.typeName;
					}
				};
				
				typedef shared_ptr<FieldDefBase> FieldDefBasePtr;
				
				typedef std::map< std::string, FieldDefBasePtr > NameToAttr;

				static NameToAttr msNameToAttr;

				// Field definition statics:
				template<typename FT> static void field(const char* name, FT T::* fieldPtr) {
					FieldDefBasePtr fd = new FieldDef<FT>(fieldPtr);
					
					msNameToAttr.insert(std::make_pair(name, fd));
				}
				
				// Needs to be filled prior to usage		
				static char* msName;

				typedef ObjectBase<T*> Object;
				
				static PyTypeObject msType;
		};

        template<typename T> char* PythonStruct<T>::msName = "PythonStruct::InvalidType!";

		// Static member initialization
		template<typename T> typename PythonStruct<T>::NameToAttr PythonStruct<T>::msNameToAttr;
		
		template<typename T> PyTypeObject PythonStruct<T>::msType = {
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			msName,                   /* char *tp_name; */
			sizeof(Object),      /* int tp_basicsize; */
			0,                        /* int tp_itemsize;       /* not used much */
			dealloc,   /* destructor tp_dealloc; */
			0,                                    /* printfunc  tp_print;   */
			getattr,  /* getattrfunc  tp_getattr; /* __getattr__ */
			0,   					  /* setattrfunc  tp_setattr;  /* __setattr__ */
			0,				          /* cmpfunc  tp_compare;  /* __cmp__ */
			repr,		              /* reprfunc  tp_repr;    /* __repr__ */
		};
	}
}

#endif
