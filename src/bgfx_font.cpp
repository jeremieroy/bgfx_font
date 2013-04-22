/* Copyright 2013 Jeremie Roy. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
*/
#include "bgfx_font.h"
#include "font_manager.h"
#include "text_buffer_manager.h"

#include <assert.h>
#include <unordered_map>

namespace bgfx_font
{

struct Context
{	
	bgfx_font::FontManager fontManager;	
	bgfx_font::TextBufferManager* textManager;	
};

static Context* g_context = NULL;

void init(const char* shaderPath)
{
	assert(g_context == NULL && "A context can only be initialized once");
	g_context = new Context;	
	g_context->textManager = new bgfx_font::TextBufferManager(&g_context->fontManager);
	g_context->textManager->init(shaderPath);
}
		
void shutdown()
{
	assert(g_context != NULL && "Context not initialized");		
	delete g_context->textManager;
	delete g_context;
	g_context = NULL;	
}

TrueTypeHandle loadTrueTypeFont(const char * _fontPath)
{
	assert(g_context != NULL && "Context not initialized. Call bgfx_text::init(); ");
	return (TrueTypeHandle)g_context->fontManager.loadTrueTypeFromFile(_fontPath, 0);	
}

TrueTypeHandle loadTrueTypeFont(const bgfx::Memory* _mem)
{
	assert(g_context != NULL && "Context not initialized. Call bgfx_text::init(); ");
	assert(false && "Not implemented yet");
	TrueTypeHandle invalid = BGFX_INVALID_HANDLE;
	return invalid;	
}

void unloadTrueTypeFont(TrueTypeHandle _handle)
{
	assert(g_context != NULL && "Context not initialized. Call bgfx_text::init(); ");
	g_context->fontManager.unLoadTrueType((bgfx_font::TrueTypeHandle)_handle);	
}
		
FontHandle createFont(TrueTypeHandle _handle, uint32_t _typefaceIndex, uint32_t _pixelSize, FontType _fontType)
{
	assert(g_context != NULL && "Context not initialized. Call bgfx_text::init(); ");
	return (FontHandle) g_context->fontManager.createFontByPixelSize(_handle, _typefaceIndex, _pixelSize, _fontType);
}

FontHandle createScaledFont(FontHandle _baseFontHandle, uint32_t _pixelSize)
{
	assert(g_context != NULL && "Context not initialized. Call bgfx_text::init(); ");
	return (FontHandle) g_context->fontManager.createScaledFontToPixelSize(_baseFontHandle, _pixelSize);
}

FontHandle loadBakedFont(const char * _fontPath, const char * _fontName)
{
	assert(g_context != NULL && "Context not initialized. Call bgfx_text::init(); ");
	assert(false && "Not implemented yet");
	FontHandle invalid = BGFX_INVALID_HANDLE;
	return invalid;
}

void destroyFont(FontHandle _handle)
{
	g_context->fontManager.destroyFont(_handle);
}

bool preloadGlyph(FontHandle _handle, const wchar_t* _string)
{
	assert(g_context != NULL && "Context not initialized. Call bgfx_text::init(); ");
	return g_context->fontManager.preloadGlyph(_handle, _string);
}

void bakeAndSaveFont(FontHandle _handle, const char * _fontPath, const char * _fontName)
{
	assert(g_context != NULL && "Context not initialized. Call bgfx_text::init(); ");
	assert(false && "Not implemented yet");
}

TextBufferHandle createTextBuffer(FontType _type, BufferType bufferType)
{
	assert(g_context != NULL && "Context not initialized. Call bgfx_text::init(); ");
	return g_context->textManager->createTextBuffer(_type,bufferType);
}

void destroyTextBuffer(TextBufferHandle _handle)
{
	assert(g_context != NULL && "Context not initialized. Call bgfx_text::init(); ");
	g_context->textManager->destroyTextBuffer(_handle);	
}

void setTextStyle(TextBufferHandle _handle, uint32_t flags)
{
	assert(g_context != NULL && "Context not initialized. Call bgfx_text::init(); ");
	g_context->textManager->setStyle(_handle, flags);
}

void setTextColor(TextBufferHandle _handle, uint32_t _rgba)
{
	assert(g_context != NULL && "Context not initialized. Call bgfx_text::init(); ");
	g_context->textManager->setTextColor(_handle, _rgba);
}
	
void setTextBackgroundColor(TextBufferHandle _handle, uint32_t _rgba)
{
	assert(g_context != NULL && "Context not initialized. Call bgfx_text::init(); ");
	g_context->textManager->setBackgroundColor(_handle, _rgba);
}

void setOverlineColor(TextBufferHandle _handle, uint32_t _rgba)
{
	assert(g_context != NULL && "Context not initialized. Call bgfx_text::init(); ");
	g_context->textManager->setOverlineColor(_handle, _rgba);
}

void setUnderlineColor(TextBufferHandle _handle, uint32_t _rgba)
{
	assert(g_context != NULL && "Context not initialized. Call bgfx_text::init(); ");
	g_context->textManager->setUnderlineColor(_handle, _rgba);
}
	
void setStrikeThroughColor(TextBufferHandle _handle, uint32_t _rgba )
{
	assert(g_context != NULL && "Context not initialized. Call bgfx_text::init(); ");
	g_context->textManager->setStrikeThroughColor(_handle, _rgba);
}
	
void setPenPosition(TextBufferHandle _handle, float x, float y)
{
	assert(g_context != NULL && "Context not initialized. Call bgfx_text::init(); ");
	g_context->textManager->setPenPosition(_handle, x,y);
}
	
void appendText(TextBufferHandle _handle, FontHandle fontHandle, const char * _string)
{
	assert(g_context != NULL && "Context not initialized. Call bgfx_text::init(); ");
	g_context->textManager->appendText(_handle, fontHandle, _string);
}

void appendText(TextBufferHandle _handle, FontHandle fontHandle, const wchar_t * _string)
{
	assert(g_context != NULL && "Context not initialized. Call bgfx_text::init(); ");
	g_context->textManager->appendText(_handle, fontHandle, _string);
}

void clearTextBuffer(TextBufferHandle _handle)
{
	assert(g_context != NULL && "Context not initialized. Call bgfx_text::init(); ");
	g_context->textManager->clearTextBuffer(_handle);
}
	
void submitTextBuffer(TextBufferHandle _handle, uint8_t _id,  int32_t _depth)
{
	assert(g_context != NULL && "Context not initialized. Call bgfx_text::init(); ");
	g_context->textManager->submitTextBuffer(_handle, _id, _depth);	
}

void submitTextBufferMask(TextBufferHandle _handle, uint32_t _viewMask, int32_t _depth)
{
	assert(g_context != NULL && "Context not initialized. Call bgfx_text::init(); ");
	g_context->textManager->submitTextBufferMask(_handle, _viewMask, _depth);
}

}

