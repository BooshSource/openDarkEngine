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


#ifndef LATLAS_H
#define LATLAS_H

#include <vector>
#include <map>
#include "OgreImage.h"
#include "OgreTextureManager.h"
#include "OgreVector2.h"
#include "OgreVector3.h"
#include "OgreHardwarePixelBuffer.h"
#include "ConsoleCommandListener.h"

// TODO: This is dated... We should just return Width and Height for lightmaps (separate), to let the tuning be simpler
// - this means some changes to the lmap allocation have to be made too

// size of lightmap atlas (both W and H)
#define ATLAS_WIDTH 512
#define ATLAS_HEIGHT 512
#define ATLAS_LMSIZE (ATLAS_WIDTH*ATLAS_LMSIZE)

namespace Opde {
	

	
typedef struct {
	int atlasnum;
	float u,v; // shift of the lmap in the atlas
	float su,sv; // size of the lmap in the atlas
} AtlasInfo;


/** Free space information storage - rectangular area in the lightmap (either used or free to use)
* Organized in a binary tree. 
* thanks to this article for a tip: http://www.blackpawn.com/texts/lightmaps/default.html
*/
class FreeSpaceInfo { 
	protected:
		int 	mMaxArea;
		bool 	mIsLeaf;
		
		FreeSpaceInfo* mChild[2];
		
		FreeSpaceInfo() {
			this->x = 0;
			this->y = 0;
			this->w = -1;
			this->h = -1;
			
			mMaxArea = 0;
			mIsLeaf = true;
			
			mChild[0] = NULL;
			mChild[1] = NULL;
		}
		
	public:
		int	x;
		int	y;
		int	w;
		int	h;
		
		

		FreeSpaceInfo(int x, int y, int w, int h) {
			this->x = x;
			this->y = y;
			this->w = w;
			this->h = h;
			
			mIsLeaf = true;
			mMaxArea = w * h;
			
			mChild[0] = NULL;
			mChild[1] = NULL;
		}
		
		~FreeSpaceInfo() {
			if (mChild[0] != NULL)
				delete mChild[0];
			
			if (mChild[1] != NULL)
				delete mChild[1];
		}
		
		bool Fits(int sw, int sh) const {
			if ((sw<=w) && (sh<=h))
				return true;
			
			return false;
		}
		
		int getArea() const {
			return w*h;
		}
		
		/** returns the maximal allocatable area of this node/leaf */
		int getMaxArea() const {
			return mMaxArea;
		}
		
		/** allocate a space in the free area. 
		* @return A free space rectangle of the requested space, or null if the space could not be allocated */
		FreeSpaceInfo* allocate(int sw, int sh) {
			if (!mIsLeaf) { // split node.
				int reqa = sw * sh;
				
				for (int i = 0; i < 2; i++) {
					FreeSpaceInfo* result = NULL;
					
					if (mChild[i] == NULL)
						continue;
					
					if (mChild[i]->getMaxArea() >= reqa)
						result = mChild[i]->allocate(sw, sh);
				
					if (result != NULL) { // allocation was ok
						// refresh the maximal area
						refreshMaxArea();
						
						return result;
					}
				} 
				
				return NULL; // no luck, sorry!
			} else {
				// we're the leaf node. Try to insert if possible
				if (!Fits(sw,sh))
					return NULL;
			
				// bottom will be created?
				if (sh<h) 
					mChild[0] = new FreeSpaceInfo(x,y+sh,w,h-sh);
							
				// right will be created?
				if (sw<w) 
					mChild[1] = new FreeSpaceInfo(x+sw,y,w-sw,sh);
				
				
				// modify this node to be non-leaf, as it was allocated
				mIsLeaf = false;
				
				w = sw;
				h = sh;
				
				// refresh the mMaxArea
				refreshMaxArea();
				
				return this;
			}
		}
		
		/** refreshes the maximal allocatable area for this node/leaf. 
		Non-leaf nodes get the maximum as the maximum area of the children */
		void refreshMaxArea() {
			if (mIsLeaf) {
				mMaxArea = w * h;
				return;
			}
			
			mMaxArea = -1;
			
			for (int j = 0; j < 2; j++) {
				if (mChild[j] == NULL)
					continue;
				
				if (mChild[j]->getMaxArea() > mMaxArea)
					mMaxArea = mChild[j]->getMaxArea();
			}
		}
		
