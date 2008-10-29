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
 *		$Id$
 *
 *****************************************************************************/

#include "RenderedProperty.h"
#include "RenderService.h"


namespace Opde {
	RenderedProperty::RenderedProperty(RenderService* rs, PropertyService* owner, const std::string& name, 
			const std::string& chunk_name, std::string inheritorName) : 
		ActiveProperty(owner, name, chunk_name,  inheritorName), mOwner(rs) {};

	EntityInfo* RenderedProperty::getEntityInfo(int oid) {
		EntityInfo* ei = mOwner->_getEntityInfo(oid);
		assert(ei);

		if (ei == NULL)
			OPDE_EXCEPT("EntityInfo not found for object", "RenderedProperty::getEntityInfo");

		return ei;
	};
};
