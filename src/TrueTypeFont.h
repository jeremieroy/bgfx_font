/* Copyright 2013 Jeremie Roy. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#include "bgfx_font_types.h"

namespace bgfx_font
{
class TrueTypeFont
{
public:	
	TrueTypeFont();
	~TrueTypeFont();

	/// Initialize from  an external buffer
	/// @remark The ownership of the buffer is external, and you must ensure it stays valid up to this object lifetime
	/// @return true if the initialization succeed
    bool init(const uint8_t* buffer, uint32_t bufferSize, int32_t fontIndex, uint32_t pixelHeight );

	/// seems hacky :s
	bool isValid() { return m_font!=NULL; }

	/// return the font descriptor of the current font
	FontInfo getFontInfo();

	/// return the details of a glyph
	/// return false if the glyph couldn't be found
	//bool getGlyphInfo(const FontInfo& fontInfo, CodePoint_t codePoint, GlyphInfo& outGlyphInfo);

	/// raster a glyph as 8bit alpha to a memory buffer
	/// @ remark buffer min size: glyphInfo.width * glyphInfo * height * sizeof(char)
    bool bakeGlyphAlpha(const FontInfo& fontInfo,CodePoint_t codePoint, GlyphInfo& outGlyphInfo, uint8_t* outBuffer);

	/// raster a glyph as 32bit rgba to a memory buffer
	/// @ remark buffer min size: glyphInfo.width * glyphInfo * height * sizeof(uint32_t)
    bool bakeGlyphSubpixel(const FontInfo& fontInfo,CodePoint_t codePoint, GlyphInfo& outGlyphInfo, uint8_t* outBuffer);

	bool bakeGlyphDistance(const FontInfo& fontInfo,CodePoint_t codePoint, GlyphInfo& outGlyphInfo, uint8_t* outBuffer);

	//bool bakeGlyphDistance2(const FontInfo& fontInfo,CodePoint_t codePoint, GlyphInfo& outGlyphInfo, uint8_t* outBuffer);

private:
	void* m_font;
};

}