		/** returns the maximal area of this node */
		int getLeafArea() {
			if (mIsLeaf) {
				return getArea();
			}
			
			int area = 0;
			
			for (int j = 0; j < 2; j++) {
				if (mChild[j] == NULL)
					continue;
				
				area += mChild[j]->getLeafArea();
			}
			
			return area;
		}
};

/** A structure representing a lightmap pixel */
typedef struct lmpixel {
	unsigned char R,G,B;
	
	/** Converts the RGB values to the A8R8G8B8 format 
	* \return uint32 holding the A8R8G8B8 version of the stored RGB */
	Ogre::uint32 ARGB() {
		return (R << 16) | (G << 8) | B;
	}
	
	lmpixel(unsigned char nR,unsigned char nG,unsigned char nB) {
		R = nR; G = nG; B = nB;
	}
	
	lmpixel() {
		R = 0; G = 0; B = 0;
	}
} lmpixel;


// Forward declaration
class LightAtlas;
	
/** A class representing a switchable lightmap. It holds one static lightmap, which can't be switched, and a set of lightmaps indexed by light number, 
* which can have their'e intensity modulated. The resulting lightmap is recalculated every time an actual chage happens. 
* Please use Ogre::LightAtlasList.setLightIntensity if you want to set an intensity to a certain light. Calling the method here would not refresh the lightmap texture. 
* @see LightAtlasList */
class LightMap {
		// So the atlasser is able to set uv && suv for us
		friend class LightAtlas;
			
		/** Information about the lightmap position in the atlas */
		FreeSpaceInfo* position;
	
		/** static lightmap */
		lmpixel* static_lmap;
	
		/** A map of the switchable lightmaps */
		std::map< int, lmpixel* > switchable_lmap;
		
		/** A map of the actual intensities */
		std::map< int, float > intensities;
	
		/** Lightmap's size in pixels */
		unsigned int sx, sy;
	
		/** Atlas holding the lightmap */
		LightAtlas *owner;
	
		/** Repositioning from 0-1 to the Atlas coords */
		Ogre::Vector2 uv; 
	
		/** Size of the lmap in the atlas*/
		Ogre::Vector2 suv;
		
	public:
		/** Constructor - takes the targetting freespaceinfo, size of the lightmap and initializes our buffer with the static lightmap.
		* This class will manage the unallocation of the lighymap as needed, just pass the pointer, it will deallocate the lmap in the destructor */
		LightMap(unsigned int sx, unsigned int sy, lmpixel *static_lightmap) {
			static_lmap = static_lightmap;
			
			this->sx = sx;
			this->sy = sy;
			
			position = NULL;
		}
		
		~LightMap() {
			if (static_lmap != NULL)
				delete[] static_lmap;
			
			for (std::map<int, lmpixel*>::iterator it = switchable_lmap.begin(); it != switchable_lmap.end(); it++)
				delete it->second;
			
			switchable_lmap.clear();
		}
	
		/** Set the targetting placement of the lightmap in the atlas _owner */
		void setPlacement(LightAtlas* _owner, FreeSpaceInfo* tgt);
		
		/** Gets an owner atlas of this lightmap
		* \return LightAtlas containing this light map */
		const LightAtlas* getOwner() {
			return owner;
		}
		
		/** Converts input UV coords to the Atlas coords */
		Ogre::Vector2 toAtlasCoords(Ogre::Vector2 in_uv) {
			in_uv = in_uv * suv; // dunno why, this one does not work written in_uv *= suv;
			in_uv += uv;
			
			return in_uv;
		}
		
		/** Helping static method. Prepares an RGB version of the given buffer containing v1 or v2 lightmap */
		static lmpixel* convert(char *data, int sx, int sy, int ver);
		
		/** Adds a switchable lightmap with identification id to the lightmap list (has to be of the same size). 
		* \param id the ID of the light the lightmaps belongs to.
		* \param data are the actual values converted to RGB. Unallocation is handled in destructor */
		void AddSwitchableLightmap(int id, lmpixel *data);
		
		/** The main intensity setting function */
		void setLightIntensity(int id, float intensity);
		
		/** Refreshes the texture's pixel buffer with our new settings*/
		void refresh();
		
		std::pair<int, int> getDimensions() const;
		
		/** Returns the atlas index */
		int getAtlasIndex();
};

/** Class holding one set of light maps. Uses a HardwarePixelBufferSharedPtr class for texture storage.
* This class is used for atlasing a light map set into one bigger lightmap texture containing the light maps.
* This accelerates rendering. */
class LightAtlas {
	private:
		/** The count of the stored light maps */
		int count;
	
