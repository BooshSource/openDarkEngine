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
 *****************************************************************************/


#ifndef __SERVICECOMMON_H
#define __SERVICECOMMON_H

/*
This file contains some global definitions common to all services
*/

// --------------------------------------
// ---  Bitmasks for service masking  ---
// --------------------------------------

/*
To be used by service factories getMask() if the service want's to be a listener of particular service.
This does not do the registration of the listener itself, but will guarantee the service is constructed prior to data manipulations.
ServiceManager::createByMask() is used to create all the services given the mask before data manipulation takes place (typically in service's constructor).
*/

// I just hope we won't run out of mask bits! :D

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

// First, worldrep is loadded
#define DBP_WORLDREP 5
// Then come the link database
#define DBP_LINK 10
// Properties are loaded (with already defined links, so inheritance refreshes take place)
#define DBP_PROPERTY 15
// Then objects are loaded
#define DBP_OBJECT 20
// And script modules are initialized
#define DBP_SCRIPT 25

#endif
