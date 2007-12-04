/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2007 openDarkEngine team
 *	  Contains code based on Thief Font Converter by Tom N Harris <telliamed@whoopdedo.cjb.net>
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

//---------------------------- El cargador de las fuentes para El Motor Oscuro --------------------
#include "FreeImage.h"
#include "ManualFonFileLoader.h"
#include <OgreLogManager.h>
#include <OgreStringConverter.h>
#include <OgreTextureManager.h>
#include <OgreHardwarePixelBuffer.h>

using namespace std;
using namespace Opde; //For the Opde::File

namespace Ogre
{
    /*-----------------------------------------------------------------*/
	/*--------------------- ManualFonFileLoader -----------------------*/
	/*-----------------------------------------------------------------*/
    ManualFonFileLoader::ManualFonFileLoader() : ManualResourceLoader()
	{
		mMemBuff = NULL;
    }

    //-------------------------------------------------------------------
    ManualFonFileLoader::~ManualFonFileLoader()
	{
		if(mMemBuff)
			delete [] mMemBuff;
    }


	/*-----------------------------------------------------------------*/
	/*------------------------ Bitmap stuff ---------------------------*/
	/*-----------------------------------------------------------------*/
	unsigned char** ManualFonFileLoader::ReadFont(FilePtr FontFile, int *ResultingColor)
	{
		struct DarkFontHeader FontHeader;
		unsigned int ImageWidth, ImageHeight, FinalSize = 1;
		unsigned int X, Y;
		unsigned int N, I;
		unsigned char *Ptr;

		FontFile->readStruct(&FontHeader, DarkFontHeader_Format, sizeof(DarkFontHeader));
		mNumChars = FontHeader.LastChar - FontHeader.FirstChar + 1;
		mNumRows = FontHeader.NumRows;

		vector <unsigned short> Widths;
		FontFile->seek(FontHeader.WidthOffset, File::FSEEK_BEG);
		for(I = 0; I < mNumChars + 1; I++)
		{
			unsigned short Temp;
			FontFile->readElem(&Temp, sizeof(Temp));
			Widths.push_back(Temp);
		}

		unsigned char *BitmapData = new unsigned char[FontFile->size() + 1];
		if (!BitmapData)
			return NULL;
		FontFile->seek(FontHeader.BitmapOffset, File::FSEEK_BEG);
		FontFile->read(BitmapData, FontFile->size() - FontHeader.BitmapOffset);
		if (FontHeader.BitmapOffset < FontHeader.WidthOffset)
		{
			delete [] BitmapData;
			return NULL;
		}

		ImageHeight = ((mNumChars / 16) + 2) * (FontHeader.NumRows + 4);

		Y = (FontHeader.NumRows / 2) + 2;
		X = 2;
		ImageWidth = 2;

		for (N = 0; N < mNumChars; N++)
		{
			CharInfo Char;
			Char.Code = FontHeader.FirstChar + N;
			Char.Column = Widths[N];
			Char.Width = Widths[N + 1] - Widths[N];
			Char.X = X;
			Char.Y = Y;
			X += Char.Width + 6;
			if ((N & 0xF) == 0xF)
			{
				Y += FontHeader.NumRows + 4;
				if (X > ImageWidth)
					ImageWidth = X;
				X = 2;
			}
			mChars.push_back(Char);
		}

		if (X > ImageWidth)
			ImageWidth = X;

		while ((FinalSize < ImageWidth) || (FinalSize < ImageHeight))
			FinalSize <<= 1;

		unsigned char ** RowPointers = new unsigned char* [FinalSize];
		if (!RowPointers)
		{
			delete [] BitmapData;
			return NULL;
		}

		unsigned char *ImageData = new unsigned char[FinalSize * FinalSize];
		if (!ImageData)
		{
			delete [] RowPointers;
			delete [] BitmapData;
			return NULL;
		}
		memset(ImageData, BLACK_INDEX, FinalSize * FinalSize);
		Ptr = ImageData;
		for (N = 0; N < FinalSize; N++)
		{
			RowPointers[N] = Ptr;
			Ptr += FinalSize;
		}

		String s = "Loading font " + FontFile->getName();
		s += " format ";
		s +=  StringConverter::toString(FontHeader.Format);

		LogManager::getSingleton().logMessage(s);

		if (FontHeader.Format == 0)
		{
			Ptr = BitmapData;
			for (I = 0; I < FontHeader.NumRows; I++)
			{
				for (N = 0; N < mNumChars; N++)
				{
					Y = mChars[N].Column;
					for (X = 0; X < mChars[N].Width; Y++, X++)
						RowPointers[mChars[N].Y + I][mChars[N].X + X] = ((Ptr[Y / 8]>>(7 - (Y % 8))) & 1) ? WHITE_INDEX : BLACK_INDEX;
				}
				Ptr += FontHeader.RowWidth;
			}
		}
		else
		{
			Ptr = BitmapData;
			for (I = 0; I < FontHeader.NumRows; I++)
			{
				for (N = 0; N < mNumChars; N++)
				{
					memcpy(RowPointers[mChars[N].Y + I] + mChars[N].X, Ptr, mChars[N].Width);
					Ptr += mChars[N].Width;
				}
			}
		}

		mImageDim = FinalSize;
		switch (FontHeader.Format)
		{
		case 0:
			*ResultingColor = 1;
			break;

		case 1:
			*ResultingColor = 2;
			break;

		case 0xCCCC:
			*ResultingColor = (FontHeader.Palette == 1) ? -1 : 0;
			break;

		default:
			*ResultingColor = 0;//Unknown pixel Format, assuming 8bpp.
			break;
		}

		delete [] BitmapData;
		return RowPointers;
	}


