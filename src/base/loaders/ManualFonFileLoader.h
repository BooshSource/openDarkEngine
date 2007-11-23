/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2007 openDarkEngine team
 *	  Includes code based on Thief Font Converter by Tom N Harris <telliamed@whoopdedo.cjb.net>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 * http://www.gnu.org/copyleft/lesser.txt.
 *****************************************************************************/

#ifndef __MANUALFONFILELOADER_H
#define __MANUALFONFILELOADER_H

#include <OgreResourceManager.h>
#include <OgreFont.h>
#include "File.h"
#include "FonFormat.h"

using namespace Opde; // For the Opde::File

namespace Ogre {

    /** ManualResourceLoader for BIN meshes. Used to load BIN meshes as Ogre::Mesh instances */
    class ManualFonFileLoader : public ManualResourceLoader {

	private:
			CharInfo *Chars;

			RGBQUAD* ReadPalette(StdFile *FilePointer);
			int FontToImage(MemoryFile* MemFile, String PaletteFileName);
			int WriteImage(StdFile* BitmapFile, RGBQUAD *ColorTable, char **RowPointers, int ImageSize);
			char** ManualFonFileLoader::ReadFont(MemoryFile* MemFile, int *ResultingSize, int *ResultingColor);			

        public:
            ManualFonFileLoader();
            virtual ~ManualFonFileLoader();

            virtual void loadResource(Resource* resource);
    };

}

#endif	//__MANUALFONFILELOADER_H
