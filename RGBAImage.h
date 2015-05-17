/*
RGBAImage.h
Copyright (C) 2006 Yangli Hector Yee

This program is free software; you can redistribute it and/or modify it under the terms of the
GNU General Public License as published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program;
if not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef _RGAIMAGE_H
#define _RGBAIMAGE_H

#include "FreeImage.h"

#include <string>
#include <cstdint> // uint8_t, uint32_t, etc.
#include <algorithm>
#include <cassert>

template <typename T>
inline T Clamp(const T& n, const T& lower, const T& upper)
{
   assert(lower <= upper);
   return std::max(lower, std::min(n, upper));
}

/** Basic pixel values types. */

typedef float   RGBAFloatComp;
typedef uint8_t RGBAInt32Comp;

inline RGBAInt32Comp ConvertRGBAFloatCompToInt32(RGBAFloatComp f) {
   return Clamp<RGBAInt32Comp>(
      static_cast<RGBAInt32Comp>(std::lround(f * 255.0f)),
      0, 255);
}
inline RGBAFloatComp ConvertRGBAInt32CompToFloat(RGBAInt32Comp i) {
   return i / 255.0f;
}

/** Type for an integer-based R,G,B,A pixel.
 *
 * The order of components is defined in the FreeImage library through macros like FI_RGBA_*_SHIFT.
 */
typedef uint32_t RGBAInt32;

/** Class encapsulating a float-based R,G,B,A pixel.
 *
 * Internal representation assumes data is in the ABGR format, with the RGB
 * color channels premultiplied by the alpha value.  Premultiplied alpha is
 * often also called "associated alpha" - see the tiff 6 specification for some
 * discussion - http://partners.adobe.com/asn/developer/PDFS/TN/TIFF6.pdf
 *
 */
class RGBAFloat
{
public:
   RGBAFloat() {};
   RGBAFloat(RGBAFloatComp v) :
      mR(v), mG(v), mB(v), mA(v) {}
   RGBAFloat(
         RGBAFloatComp r,
         RGBAFloatComp g,
         RGBAFloatComp b,
         RGBAFloatComp a) :
      mR(r), mG(g), mB(b), mA(a) {}
   RGBAFloat(
         RGBAFloatComp r,
         RGBAFloatComp g,
         RGBAFloatComp b) :
      mR(r), mG(g), mB(b), mA(1.0) {}

   void Set(const RGBAFloatComp (&rgb)[3]) {
      mR = rgb[0];
      mG = rgb[1];
      mB = rgb[2];
      mA = 1.0f;
   }
   void Set(const RGBAFloatComp (&rgba)[4]) {
      mR = rgba[0];
      mG = rgba[1];
      mB = rgba[2];
      mA = rgba[3];
   }
   void Set(
         RGBAFloatComp r,
         RGBAFloatComp g,
         RGBAFloatComp b,
         RGBAFloatComp a) {
      mR = r;
      mG = g;
      mB = b;
      mA = a;
   }
   void Set(
         RGBAInt32Comp r,
         RGBAInt32Comp g,
         RGBAInt32Comp b,
         RGBAInt32Comp a) {
      mR = ConvertRGBAInt32CompToFloat(r);
      mG = ConvertRGBAInt32CompToFloat(g);
      mB = ConvertRGBAInt32CompToFloat(b);
      mA = ConvertRGBAInt32CompToFloat(a);
   }
   void Set(RGBAInt32 rgba) {
      mR = ConvertRGBAInt32CompToFloat((rgba >> FI_RGBA_RED_SHIFT)   & 0xFF);
      mG = ConvertRGBAInt32CompToFloat((rgba >> FI_RGBA_GREEN_SHIFT) & 0xFF);
      mB = ConvertRGBAInt32CompToFloat((rgba >> FI_RGBA_BLUE_SHIFT)  & 0xFF);
      mA = ConvertRGBAInt32CompToFloat((rgba >> FI_RGBA_ALPHA_SHIFT) & 0xFF);
   }

   RGBAFloat& operator+=(const RGBAFloat& a) {
      mR += a.mR;
      mG += a.mG;
      mR += a.mR;
      mA += a.mA;
      return *this;
   }

   const RGBAFloatComp& GetComp(unsigned int i) const {
      return reinterpret_cast<const RGBAFloatComp*>(this)[i];
   }
   RGBAFloatComp& GetComp(unsigned int i) {
      return reinterpret_cast<RGBAFloatComp*>(this)[i];
   }

