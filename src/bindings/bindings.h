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

#ifndef __BINDINGS_H
#define __BINDINGS_H

#include "DVariant.h"

#include <Python.h>
#include <OgreVector3.h>

namespace Opde {
	namespace Python {
	    // Type converters
	    enum VariableType { VT_INVALID, VT_BOOL, VT_INT, VT_LONG, VT_FLOAT, VT_CHARPTR, VT_STRING, VT_CUSTOM_TYPE };
		
		struct TypeInfoBase {
		};
		
		template<typename T> struct TypeInfo : public TypeInfoBase {
			VariableType type;
			char* typeName;
			
			TypeInfo() : typeName("invalid"), type(VT_INVALID) {};
			
			PyObject* toPyObject(T val) const {
				PyErr_SetString(PyExc_TypeError, "Binding error: Type has no conversion or TypeInfo specified!");
				return NULL;
			}
		};
		
		
		template<> struct TypeInfo<bool> {
			VariableType type;
			char* typeName;
			
			TypeInfo() : typeName("bool"), type(VT_BOOL) {};
			
			PyObject* toPyObject(bool val) const {
				PyObject* res = val ? Py_True : Py_False;
				
				Py_INCREF(res);
				
				return res;
			}
		};
		
		template<> struct TypeInfo<int> {
			VariableType type;
			char* typeName;
			
			TypeInfo() : typeName("int"), type(VT_INT) {};
			
			PyObject* toPyObject(int val) const {
				return PyLong_FromLong(val);
			}
		};
		
		template<> struct TypeInfo<long> {
			VariableType type;
			char* typeName;
			
			TypeInfo() : typeName("long"), type(VT_LONG) {};
			
			PyObject* toPyObject(long val) const {
				return PyLong_FromLong(val);
			}
		};
				
		template<> struct TypeInfo<std::string> {
			VariableType type;
			char* typeName;
			
			TypeInfo() : typeName("std::string"), type(VT_STRING) {};
			
			PyObject* toPyObject(const std::string& val) const {
				return PyString_FromString(val.c_str());
			}
		};
		
		// Global utilities - object conversion and such
		PyObject* DVariantToPyObject(const DVariant& inst);
		DVariant PyObjectToDVariant(PyObject* obj);
		
		// DVariant type info
		template<> struct TypeInfo<DVariant> {
			VariableType type;
			char* typeName;
			
			TypeInfo() : typeName("DVariant"), type(VT_CUSTOM_TYPE) {};
			
			PyObject* toPyObject(const DVariant& val) const {
				return DVariantToPyObject(val);
			}
		};
		
		/// Template definition of a Python instance holding a single object
		template<typename T> struct ObjectBase {
			PyObject_HEAD
			T mInstance;
		};

		/// helper function to get type from Object
		template<typename T> T python_cast(PyObject* obj, PyTypeObject* type) {
			// More generic: Checks for ancestors as well
			assert(PyObject_TypeCheck(obj, type));

			return reinterpret_cast< T >(obj);
		}

		/// A template that binds sharedptr typed classes
		template <typename T> class shared_ptr_binder {
			protected:
				/// A python object type
				typedef ObjectBase<T> Object;

				/// A sort-of constructor method. To be used to create a new NULL Object*
				static Object* construct(PyTypeObject* type) {
					Object* object;

					object = PyObject_New(Object, type);

					// At this point, the shared_ptr instance in the object is invalid (I.E. Allocated, but not initialized).
					// If we try to assign into it, we'll segfault. Because of that, we have to do placed new to initialize the object
					if (object != NULL) {
						// Here, tidy!
						::new (&object->mInstance) T();
					}

					return object;
				}

				/// Destructor for the python object. Safely decreases the reference to the shared_ptr. To be used in msType
				static void dealloc(PyObject *self) {
					// cast the object to T::Object
					Object* o = reinterpret_cast<Object*>(self);

					// Decreases the shared_ptr counter
					o->mInstance.setNull();

					// Call the destructor to clean up
					(&o->mInstance)->~T();

					// Finally delete the object
					PyObject_Del(self);
				}
		};
		
		/// A template that binds a pointer to class (simmilar to shared_ptr_binder, but no special handling is used)
		template <typename T> class class_ptr_binder {
			protected:
				/// A python object type
				typedef ObjectBase<T*> Object;

				/// A sort-of constructor method. To be used to create a new NULL Object*
				static Object* construct(PyTypeObject* type) {
					Object* object;

					object = PyObject_New(Object, type);

					// At this point, the shared_ptr instance in the object is invalid (I.E. Allocated, but not initialized).
					// If we try to assign into it, we'll segfault. Because of that, we have to do placed new to initialize the object
					if (object != NULL) {
						// Here, tidy!
						object->mInstance = NULL;
					}

					return object;
				}

				/// Destructor for the python object. Safely decreases the reference to the shared_ptr. To be used in msType
				static void dealloc(PyObject *self) {
					// cast the object to LinkServiceBinder::Object
					Object* o = reinterpret_cast<Object*>(self);

					// delete the object
					PyObject_Del(self);
				}
		};

	};

    /** Central class for python bindings. Call PythonLanguage::Init() to prepare python environment */
    class PythonLanguage {
    	public:
			/** Initializes python lang and all the bindings */
			static void init();

			/** Finalizes python lang */
			static void term();

			/** Runs a script loaded in memory on a given address */
			static void runScriptPtr(const char* ptr);
    };
}


#endif