		/** Global index of this atlas in the atlas list */
		int idx;
	
		/** Tecture pointer for this atlas */
		Ogre::TexturePtr ptex;
	
		/** The resulting pixel buffer */
		Ogre::HardwarePixelBufferSharedPtr atlas; 
	
		/** The name of the resulting resource */
		Ogre::StringUtil::StrStreamType name;
	
		/** A vector containing Free Space rectangles left in the Atlas */
		FreeSpaceInfo* freeSpace;
	
		/** Already inserted lightmaps - mainly for dealocation */
		std::vector < LightMap * > lightmaps;
	
		/** Light number to the LightMap set mapping - changing light intensity will iterate throught the set of lmaps, and regenerate */
		std::map<int, std::set<LightMap*> > lights;
		
		/** A buffer which holds a copy of the image data */
		lmpixel *copybuffer; // I keep the data of the lightmap here before I place them to Texture buffer.

		/** return an origin - XY coords for the next free space (for a lightmap sized w*h) */
		std::pair<FreeSpaceInfo, bool> getOrigin(int w, int h);
	
	public:
		LightAtlas(int idx);
		
		~LightAtlas();
		
		/** Adds a light map to this atlas. 
		* @param lmap Lightmap to be added 
		*/
		//  TODO: Deprecate destinfo
		bool addLightMap(LightMap *lmap);

		/** renders the prepared light map buffers into a texture - copies the pixel data to the 'atlas' */
		bool render();
		
		/** Updated the pixel buffer with a new version of the lightmap. This version converts the Vector3 floats to 0-255 range (checks limits). 
		* \warning Must be called after atlas locking, otherwise the program will crash ! */
		void updateLightMapBuffer(FreeSpaceInfo& fsi, lmpixel* rgb);
		
		/** Updated the pixel buffer with a new version of the lightmap. This version converts the Vector3 floats to 0-255 range (checks limits) 
		* \warning Must be called after atlas locking, otherwise the program will crash ! */
		void updateLightMapBuffer(FreeSpaceInfo& fsi, Ogre::Vector3* rgb);
		
		/** Register that animated light ID maps to the LightMap instance */
		void registerAnimLight(int id, LightMap* target);
		
		void setLightIntensity(int id, float intensity);
		
		/** Returns the Light Map Atlas order number */
		int getIndex();
		
		/** Returns the pixel count of unmapped area of this atlas */
		int getUnusedArea();
};


/** @brief A holder of a number of the light map atlases. 
*
* Use this class to work with the light map storage and light switching. */
class LightAtlasList : public ConsoleCommandListener {
	public:
		struct lightMapLess {
             		bool operator()(const LightMap* a, const LightMap* b) const;
         	};
		
	private:
		/** A list of the light map atlases */
		std::vector<LightAtlas *> list;
	
		/** Pre-render lightmap list*/
		typedef std::map<int, std::vector<LightMap*> > TextureLightMapQueue;
		TextureLightMapQueue mLightMapQueue;
		
		int mSplitCount; /// The split count of the same-texture lightmaps (e.g. the spread number. Higher number -> worse performance)
		
	protected:
		bool placeLightMap(LightMap* lmap);
		
	public:
		LightAtlasList();
	
		/** A destructor, unallocates all the previously allocated lightmaps */
		virtual ~LightAtlasList();
		
		/** Adds a light map holding place and a static lightmap. 
		* @note The U/V mapping of the lightmap is not valid until the atlas list is rendered */
		LightMap* addLightmap(int ver, int texture, char *buf, int w, int h, AtlasInfo &destinfo);

		/** get's the current count of the atlases stored. 
		* \return int Count of the atlases */
		int getCount();

		/** Prepares the Light Atlases to be used as textures. 
		* atlases all the lightmaps, in Texture, size orders (texture being major ordering rule) 
		* Then calls the LightAtlas.render method
		* @see Ogre::LightAtlas.render() */
		bool render();
	
		void setLightIntensity(int id, float value) {
			std::vector< LightAtlas* >::iterator atlases = list.begin();
			
			for (;atlases != list.end(); atlases++) {
				(*atlases)->setLightIntensity(id, value);
			}
		};
		
		virtual void commandExecuted(std::string command, std::string parameters);
};

} // namespace Ogre
#endif
