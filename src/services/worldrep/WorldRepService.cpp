/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
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
 
#include <Ogre.h>
#include "WorldRepService.h"
#include "WRTypes.h"
#include "OgreBspNode.h"
#include "OgreDarkSceneManager.h"
#include "integers.h"
#include "OpdeException.h"
#include "WRCommon.h" 
#include "logger.h"
#include "File.h"

using namespace Ogre;
 
namespace Opde {
	// Implementation of the WorldRep service
	WorldRepService::WorldRepService(ServiceManager *manager) : Service(manager) {
		// Get a reference to the sceneManager. We can get DarkSceneManager directly because of the format of the data we load (BSP/Portals)
		mRoot = Ogre::Root::getSingletonPtr();
		mSceneMgr = dynamic_cast<DarkSceneManager *>(mRoot->getSceneManager("DarkSceneManager"));
		
		mCells = NULL;
		mExtraPlanes = NULL;
		
		mAtlas = NULL;
		
		// Some standard image format extensions to try, when constructing the material manually
		mTextureExtensions.insert(String(".tga"));
		mTextureExtensions.insert(String(".jpg"));
		mTextureExtensions.insert(String(".jpeg"));
		mTextureExtensions.insert(String(".pcx"));
		mTextureExtensions.insert(String(".png"));
		
		// try to create lightmap resource group used for lightmap storage
		try {
			ResourceGroupManager::getSingleton().createResourceGroup(TEMPTEXTURE_RESOURCE_GROUP);
		} catch (Exception &e) {
			LOG_ERROR("Cannot create temporary texture/materials resource group '%s'. Exception : %s", 
				 TEMPTEXTURE_RESOURCE_GROUP, e.getDescription().c_str());
		}
		
		mTextures = NULL;
		mFamilies = NULL;
	}

	WorldRepService::~WorldRepService() {
		clearData();
	}
			
	// ---- The ServiceInterface mathods ---
	void WorldRepService::loadFromDarkDatabase(FileGroup *db) {
		// We'll load the level geometry, and initialize the scenemanager with it.
		// To get sure we are not putting two levels together
		mSceneMgr->clearScene();
		
		// Initialize materials here:
		loadMaterials(db);
		
		int lightSize = 1;
		
		File* wrChunk;
		
		if (db->hasFile("WR")) {
			wrChunk = db->getFile("WR");
		} else if (db->hasFile("WRRGB")) {
			lightSize = 2;
			wrChunk = db->getFile("WRRGB");
		} else {
			// Still no data?
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, "Could not find WR nor WRRGB chunk...","WorldRepService::loadFromDarkDatabase");
		}
		
		loadFromChunk(wrChunk, lightSize);
		
		wrChunk->release();
		
