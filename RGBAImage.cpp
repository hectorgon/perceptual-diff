/*
RGBAImage.cpp
Copyright (C) 2006 Yangli Hector Yee

(This entire file was rewritten by Jim Tilander)

This program is free software; you can redistribute it and/or modify it under the terms of the
GNU General Public License as published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program;
if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include "RGBAImage.h"
#include <cstdio>
#include <cstring>
#include <cstdint> // for int32_t, uint32_t, etc.

RGBAFloatImage* RGBAFloatImage::DownSample() const {
   if (Width <=1 || Height <=1)
      return NULL;

   int nw = Width / 2;
   int nh = Height / 2;
   RGBAFloatImage* img = new RGBAFloatImage(nw, nh, Name.c_str());

   for (int y = 0; y < nh; y++) {
      for (int x = 0; x < nw; x++) {
         RGBAFloat d[4];
         // Sample a 2x2 patch from the parent image.
         d[0] = Get(2 * x + 0, 2 * y + 0);
         d[1] = Get(2 * x + 1, 2 * y + 0);
         d[2] = Get(2 * x + 0, 2 * y + 1);
         d[3] = Get(2 * x + 1, 2 * y + 1);
         RGBAFloat avgColor(0);
         // Compute average color.
         for (int i = 0; i < 4; i++) {
            RGBAFloatComp c;
            c  = d[0].GetComp(i);
            c += d[1].GetComp(i);
            c += d[2].GetComp(i);
            c += d[3].GetComp(i);
            c /= 4;
            avgColor.GetComp(i) = c;
         }
         img->Set(avgColor, x, y);
      }
   }

   return img;
}

bool RGBAFloatImage::WriteToFile(const char* filename)
{
	const FREE_IMAGE_FORMAT fileType = FreeImage_GetFIFFromFilename(filename);
	if(FIF_UNKNOWN == fileType)
	{
		printf("Can't save to unknown filetype %s\n", filename);
		return false;
	}

	FIBITMAP* bitmap = FreeImage_Allocate(Width, Height, 32);
	if(!bitmap)
	{
		printf("Failed to create freeimage for %s\n", filename);
		return false;
	}

	const RGBAFloat* source = Data;
   //if (32bit uint image)
   //{
      int idx = 0;
	   for( int y=0; y < Height; y++, source += Width )
	   {
         RGBAInt32* scanline = (RGBAInt32*)FreeImage_GetScanLine(bitmap, Height - y - 1);
		   //memcpy(scanline, source, sizeof(source[0]) * Width);
         for (int x = 0; x < Width; x++, idx++)
            scanline[x] = GetInt32(idx);
      }
   //}
   // else if (float image)
   //{
   //  float save...
   //}
	
	FreeImage_SetTransparent(bitmap, false);
	FIBITMAP* converted = FreeImage_ConvertTo24Bits(bitmap);
	
	
	const bool result = !!FreeImage_Save(fileType, converted, filename);
	if(!result)
		printf("Failed to save to %s\n", filename);
	
	FreeImage_Unload(converted);
	FreeImage_Unload(bitmap);
	return result;
}

RGBAFloatImage* RGBAFloatImage::ReadFromFile(const char* filename)
{
	const FREE_IMAGE_FORMAT fileType = FreeImage_GetFileType(filename);
	if(FIF_UNKNOWN == fileType)
	{
		printf("Unknown filetype %s\n", filename);
		return 0;
	}
	
	FIBITMAP* freeImage = 0;
	if(FIBITMAP* temporary = FreeImage_Load(fileType, filename, 0))
	{
		freeImage = FreeImage_ConvertTo32Bits(temporary);
		FreeImage_Unload(temporary);
	}
	if(!freeImage)
	{
		printf( "Failed to load the image %s\n", filename);
		return 0;
	}

	const int w = FreeImage_GetWidth(freeImage);
	const int h = FreeImage_GetHeight(freeImage);

	RGBAFloatImage* result = new RGBAFloatImage(w, h, filename);
	// Copy the image over to our internal format, FreeImage has the scanlines bottom to top though.
   //if (32bit uint image)
   //{
      int resIdx = 0;
	   for( int y=0; y < h; y++ )
	   {
         const RGBAInt32* scanline = (const RGBAInt32*)FreeImage_GetScanLine(freeImage, h - y - 1);
         for (int x = 0; x < w; x++, resIdx++)
            result->Set(scanline[x], resIdx);
      }
   //}
   // else if (float image)
   //{
   //  float load...
   //}

	FreeImage_Unload(freeImage);
	return result;
}


