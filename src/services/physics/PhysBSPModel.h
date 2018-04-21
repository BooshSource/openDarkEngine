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
 *	  $Id$
 *
 *****************************************************************************/

#ifndef __PHYSBSPMODEL_H
#define __PHYSBSPMODEL_H

#include "PhysModel.h"

namespace Opde {
/** @brief BSP based physics model - models that were handled via BSP in
 * original implementation.
 * @note There are two approaches to do collision detection with this - either
 * use the polygon soap directly or produce the needed model in manual loader.
 */
class PhysBSPModel : public PhysModel {
public:
    PhysBSPModel(int objid);
    ~PhysBSPModel();
};
} // namespace Opde

#endif
