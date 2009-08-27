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
 *	  $Id$
 *
 *****************************************************************************/


#ifndef __SERVICECOMMON_H
#define __SERVICECOMMON_H

// just to be sure we load it
#include "config.h"

/*
This file contains some global definitions common to all services
*/

// TODO: Redo these constants to const int ....,

// -------------------------------------
// ----------- Service ID's  -----------
// -------------------------------------
#define __SERVICE_ID_BINARY 1
#define __SERVICE_ID_CONFIG 2
#define __SERVICE_ID_DATABASE 3
#define __SERVICE_ID_DRAW 4
#define __SERVICE_ID_GAME 5
#define __SERVICE_ID_GUI 6
#define __SERVICE_ID_INHERIT 7
#define __SERVICE_ID_INPUT 8
#define __SERVICE_ID_LIGHT 9
#define __SERVICE_ID_LINK 10
#define __SERVICE_ID_LOOP 11
#define __SERVICE_ID_MATERIAL 12
#define __SERVICE_ID_OBJECT 13
#define __SERVICE_ID_PHYSICS 14
#define __SERVICE_ID_PROPERTY 15
#define __SERVICE_ID_RENDER 16
#define __SERVICE_ID_SCRIPT 17
#define __SERVICE_ID_SIM 18
#define __SERVICE_ID_WORLDREP 19
#define __SERVICE_ID_ROOM 20

// --------------------------------------
// ---  Bitmasks for service masking  ---
// --------------------------------------

/*
To be used by service factories getMask() if the service want's to be a listener of particular service.
This does not do the registration of the listener itself, but will guarantee the service is constructed prior to data manipulations.
ServiceManager::createByMask() is used to create all the services given the mask before data manipulation takes place (typically in service's constructor).

The lower word of this 32bit unsigned integer is targetted at listener masking. The upper word is used for service ranges (core services, rendering services, game engine services)


I just hope we won't run out of mask bits! :D
*/

// All services constant
#define SERVICE_ALL 0x0FFFFFFFF

// Core services, fundamental
#define SERVICE_CORE 0x00010000
// Services related to rendering (dropping out these will cause no graphics to be displayed, no renderer window displayed)
#define SERVICE_RENDERER 0x00020000
// Services related to engine work (dropping these will result in the engine doing nothing)
#define SERVICE_ENGINE 0x00040000

/// Link listener mask
#define SERVICE_LINK_LISTENER 0x0001

/// Property listener mask
#define SERVICE_PROPERTY_LISTENER 0x0002

/// Object listener mask
#define SERVICE_OBJECT_LISTENER 0x0004

/// Database listener mask
#define SERVICE_DATABASE_LISTENER 0x0008

/// Input service listener mask
#define SERVICE_INPUT_LISTENER 0x0010



// ----------------------------------------
// --- Dark Database loading priorities ---
// ----------------------------------------
/*
The database service handles mission/gam/savegame database files (both loading and saving). Because there is some loading order needed to be done, here is the place to specify all the
priorities (not necessary unique) for the database service listeners
*/

// First, materials are loaded
#define DBP_MATERIAL 5
// then worldrep is loaded
#define DBP_WORLDREP 10
// then room database
#define DBP_ROOM 15
// Links and Properties are loaded in object system
// Object system loading order
#define DBP_OBJECT 20
// And script modules are initialized
#define DBP_SCRIPT 25


// ---------------------------------------------
// --- Loop modes id's and client priorities ---
// ---------------------------------------------
#define LOOPMODE_INPUT 1
#define LOOPMODE_RENDER 2
#define LOOPMODE_GUI 4


// Loop client ids and priorities
#define LOOPCLIENT_ID_INPUT 1
#define LOOPCLIENT_ID_RENDERER 2
#define LOOPCLIENT_ID_GUI 4

// Input first
#define LOOPCLIENT_PRIORITY_INPUT 1
// GUI some time before render
#define LOOPCLIENT_PRIORITY_GUI 900
// Renderer last
#define LOOPCLIENT_PRIORITY_RENDERER 1024

// TODO: Forward decl. anything service related here to shorten the compilation time (in combination with header inclusion removal...)

#endif
