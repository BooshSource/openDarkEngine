/******************************************************************************
 *    DarkFontConverter.h
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2007 openDarkEngine team
 *    Based on Thief Font Converter by Tom N Harris <telliamed@whoopdedo.cjb.net>
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
#ifndef __DARKFONTCONVERTER_H
#define __DARKFONTCONVERTER_H

#include "integers.h"

/*
typedef unsigned long	uint32;
typedef signed long		sint32;
typedef unsigned short	uint16;
typedef signed short	sint16;
typedef unsigned char	uint8;
*/
typedef uint32_t  DWORD;
typedef int32_t   BOOL;
typedef uint8_t   BYTE;
typedef uint16_t  WORD;
typedef DWORD COLORREF;


struct DarkFontHeader
{
	uint16_t	Format;		/* 1 - anti-aliased, 0xCCCC - 8bpp, else - 1bpp */
	uint8_t	Unknown;	/* set to 10 if Format is 1 */
	uint8_t	Palette;	/* 0 - use current, 1 - standard */
	uint8_t	Zeros1[32];
	int16_t	FirstChar;
	int16_t	LastChar;
	uint8_t	Zeros2[32];
	uint32_t	WidthOffset;
	uint32_t	BitmapOffset;
	uint16_t	RowWidth;	/* in bytes */
	uint16_t	NumRows;
};

struct CharInfo
{
	int16_t	Code;
	uint16_t	Column;
	uint16_t	Width;
	uint16_t	Height;
	int32_t	X,Y;
};

#pragma pack(push, 1)

