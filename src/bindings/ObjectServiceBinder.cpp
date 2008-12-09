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

#include "bindings.h"
#include "ObjectServiceBinder.h"

namespace Opde
{

	namespace Python
	{

		// -------------------- Object Service --------------------
		char* ObjectServiceBinder::msName = "ObjectService";

		// ------------------------------------------
		PyTypeObject ObjectServiceBinder::msType =
		{
			PyObject_HEAD_INIT(&PyType_Type)
			0,
			"Opde.Services.ObjectService",		// char *tp_name; */
			sizeof(ObjectServiceBinder::Object), // int tp_basicsize; */
			0,									// int tp_itemsize;       /* not used much */
			ObjectServiceBinder::dealloc,		// destructor tp_dealloc; */
			0,									// printfunc  tp_print;   */
			ObjectServiceBinder::getattr,		// getattrfunc  tp_getattr; /* __getattr__ */
			0,   								// setattrfunc  tp_setattr;  /* __setattr__ */
			0,									// cmpfunc  tp_compare;  /* __cmp__ */
			0,									// reprfunc  tp_repr;    /* __repr__ */
			0,									// PyNumberMethods *tp_as_number; */
			0,									// PySequenceMethods *tp_as_sequence; */
			0,									// PyMappingMethods *tp_as_mapping; */
			0,									// hashfunc tp_hash;     /* __hash__ */
			0,									// ternaryfunc tp_call;  /* __call__ */
			0,									// reprfunc tp_str;      /* __str__ */
			0,									// getattrofunc tp_getattro; */
			0,									// setattrofunc tp_setattro; */
			0,									// PyBufferProcs *tp_as_buffer; */
			0,									// long tp_flags; */
			0,									// char *tp_doc;  */
			0,									// traverseproc tp_traverse; */
			0,									// inquiry tp_clear; */
			0,									// richcmpfunc tp_richcompare; */
			0,									// long tp_weaklistoffset; */
			0,									// getiterfunc tp_iter; */
			0,									// iternextfunc tp_iternext; */
			msMethods,							// struct PyMethodDef *tp_methods; */
			0,									// struct memberlist *tp_members; */
			0,									// struct getsetlist *tp_getset; */
		};

		// ------------------------------------------
		PyMethodDef ObjectServiceBinder::msMethods[] =
		{
			{"create", objectCreate, METH_VARARGS}, // method named differently to avoid confusion with PyObject* constructor
			{"beginCreate", beginCreate, METH_VARARGS},
			{"endCreate", endCreate, METH_VARARGS},
			{"exists", exists, METH_VARARGS},
			{"position", position, METH_VARARGS},
			{"orientation", orientation, METH_VARARGS},
			{"getName", getName, METH_VARARGS},
			{"setName", setName, METH_VARARGS},
			{"named", named, METH_VARARGS},
			{"teleport", teleport, METH_VARARGS},
			{"addMetaProperty", addMetaProperty, METH_VARARGS},
			{"removeMetaProperty", removeMetaProperty, METH_VARARGS},
			{"hasMetaProperty", hasMetaProperty, METH_VARARGS},
		    {NULL, NULL},
		};

