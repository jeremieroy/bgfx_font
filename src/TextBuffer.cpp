/* Copyright 2013 Jeremie Roy. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
*/
#include "TextBuffer.h"
#include "FontManager.h"
#include <assert.h>
#include <math.h>
#include <stddef.h>     /* offsetof */



// Table from Flexible and Economical UTF-8 Decoder
// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

static const uint8_t utf8d[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
  8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
  0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
  0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
  0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
  1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
  1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
  1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
};

#define UTF8_ACCEPT 0
#define UTF8_REJECT 1

inline uint32_t utf8_decode(uint32_t* state, uint32_t* codep, uint32_t byte) {
  uint32_t type = utf8d[byte];

  *codep = (*state != UTF8_ACCEPT) ?
    (byte & 0x3fu) | (*codep << 6) :
    (0xff >> type) & (byte);

  *state = utf8d[256 + *state*16 + type];
  return *state;
}

inline int utf8_strlen(uint8_t* s, size_t* count) {
  uint32_t codepoint;
  uint32_t state = 0;

  for (*count = 0; *s; ++s)
    if (!utf8_decode(&state, &codepoint, *s))
      *count += 1;

  return state != UTF8_ACCEPT;
}


namespace bgfx_font
{


TextBuffer::TextBuffer(FontManager* fontManager)
{		
	m_styleFlags = STYLE_NORMAL;
	//0xAABBGGRR
	m_textColor = 0xFFFFFFFF;
	m_backgroundColor = 0xFFFFFFFF;
	m_backgroundColor = 0xFFFFFFFF;
	m_overlineColor = 0xFFFFFFFF;
	m_underlineColor = 0xFFFFFFFF;
	m_strikeThroughColor = 0xFFFFFFFF;
	m_penX = 0;
	m_penY = 0;
	m_originX = 0;
	m_originY = 0;
	m_lineAscender = 0;
	m_lineDescender = 0;
	m_lineGap = 0;
	m_fontManager = fontManager;	

	
	m_vertexBuffer = new TextVertex[MAX_BUFFERED_CHARACTERS * 4];
	m_indexBuffer = new uint16_t[MAX_BUFFERED_CHARACTERS * 6];
	m_styleBuffer = new uint8_t[MAX_BUFFERED_CHARACTERS * 4];
	m_vertexCount = 0;
	m_indexCount = 0;
	m_lineStartIndex = 0;
	
	
}

TextBuffer::~TextBuffer()
{
	delete[] m_vertexBuffer;
	delete[] m_indexBuffer;
}

void TextBuffer::appendText(FontHandle fontHandle, const char * _string)
{	
	/*
	TextLayoutHelper helper(m_fontManager);
	helper.parseText(fontHandle,_string, helper.Measure );
	printf("size= %f - %f\n", helper.getWidth(), helper.getHeight());
	m_layoutHelper.parseText(fontHandle, _string, m_bufferBehavior);
	return;
	*/

	GlyphInfo glyph;
	const FontInfo& font = m_fontManager->getFontInfo(fontHandle);
	//assert((m_textureHandle.idx ==  m_fontManager->getTextureHandle(font.textureAtlas).idx) && "You cannot mix font from different atlas in the same textbuffer");
		
	if(m_vertexCount == 0)
	{
		m_originX = m_penX;
		m_originY = m_penY;
		m_lineDescender = 0;// font.descender;
		m_lineAscender = 0;//font.ascender;
	}
	
	uint32_t codepoint;
	uint32_t state = 0;

	for (; *_string; ++_string)
		if (!utf8_decode(&state, &codepoint, *_string))
		{
			if(m_fontManager->getGlyphInfo(fontHandle, (CodePoint_t)codepoint, glyph))
			{
				appendGlyph((CodePoint_t)codepoint, font, glyph);
			}else
			{
				assert(false && "Glyph not found");
			}
		}
	  //printf("U+%04X\n", codepoint);

	if (state != UTF8_ACCEPT)
	{
	//	assert(false && "The string is not well-formed");
		return; //"The string is not well-formed\n"
	}
}

void TextBuffer::appendText(FontHandle fontHandle, const wchar_t * _string)
{	
	
	GlyphInfo glyph;
	const FontInfo& font = m_fontManager->getFontInfo(fontHandle);
	//assert((m_textureHandle.idx ==  m_fontManager->getTextureHandle(font.textureAtlas).idx) && "You cannot mix font from different atlas in the same textbuffer");
	
	if(m_vertexCount == 0)
	{
		m_originX = m_penX;
		m_originY = m_penY;
		m_lineDescender = 0;// font.descender;
		m_lineAscender = 0;//font.ascender;
		m_lineGap = 0;
	}

	//parse string
	for( size_t i=0, end = wcslen(_string) ; i < end; ++i )
	{
		//if glyph cached, continue
		uint32_t codePoint = _string[i];
		if(m_fontManager->getGlyphInfo(fontHandle, codePoint, glyph))
		{
			appendGlyph(codePoint, font, glyph);
		}else
		{
			assert(false && "Glyph not found");
		}
	}
}
/*
TextBuffer::Rectangle TextBuffer::measureText(FontHandle fontHandle, const char * _string)
{	
}

TextBuffer::Rectangle TextBuffer::measureText(FontHandle fontHandle, const wchar_t * _string)
{
}
*/

void TextBuffer::clearTextBuffer()
{
	m_vertexCount = 0;
	m_indexCount = 0;
	m_lineStartIndex = 0;
	m_lineAscender = 0;
	m_lineDescender = 0;
}

void TextBuffer::appendGlyph(CodePoint_t codePoint, const FontInfo& font, const GlyphInfo& glyphInfo)
{	
	//handle newlines
	if(codePoint == L'\n' )
    {
        m_penX = m_originX;
        m_penY -= m_lineDescender;
		m_penY += m_lineGap;
		m_lineDescender = 0;
		m_lineAscender = 0;
        m_lineStartIndex = m_vertexCount;
		return;
    }
	
	if( font.ascender > m_lineAscender || (font.descender < m_lineDescender) )
    {
		if( font.descender < m_lineDescender )
		{
			m_lineDescender = font.descender;
			m_lineGap = font.lineGap;
		}
				
		float txtDecals = (font.ascender - m_lineAscender);
		m_lineAscender = font.ascender;
		m_lineGap = font.lineGap;		
				
		m_penY += txtDecals;
		verticalCenterLastLine((txtDecals), (m_penY - m_lineAscender), (m_penY - m_lineDescender+m_lineGap));		
    }
			
	//handle kerning
	float kerning = 0;
	/*	
    if( previous && markup->font->kerning )
    {
        kerning = texture_glyph_get_kerning( glyph, previous );
    }
	*/
	m_penX += kerning * font.scale;

	GlyphInfo& blackGlyph = m_fontManager->getBlackGlyph();
	
	if( m_styleFlags & STYLE_BACKGROUND && m_backgroundColor & 0xFF000000)
	{
		float x0 = ( m_penX - kerning );
		float y0 = ( m_penY  - m_lineAscender);
		float x1 = ( (float)x0 + (glyphInfo.advance_x));
		float y1 = ( m_penY - m_lineDescender + m_lineGap );

		m_fontManager->getAtlas().packUV(blackGlyph.regionIndex, (uint8_t*)m_vertexBuffer,sizeof(TextVertex) *m_vertexCount + offsetof(TextVertex, u), sizeof(TextVertex));

		setVertex(m_vertexCount+0, font.scale, x0, y0, m_backgroundColor,STYLE_BACKGROUND);
		setVertex(m_vertexCount+1, font.scale, x0, y1, m_backgroundColor,STYLE_BACKGROUND);
		setVertex(m_vertexCount+2, font.scale, x1, y1, m_backgroundColor,STYLE_BACKGROUND);
		setVertex(m_vertexCount+3, font.scale, x1, y0, m_backgroundColor,STYLE_BACKGROUND);

		m_indexBuffer[m_indexCount + 0] = m_vertexCount+0;
		m_indexBuffer[m_indexCount + 1] = m_vertexCount+1;
		m_indexBuffer[m_indexCount + 2] = m_vertexCount+2;
		m_indexBuffer[m_indexCount + 3] = m_vertexCount+0;
		m_indexBuffer[m_indexCount + 4] = m_vertexCount+2;
		m_indexBuffer[m_indexCount + 5] = m_vertexCount+3;
		m_vertexCount += 4;
		m_indexCount += 6;
	}
	
	if( m_styleFlags & STYLE_UNDERLINE && m_underlineColor & 0xFF000000)
	{
		float x0 = ( m_penX - kerning );
		float y0 = (m_penY - m_lineDescender/2 );
		float x1 = ( (float)x0 + (glyphInfo.advance_x));
		float y1 = y0+font.underline_thickness;

		m_fontManager->getAtlas().packUV(blackGlyph.regionIndex, (uint8_t*)m_vertexBuffer,sizeof(TextVertex) *m_vertexCount + offsetof(TextVertex, u), sizeof(TextVertex));

		setVertex(m_vertexCount+0, font.scale, x0, y0, m_underlineColor,STYLE_UNDERLINE);
		setVertex(m_vertexCount+1, font.scale, x0, y1, m_underlineColor,STYLE_UNDERLINE);
		setVertex(m_vertexCount+2, font.scale, x1, y1, m_underlineColor,STYLE_UNDERLINE);
		setVertex(m_vertexCount+3, font.scale, x1, y0, m_underlineColor,STYLE_UNDERLINE);

		m_indexBuffer[m_indexCount + 0] = m_vertexCount+0;
		m_indexBuffer[m_indexCount + 1] = m_vertexCount+1;
		m_indexBuffer[m_indexCount + 2] = m_vertexCount+2;
		m_indexBuffer[m_indexCount + 3] = m_vertexCount+0;
		m_indexBuffer[m_indexCount + 4] = m_vertexCount+2;
		m_indexBuffer[m_indexCount + 5] = m_vertexCount+3;
		m_vertexCount += 4;
		m_indexCount += 6;
	}
	
	if( m_styleFlags & STYLE_OVERLINE && m_overlineColor & 0xFF000000)
	{
		float x0 = ( m_penX - kerning );
		float y0 = (m_penY - font.ascender );
		float x1 = ( (float)x0 + (glyphInfo.advance_x));
		float y1 = y0+font.underline_thickness;

		m_fontManager->getAtlas().packUV(blackGlyph.regionIndex, (uint8_t*)m_vertexBuffer,sizeof(TextVertex) *m_vertexCount + offsetof(TextVertex, u), sizeof(TextVertex));

		setVertex(m_vertexCount+0, font.scale, x0, y0, m_overlineColor,STYLE_OVERLINE);
		setVertex(m_vertexCount+1, font.scale, x0, y1, m_overlineColor,STYLE_OVERLINE);
		setVertex(m_vertexCount+2, font.scale, x1, y1, m_overlineColor,STYLE_OVERLINE);
		setVertex(m_vertexCount+3, font.scale, x1, y0, m_overlineColor,STYLE_OVERLINE);

		m_indexBuffer[m_indexCount + 0] = m_vertexCount+0;
		m_indexBuffer[m_indexCount + 1] = m_vertexCount+1;
		m_indexBuffer[m_indexCount + 2] = m_vertexCount+2;
		m_indexBuffer[m_indexCount + 3] = m_vertexCount+0;
		m_indexBuffer[m_indexCount + 4] = m_vertexCount+2;
		m_indexBuffer[m_indexCount + 5] = m_vertexCount+3;
		m_vertexCount += 4;
		m_indexCount += 6;
	}
	
		
	if( m_styleFlags & STYLE_STRIKE_THROUGH && m_strikeThroughColor & 0xFF000000)
	{
 		float x0 = ( m_penX - kerning );
		float y0 = (m_penY - font.ascender/3 );
		float x1 = ( (float)x0 + (glyphInfo.advance_x) );
		float y1 = y0+font.underline_thickness;
		
		m_fontManager->getAtlas().packUV(blackGlyph.regionIndex, (uint8_t*)m_vertexBuffer,sizeof(TextVertex) *m_vertexCount + offsetof(TextVertex, u), sizeof(TextVertex));

		setVertex(m_vertexCount+0, font.scale, x0, y0, m_strikeThroughColor,STYLE_STRIKE_THROUGH);
		setVertex(m_vertexCount+1, font.scale, x0, y1, m_strikeThroughColor,STYLE_STRIKE_THROUGH);
		setVertex(m_vertexCount+2, font.scale, x1, y1, m_strikeThroughColor,STYLE_STRIKE_THROUGH);
		setVertex(m_vertexCount+3, font.scale, x1, y0, m_strikeThroughColor,STYLE_STRIKE_THROUGH);

		m_indexBuffer[m_indexCount + 0] = m_vertexCount+0;
		m_indexBuffer[m_indexCount + 1] = m_vertexCount+1;
		m_indexBuffer[m_indexCount + 2] = m_vertexCount+2;
		m_indexBuffer[m_indexCount + 3] = m_vertexCount+0;
		m_indexBuffer[m_indexCount + 4] = m_vertexCount+2;
		m_indexBuffer[m_indexCount + 5] = m_vertexCount+3;
		m_vertexCount += 4;
		m_indexCount += 6;
	}
	

	//handle glyph
	float x0_precise = m_penX + (glyphInfo.offset_x);
	float x0 = ( x0_precise);
	float y0 = ( m_penY + (glyphInfo.offset_y));
	float x1 = ( x0 + glyphInfo.width );
	float y1 = ( y0 + glyphInfo.height );

	float shift = x0_precise - x0;
	
	m_fontManager->getAtlas().packUV(glyphInfo.regionIndex, (uint8_t*)m_vertexBuffer, sizeof(TextVertex) *m_vertexCount + offsetof(TextVertex, u), sizeof(TextVertex));

	setVertex(m_vertexCount+0, font.scale, x0, y0, m_textColor);
	setVertex(m_vertexCount+1, font.scale, x0, y1, m_textColor);
	setVertex(m_vertexCount+2, font.scale, x1, y1, m_textColor);
	setVertex(m_vertexCount+3, font.scale, x1, y0, m_textColor);

	m_indexBuffer[m_indexCount + 0] = m_vertexCount+0;
	m_indexBuffer[m_indexCount + 1] = m_vertexCount+1;
	m_indexBuffer[m_indexCount + 2] = m_vertexCount+2;
	m_indexBuffer[m_indexCount + 3] = m_vertexCount+0;
	m_indexBuffer[m_indexCount + 4] = m_vertexCount+2;
	m_indexBuffer[m_indexCount + 5] = m_vertexCount+3;
	m_vertexCount += 4;
	m_indexCount += 6;
	
	//TODO see what to do when doing subpixel rendering
	m_penX += glyphInfo.advance_x;
}

void TextBuffer::verticalCenterLastLine(float dy, float top, float bottom)
{		
	for( size_t i=m_lineStartIndex; i < m_vertexCount; i+=4 )
    {	
		if( m_styleBuffer[i] == STYLE_BACKGROUND)
		{
			m_vertexBuffer[i+0].y = top;
			m_vertexBuffer[i+1].y = bottom;
			m_vertexBuffer[i+2].y = bottom;
			m_vertexBuffer[i+3].y = top;
		}else{
			m_vertexBuffer[i+0].y += dy;
			m_vertexBuffer[i+1].y += dy;
			m_vertexBuffer[i+2].y += dy;
			m_vertexBuffer[i+3].y += dy;
		}
    }
}

}