		// --- Finally, set sky according to the SKY chunk
		setSkyBox(db);
	}
	
	
	void WorldRepService::setSkyBox(FileGroup *db) {
		File* skyChunk = db->getFile("SKYMODE");
		
		if (skyChunk != NULL) { // Thief1 sky. Thief2 has NewSky. Will need to make a custom scene node for this. 
			uint32_t skyMode;
			
			skyChunk->readElem(&skyMode, sizeof(skyMode), 1);
			
			if (skyMode == 0) {
				mSceneMgr->setSkyBox(true, "Skybox/daysky", 1000);
			} else {
				mSceneMgr->setSkyBox(true, "Skybox/nightsky", 1000);
				
			}
		}

		skyChunk->release();
	}
	
	
	void WorldRepService::saveToDarkDatabase(FileGroup *db) {
		// nothing. 
	}
	
	void WorldRepService::unload() {
		clearData();
	}
	
	void WorldRepService::clearData() {
		LOG_INFO("WorldRepService::clearData called");
		mSceneMgr->clearScene();
		
		if (mCells != NULL)
			delete[] mCells;
		
		if (mExtraPlanes != NULL)
			delete[] mExtraPlanes;
		
		if (mAtlas != NULL)
			delete mAtlas;
		
		// Unregister all the resources in the WorldResourceGroup, including unreloadable
		
		// if there is a LightMap or WrTextures resource group, clean up those...
		
		try {
			ResourceGroupManager::getSingleton().clearResourceGroup(TEMPTEXTURE_RESOURCE_GROUP);
		} catch (Exception &e) {
			LOG_INFO("Problem cleaning up the temporary resource groups. Exception : %s", 
				 TEMPTEXTURE_RESOURCE_GROUP, e.getDescription().c_str());
		}
		
		if (mFamilies != NULL)
			delete[] mFamilies;
		
		if (mTextures != NULL)
			delete[] mTextures;
		
		LOG_INFO("WorldRepService::clearData : finished cleaning up");
	}
	
	// ----------------------- The level loading methods follow
	void WorldRepService::loadFromChunk(File* wrChunk, int lightSize) {
		wr_hdr_t header;
		wrChunk->read(&header, sizeof(wr_hdr_t));
		
		SceneNode *rootSceneNode = mSceneMgr->getRootSceneNode();
		
		// If there is some scene already, clear it
		clearData();
		
		mNumCells = header.num_cells;
		
		mAtlas = new LightAtlasList();
		mCells = new WRCell[header.num_cells];
		mLeafNodes = new BspNode[header.num_cells];
		
		unsigned int idx;
		for (idx=0; idx < header.num_cells; idx++) {
			// Load one Cell
			mCells[idx].loadFromChunk(idx, wrChunk, lightSize);
		}
		
		// -- Load the extra planes
		wrChunk->read(&mExtraPlaneCount, sizeof(uint32_t));
		
		mExtraPlanes = new wr_plane_t[mExtraPlaneCount];
		wrChunk->read(mExtraPlanes, sizeof(wr_plane_t) * mExtraPlaneCount);
		
		LOG_INFO("Worldrep: queueing lightmaps");
		// -- Atlas the lightmaps
		for (idx=0; idx < header.num_cells; idx++) 
			mCells[idx].atlasLightMaps(mAtlas);
	
		
		LOG_INFO("Worldrep: Atlasing lightmaps");
		// Render the lmaps! This could be moved to a pre-run stage, as the mission difficulties alter the light states (and we do not know a s**t about that here)
		mAtlas->render();
		
		LOG_INFO("Worldrep: Atlasing Done");
		
		// Attach the leaf BspNodes to their cells
		for (idx=0; idx < header.num_cells; idx++) {
			mCells[idx].setBspNode(&mLeafNodes[idx]);
			mLeafNodes[idx].setCellNum(idx);
			mLeafNodes[idx].setOwner(mSceneMgr);
		}
		
		// --------------------------------------------------------------------------------
		// Now construct the static geometry
		
		// TODO: Finish this stuff:
		int totalVertices = 0;
		int totalIndices = 0;
		int totalFaceGroups = 0;
		
		for (idx=0; idx < header.num_cells; idx++) {
			totalVertices += mCells[idx].getVertexCount();
			totalIndices += mCells[idx].getIndexCount();
			totalFaceGroups += mCells[idx].getFaceCount();
		}
		
		// nowprepare the buffers with the knowledge of the total vbuf/idxbuf sizes
		mVertexData = new VertexData();

		/// Create vertex declaration
		VertexDeclaration* decl = mVertexData->vertexDeclaration;
		size_t offset = 0;
		decl->addElement(0, offset, VET_FLOAT3, VES_POSITION);
		offset += VertexElement::getTypeSize(VET_FLOAT3);
		decl->addElement(0, offset, VET_FLOAT3, VES_NORMAL);
		offset += VertexElement::getTypeSize(VET_FLOAT3);
		// do I need colour? (I sure do not use it)
		decl->addElement(0, offset, VET_COLOUR, VES_DIFFUSE);
		offset += VertexElement::getTypeSize(VET_COLOUR);
		decl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0); // txt
		offset += VertexElement::getTypeSize(VET_FLOAT2);
		decl->addElement(0, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 1); // lmap

		/// Create the vertex buffer, allow space for patches
		
		HardwareVertexBufferSharedPtr vbuf = HardwareBufferManager::getSingleton()
			.createVertexBuffer(sizeof(BspVertex), 
			totalVertices, 
			HardwareBuffer::HBU_STATIC_WRITE_ONLY);
		
		// Lock just the non-patch area for now
		BspVertex* pVert = static_cast<BspVertex*>(
			vbuf->lock(0, totalIndices * sizeof(BspVertex), HardwareBuffer::HBL_DISCARD) );

		// Index buffer, this is what we also need
		mIndexes.bind(new DefaultHardwareIndexBuffer(
			HardwareIndexBuffer::IT_32BIT, 
			totalIndices, 
			HardwareBuffer::HBU_DYNAMIC));
	
		// temporary index buffer, will be copied after filling
		unsigned int *srcIdx;
		srcIdx = new unsigned int[totalIndices];
		
		// face groups buffer
		mFaceGroups = new StaticFaceGroup[totalFaceGroups]; 
		
		int optimized = 0;
		
		// Actual vertex, index and face position
		int actVert = 0;
		int actIdx  = 0;
		int actFace = 0;
		
		// Build the static part of the geometry
		for (idx=0; idx < header.num_cells; idx++) {
			LOG_DEBUG("Worldrep: Building cell %d / %d geom.", idx, header.num_cells);
			
			optimized += mCells[idx].attachPortals(mCells);
			mCells[idx].buildStaticGeometry(&pVert[actVert], &srcIdx[actIdx], &mFaceGroups[actFace], actVert, actIdx, actFace);
			
			actVert += mCells[idx].getVertexCount();
			actIdx  += mCells[idx].getIndexCount();
			actFace += mCells[idx].getFaceCount();
			
		}
		
		// Did we do it right? Check the limits
		assert(totalVertices   == actVert);
		assert(totalIndices    == actIdx);
		assert(totalFaceGroups == actFace);
		
		LOG_INFO("Worldrep: Optimization removed %d vertices", optimized);
		
		// --------------------------------------------------------------------------------
		
		// -- Load and process the BSP tree
		uint32_t BspRows;
		wrChunk->read(&BspRows, sizeof(uint32_t));
		
		// Load the BSP, and construct it
		wr_BSP_node_t *Bsp = new wr_BSP_node_t[BspRows];
		wrChunk->read(Bsp, BspRows * sizeof(wr_BSP_node_t));
		
		// Create the BspTree
		createBSP(BspRows, Bsp);
		
		
		//TODO: Portal meshes need to be constructed. This will guarantee the other meshes can be attached if this one works ok
		// Hmm. Actually, it renders quite a lot of the meshes that should not be seen.
		for (idx=0; idx < header.num_cells; idx++) {
			mCells[idx].constructPortalMeshes(mSceneMgr);
		}
		//*/
		
		// --------------------------------------------------------------------------------
		// the final move - copy the index data to the hw idx buffer
		mIndexes->writeData(0, sizeof(unsigned int) * totalIndices, srcIdx);
		vbuf->unlock();
		
		// Setup binding
		mVertexData->vertexBufferBinding->setBinding(0, vbuf);
		
		// Set other data
		mVertexData->vertexStart = 0;
		mVertexData->vertexCount = totalVertices;
		
		// Set the static geometry buffers in the scene manager
		mSceneMgr->setStaticGeometry(mVertexData, mIndexes, mFaceGroups, totalIndices, totalFaceGroups);
		
		// We have done all we could, bringing the level data to the SceneManager. Now delete the used data
		LOG_DEBUG("Worldrep: Freeing temporary buffers");
		delete[] srcIdx; // and delete the source
		delete[] mCells;
		mCells = NULL;
		delete[] mExtraPlanes;
		mExtraPlanes = NULL;
		delete[] Bsp;
		LOG_DEBUG("Worldrep: Freeing done");
	}
	
	// ---------------------------------------------------------------------
	void WorldRepService::createBSP(unsigned int BspRows, wr_BSP_node_t *tree) {
		// BspNode *mRootNode = new BspNode[BspRows];
		
		/* Step one. Scan through the tree to find leaf and non-leaf rows. We already have a BspNode array of leaf nodes, 
		so we have to convert the row indexes to the no-gap indices of a table of non-leaf nodes only. With this. we can then proceed
		*/
		unsigned int nonleafs = 0;
		unsigned int leafs = 0;
		
		LOG_DEBUG("Worldrep: Preparing for BSP build");
		// the map of the non-leaf row indexes to the non-gap version of the array
		std::map<int, int> nonLeafMap;
		
		// the map of the row index to cell index
		std::map<int, int> leafMap;
		
		for (unsigned int i = 0; i < BspRows; i++) {
			wr_BSP_node_t *wr_node =  &tree[i];
			
			if (_BSP_FLAGS(wr_node) & 0x01) { // leaf node
				// 'Front' in this case contains the cell number
				leafMap.insert(std::make_pair(i,wr_node->front));
				leafs++;
			} else { // Non-leaf node
				nonLeafMap.insert(std::make_pair(i,nonleafs));
				nonleafs++;
			}
		}
		
		LOG_DEBUG("Worldrep: Finished preparing for BSP build - Non-Leaf's: %d", nonleafs);
		
		// Allocate for all the non-leaf members.
		mNonLeafNodes = new BspNode[nonleafs];
		
		LOG_DEBUG("Worldrep: Doing the BSP build");
		
		// Step two, fill the non-leaf rows now. 
		for (unsigned int i = 0; i < BspRows; i++) {
			// BspNode* node = &mRootNode[i];
			wr_BSP_node_t *wr_node =  &tree[i];
			
			// Set bounding box
			// node->mBounds.setMinimum(Vector3(&q3node->bbox[0]));
			// node->mBounds.setMaximum(Vector3(&q3node->bbox[3]));
			
			if (_BSP_FLAGS(wr_node) & 0x01) {
				// leaf node. Nothing to do...
				// node->setIsLeaf(true);
				// node->setSceneNode(mCellNodes[wr_node->front]);
				// Debugging cell number
				// mCellNodes[wr_node->front]->setCellNum(wr_node->front);
			} else {
				// find the BspNode for this index
				std::map<int, int>::const_iterator it = nonLeafMap.find(i);
				
				assert(it != nonLeafMap.end());
				
				BspNode* node = &mNonLeafNodes[it->second];
				
				node->setOwner(mSceneMgr);
				
				// set the split plane of the non-leaf node
				if (wr_node->cell < 0) {
					assert(wr_node->plane < mExtraPlaneCount);
					node->setSplitPlane(constructPlane(mExtraPlanes[wr_node->plane])); // Extra planes
				} else {
					assert(wr_node->cell < mNumCells);
					node->setSplitPlane(mCells[wr_node->cell].getPlane(wr_node->plane));
				}
								
				// node->mCellNum = i;
				
				BspNode* front = NULL;
				BspNode* back = NULL;
				
				// Set back pointer (and front pointer)
				if (wr_node->back != 0xFFFFFF) {
					assert(wr_node->back < BspRows);
					
					// If I do not find the index in the nonLeafMap, 
					// it is a leaf and will be in the leafMap
					
					std::map<int, int>::const_iterator it = nonLeafMap.find(wr_node->back);
					
					if (it != nonLeafMap.end()) {
						back = &mNonLeafNodes[it->second];
					} else { // search in the leafMap
						std::map<int, int>::const_iterator it = leafMap.find(wr_node->back);
						
						if (it != leafMap.end()) {
							assert(it->second < mNumCells);
							back = mCells[it->second].getBspNode();
						} else {
							OPDE_EXCEPT("Row index not found in leaf or non-leaf", "WorldRepService::createBSP");
						}
					}
				} 
				
				// set front pointer
				if (wr_node->front != 0xFFFFFF) {
					assert(wr_node->front < BspRows);
					
					std::map<int, int>::const_iterator it = nonLeafMap.find(wr_node->front);
					
					if (it != nonLeafMap.end()) {
						front = &mNonLeafNodes[it->second];
					} else { // search in the leafMap
						std::map<int, int>::const_iterator it = leafMap.find(wr_node->front);
						
						if (it != leafMap.end()) {
							front = mCells[it->second].getBspNode();
						} else {
							OPDE_EXCEPT("Row index not found in leaf or non-leaf", "WorldRepService::createBSP");
						}
					}
				}
		
					
				// 0x4 flag means inverted plane normal (I can't invert the plane, because this leads to errors)
				if (_BSP_FLAGS(wr_node) & 0x04) { // inverted plane normal, swap front n' back child
					node->setBackChild(front);
					node->setFrontChild(back);
				} else {
					node->setBackChild(back);
					node->setFrontChild(front);
				}
			}
		}
		
		LOG_DEBUG("Worldrep: Finished the BSP build");
		
		((mSceneMgr))->setBspTree(mNonLeafNodes, mNonLeafNodes, mLeafNodes, nonleafs, leafs);
		LOG_DEBUG("Worldrep: SceneManager's BSP set");
	}
	
	//-----------------------------------------------------------------------
	Ogre::Plane WorldRepService::constructPlane(wr_plane_t plane) {
		Vector3	normal(plane.normal.x, plane.normal.y, plane.normal.z);
		float dist = plane.d;
		Ogre::Plane oplane;
		oplane.normal = normal;
		oplane.d = dist;
		return oplane;
	}
	
	//-----------------------------------------------------------------------
	void WorldRepService::loadFlowTextures(FileGroup *db) {
		// Load the TXLIST chunk from the resource mission file.
		Opde::File* flow_tex = db->getFile("FLOW_TEX");
		
		if (flow_tex == NULL) {
			LOG_INFO("Flow chunk does not exist. Water materials may not be correctly displayed", "WorldRepService::loadFlowTextures");
			return;
		}
		
		// TODO: Exception handling on the chunk readout!
		// Okay, we are ready to map the arrays
		DarkDBChunkFLOW_TEX flows;
		int flow_count = flow_tex->size() / 32; // The record is 32 bytes long, this way we do not fail if the chunk is shorter
		
		try {
			// load
			flow_tex->read(&flows, flow_tex->size()); // To be sure we do not overlap
		} catch (Ogre::Exception& e) {
			flow_tex->release();
			
			// Connect the original exception to the printout:
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("Could not load flow textures : ") + e.getFullDescription(), "WorldRepService::loadFlowTextures");
		}
		
		
		// now try to load non-zero flow textures as materials
		for (int fnum = 0; fnum < flow_count; fnum++) {
			if (strlen(flows.flow[fnum].name) > 0) { // nonzero name, try to load
				// Construct the basic name of the material
				std::string matname("water/");
				matname += flows.flow[fnum].name;
				
				// try to find the texture definition. If found, clone to the @template + the in_texture/out_texture number
				if (MaterialManager::getSingleton().resourceExists(matname+"_in")) {
					MaterialPtr origMat = MaterialManager::getSingleton().getByName(matname+"_in");
				
					StringUtil::StrStreamType matName;
					matName << "@template" << flows.flow[fnum].in_texture;	
					
					if (MaterialManager::getSingleton().resourceExists(matName.str())) {
						MaterialManager::getSingleton().remove(matName.str());
					}
					
					MaterialPtr shadMat = origMat->clone(matName.str() );
					shadMat->load();
					LOG_INFO("Flow now defined : %s (template %s_in)", matName.str().c_str(), matname.c_str());
				} else {
					LOG_ERROR("Material not found : %s_in", matname.c_str());
				}
				
				// OUT
				if (MaterialManager::getSingleton().resourceExists(matname+"_out")) {
					MaterialPtr origMat = MaterialManager::getSingleton().getByName(matname+"_out");
					
					StringUtil::StrStreamType matName;
					matName << "@template" << flows.flow[fnum].out_texture;	
					
					if (MaterialManager::getSingleton().resourceExists(matName.str())) {
						MaterialManager::getSingleton().remove(matName.str());
					}
					
					MaterialPtr shadMat = origMat->clone(matName.str() );
					shadMat->load();
					LOG_INFO("Flow now defined : %s (template %s_in)", matName.str().c_str(), matname.c_str());
				} else {
					LOG_ERROR("Material not found : %s_out", matname.c_str());
				}
				
			}
		}
		
		// Release the file handle for flow tex
		flow_tex->release();
	}
	//-----------------------------------------------------------------------
	void WorldRepService::createStandardMaterial(std::string matName, std::string textureName, std::string resourceGroup) {
		Image tex;
		bool loaded = false; // indicates we were succesful finding the texture
		
		// Let's try the extensions from the extensions vector
		std::set< String >::iterator it = mTextureExtensions.begin();
		
		for (;it != mTextureExtensions.end(); it++) { // Try loading 
			try {
				tex.load(textureName + (*it), resourceGroup);
				
				TextureManager::getSingleton().loadImage(textureName, resourceGroup, tex, TEX_TYPE_2D, 0, 1.0f );
			
				loaded = true;
				
				break; // we got it!
			} catch (Ogre::Exception& e) {
				// Nothing. We are trying more extensions
			}
		}
		
		if (!loaded) // TODO: Find a replacement? I don't think so...
			LogManager::getSingleton().logMessage("Image " + textureName + " was not found, texture will be invalid!");
		
		
		// Construct a material out of this texture. We'll just clone the material upstairs to enable lmap-txture combinations
		MaterialPtr shadMat = MaterialManager::getSingleton().create(matName, resourceGroup);
		
		// TODO: there is something funky in the texture/material definition. That causes the textures to be overexposed.
		
		Pass *shadPass = shadMat->getTechnique(0)->getPass(0);
		
		// Texture unit state for the main texture...
		TextureUnitState* tus = shadPass->createTextureUnitState(textureName);
		
		// Set replace on all first layer textures for now
		tus->setColourOperation(LBO_REPLACE);
		tus->setTextureAddressingMode(TextureUnitState::TAM_WRAP);
		tus->setTextureCoordSet(0);
		tus->setTextureFiltering(TFO_BILINEAR);
		
		// Set culling mode to none
		shadMat->setCullingMode(CULL_NONE);
		
		// No dynamic lighting
		shadMat->setLightingEnabled(false);

		
		shadMat->load();
	}
	
	// ---------------------------------------------------------------------
	void WorldRepService::loadMaterials(FileGroup *db) {
		if (!db->hasFile("TXLIST"))
			throw Exception(Exception::ERR_ITEM_NOT_FOUND,
				"Mission file does not contain Texture list chunk (TXLIST)",
				"WorldRepService::loadMaterials");
		
		// Load the TXLIST chunk from the resource mission file.
		Opde::File* txtList = db->getFile("TXLIST");
		
		assert(txtList != false);
		
		// TODO: Exception handling on the chunk readout!
		// Okay, we are ready to map the arrays
		if (mFamilies != NULL)
			delete[] mFamilies;
		
		if (mTextures != NULL)
			delete[] mTextures;

		try {
			// Read the header...
			txtList->read(&mTxlistHeader, sizeof(DarkDBChunkTXLIST));
			
			// now read all the families
			
			// allocate the needed space
			mFamilies = new DarkDBTXLIST_fam[mTxlistHeader.fam_count];
			// load
			txtList->read(&(mFamilies[0]), sizeof(DarkDBTXLIST_fam) * mTxlistHeader.fam_count);
			
			// Now read the textures. Same as before
			
			// allocate the needed space
			mTextures = new DarkDBTXLIST_texture[mTxlistHeader.txt_count];
			// load texture names
			txtList->read(&(mTextures[0]), sizeof(DarkDBTXLIST_texture) * mTxlistHeader.txt_count);
		} catch (Ogre::Exception& e) {
			delete txtList;
			if (mFamilies != NULL)
				delete[] mFamilies;
			if (mTextures != NULL)
				delete[] mTextures;
			
			txtList->release();
			
			// Connect the original exception to the printout:
			OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR, String("Could not load texture list : ") + e.getFullDescription(), "BspLevel::loadMaterials");
		}
		
		// Okay, we are ready to load the materials now!
		
		// Our resource group
		String resourceGroup = ResourceGroupManager::getSingleton().getWorldResourceGroupName();
		
		createSkyHack(resourceGroup);

		// ------------- Following code loads the standard materials -------------------
		
		// Iterate through all materials, and load them (tries to load material as a script (named family/texture), and if it fails, 
		// it constructs one with the default settings, and tries a few extensions too for the image)

		for (unsigned int id = 1; id < mTxlistHeader.txt_count; id++) {
			// Try to find the family for the texture
			std::string path = getMaterialName(id);
			
			// Resulting material name
			StringUtil::StrStreamType matName;
			matName << "@template" << id;
			
			if (MaterialManager::getSingleton().resourceExists(matName.str())) // if the material is already defined
				// remove, as we have to redefine it
				MaterialManager::getSingleton().remove(matName.str());
			
			// See if the material is defined by a script. If so, clone it to be named @templateXXXX (XXXX = texture number)
			// We seek material named: familly/texture
			if (MaterialManager::getSingleton().resourceExists(path)) {
				LOG_INFO("loadMaterials: Found material definition for %s", path.c_str());
				MaterialPtr origMat = MaterialManager::getSingleton().getByName(path);
					
				MaterialPtr shadMat = origMat->clone(matName.str(), true, resourceGroup);
				
				shadMat->load();
			} else { // The material script was not found
				createStandardMaterial(matName.str(), path, resourceGroup);
			}
			// This is it. Material @templateXX created
		}
		
		txtList->release();
		
		// Initialize the flow textures (do this first so water specialisation will override)
		loadFlowTextures(db);
		

	}
	
	// ---------------------------------------------------------------------
	Ogre::SceneManager *WorldRepService::getSceneManager() {
		return mSceneMgr;
	}
	
	// ---------------------------------------------------------------------
	void WorldRepService::createSkyHack(Ogre::String resourceGroup) {
		// First, we'll create the sky materials, named SkyShader and WaterShader
		// The sky material does basically only write the Z value (no color write), and should be rendered prior to any other material
		// This is because we render sky(box/sphere/dome/plane) first, and we want it to be visible through the faces textured by this material
		std::string shaderName("SkyShader");
		
		if (!MaterialManager::getSingleton().resourceExists(shaderName)) {
			MaterialPtr SkyShader = MaterialManager::getSingleton().create(shaderName, resourceGroup);
			
			Pass *shadPass = SkyShader->getTechnique(0)->getPass(0);
			
			// No texture for this pass
			// Set the pass to be totally transparent - no color writing
			shadPass->setColourWriteEnabled(false);
			
			// Set culling mode to none
			shadPass->setCullingMode(CULL_NONE);
			
			// No dynamic lighting (Sky!)
			shadPass->setLightingEnabled(false);
			// ---- End of skyhack ----
		}
	}
	
	// ---------------------------------------------------------------------
	Ogre::String WorldRepService::getMaterialName(int mat_index) {
		std::string path = "";
		if ((mTextures[mat_index].fam != 0) && ((mTextures[mat_index].fam - 1) < (int)mTxlistHeader.fam_count)) {
			path = mFamilies[mTextures[mat_index].fam - 1].name;
			path += "/";
		}
		
		path += mTextures[mat_index].name; // TODO: Multiplatformness... is it solved by ogre?
		
		return path;
	}
	
	//-------------------------- Factory implementation
	WorldRepServiceFactory::WorldRepServiceFactory() { 
		ServiceName = "WorldRepService";
		
		ServiceManager::getSingleton().addServiceFactory(this);
	};
	
	Service* WorldRepServiceFactory::createInstance(ServiceManager* manager) {
		return new WorldRepService(manager);
	}
	
}
