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


#ifndef __DRAWSERVICE_H
#define __DRAWSERVICE_H

#include "config.h"

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "SharedPtr.h"
#include "DrawSheet.h"
#include "ManualFonFileLoader.h"
#include "Array.h"
#include "RenderedImage.h"

#include <OgreViewport.h>
#include <OgreRenderQueueListener.h>

#include <stack>

namespace Opde {
	// Forward decl.
	class TextureAtlas;
	class FontDrawSource;

	/** @brief Draw Service - 2D rendering service.
	 * @author volca
	 * @note some parts written by Patryn as well
	*/
	class OPDELIB_EXPORT DrawService : public Service, public Ogre::RenderQueueListener {
		public:
			/// Constructor
			DrawService(ServiceManager *manager, const std::string& name);

			/// Destructor
			virtual ~DrawService();

			/** Creates a new DrawSheet and inserts it into internal map.
			 * @param sheetName a unique sheet name
			 * @return new DrawSheet if none of that name exists, or the currently existing if sheet with that name already found
			 * */
			DrawSheet* createSheet(const std::string& sheetName);

			/** Destroys the given sheet, and removes it from the internal map
			 * @param sheet The sheet to destroy
			 */
			void destroySheet(DrawSheet* sheet);

			/** Returns the sheet of the given name
			 * @param sheetName the name of the sheet to return
			 * @return The sheet pointer, or NULL if none of that name exists
			 */
			DrawSheet* getSheet(const std::string& sheetName) const;

			/** Sets the active (currently displayed) sheet.
			 * @param sheet The sheet to display (or none if the parameter is NULL)
			 */
			void setActiveSheet(DrawSheet* sheet);

			/** Creates a DrawSource that represents a specified image.
			 * Also creates a material that is used to render the image.
			 * @param img The image name
			 * @return Shared ptr to the draw source usable for draw operations
			 */
			DrawSourcePtr createDrawSource(const std::string& img, const std::string& group);

			/** Creates a rendered image (e.g. a sprite)
			 * @param draw The image source for this operation
			 */
			RenderedImage* createRenderedImage(DrawSourcePtr& draw);

			/** Destroys the specified draw operation (any ancestor)
			 * @param dop The draw operation to destroy
			 */
			void destroyDrawOperation(DrawOperation* dop);

			/** Atlas factory. Generates a new texture atlas usable for storing numerous different images - which are then combined into a single draw call.
			 * @return New atlas pointer
			 */
			TextureAtlas* createAtlas();

			/** Destroys the given instance of texture atlas.
			 * @param atlas The atlas to be destroyed
			 */
			void destroyAtlas(TextureAtlas* atlas);

			/** Converts the given coordinates to the screen space x,y,z coordinates
			 */
			Ogre::Vector3 convertToScreenSpace(int x, int y, int z);

			/** Converts the given depth to screen-space
			 * @param z the depth in 0 - MAX_Z_VALUE range
			 * @return Real number describing the depth
			 */
			Ogre::Real getConvertedDepth(int z);

			/// Queues an atlas for rebuilding (on render queue started event)
			void _queueAtlasForRebuild(TextureAtlas* atlas);

			static const int MAX_Z_VALUE;

			/// Render queue listener related
			void renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation);

			/// Render queue listener related
			void renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation);

			/** Font loading interface. It is capable of loading .fon files only at the moment.
			 * @note To supply additional settings, use the
			 */
			FontDrawSource* loadFont(TextureAtlas* atlas, const std::string& name, const std::string& group);

			/** supplies the palette info - needed for 8Bit palletized font color loads. The specified palette is then used
			 * in further font loading operations.
			 */
			void setFontPalette(Ogre::ManualFonFileLoader::PaletteType paltype, const Ogre::String& fname = "");

		protected:
			// Service related:
			bool init();
			void bootstrapFinished();
			void shutdown();
			
			/// Loads the LG's fon file and populates the given font instance with it's glyphs 
			void loadFonFile(const std::string& name, const std::string& group, FontDrawSource* fon);

			DrawOperation::ID getNewDrawOperationID();

			/// Rebuilds all queued atlasses (so those can be used for rendering)
			void rebuildAtlases();

			typedef std::map<std::string, DrawSheet*> SheetMap;
			typedef std::stack<size_t> IDStack;
			typedef SimpleArray<DrawOperation*> DrawOperationArray;
			typedef std::map<DrawSource::ID, TextureAtlas*> TextureAtlasMap;
			typedef std::set<TextureAtlas*> AtlasSet;

			SheetMap mSheetMap;
			DrawSheet* mActiveSheet;
			DrawOperation::ID mDrawOpID;
			DrawSource::ID mDrawSourceID; // draw sources in atlas have the same ID
			IDStack mFreeIDs;
			DrawOperationArray mDrawOperations;

			Ogre::RenderSystem* mRenderSystem;

			Ogre::Real mXTextelOffset;
			Ogre::Real mYTextelOffset;

			Ogre::Viewport* mViewport;

			Ogre::SceneManager* mSceneManager;

			AtlasSet mAtlasesForRebuild;
			
			static const RGBAQuad msMonoPalette[2];
			RGBAQuad* mCurrentPalette; 
	};

	/// Shared pointer to the draw service
	typedef shared_ptr<DrawService> DrawServicePtr;


	/// Factory for the DrawService objects
	class OPDELIB_EXPORT DrawServiceFactory : public ServiceFactory {
		public:
			DrawServiceFactory();
			~DrawServiceFactory() {};

			/** Creates a GameService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

			virtual const uint getMask();
		private:
			static std::string mName;
	};
}


#endif
