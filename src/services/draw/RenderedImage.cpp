/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2009 openDarkEngine team
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

#include "RenderedImage.h"
#include "DrawBuffer.h"
#include "DrawCommon.h"
#include "DrawService.h"
#include "DrawSheet.h"

using namespace Ogre;

namespace Opde {

/*----------------------------------------------------*/
/*-------------------- RenderedImage -----------------*/
/*----------------------------------------------------*/
RenderedImage::RenderedImage(DrawService *owner, DrawOperation::ID id,
                             const DrawSourcePtr &ds)
    : DrawOperation(owner, id), mDrawSource(ds) {
    mDrawQuad.texCoords.left = ds->transformX(0);
    mDrawQuad.texCoords.right = ds->transformX(1.0f);
    mDrawQuad.texCoords.top = ds->transformY(0);
    mDrawQuad.texCoords.bottom = ds->transformY(1.0f);

    mDrawQuad.color = ColourValue(1.0f, 1.0f, 1.0f);

    mInClip = true;

    // to be sure we are up-to-date
    _markDirty();
}

//------------------------------------------------------
RenderedImage::~RenderedImage() {
    // nothing
}

//------------------------------------------------------
void RenderedImage::visitDrawBuffer(DrawBuffer *db) {
    // are we in the clip area?
    if (mInClip)
        db->_queueDrawQuad(&mDrawQuad);
}

//------------------------------------------------------
DrawSourceBasePtr RenderedImage::getDrawSourceBase() {
    return static_pointer_cast<DrawSourceBase>(mDrawSource);
}

//------------------------------------------------------
void RenderedImage::setDrawSource(const DrawSourcePtr &nsrc) {
    DrawSourcePtr olds = mDrawSource;
    mDrawSource = nsrc;

    _sourceChanged(olds);
}

//------------------------------------------------------
void RenderedImage::_rebuild() {
    const PixelSize &ps = mDrawSource->getPixelSize();

    mDrawQuad.texCoords.left = mDrawSource->transformX(0);
    mDrawQuad.texCoords.right = mDrawSource->transformX(1.0f);
    mDrawQuad.texCoords.top = mDrawSource->transformY(0);
    mDrawQuad.texCoords.bottom = mDrawSource->transformY(1.0f);

    mDrawQuad.positions.left =
        mActiveSheet->convertToScreenSpaceX(mPosition.first);
    mDrawQuad.positions.right =
        mActiveSheet->convertToScreenSpaceX(mPosition.first + ps.width);
    mDrawQuad.positions.top =
        mActiveSheet->convertToScreenSpaceY(mPosition.second);
    mDrawQuad.positions.bottom =
        mActiveSheet->convertToScreenSpaceY(mPosition.second + ps.height);
    mDrawQuad.depth = mActiveSheet->convertToScreenSpaceZ(mZOrder);

    mInClip = mClipOnScreen.clip(mDrawQuad);
}

} // namespace Opde
