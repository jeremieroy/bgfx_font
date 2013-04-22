/* Copyright 2013 Jeremie Roy. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
*/
#pragma once

#include "bgfx_font_types.h"
#include "cube_atlas.h"
#include <bgfx.h>
#include <bx/handlealloc.h>

namespace bgfx_font
{
class FontManager
{
public:
	/// create the font manager using an external cube atlas (doesn't take ownership of the atlas)
	FontManager(Atlas* atlas);
	/// create the font manager and create the texture cube as BGRA8 with linear filtering
	FontManager(uint32_t textureSideWidth = 512);

	~FontManager();

	/// retrieve the atlas used by the font manager (e.g. to add stuff to it)
	Atlas& getAtlas() { return (*m_atlas); }	
	
	/// load a TrueType font from a file path
	/// @return invalid handle if the loading fail
	TrueTypeHandle loadTrueTypeFromFile(const char* fontPath, int32_t fontIndex = 0);

	/// load a TrueType font from a given buffer.
	/// the buffer is copied and thus can be freed or reused after this call
	/// @return invalid handle if the loading fail
	TrueTypeHandle loadTrueTypeFromMemory(const uint8_t* buffer, uint32_t size, int32_t fontIndex = 0);

	/// unload a TrueType font (free font memory) but keep loaded glyphs
	void unLoadTrueType(TrueTypeHandle handle);
	
	/// return a font whose height is a fixed pixel size	
	FontHandle createFontByPixelSize(TrueTypeHandle handle, uint32_t typefaceIndex, uint32_t pixelSize, FontType fontType = FONT_TYPE_ALPHA);

	/// return a scaled child font whose height is a fixed pixel size
	FontHandle createScaledFontToPixelSize(FontHandle baseFontHandle, uint32_t pixelSize);

	/// load a baked font (the set of glyph is fixed)
	/// @return INVALID_HANDLE if the loading fail
	FontHandle loadBakedFontFromFile(const char* imagePath, const char* descriptorPath);

	/// load a baked font (the set of glyph is fixed)
	/// @return INVALID_HANDLE if the loading fail
	FontHandle loadBakedFontFromMemory(const uint8_t* imageBuffer, uint32_t imageSize, const uint8_t* descriptorBuffer, uint32_t descriptorSize);

	/// destroy a font (truetype or baked)
	void destroyFont(FontHandle _handle);

	/// Preload a set of glyphs from a TrueType file
	/// @return true if every glyph could be preloaded, false otherwise	
	/// if the Font is a baked font, this only do validation on the characters
	bool preloadGlyph(FontHandle handle, const wchar_t* _string);

	/// Preload a single glyph, return true on success
	bool preloadGlyph(FontHandle handle, CodePoint_t character);

	/// bake a font to disk (the set of preloaded glyph)
	/// @return true if the baking succeed, false otherwise
	bool saveBakedFont(FontHandle handle, const char* fontDirectory, const char* fontName );
	
	/// return the font descriptor of a font
	/// @remark the handle is required to be valid
	const FontInfo& getFontInfo(FontHandle handle);
	
	/// Return the rendering informations about the glyph region
	/// Load the glyph from a TrueType font if possible
	/// @return true if the Glyph is available
	bool getGlyphInfo(FontHandle fontHandle, CodePoint_t codePoint, GlyphInfo& outInfo);	

	GlyphInfo& getBlackGlyph(){ return m_blackGlyph; }

	class TrueTypeFont; //public to shut off Intellisense warning
private:
	
	struct CachedFont;
	struct CachedFile
	{		
		uint8_t* buffer;
		uint32_t bufferSize;
	};	

	void init(uint32_t textureSideWidth);
	bool addBitmap(GlyphInfo& glyphInfo, const uint8_t* data);	

	bool m_ownAtlas;
	Atlas* m_atlas;
	
	bx::HandleAlloc m_fontHandles;	
	CachedFont* m_cachedFonts;	
	
	bx::HandleAlloc m_filesHandles;
	CachedFile* m_cachedFiles;	
		
	GlyphInfo m_blackGlyph;

	//temporary buffer to raster glyph
	uint8_t* m_buffer;	
};

}
