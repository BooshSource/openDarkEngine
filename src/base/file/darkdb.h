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
 
// Dark Database structural parts. Taken from Telliamed's DarkUtils (thanks Telliamed!)
 
#ifndef __DARKDB_H
#define __DARKDB_H

#include "integers.h"

#pragma pack(push, 1)

namespace Opde {
	/*
	* Header of Dark Engine's tag-file database.
	*/
	typedef struct DarkDBHeader
	{
		uint32_t	inv_offset;		// Offset to inventory TOC from top of header
		uint32_t	zero;
		uint32_t	one;
		uint8_t		zeros[256];
		uint32_t	dead_beef;		// 0xEFBEADDE (damn little-endian)
	} DarkDBHeader;
	
	/*
	* Item in chunk index.
	*/
	typedef struct DarkDBInvItem
	{
		char		name[12];
		uint32_t	offset;
		uint32_t	length;
	} DarkDBInvItem;
	
	/*
	* Universal chunk header.
	*/
	typedef struct DarkDBChunkHeader
	{
		char	name[12];
		uint32_t	version_high;
		uint32_t	version_low;
		uint32_t	zero;
	} DarkDBChunkHeader;
 };

#pragma pack(pop)

#endif
