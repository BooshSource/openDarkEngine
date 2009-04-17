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
 *
 *	  $Id$
 *
 *****************************************************************************/

// This source code is a fixed and maintained version of the Null render system written by xyzzy@ogre3d.org forum

#include "stdafx.h"
#include "NullRenderWindow.h"

namespace Ogre {

	unsigned int NULLRenderWindow::mWindowHandle = 1;

	NULLRenderWindow::NULLRenderWindow() 
	{
		mIsFullScreen = false;
		mIsExternal = false;
		mSizing = false;
		mClosed = false;
	}

	NULLRenderWindow::~NULLRenderWindow()
	{
		destroy();
	}


	void NULLRenderWindow::create(const String& name, unsigned int width, unsigned int height,
		bool fullScreen, const NameValuePairList *miscParams)
	{
		mWindowHandle++;

	}

	void NULLRenderWindow::destroy()
	{
		
	}

	void NULLRenderWindow::reposition(int top, int left)
	{
	}

	void NULLRenderWindow::resize(unsigned int width, unsigned int height)
	{
	}

	//-----------------------------------------------------------------------------
	void NULLRenderWindow::update(bool swap)
	{
		RenderWindow::update(swap);
	}

}
