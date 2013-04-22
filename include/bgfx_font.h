/* Copyright 2013 Jeremie Roy. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#include <bgfx.h> 
#include "../src/font_manager.h"
#include "../src/text_buffer_manager.h"
namespace bgfx_font
{
	/// initialize bgfx_text library
	/// Create font rendering shader program, and vertex format.
	/// @remark assume bgfx is initialized
	void init(const char* shaderPath);	

	/// shutdown bgfx_text library
	/// @remark assume bgfx is (still) initialized
	void shutdown();

	/// retrieve the handle to texture cube resource used by the font system (e.g. to render it)
	/// The bgfx texture belong to the system, do not destroy it ! 
	bgfx::TextureHandle getTextureHandle();

	//TODO access to rectanglePacker as atlas

	/// Load a truetype resource from a file, glyph can be generated if the font is loaded
	TrueTypeHandle loadTrueTypeFont(const char * _fontPath);

	/// Load a truetype resource from a buffer, glyph can be generated if the font is loaded
	TrueTypeHandle loadTrueTypeFont(const bgfx::Memory* _mem);
	
	/// free the resource allocated for the font (but keep loaded glyphs)
	void unloadTrueTypeFont(TrueTypeHandle _handle);

	/// return a font descriptor to a truetype font whose height is a fixed size in pixels
	FontHandle createFont(TrueTypeHandle _handle, uint32_t _typefaceIndex, uint32_t _heightPx, FontType _fontType = FONT_TYPE_ALPHA);
	
	/// return a font descriptor to a font which is a scaled verstion of another font. Best used with distance field font.
	FontHandle createScaledFont(FontHandle _baseFontHandle, uint32_t _heightPx);
	
	/// Load a baked font and return a font descriptor corresponding to a baked font
	FontHandle loadBakedFont(const char * _fontPath, const char * _fontName);
	//TODO load from a given buffer

	void destroyFont(FontHandle _handle);

	/// Preload a set of glyphs from a TrueType file
	/// @return true if every glyph could be preloaded, false otherwise
	/// if the Font is a baked font, this only do validation on the characters
	bool preloadGlyph(FontHandle _handle, const wchar_t* _string);

	/// bake a font and save it to disk
	void bakeAndSaveFont(FontHandle _handle, const char * _fontPath, const char * _fontName);
		
	/// Create a text buffer of the specified font type. It will only be compatible with fonts of the same type.
	TextBufferHandle createTextBuffer(FontType _type, BufferType bufferType);
	
	/// Destroy a text buffer
	void destroyTextBuffer(TextBufferHandle _handle);

	/// Set the current style state of the text buffer
	void setTextStyle(TextBufferHandle _handle, uint32_t _flags = STYLE_NORMAL);

	/// Set the current text color state of the text buffer
	void setTextColor(TextBufferHandle _handle, uint32_t _rgba = 0x000000ff);
	
	/// Set the current background color state of the text buffer
	void setTextBackgroundColor(TextBufferHandle _handle, uint32_t _rgba = 0x000000FF);

	/// Set the overline color state of the text buffer
	void setOverlineColor(TextBufferHandle _handle,uint32_t _rgba = 0x000000FF);

	/// Set the underline color state of the text buffer
	void setUnderlineColor(TextBufferHandle _handle,uint32_t _rgba = 0x000000FF);
	
	/// Set the strike trough color state of the text buffer
	void setStrikeThroughColor(TextBufferHandle _handle,uint32_t _rgba = 0x000000FF);
	
	/// Set the pen position state of the text buffer
	void setPenPosition(TextBufferHandle _handle, float x, float y);
	
	/// append an ASCII/utf-8 string to the buffer using current pen position and color
	void appendText(TextBufferHandle _handle, FontHandle _fontHandle, const char * _string);

	/// append a wide char unicode string to the buffer using current pen position and color
	void appendText(TextBufferHandle _handle, FontHandle _fontHandle, const wchar_t * _string);

	/// Clear the text buffer and reset its state (pen/color)
	void clearTextBuffer(TextBufferHandle _handle);
	
	/// submit the text buffer for rendering
	void submitTextBuffer(TextBufferHandle _handle, uint8_t _id, int32_t _depth = 0);

	/// Submit the text buffer for rendering into multiple views.
	void submitTextBufferMask(TextBufferHandle _handle, uint32_t _viewMask, int32_t _depth = 0);
}

