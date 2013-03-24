/* Copyright 2013 Jeremie Roy. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
*/
#include <assert.h>
#include "FontManager.h"
#include <math.h>


#define BGFX_FONT_ASSERT(cond, message) assert((cond) && (message));

namespace bgfx_font
{
const uint16_t MAX_OPENED_FILES = 64;
const uint16_t MAX_OPENED_FONT = 64;
const uint32_t MAX_FONT_BUFFER_SIZE = 512*512*4;

FontManager::FontManager(uint32_t textureSideWidth, bgfx::TextureHandle _handle):m_filesHandles(MAX_OPENED_FILES), m_fontHandles(MAX_OPENED_FONT)
{
	m_cachedFiles = new CachedFile[MAX_OPENED_FILES];
	m_cachedFonts = new CachedFont[MAX_OPENED_FONT];
	m_buffer = new uint8_t[MAX_FONT_BUFFER_SIZE];	
	m_textureHandle = _handle;
	initAtlas(textureSideWidth);
	m_ownTexture = false;
}

FontManager::FontManager(uint32_t textureSideWidth):m_filesHandles(MAX_OPENED_FILES), m_fontHandles(MAX_OPENED_FONT)
{
	m_cachedFiles = new CachedFile[MAX_OPENED_FILES];
	m_cachedFonts = new CachedFont[MAX_OPENED_FONT];
	m_buffer = new uint8_t[MAX_FONT_BUFFER_SIZE];
	m_packer.init(textureSideWidth);
	createAtlas(textureSideWidth);
	initAtlas(textureSideWidth);
	m_ownTexture = true;
}

FontManager::~FontManager()
{
	assert(m_fontHandles.getNumHandles() == 0 && "All the fonts must be destroyed before destroying the manager");
	delete [] m_cachedFonts;

	assert(m_filesHandles.getNumHandles() == 0 && "All the font files must be destroyed before destroying the manager");
	delete [] m_cachedFiles;	
	
	delete [] m_buffer;
	if(m_ownTexture)
	{
		//destroy the texture atlas
		bgfx::destroyTexture(m_textureHandle);
	}
}

void FontManager::createAtlas(uint32_t textureSideWidth)
{
	assert(textureSideWidth >= 64 );
	//BGFX_TEXTURE_MIN_POINT|BGFX_TEXTURE_MAG_POINT|BGFX_TEXTURE_MIP_POINT;
	//BGFX_TEXTURE_MIN_ANISOTROPIC|BGFX_TEXTURE_MAG_ANISOTROPIC|BGFX_TEXTURE_MIP_POINT
	//BGFX_TEXTURE_U_CLAMP|BGFX_TEXTURE_V_CLAMP
	uint32_t flags = 0;// BGFX_TEXTURE_MIN_ANISOTROPIC|BGFX_TEXTURE_MAG_ANISOTROPIC|BGFX_TEXTURE_MIP_POINT;

	//Uncomment this to debug atlas
	//const bgfx::Memory* mem = bgfx::alloc(width*height);
	//memset(mem->data, 0, mem->size);
	//const bgfx::Memory* mem = NULL;
		
	const uint32_t textureSide = 512;
	m_textureHandle = 
		bgfx::createTextureCube(6
			, textureSide
			, 1
			, bgfx::TextureFormat::L8
			, flags
			);	
}

void FontManager::initAtlas(uint32_t textureSideWidth)
{
	assert(textureSideWidth >= 64 );
	m_packer.init(textureSideWidth);

	m_textureWidth = textureSideWidth;
	m_depth = 1;

	// Create filler rectangle
	uint8_t buffer[4*4*4];
	memset( buffer, 255, 4 * 4 * 4);

	m_blackGlyph.width=3;
	m_blackGlyph.height=3;
	assert( addBitmap(m_blackGlyph, buffer) );
	//make sure the black glyph doesn't bleed
	int16_t texUnit = 65535 / m_textureWidth;
	m_blackGlyph.texture_x0 += texUnit;
	m_blackGlyph.texture_y0 += texUnit;
	m_blackGlyph.texture_x1 -= texUnit;
	m_blackGlyph.texture_y1 -= texUnit;
}

TrueTypeHandle FontManager::loadTrueTypeFromFile(const char* fontPath, int32_t fontIndex)
{
	FILE * pFile;
	pFile = fopen (fontPath, "rb");
	if (pFile==NULL)
	{
		return TrueTypeHandle(INVALID_HANDLE_ID);
	}
	
	// Go to the end of the file.
	if (fseek(pFile, 0L, SEEK_END) == 0)
	{
		// Get the size of the file.
		long bufsize = ftell(pFile);
		if (bufsize == -1) 
		{
			fclose(pFile);
			return TrueTypeHandle(INVALID_HANDLE_ID);
		}
		
		uint8_t* buffer = new uint8_t[bufsize];

		// Go back to the start of the file.
		fseek(pFile, 0L, SEEK_SET);

		// Read the entire file into memory.
		size_t newLen = fread((void*)buffer, sizeof(char), bufsize, pFile);						
		if (newLen == 0) 
		{
			fclose(pFile);
			delete [] buffer;
			return TrueTypeHandle(INVALID_HANDLE_ID);
		}
		fclose(pFile);

		uint16_t id = m_filesHandles.alloc();
		assert(id != bx::HandleAlloc::invalid);
		m_cachedFiles[id].buffer = buffer;
		m_cachedFiles[id].bufferSize = bufsize;
		return TrueTypeHandle(id);
	}
	//TODO validate font
	return TrueTypeHandle(INVALID_HANDLE_ID);
}

TrueTypeHandle FontManager::loadTrueTypeFromMemory(const uint8_t* buffer, uint32_t size, int32_t fontIndex)
{	
	uint16_t id = m_filesHandles.alloc();
	assert(id != bx::HandleAlloc::invalid);
	m_cachedFiles[id].buffer = new uint8_t[size];
	m_cachedFiles[id].bufferSize = size;
	memcpy(m_cachedFiles[id].buffer, buffer, size);
	
	//TODO validate font
	return TrueTypeHandle(id);
}

void FontManager::unLoadTrueType(TrueTypeHandle handle)
{
	assert(handle.isValid());
	delete m_cachedFiles[handle.idx].buffer;
	m_cachedFiles[handle.idx].bufferSize = 0;
	m_cachedFiles[handle.idx].buffer = NULL;
	m_filesHandles.free(handle.idx);
}

FontHandle FontManager::createFontByPixelSize(TrueTypeHandle handle, uint32_t typefaceIndex, uint32_t pixelSize, FontType fontType)
{
	assert(handle.isValid());	

	TrueTypeFont* ttf = new TrueTypeFont();
	if(!ttf->init(  m_cachedFiles[handle.idx].buffer,  m_cachedFiles[handle.idx].bufferSize, typefaceIndex, pixelSize))
	{
		delete ttf;
		return FontHandle(INVALID_HANDLE_ID);
	}
	
	uint16_t fontIdx = m_fontHandles.alloc();
	assert(fontIdx != bx::HandleAlloc::invalid);	
	
	m_cachedFonts[fontIdx].trueTypeFont = ttf;
	m_cachedFonts[fontIdx].fontInfo = ttf->getFontInfo();
	m_cachedFonts[fontIdx].fontInfo.fontType = fontType;	
	m_cachedFonts[fontIdx].fontInfo.pixelSize = pixelSize;
	m_cachedFonts[fontIdx].cachedGlyphs.clear();
	m_cachedFonts[fontIdx].masterFontHandle.idx = -1;
	return FontHandle(fontIdx);
}

FontHandle FontManager::createScaledFontToPixelSize(FontHandle _baseFontHandle, uint32_t _pixelSize)
{
	assert(_baseFontHandle.isValid());
	CachedFont& font = m_cachedFonts[_baseFontHandle.idx];
	FontInfo& fontInfo = font.fontInfo;

	FontInfo newFontInfo = fontInfo;
	newFontInfo.pixelSize = _pixelSize;
	newFontInfo.scale = (float)_pixelSize / (float) fontInfo.pixelSize;
	newFontInfo.ascender = (newFontInfo.ascender * newFontInfo.scale);
	newFontInfo.descender = (newFontInfo.descender * newFontInfo.scale);
	newFontInfo.lineGap = (newFontInfo.lineGap * newFontInfo.scale);
	newFontInfo.underline_thickness = (newFontInfo.underline_thickness * newFontInfo.scale);
	newFontInfo.underline_position = (newFontInfo.underline_position * newFontInfo.scale);


	uint16_t fontIdx = m_fontHandles.alloc();
	assert(fontIdx != bx::HandleAlloc::invalid);
	m_cachedFonts[fontIdx].cachedGlyphs.clear();
	m_cachedFonts[fontIdx].fontInfo = newFontInfo;
	m_cachedFonts[fontIdx].trueTypeFont = NULL;
	m_cachedFonts[fontIdx].masterFontHandle = _baseFontHandle;
	return FontHandle(fontIdx);
}

FontHandle FontManager::loadBakedFontFromFile(const char* fontPath,  const char* descriptorPath)
{
	assert(false); //TODO implement
	return FontHandle(INVALID_HANDLE_ID);
}

FontHandle FontManager::loadBakedFontFromMemory(const uint8_t* imageBuffer, uint32_t imageSize, const uint8_t* descriptorBuffer, uint32_t descriptorSize)
{
	assert(false); //TODO implement
	return FontHandle(INVALID_HANDLE_ID);
}

void FontManager::destroyFont(FontHandle _handle)
{
	assert(_handle.isValid());

	if(m_cachedFonts[_handle.idx].trueTypeFont != NULL)
	{
		delete m_cachedFonts[_handle.idx].trueTypeFont;
		m_cachedFonts[_handle.idx].trueTypeFont = NULL;
	}
	m_cachedFonts[_handle.idx].cachedGlyphs.clear();	
	m_fontHandles.free(_handle.idx);
}

bool FontManager::preloadGlyph(FontHandle handle, const wchar_t* _string)
{
	assert(handle.isValid());	
	CachedFont& font = m_cachedFonts[handle.idx];
	FontInfo& fontInfo = font.fontInfo;	

	//if truetype present
	if(font.trueTypeFont != NULL)
	{	
		//parse string
		for( size_t i=0, end = wcslen(_string) ; i < end; ++i )
		{
			//if glyph cached, continue
			CodePoint_t codePoint = _string[i];
			if(!preloadGlyph(handle, codePoint))
			{
				return false;
			}
		}
		return true;
	}

	return false;
}

bool FontManager::preloadGlyph(FontHandle handle, CodePoint_t codePoint)
{
	assert(handle.isValid());	
	CachedFont& font = m_cachedFonts[handle.idx];
	FontInfo& fontInfo = font.fontInfo;

	//check if glyph not already present
	GlyphHash_t::iterator iter = font.cachedGlyphs.find(codePoint);
	if(iter != font.cachedGlyphs.end())
	{
		return true;
	}

	//if truetype present
	if(font.trueTypeFont != NULL)
	{
		GlyphInfo glyphInfo;
		
		//bake glyph as bitmap to buffer
		switch(font.fontInfo.fontType)
		{
		case FONT_TYPE_ALPHA:
			font.trueTypeFont->bakeGlyphAlpha(fontInfo,codePoint, glyphInfo, m_buffer);
			break;
		case FONT_TYPE_LCD:
			font.trueTypeFont->bakeGlyphSubpixel(fontInfo,codePoint, glyphInfo, m_buffer);
			break;
		case FONT_TYPE_DISTANCE:
			font.trueTypeFont->bakeGlyphDistance(fontInfo,codePoint, glyphInfo, m_buffer);
			break;
		default:
			assert(false && "TextureType not supported yet");
		};

		//copy bitmap to texture
		if(!addBitmap(glyphInfo, m_buffer) )
		{
			return false;
		}

		glyphInfo.advance_x = (glyphInfo.advance_x * fontInfo.scale);
		glyphInfo.advance_y = (glyphInfo.advance_y * fontInfo.scale);
		glyphInfo.offset_x = (glyphInfo.offset_x * fontInfo.scale);
		glyphInfo.offset_y = (glyphInfo.offset_y * fontInfo.scale);
		glyphInfo.height = (glyphInfo.height * fontInfo.scale);
		glyphInfo.width =  (glyphInfo.width * fontInfo.scale);

		// store cached glyph
		font.cachedGlyphs[codePoint] = glyphInfo;
		return true;
	}else
	{
		//retrieve glyph from parent font if any
		if(font.masterFontHandle.isValid())
		{
			if(preloadGlyph(font.masterFontHandle, codePoint))
			{
				GlyphInfo glyphInfo;
				getGlyphInfo(font.masterFontHandle, codePoint, glyphInfo);

				glyphInfo.advance_x = (glyphInfo.advance_x * fontInfo.scale);
				glyphInfo.advance_y = (glyphInfo.advance_y * fontInfo.scale);
				glyphInfo.offset_x = (glyphInfo.offset_x * fontInfo.scale);
				glyphInfo.offset_y = (glyphInfo.offset_y * fontInfo.scale);
				glyphInfo.height = (glyphInfo.height * fontInfo.scale);
				glyphInfo.width = (glyphInfo.width * fontInfo.scale);

				// store cached glyph
				font.cachedGlyphs[codePoint] = glyphInfo;
				return true;
			}
		}
	}

	return false;
}

const FontInfo& FontManager::getFontInfo(FontHandle handle)
{ 
	assert(handle.isValid());
	return m_cachedFonts[handle.idx].fontInfo;
}

bool FontManager::getGlyphInfo(FontHandle fontHandle, CodePoint_t codePoint, GlyphInfo& outInfo)
{	
	GlyphHash_t::iterator iter = m_cachedFonts[fontHandle.idx].cachedGlyphs.find(codePoint);
	if(iter == m_cachedFonts[fontHandle.idx].cachedGlyphs.end())
	{
		if(preloadGlyph(fontHandle, codePoint))
		{
			iter = m_cachedFonts[fontHandle.idx].cachedGlyphs.find(codePoint);
		}else
		{
			return false;
		}
	}
	outInfo = iter->second;
	return true;
}

// ****************************************************************************


bool FontManager::addBitmap(GlyphInfo& glyphInfo, const uint8_t* data)
{
	uint16_t x,y,side;
	// We want each bitmap to be separated by at least one black pixel
	if(!m_packer.addRectangle((uint16_t) (glyphInfo.width + 1), (uint16_t) (glyphInfo.height + 1),  x, y,side))
	{
		return false;
	}

	//this allocation could maybe be avoided, will see later
	const bgfx::Memory* mem = bgfx::alloc((uint32_t)(glyphInfo.width*glyphInfo.height*m_depth));
	memcpy(mem->data, data, (size_t) (glyphInfo.width*glyphInfo.height*m_depth));	
	bgfx::updateTextureCube(m_textureHandle, (uint8_t)side, 0, x, y, (uint16_t) glyphInfo.width, (uint16_t) glyphInfo.height, mem);	

	glyphInfo.texture_x0 = x;
	glyphInfo.texture_y0 = y;
	glyphInfo.texture_x1 = x+(int16_t) glyphInfo.width;
	glyphInfo.texture_y1 = y+(int16_t) glyphInfo.height;

	float texMult = 65535.0f / ((float)(m_textureWidth));

	const float centering = 0.0;//5f;
	glyphInfo.texture_x0 =  ((int32_t)((glyphInfo.texture_x0 + centering) * texMult)-32768);
	glyphInfo.texture_y0 =  ((int32_t)((glyphInfo.texture_y0 + centering) * texMult)-32768);
	glyphInfo.texture_x1 =  ((int32_t)((glyphInfo.texture_x1 + centering) * texMult)-32768);
	glyphInfo.texture_y1 =  ((int32_t)((glyphInfo.texture_y1 + centering) * texMult)-32768);
	glyphInfo.side = side;

	//assert(((glyphInfo.texture_x0 * m_textureWidth)/65535) == x);
	//assert(((glyphInfo.texture_y0 * m_textureWidth)/65535) == y);
	//assert(((glyphInfo.texture_x1 * m_textureWidth)/65535) == x+glyphInfo.width);
	//assert(((glyphInfo.texture_y1 * m_textureWidth)/65535) == y+glyphInfo.height);

	return true;
}

}