   RGBAInt32 GetInt32() {
      return
           (ConvertRGBAFloatCompToInt32(mR) & 0xFF) << FI_RGBA_RED_SHIFT
         | (ConvertRGBAFloatCompToInt32(mG) & 0xFF) << FI_RGBA_GREEN_SHIFT
         | (ConvertRGBAFloatCompToInt32(mB) & 0xFF) << FI_RGBA_BLUE_SHIFT
         | (ConvertRGBAFloatCompToInt32(mA) & 0xFF) << FI_RGBA_ALPHA_SHIFT;
   }
   void GetRGBTriplet(RGBAFloatComp(&rgb)[3]) {
      rgb[0] = mR;
      rgb[1] = mG;
      rgb[2] = mB;
   }
   void GetRGBQuad(RGBAFloatComp(&rgb)[4]) {
      rgb[0] = mR;
      rgb[1] = mG;
      rgb[2] = mB;
      rgb[3] = mA;
   }

   bool operator==(const RGBAFloat& a) {
      return mR == a.mR && mG == a.mG && mB == a.mB && mA == a.mA;
   }
   bool operator!=(const RGBAFloat& a) {
      return !operator==(a);
   }

   RGBAFloatComp mR, mG, mB, mA;
};

/** Class encapsulating an image containing float-based R,G,B,A channels.
 *
 * Internal representation assumes data is in the ABGR format, with the RGB
 * color channels premultiplied by the alpha value.  Premultiplied alpha is
 * often also called "associated alpha" - see the tiff 6 specification for some
 * discussion - http://partners.adobe.com/asn/developer/PDFS/TN/TIFF6.pdf
 *
 */
class RGBAFloatImage
{
   RGBAFloatImage(const RGBAFloatImage&);
   RGBAFloatImage& operator=(const RGBAFloatImage&);

public:
   RGBAFloatImage(int w, int h, const char *name = 0) {
      Width = w;
      Height = h;
      if (name) Name = name;
      Data = new RGBAFloat[w * h];
   };
   ~RGBAFloatImage() { if (Data) delete[] Data; }

   void Set(RGBAFloat rgba, int x, int y) {
      Data[x + y * Width] = rgba;
   }
   void Set(const RGBAFloatComp (&rgb)[3], unsigned int i) {
      Data[i].Set(rgb);
   }
   void Set(const RGBAFloatComp (&rgba)[4], unsigned int i) {
      Data[i].Set(rgba);
   }
   void Set(
         RGBAFloatComp r,
         RGBAFloatComp g,
         RGBAFloatComp b,
         RGBAFloatComp a,
         unsigned int i) {
      Data[i].Set(r, g, b, a);
   }
   void Set(
         RGBAInt32Comp r,
         RGBAInt32Comp g,
         RGBAInt32Comp b,
         RGBAInt32Comp a,
         unsigned int i) {
      Data[i].Set(r, g, b, a);
   }
   void Set(RGBAInt32 rgba, int i) {
      Data[i].Set(rgba);
   }

   RGBAFloat Get(int x, int y) const {
      return Data[x + y * Width];
   }
   RGBAFloat Get(int i) const {
      return Data[i];
   }
   uint32_t GetInt32(int i) {
      return Data[i].GetInt32();
   }
   void GetRGBTriplet(RGBAFloatComp(&rgb)[3], unsigned int i) {
      Data[i].GetRGBTriplet(rgb);
   }
   void GetRGBQuad(RGBAFloatComp(&rgb)[4], unsigned int i) {
      Data[i].GetRGBQuad(rgb);
   }

   RGBAFloatComp Get_Red(unsigned int i) {
      return Data[i].mR;
   }
   RGBAFloatComp Get_Green(unsigned int i) {
      return Data[i].mG;
   }
   RGBAFloatComp Get_Blue(unsigned int i) {
      return Data[i].mB;
   }
   RGBAFloatComp Get_Alpha(unsigned int i) {
      return Data[i].mA;
   }

   int Get_Width(void) const {
      return Width;
   }
   int Get_Height(void) const {
      return Height;
   }

   const std::string &Get_Name(void) const {
      return Name;
   }
   RGBAFloatImage* DownSample() const;

   bool WriteToFile(const char* filename);
   static RGBAFloatImage* ReadFromFile(const char* filename);

protected:
   static bool CanOpenFile(const char *filename);

protected:
   int Width;
   int Height;
   std::string Name;
   RGBAFloat *Data;
};

#endif
