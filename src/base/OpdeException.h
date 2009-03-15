/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
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
 *
 *
 *		$Id$
 *
 *****************************************************************************/
 
 
#ifndef __OPDEEXCEPTION_H
#define __OPDEEXCEPTION_H

#include "config.h"

#include <stdexcept>
#include <string>

namespace Opde {
	
	/** A usage - simplifying macro */
	#define OPDE_EXCEPT(desc, src) throw( Opde::BasicException(desc, src, __FILE__, __LINE__) )
	
	/** @brief A standard OPDE exception
	*
	* This is a basic OPDE exception. Based largely on the Ogre's exception code
	* For simple usage, use the OPDE_EXCEPT macro */
	class OPDELIB_EXPORT BasicException : public std::exception {
		protected:
			std::string description;
			std::string source;
			std::string fileName;
		
			std::string details; // Cached version of the exception data
			long lineNum;
		public:
			/** Constructor. */
			BasicException(const std::string& desc, const std::string& src, const char* file = NULL, long line = -1);
		
			~BasicException() throw();
		
			/** Gets the details about the exception that happened */
			const std::string& getDetails();
		
		
	};
	
	class IndexOutOfBoundsException : public BasicException {
		public:
			IndexOutOfBoundsException(const std::string& txt, const char* file = NULL, long line = -1)
				: BasicException(txt, "", file, line) {};
	};
	
	#define OPDE_ARRAY_EXCEPT(txt) throw( Opde::IndexOutOfBoundsException(txt, __FILE__, __LINE__) )
	
}

#endif
