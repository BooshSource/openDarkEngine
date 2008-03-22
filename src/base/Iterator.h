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

#ifndef __ITERATOR_H
#define __ITERATOR_H

namespace Opde {

    /** A java-like iterator approach */
    template <typename T> class Iterator {
	public:
	    virtual T& next() = 0;
	    virtual bool end() const = 0;
    };


    /** A const, java-like iterator approach */
    template <typename T> class ConstIterator {
	public:
	    virtual const T& next() = 0;
	    virtual bool end() const = 0;
    };
    
  	/// Class representing a const string iterator
	typedef ConstIterator< std::string > StringIterator;

	/// Shared pointer instance to const string iterator
	typedef shared_ptr< StringIterator > StringIteratorPtr;
	

}

#endif