		// ------------------------------------------
		PyObject* ObjectServiceBinder::objectCreate(PyObject* self, PyObject* args) {
			Object* o = python_cast<Object*>(self, &msType);

			// param: archetype object to inherit from
			int archetype;

			if (PyArg_ParseTuple(args, "i", &archetype)) {
				int newid;

			    try {
                    newid = o->mInstance->create(archetype);
			    } catch (BasicException& e) {
			        PyErr_Format(PyExc_IOError, "Exception catched while trying to create object : %s", e.getDetails().c_str());
			        return NULL;
			    }

				return PyInt_FromLong(newid);
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a integer (archetype object id) argument!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* ObjectServiceBinder::beginCreate(PyObject* self, PyObject* args) {
			Object* o = python_cast<Object*>(self, &msType);

			// param: archetype object to inherit from
			int archetype;

			if (PyArg_ParseTuple(args, "i", &archetype)) {
				int newid;

			    try {
                    newid = o->mInstance->beginCreate(archetype);
			    } catch (BasicException& e) {
			        PyErr_Format(PyExc_IOError, "Exception catched while trying to beginCreate object : %s", e.getDetails().c_str());
			        return NULL;
			    }

				return PyInt_FromLong(newid);
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a integer (archetype object id) argument!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* ObjectServiceBinder::endCreate(PyObject* self, PyObject* args) {
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);

			// param: archetype object to inherit from
			int objid;

			if (PyArg_ParseTuple(args, "i", &objid)) {
			    try {
                    o->mInstance->endCreate(objid);
			    } catch (BasicException& e) {
			        PyErr_Format(PyExc_IOError, "Exception catched while trying to endCreate object : %s", e.getDetails().c_str());
			        return NULL;
			    }

				result = Py_None;
				Py_INCREF(result);
				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a integer (object id) argument!");
				return NULL;
			}

		}

		// ------------------------------------------
		PyObject* ObjectServiceBinder::exists(PyObject* self, PyObject* args) {
			Object* o = python_cast<Object*>(self, &msType);

			// param: The object ID to test for existence
			int objid;

			if (PyArg_ParseTuple(args, "i", &objid)) {
			    bool res = o->mInstance->exists(objid);

			    PyObject* ret = res ? Py_True : Py_False;
				Py_INCREF(ret);
				return ret;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a integer (object id) argument!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* ObjectServiceBinder::position(PyObject* self, PyObject* args) {
			PyErr_SetString(PyExc_NotImplementedError, "Not Implemented yet!");
			return NULL;
		}

		// ------------------------------------------
		PyObject* ObjectServiceBinder::orientation(PyObject* self, PyObject* args) {
			PyErr_SetString(PyExc_NotImplementedError, "Not Implemented yet!");
			return NULL;
		}

		// ------------------------------------------
		PyObject* ObjectServiceBinder::getName(PyObject* self, PyObject* args) {
			Object* o = python_cast<Object*>(self, &msType);

			// param: The object ID to get name for
			int objid;

			if (PyArg_ParseTuple(args, "i", &objid)) {
			    std::string name = o->mInstance->getName(objid);

				return PyString_FromString(name.c_str());
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a integer (object id) argument!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* ObjectServiceBinder::setName(PyObject* self, PyObject* args) {
			PyObject *result = NULL;
			Object* o = python_cast<Object*>(self, &msType);

			// param: The object ID to set name for
			int objid;
			char* name;

			if (PyArg_ParseTuple(args, "is", &objid, &name)) {
			    o->mInstance->setName(objid, name);

				result = Py_None;
				Py_INCREF(result);
				return result;
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a integer and string arguments!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* ObjectServiceBinder::named(PyObject* self, PyObject* args) {
			Object* o = python_cast<Object*>(self, &msType);

			// param: The object ID to set name for
			char* name;

			if (PyArg_ParseTuple(args, "s", &name)) {
				int objid = o->mInstance->named(name);

				return PyInt_FromLong(objid);
			} else {
				// Invalid parameters
				PyErr_SetString(PyExc_TypeError, "Expected a string argument!");
				return NULL;
			}
		}

		// ------------------------------------------
		PyObject* ObjectServiceBinder::teleport(PyObject* self, PyObject* args) {
			PyErr_SetString(PyExc_NotImplementedError, "Not Implemented yet!");
			return NULL;
		}

		// ------------------------------------------
		PyObject* ObjectServiceBinder::addMetaProperty(PyObject* self, PyObject* args) {
			PyErr_SetString(PyExc_NotImplementedError, "Not Implemented yet!");
			return NULL;
		}

		// ------------------------------------------
		PyObject* ObjectServiceBinder::removeMetaProperty(PyObject* self, PyObject* args) {
			PyErr_SetString(PyExc_NotImplementedError, "Not Implemented yet!");
			return NULL;
		}

		// ------------------------------------------
		PyObject* ObjectServiceBinder::hasMetaProperty(PyObject* self, PyObject* args) {
			PyErr_SetString(PyExc_NotImplementedError, "Not Implemented yet!");
			return NULL;
		}


		// ------------------------------------------
		PyObject* ObjectServiceBinder::getattr(PyObject *self, char *name)
		{
			return Py_FindMethod(msMethods, self, name);
		}

		// ------------------------------------------
		PyObject* ObjectServiceBinder::create()
		{
			Object* object = construct(&msType);

			if (object != NULL)
			{
				object->mInstance = static_pointer_cast<ObjectService>(ServiceManager::getSingleton().getService(msName));
			}
			return (PyObject *)object;
		}

		// ------------------------------------------
		void ObjectServiceBinder::init(PyObject* module) {
			publishType(module, &msType, msName);
		}
	}

} // namespace Opde