	//-------------------------------------------------------------------
	RGBQUAD* ManualFonFileLoader::ReadPalette(StdFile *FilePointer)
	{
		ExternalPaletteHeader PaletteHeader;
		WORD Count;
		char *Buffer, *C;
		BYTE S;
		unsigned int I;

		RGBQUAD *Palette = new RGBQUAD[256];
		if (!Palette)
			return NULL;
		FilePointer->readStruct(&PaletteHeader, ExternalPaletteHeader_Format, sizeof(ExternalPaletteHeader));
		if (PaletteHeader.RiffSig == 0x46464952)
		{
			if (PaletteHeader.PSig1 != 0x204C4150)
			{
				delete []Palette;
				return NULL;
			}
			FilePointer->seek(2L, StdFile::FSEEK_CUR);
			FilePointer->readElem(&Count, 2);
			if (Count > 256)
				Count = 256;
			for (I = 0; I < Count; I++)
			{
				FilePointer->readStruct(&Palette[I], RGBQUAD_Format, sizeof(RGBQUAD));
				S = Palette[I].rgbRed;
				Palette[I].rgbRed = Palette[I].rgbBlue;
				Palette[I].rgbBlue = S;
			}
		}
		else if (PaletteHeader.RiffSig == 0x4353414A)
		{
			FilePointer->seek(0);
			Buffer = new char[3360];
			if (!Buffer)
			{
				delete []Palette;
				return NULL;
			}
			FilePointer->read(Buffer, 3352);
			if (strncmp(Buffer, "JASC-PAL", 8))
			{
				delete []Buffer;
				delete []Palette;
				return NULL;
			}
			C = strchr(Buffer, '\n')+1;
			C = strchr(C, '\n')+1;
			Count = (WORD)strtoul(C, NULL, 10);
			if (Count > 256)
				Count = 256;
			for (I = 0; I < Count; I++)
			{
				C = strchr(C, '\n')+1;
				Palette[I].rgbRed = (BYTE)strtoul(C, &C, 10);
				C++;
				Palette[I].rgbGreen = (BYTE)strtoul(C, &C, 10);
				C++;
				Palette[I].rgbBlue = (BYTE)strtoul(C, &C, 10);
			}
			delete []Buffer;
		}
		else
		{
			FilePointer->seek(0);
			RGBTRIPLE P;
			for (I = 0; I < FilePointer->size() / sizeof(RGBTRIPLE); I++)
			{
				FilePointer->readStruct(&P, RGBTRIPLE_Format, sizeof(RGBTRIPLE));
				Palette[I].rgbRed = P.rgbtBlue;
				Palette[I].rgbGreen = P.rgbtGreen;
				Palette[I].rgbBlue = P.rgbtRed;
			}
		}
		return Palette;
	}

	//-------------------------------------------------------------------
	int ManualFonFileLoader::WriteImage(RGBQUAD *ColorTable, char **RowPointers)
	{
		BITMAPFILEHEADER	FileHeader;
		BITMAPINFOHEADER	BitmapHeader;
		int 	RowWidth, Row;
		char	Zero[4] = {0,0,0,0};

		mBmpFileSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * 256) + (mImageDim * mImageDim);
		mMemBuff = new unsigned char[mBmpFileSize];
		if(!mMemBuff)
			return -1;
		unsigned char *Ptr = mMemBuff;

