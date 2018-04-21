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

#include "RenderTypeProperty.h"
#include "RenderService.h"
#include "SingleFieldDataStorage.h"
#include "property/PropertyService.h"
#include "EntityInfo.h"

namespace Opde {

/*--------------------------------------------------------*/
/*-------------------- RenderTypeProperty ----------------*/
/*--------------------------------------------------------*/
RenderTypeProperty::RenderTypeProperty(RenderService *rs,
                                       PropertyService *owner)
    : RenderedProperty(rs, owner, "RenderType", "RenderTyp", "always"),
      mEnum(new Enumeration("RenderType", Variant::DV_UINT, false))
{
    mEnum->insert("Normal", RENDER_TYPE_NORMAL);
    mEnum->insert("Not Rendered", RENDER_TYPE_NOT_RENDERED);
    mEnum->insert("No Lightmap", RENDER_TYPE_NO_LIGHTMAP);
    mEnum->insert("Editor Only", RENDER_TYPE_EDITOR_ONLY);

    mPropertyStorage = DataStoragePtr(new UIntDataStorage(mEnum.get()));

    // TODO: Check the version
    setChunkVersions(2, 4);

    mSceneMgr = rs->getSceneManager();
};

// --------------------------------------------------------------------------
RenderTypeProperty::~RenderTypeProperty(void) {}

// --------------------------------------------------------------------------
void RenderTypeProperty::addProperty(int oid) {
    Variant val;

    if (!get(oid, "", val))
        OPDE_EXCEPT("Property not defined for object.",
                    "RenderTypeProperty::addProperty");

    setRenderType(oid, val.toUInt());
};

// --------------------------------------------------------------------------
void RenderTypeProperty::removeProperty(int oid) {
    // reinit to true - the object's default
    setRenderType(oid, 0);
};

// --------------------------------------------------------------------------
void RenderTypeProperty::setPropertySource(int oid, int effid) {
    // re-read the property
    addProperty(oid);
};

// --------------------------------------------------------------------------
void RenderTypeProperty::valueChanged(int oid, const std::string &field,
                                      const Variant &value) {
    // just call the setter
    setRenderType(oid, value.toUInt());
};

// --------------------------------------------------------------------------
void RenderTypeProperty::setRenderType(int oid, uint32_t renderType) {
    EntityInfo *ei = getEntityInfo(oid);
    ei->setRenderType(renderType);
};
}; // namespace Opde
