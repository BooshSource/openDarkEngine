/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
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
#include "Root.h"

#include <Python.h>
#include <OgreVector3.h>

// Exception guard for the python code. use this as first/last lines of the binding function to have it prepared for exceptions
#define __PYTHON_EXCEPTION_GUARD_BEGIN_ try {
#define __PYTHON_EXCEPTION_GUARD_END_ } catch (BasicException& e) { \
			        PyErr_Format(PyExc_RuntimeError, "C++ side exception (%s:%u) : %s", __FILE__, __LINE__, e.getDetails().c_str()); \
			        return NULL; \
			    }

namespace Opde {
	namespace Python {
	    // Type converters
	    enum VariableType { VT_INVALID, VT_BOOL, VT_INT, VT_LONG, VT_FLOAT, VT_CHARPTR, VT_STRING, VT_CUSTOM_TYPE };

		template<typename T> struct TypeInfoBase {
			const char* typeName;
			VariableType type;
			
			TypeInfoBase(const char* tn, VariableType tt) : typeName(tn), type(tt) {};
		};

		template<typename T> struct TypeInfo : public TypeInfoBase<T> {
			TypeInfo() : TypeInfoBase<T>("invalid", VT_INVALID) {};

			// TypeInfo(const char* tname, VariableType tt) : TypeInfoBase(tname, tt) {};

			PyObject* toPyObject(T val) const {
				PyErr_SetString(PyExc_TypeError, "Binding error: Type has no conversion or TypeInfo specified!");
				return NULL;
			}
		};


		template<> struct TypeInfo<bool> : TypeInfoBase<bool> {
			TypeInfo() : TypeInfoBase<bool>("bool", VT_BOOL) {};

			PyObject* toPyObject(bool val) const {
				PyObject* res = val ? Py_True : Py_False;

				Py_INCREF(res);

				return res;
			}
		};

		template<> struct TypeInfo<int> : TypeInfoBase<int> {
			TypeInfo() : TypeInfoBase<int>("int", VT_INT) {};

			PyObject* toPyObject(int val) const {
				return PyLong_FromLong(val);
			}
		};

		template<> struct TypeInfo<long> : TypeInfoBase<long> {
			TypeInfo() : TypeInfoBase<long>("long", VT_LONG) {};

			PyObject* toPyObject(long val) const {
				return PyLong_FromLong(val);
			}
		};


		template<> struct TypeInfo<float> : TypeInfoBase<float> {
			TypeInfo() : TypeInfoBase<float>("float", VT_FLOAT) {};

			PyObject* toPyObject(float val) const {
				return PyFloat_FromDouble(val);
			}
		};

		template<> struct TypeInfo<std::string>  : TypeInfoBase<std::string> {
			TypeInfo() : TypeInfoBase<std::string>("std::string", VT_STRING) {};

			PyObject* toPyObject(const std::string& val) const {
				return PyString_FromString(val.c_str());
			}
		};

		// Global utilities - object conversion and such
		PyObject* DVariantToPyObject(const DVariant& inst);
		DVariant PyObjectToDVariant(PyObject* obj);

		// DVariant type info
		template<> struct TypeInfo<DVariant> : TypeInfoBase<DVariant> {
			TypeInfo() : TypeInfoBase<DVariant>("DVariant", VT_CUSTOM_TYPE) {};

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

		/// Common ancestor for all python published C types
		class PythonPublishedType {
			protected:
				/** Publishes the type as a member of a specified module
					@param containter The module to publish the type in
					@param type The python type object to publish
					@param name The name of the type to use
				*/
				static void publishType(PyObject* containter, PyTypeObject* type, const char* name);
		};

		/// A template that binds sharedptr typed classes
		template <typename T> class shared_ptr_binder : public PythonPublishedType {
			public:
				/// A python object type
				typedef ObjectBase<T> Object;
				
			protected:
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
		template <typename T> class class_ptr_binder : public PythonPublishedType {
			protected:
				static PyTypeObject msType;
				
			public:
				/// A python object type
				typedef ObjectBase<T*> Object;
			

			protected:
				/// A sort-of constructor method. To be used to create a new NULL Object*
				static Object* construct(PyTypeObject* type) {
					Object* object;

					object = PyObject_New(Object, type);

					if (object != NULL) {
						// Here, tidy!
						object->mInstance = NULL;
					}

					return object;
				}

				/// Destructor for the python object. Safely decreases the reference to the shared_ptr. To be used in msType
				static void dealloc(PyObject *self) {
					// Object* o = reinterpret_cast<Object*>(self);
					// delete the object
					PyObject_Del(self);
				}
		};

	};

    /** Central class for python bindings. Call PythonLanguage::Init() to prepare python environment */
    class OPDELIB_EXPORT PythonLanguage {
    	protected:
			static Opde::Root* msRoot;

    	public:
			/** Initializes python lang and all the bindings */
			static void init(int argc, char **argv);
			
			/** Initializes the Opde module itself, without the interpretter */
			static void initModule();
			
			/** Finalizes python lang */
			static void term();

			/** Runs a script loaded in memory on a given address */
			static void runScriptPtr(const char* ptr);

			/** Runs a script from a file */
			static bool runScript(const char* fname);

			/// Python side Root singleton handler
			static PyObject* createRoot(PyObject *self, PyObject* args);

			/// Python side Root singleton handler
			static PyObject* getRoot(PyObject *self, PyObject* args);
    };
}


#endif
