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

// ---  Bitmasks for service masking  ---
/* 
To be used by service factories getMask() if the service want's to be a listener of particular service.
This does not do the registration of the listener itself, but will guarantee the service is constructed prior to data manipulations.
ServiceManager::createByMask() is used to create all the services given the mask before data manipulation takes place (typically in service's constructor).
*/

/// Link listener mask
#define SERVICE_LINK_LISTENER 0x0001

/// Property listener mask
#define SERVICE_PROPERTY_LISTENER 0x0002

/// Object listener mask
#define SERVICE_OBJECT_LISTENER 0x0004

namespace Opde {

}


#endif
