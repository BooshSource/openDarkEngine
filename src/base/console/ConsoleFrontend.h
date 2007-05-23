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
 
 
#ifndef __CONSOLEFRONTEND_H
#define __CONSOLEFRONTEND_H

#include <OgreRoot.h>
#include <OgreOverlayManager.h>
#include <OgreOverlayElement.h>

#include <OIS.h>

#include "ConsoleBackend.h"



namespace Opde {
	
	class ConsoleFrontend {
                public:
                        ConsoleFrontend();
			~ConsoleFrontend();
			
			/// Injects an ois keyboard event into the console
			void injectKeyPress(const OIS::KeyEvent &e);
			
			void setActive(bool active);
			
			inline bool isActive() const { return mIsActive; };
			 
			/** Frame update method. Call every time frame update happens */
			void update(int timems);
			
		protected:
			bool mIsActive;
			
			Ogre::Root *mRoot;
			Ogre::OverlayManager *mOverlayMgr;
			Ogre::Overlay* mConsoleOverlay;
			
			Ogre::OverlayElement* mCommandLine;
			Ogre::OverlayElement* mConsoleText;
			
			Ogre::String mCommand;
			
			ConsoleBackend* mConsoleBackend;
	};
	
}

#endif