typedef struct tagBITMAPFILEHEADER { 
  WORD    bfType; 
  DWORD   bfSize; 
  WORD    bfReserved1; 
  WORD    bfReserved2; 
  DWORD   bfOffBits; 
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
    DWORD  biSize;
    DWORD   biWidth;
    DWORD   biHeight;
    WORD   biPlanes;
    WORD   biBitCount;
    DWORD  biCompression;
    DWORD  biSizeImage;
    DWORD   biXPelsPerMeter;
    DWORD   biYPelsPerMeter;
    DWORD  biClrUsed;
    DWORD  biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagRGBQUAD {
  BYTE    rgbBlue; 
  BYTE    rgbGreen; 
  BYTE    rgbRed; 
  BYTE    rgbReserved; 
} RGBQUAD;

typedef struct tagBITMAPINFO {
   BITMAPINFOHEADER bmiHeader;
   RGBQUAD bmiColors[1];
} BITMAPINFO;

typedef struct tagRGBTRIPLE { 
  BYTE rgbtBlue; 
  BYTE rgbtGreen; 
  BYTE rgbtRed; 
} RGBTRIPLE; 

static const COLORREF	ColorTable[] = {
	0x000000, 0xDDDDDD, 0xB6B6B6, 0x969696, 0x7C7C7C, 0x666666, 0x545454, 0x454545,
	0x393939, 0x2F2F2F, 0x272727, 0x202020, 0x1A1A1A, 0x161616, 0x121212, 0x0F0F0F,
	0x0C0C0C, 0x0A0A0A, 0x080808, 0x060606, 0x050505, 0x040404, 0x030303, 0x7A8449,
	0x707A43, 0x65703D, 0x5B6536, 0x0087FE, 0x007DF2, 0x0073E6, 0x0069DA, 0x005FCE,
	0xB18C82, 0xA38178, 0x96776E, 0x886C64, 0x7B615A, 0x6D5650, 0x604C46, 0x52413C,
	0x443632, 0x372B28, 0x29211E, 0x1C1614, 0x0E0B0A, 0xA30202, 0x980202, 0x8E0202,
	0x830202, 0x780101, 0x6D0101, 0x630101, 0x580101, 0x4D0101, 0x380101, 0x220000,
	0x0D0000, 0x083015, 0x072B13, 0x062510, 0x05200E, 0x041A0B, 0x031509, 0x020F06,
	0x8E6D43, 0x87683F, 0x81623C, 0x7A5D38, 0x735734, 0x6D5231, 0x664C2D, 0x60472A,
	0x573F25, 0x4D3820, 0x44301A, 0x3A2915, 0x312110, 0x3E1210, 0x3A110F, 0x36100E,
	0x320E0D, 0x2E0D0C, 0x2A0C0B, 0x260B0A, 0x220A09, 0x1D0807, 0x180606, 0x120504,
	0x0D0303, 0x321D46, 0x2D1A3F, 0x271736, 0x20132C, 0x1A1023, 0x140C1B, 0x0E0912,
	0xEE9A47, 0xDE9042, 0xCF863E, 0xBF7C39, 0xAF7134, 0xA0672F, 0x905D2B, 0x815326,
	0x714922, 0x613F1D, 0x513519, 0x412B14, 0x312110, 0x312110, 0x2E1F0F, 0x2B1D0E,
	0x271A0D, 0x24180C, 0x21160B, 0x1E140A, 0x1B1209, 0x170F07, 0x140D06, 0x110B05,
	0x0A0603, 0xFFFFBD, 0xFFB510, 0xFFAD52, 0xFFFF7B, 0xF1E657, 0xE4CE34, 0xD6B510,
	0x8C7B5A, 0x867555, 0x7F6E4F, 0x79684A, 0x726145, 0x6C5B40, 0x65543A, 0x5C4C33,
	0x54432C, 0x4B3B25, 0x42321E, 0x3A2A17, 0x312110, 0x505B30, 0x4B552D, 0x464F2A,
	0x404927, 0x3B4423, 0x363E20, 0x31381D, 0x2C321A, 0x262C17, 0x212614, 0x1C2011,
	0x171B0D, 0xAD10B5, 0x9C29AD, 0xB51894, 0x9400C6, 0x6300AD, 0x4A0080, 0x310052,
	0x6D7787, 0x666F7E, 0x5F6775, 0x575F6C, 0x505764, 0x494F5A, 0x424752, 0x3B4048,
	0x33373E, 0x2B2E34, 0x222529, 0x1A1C1F, 0x121315, 0x5A3139, 0x542E35, 0x4E2B31,
	0x48272E, 0x42242A, 0x3C2126, 0x361E22, 0x311B1F, 0x2B171B, 0x231316, 0x1B0E11,
	0x130A0C, 0xE71821, 0xCE2118, 0xFF8C18, 0xD97415, 0xB25C12, 0x8C440F, 0x652C0C,
	0x6D5370, 0x664E69, 0x5E4760, 0x564158, 0x4D3B4E, 0x463546, 0x3D2E3D, 0x352835,
	0x2D222D, 0x241B24, 0x1C151C, 0x130E13, 0x0B080B, 0x9C4A4A, 0xA86161, 0xB57777,
	0xC18E8E, 0xCEA5A5, 0xDABBBB, 0xE6D2D2, 0xF3E8E8, 0xFFFFFF, 0x0055C2, 0x004BB6,
	0x0041A2, 0xB5BDFF, 0x8C8CE7, 0x9CADEF, 0x7E9DE2, 0x5F8ED4, 0x417EC7, 0x226EB9,
	0xDED684, 0xCFC87B, 0xC1BA73, 0xB2AC6A, 0xA49E61, 0x959058, 0x878250, 0x746F44,
	0x605C39, 0x4D4A2D, 0x393721, 0x262416, 0x12110A, 0x74959D, 0x97B0B6, 0xBACACE,
	0xDCE5E7, 0xFFFFFF, 0x3A5B4B, 0x466E57, 0x538063, 0x5F936F, 0x6BA57B, 0x00378D,
	0x002D79, 0x002364, 0x001950, 0x000F3B, 0x0000FF, 0x0000FF, 0xFF00FF, 0x000000
};

/* Anti-aliased fonts have a pixel Width of 8 bits, but only the lower 4 bits are used. */
static const COLORREF	AntiAliasedColorTable[] = {
	0xFFFFFF, 0xEEEEEE, 0xDDDDDD, 0xCCCCCC, 0xBBBBBB, 0xAAAAAA, 0x999999, 0x888888,
	0x777777, 0x666666, 0x555555, 0x444444, 0x333333, 0x222222, 0x111111, 0x000000,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
	0xFFFFFF, 0xEEEEEE, 0xDDDDDD, 0xCCCCCC, 0xBBBBBB, 0xAAAAAA, 0x999999, 0x888888,
	0x777777, 0x666666, 0x555555, 0x444444, 0x333333, 0x222222, 0x111111, 0x000000
};

#pragma pack(pop)

#endif //__DARKFONTCONVERTER_H
