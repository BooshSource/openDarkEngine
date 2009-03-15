/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
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


#ifndef __FILEGROUP_H
#define __FILEGROUP_H

#include "config.h"

#include "OpdeException.h"
#include "File.h"
#include "SharedPtr.h"
#include "darkdb.h"

namespace Opde {
	//------------------------ group of files
	/** File group stored inside a single file. This is an abstract class for file group manipulation. For implementation see DarkFileGroup.
	* The loaded file group does not modify the source file in any way. That means all operations are safe and do not destroy/modify original
	file. */
	class OPDELIB_EXPORT FileGroup {
		public:
			/** File source FileGroup constructor. Loads the chunk list from a given file. */
			FileGroup(FilePtr& source);

			/** Empty FileGroup constructor. Initializes an empty file group */
			FileGroup();

			/** Destructor.
			* @note Does not write changes. Call write to do that. */
			virtual ~FileGroup();

			/** Returns true if this file group contains the given file */
			virtual bool hasFile(const std::string& name) const = 0;

			/** Get stored file handle
			* @note This function does not return a clone to the file, rather always the file itself. Usage at two places simultaneously will cause an unexpected seek behavior
			* @return Will return FilePtr seeked at the beginning of the file */
			virtual FilePtr getFile(const std::string& name) = 0;

			/** Get the file header (Name, version info) */
			virtual const DarkDBChunkHeader& getFileHeader(const std::string& name) = 0;

			/** Creates a file named 'name' and returns a handle to it.
			* @return newly created file
			* @note Old file is disposed if existed
			* @note The previous file is returned if read only (May be null) */
			virtual FilePtr createFile(const std::string& name, uint32_t ver_maj, uint32_t ver_min) = 0;

			/** Deletes a certain file from the group */
			virtual void deleteFile(const std::string& name) = 0;

			/** Write the current state of the FileGroup to the file dest.
			@warning Do not use the source file as destination. The source file data are read on demand, so owerwriting the source is not a good idea
			@note the above limitation should not cause any complications, as all the standard writing is done into an empty file group */
			virtual void write(FilePtr& dest) = 0;

			/** Get the File name of this database, or empty string if no underlying file is used */
			std::string getName();

			/** Chunk duo - it's header and the file that contains the data */
			struct Chunk {
				DarkDBChunkHeader header;
				FilePtr file;
			};

			typedef std::map<std::string, Chunk> ChunkMap;

			typedef ChunkMap::iterator iterator;
			typedef ChunkMap::const_iterator const_iterator;

			/** Iterator's begin for file iteration */
			virtual const_iterator begin() const = 0;

			/** Iterator's end for file iteration */
			virtual const_iterator end() const = 0;
		protected:
			/** Source file, if not created empty */
			FilePtr mSrcFile;
	};

    /// Shared pointer FileGroup instance
    typedef shared_ptr< FileGroup > FileGroupPtr;

	//------------------------ DarkDatabase FileGroup

	/** File group implemented on LG's MIS/COW/GAM/SAV type files.
	* This class exposes the chunks of that format as separate files, which can be replaced, deleted or added. The new database
	of chunks can be written to a new file. This allows modifications to be done on a database, leaving the chunks in an original form
	if not touched. The order of the chunks is not guaranteed to stay equal. This is not a problem for DarkEngine. */
	class OPDELIB_EXPORT DarkFileGroup : public FileGroup {
		public:
			/** @copydoc FileGroup::FileGroup(FilePtr&) */
			DarkFileGroup(FilePtr& source);

			/** @copydoc FileGroup::FileGroup() */
			DarkFileGroup();

			/** @copydoc FileGroup::~FileGroup() */
			virtual ~DarkFileGroup();

			/** @copydoc FileGroup::hasFile() */
			virtual bool hasFile(const std::string& name) const;

			/** @copydoc FileGroup::getFileHeader() */
			virtual const DarkDBChunkHeader& getFileHeader(const std::string& name);

			/** @copydoc FileGroup::getFile() */
			virtual FilePtr getFile(const std::string& name);

			/** @copydoc FileGroup::createFile() */
			virtual FilePtr createFile(const std::string& name, uint32_t ver_maj, uint32_t ver_min);

			/** @copydoc FileGroup::deleteFile() */
			virtual void deleteFile(const std::string& name);

			/** @copydoc FileGroup::write() */
			virtual void write(FilePtr& dest);

			/** @copydoc FileGroup::begin() */
			virtual const_iterator begin() const;

			/** @copydoc FileGroup::end() */
			virtual const_iterator end() const;
		protected:
			/** Initialize the mFiles from the given file, throw an error if something goes wrong
			@throw Opde::FileException if the source file contains errors
			*/
			void _initSource();

			/** Platform safe inventory reader
			* @param inventory Pre-allocated inventory array to read to
			* @param count the count of inventory items to read */
			void readInventory(DarkDBInvItem* inventory, uint count);

			/** Platform safe inventory writer
			* @param inventory Pre-allocated inventory array to write
			* @param count the count of inventory items to write */
			void writeInventory(FilePtr& dest, DarkDBInvItem* inventory, uint count);

			/** Platform safe chunk header reader
			* @param hdr The header to read
			*/
			void readChunkHeader(DarkDBChunkHeader* hdr);

			/** Platform safe chunk header writer
			* @param hdr The header to written
			*/
			void writeChunkHeader(FilePtr& dest, const DarkDBChunkHeader& hdr);

			/** File (Chunk) map */
			ChunkMap mFiles;

			/** Source File header */
			DarkDBHeader mHeader;
	};
}

#endif
