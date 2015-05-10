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
#include <cstdint> // uint8_t, uint32_t, etc.

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

         // Compute average color.
         RGBAFloat avgColor(0);
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

   FIBITMAP* finalBitmap = NULL;

   if (FreeImage_FIFSupportsExportType(fileType, FIT_RGBF)) {
      FIBITMAP* bitmap = FreeImage_AllocateT(FIT_RGBF, Width, Height);
      if(!bitmap) {
         printf("Failed to create FreeImage bitmap for %s\n", filename);
         return false;
      }

      const RGBAFloat* source = Data;
      int idx = 0;
      for( int y=0; y < Height; y++, source += Width ) {
         RGBAFloatComp(*scanlineTriplet)[3] =
            reinterpret_cast<RGBAFloatComp(*)[3]>(
               reinterpret_cast<void*>(
                  FreeImage_GetScanLine(bitmap, Height - y - 1)));
         for (int x = 0; x < Width; x++, idx++) {
            GetRGBTriplet(scanlineTriplet[x], idx);
         }
      }

      finalBitmap = bitmap;
   } else if (FreeImage_FIFSupportsExportType(fileType, FIT_BITMAP)) {
      FIBITMAP* bitmap = FreeImage_Allocate(Width, Height, 32);
      if(!bitmap) {
         printf("Failed to create FreeImage bitmap for %s\n", filename);
         return false;
      }

      const RGBAFloat* source = Data;
      int idx = 0;
      for( int y=0; y < Height; y++, source += Width ) {
         RGBAInt32* scanline = (RGBAInt32*)FreeImage_GetScanLine(bitmap, Height - y - 1);
         for (int x = 0; x < Width; x++, idx++)
            scanline[x] = GetInt32(idx);
      }

      FreeImage_SetTransparent(bitmap, false);
      finalBitmap = FreeImage_ConvertTo24Bits(bitmap);
      FreeImage_Unload(bitmap);
   }

   const bool result = !!FreeImage_Save(fileType, finalBitmap, filename);
   if(!result)
      printf("Failed to save to %s\n", filename);

   FreeImage_Unload(finalBitmap);
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

   int bpp = 0;
   FREE_IMAGE_TYPE origImageType = FIT_UNKNOWN;

   FIBITMAP* freeImage = 0;
   if(FIBITMAP* temporary = FreeImage_Load(fileType, filename, 0))
   {
      bpp = FreeImage_GetBPP(temporary);
      origImageType = FreeImage_GetImageType(temporary);

      if ((origImageType == FIT_BITMAP) || (origImageType == FIT_RGB16) || (origImageType == FIT_RGBA16)) {
         freeImage = FreeImage_ConvertTo32Bits(temporary);
         FreeImage_Unload(temporary);
      } else if ((origImageType == FIT_RGBF) || (origImageType == FIT_RGBAF)) {
         // Float
         freeImage = temporary;
      }
   }
   if(!freeImage)
   {
      printf( "Failed to load the image %s\n", filename);
      return 0;
   }

   const int w = FreeImage_GetWidth(freeImage);
   const int h = FreeImage_GetHeight(freeImage);

   RGBAFloatImage* result = new RGBAFloatImage(w, h, filename);
   // Copy the image over to our internal format, FreeImage has scanlines bottom to top though.
   int resIdx = 0;
   if ((origImageType == FIT_BITMAP) || (origImageType == FIT_RGB16) || (origImageType == FIT_RGBA16)) {
      for (int y = 0; y < h; y++) {
         const RGBAInt32* scanline =
            reinterpret_cast<const RGBAInt32*>(FreeImage_GetScanLine(freeImage, h - y - 1));
         for (int x = 0; x < w; x++, resIdx++)
            result->Set(scanline[x], resIdx);
      }
   } else if (origImageType == FIT_RGBF) {
      for (int y = 0; y < h; y++) {
         const RGBAFloatComp(*scanlineTriplet)[3] =
            reinterpret_cast<const RGBAFloatComp(*)[3]>(
               reinterpret_cast<const void*>(
                  FreeImage_GetScanLine(freeImage, h - y - 1)));
         for (int x = 0; x < w; x++, resIdx++) {
            result->Set(scanlineTriplet[x], resIdx);
         }
      }
   } else if (origImageType == FIT_RGBAF) {
      for (int y = 0; y < h; y++) {
         const RGBAFloatComp(*scanlineQuad)[4] =
            reinterpret_cast<const RGBAFloatComp(*)[4]>(
               reinterpret_cast<const void*>(
                  FreeImage_GetScanLine(freeImage, h - y - 1)));
         for (int x = 0; x < w; x++, resIdx++) {
            result->Set(scanlineQuad[x], resIdx);
         }
      }
   }

   FreeImage_Unload(freeImage);
   return result;
}