		FileHeader.bfType = 0x4D42;
		FileHeader.bfReserved1 = 0;
		FileHeader.bfReserved2 = 0;
		FileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD)*256);
		BitmapHeader.biSize = sizeof(BITMAPINFOHEADER);
		BitmapHeader.biWidth = mImageDim;
		BitmapHeader.biHeight = mImageDim;
		BitmapHeader.biPlanes = 1;
		BitmapHeader.biBitCount = 8;
		BitmapHeader.biCompression = 0;
		BitmapHeader.biXPelsPerMeter = 0xB13;
		BitmapHeader.biYPelsPerMeter = 0xB13;
		BitmapHeader.biClrUsed = 256;
		BitmapHeader.biClrImportant = 256;
		RowWidth = (mImageDim + 3) & ~3;
		BitmapHeader.biSizeImage = RowWidth * mImageDim;
		FileHeader.bfSize = FileHeader.bfOffBits + BitmapHeader.biSizeImage;

		memcpy(Ptr, &FileHeader, sizeof(BITMAPFILEHEADER));
		Ptr += sizeof(BITMAPFILEHEADER);

		memcpy(Ptr, &BitmapHeader, sizeof(BITMAPINFOHEADER));
		Ptr += sizeof(BITMAPINFOHEADER);

		memcpy(Ptr, ColorTable, sizeof(RGBQUAD) * 256);
		Ptr += sizeof(RGBQUAD) * 256;

		RowWidth -= mImageDim;
		for (Row = mImageDim - 1; Row >= 0; Row--)
		{
			memcpy(Ptr, RowPointers[Row], mImageDim);
			Ptr += mImageDim;

			if (RowWidth != 0)
			{
				memcpy(Ptr, Zero, mImageDim);
				Ptr += RowWidth;
			}
		}

		//Uncommenting the following block will generate the intermediate bitmap file
		/*StdFile* BitmapFile = new StdFile("Font.bmp", File::FILE_W);
		BitmapFile->write(mMemBuff, mBmpFileSize);
		delete BitmapFile;*/

		return 0;
	}

	//-------------------------------------------------------------------
	int ManualFonFileLoader::LoadDarkFont(FilePtr FontFile, String PaletteFileName)
	{
		COLORREF *PaletteData;
		unsigned char **ImageRows;
		int Color;

		if (PaletteFileName != "")
		{
			// Small comment: This would better be done using Ogre::DataStream
			// Also, the StdFile (as others) will throw an exception when the file was not found,
			// so the condition will always fail...
			StdFile *PaletteFile = new StdFile(PaletteFileName, File::FILE_R);
			if (PaletteFile == 0)
			{
				LogManager::getSingleton().logMessage(PaletteFileName + " not found.");
				return -1;
			}
			PaletteData = (COLORREF*)ReadPalette(PaletteFile);
			delete PaletteFile;
			if (!PaletteData)
			{
				LogManager::getSingleton().logMessage("Invalid palette file : " + PaletteFileName);
				return -2;
			}
		}
		else
			PaletteData = (COLORREF*)ColorTable;

		ImageRows = ReadFont(FontFile, &Color);
		if (!ImageRows)
		{
			if (PaletteData != ColorTable)
				delete []PaletteData;
			return 2;
		}
		if (Color == 2 && PaletteData == ColorTable)
			PaletteData = (COLORREF*)AntiAliasedColorTable;

		// WriteImage((RGBQUAD*)PaletteData, ImageRows);
		// For now... Can also be done directly when loading (And probably better too)
		createOgreTexture(ImageRows, (RGBQUAD*)PaletteData);


		delete [] ImageRows;
		if (PaletteData != ColorTable && PaletteData != AntiAliasedColorTable)
			delete []PaletteData;
		return 0;
	}

    //-------------------------------------------------------------------
	void ManualFonFileLoader::createOgreTexture(unsigned char** img, RGBQUAD* palette) {
		// Create a texure, then fill it
		TexturePtr txt = TextureManager::getSingleton().createManual(mTxtName, mFontGroup, TEX_TYPE_2D, mImageDim, mImageDim, 1, PF_A8R8G8B8);

		// Lock the texture, obtain a pointer
		HardwarePixelBufferSharedPtr pbuf = txt->getBuffer(0, 0);

		// Erase the lmap atlas pixel buffer
		pbuf->lock(HardwareBuffer::HBL_DISCARD);
		const PixelBox &pb = pbuf->getCurrentLock();

		// Copy the image data, converting to 32bit on the fly...
		for (int y = 0; y < mImageDim; y++) {
		    unsigned char* row = img[y];
			uint32 *data = static_cast<uint32*>(pb.data) + y*pb.rowPitch;

			for (int x = 0; x < mImageDim; x++) {
				int palidx = 255 - row[x];

                unsigned char r,g,b,a;
                r = palette[palidx].rgbRed;
                g = palette[palidx].rgbGreen;
                b = palette[palidx].rgbBlue;

                a = 0;

                if (palidx == WHITE_INDEX)
                    a = 255;

				// Write the ARGB data
				data[x] = a | (r << 8) | (g << 16) | (b << 24);
			}

		}

		pbuf->unlock();
	}

	/*-----------------------------------------------------------------*/
	/*------------------------- Alpha stuff ---------------------------*/
	/*-----------------------------------------------------------------*/
	int ManualFonFileLoader::AddAlpha()
	{
#ifdef FREEIMAGE_LIB
		FreeImage_Initialise();
#endif //FREEIMAGE_LIB

		FIMEMORY *BmpMem = FreeImage_OpenMemory(mMemBuff, mBmpFileSize);

		FIBITMAP *Src = FreeImage_LoadFromMemory(FIF_BMP, BmpMem, 0);
		if(!Src)
			return -1;

		FIBITMAP *Dst = FreeImage_ConvertTo32Bits(Src);

		FreeImage_Invert(Src);
		FIBITMAP *Mask = FreeImage_ConvertToGreyscale(Src);
		FreeImage_Invert(Src);
		FreeImage_SetChannel(Dst, Mask, FICC_ALPHA);
		FreeImage_Unload(Mask);

		FreeImage_Save(FIF_PNG, Dst, "Font.PNG");

		FreeImage_Unload(Dst);
		FreeImage_Unload(Src);
		FreeImage_CloseMemory(BmpMem);
		delete [] mMemBuff;
		mMemBuff = NULL;

#ifdef FREEIMAGE_LIB
		FreeImage_DeInitialise();
#endif //FREEIMAGE_LIB

		return 0;
	}


	/*-----------------------------------------------------------------*/
	/*------------------------- Ogre stuff ----------------------------*/
	/*-----------------------------------------------------------------*/
	int ManualFonFileLoader::CreateOgreFont(Font* DarkFont)
	{
		DarkFont->setSource(mTxtName);
		DarkFont->setType(FT_IMAGE);

		for(CharInfoList::const_iterator It = mChars.begin(); It != mChars.end(); It++)
		{
			const CharInfo& Char = *It;
			DarkFont->setGlyphTexCoords(Char.Code, (float)(Char.X - 2) / mImageDim,
				(float)(Char.Y - 2) / mImageDim, (float)(Char.X + Char.Width + 2) / mImageDim,
				(float)(Char.Y + mNumRows + 2) / mImageDim, 1.0);
		}

		DarkFont->setParameter("size", StringConverter::toString(mNumRows)); // seems mNumRows (NumRows) means Y size of the font...
		DarkFont->load();			//Let's rock!


		return 0;
	}

	//-------------------------------------------------------------------
    void ManualFonFileLoader::loadResource(Resource* resource)
	{
        // Cast to font, and fill
        Font* DarkFont = static_cast<Font*>(resource);

        // Fill. Find the file to be loaded by the name, and load it
        String FontName = DarkFont->getName();
        mFontGroup = DarkFont->getGroup();

        // Get the real filename from the nameValuePairList
        // That means: truncate to the last dot, append .fon to the filename
        size_t DotPos = FontName.find_last_of(".");

        String BaseName = FontName;
        if (DotPos != String::npos){
            BaseName = FontName.substr(0, DotPos);
        }

		mTxtName = BaseName + "_Txt";

        BaseName += ".fon";

        //Open the file
        Ogre::DataStreamPtr Stream = Ogre::ResourceGroupManager::getSingleton().openResource(BaseName, DarkFont->getGroup(), true, resource);

        FilePtr FontFile = new OgreFile(Stream);

        if(LoadDarkFont(FontFile, ""))
			LogManager::getSingleton().logMessage("An error occurred while loading the font " + BaseName);
		/*
		if(AddAlpha())
			LogManager::getSingleton().logMessage("An error occurred while adding Alpha Channel");*/

		if(CreateOgreFont(DarkFont))
			LogManager::getSingleton().logMessage("An error occurred creating Ogre font");
    }
}
