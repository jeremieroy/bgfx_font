/* Copyright 2013 Jeremie Roy. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
*/
#pragma once

#include "FontManager.h"

namespace bgfx_font
{



class TextBuffer
{
public:
	
	/// TextBuffer is bound to a fontManager for glyph retrieval
	/// @remark the ownership of the manager is not taken
	TextBuffer(FontManager* fontManager=NULL);
	~TextBuffer();
	void setFontManager(FontManager* mgr) { m_fontManager = mgr; }

	void setStyle(uint32_t flags = STYLE_NORMAL) { m_styleFlags = flags; }
	void setTextColor(uint32_t rgba = 0x000000FF) { m_textColor = toABGR(rgba); }
	void setBackgroundColor(uint32_t rgba = 0x000000FF) { m_backgroundColor = toABGR(rgba); }

	void setOverlineColor(uint32_t rgba = 0x000000FF) { m_overlineColor = toABGR(rgba); }
	void setUnderlineColor(uint32_t rgba = 0x000000FF) { m_underlineColor = toABGR(rgba); }
	void setStrikeThroughColor(uint32_t rgba = 0x000000FF) { m_strikeThroughColor = toABGR(rgba); }
	
	void setPenPosition(float x, float y) { m_penX = x; };// m_penY = y; }

	/// append an ASCII/utf-8 string to the buffer using current pen position and color
	void appendText(FontHandle fontHandle, const char * _string);

	/// append a wide char unicode string to the buffer using current pen position and color
	void appendText(FontHandle fontHandle, const wchar_t * _string);

	/// Clear the text buffer and reset its state (pen/color)
	void clearTextBuffer();
	
	/// get pointer to the vertex buffer to submit it to the graphic card
	const uint8_t* getVertexBuffer(){ return (uint8_t*) m_vertexBuffer; }
	/// number of vertex in the vertex buffer
	uint32_t getVertexCount(){ return m_vertexCount; }
	/// size in bytes of a vertex
	uint32_t getVertexSize(){ return sizeof(TextVertex); }
		
	/// get a pointer to the index buffer to submit it to the graphic
	const uint16_t* getIndexBuffer(){ return m_indexBuffer; }
	/// number of index in the index buffer
	uint32_t getIndexCount(){ return m_indexCount; }
	/// size in bytes of an index
	uint32_t getIndexSize(){ return sizeof(uint16_t); }

	uint32_t getTextColor(){ return toABGR(m_textColor); }
private:
	void appendGlyph(CodePoint_t codePoint, const FontInfo& font, const GlyphInfo& glyphInfo);
	void verticalCenterLastLine(float txtDecalY, float top, float bottom);
	uint32_t toABGR(uint32_t rgba) 
	{ 
		return (((rgba >> 0) & 0xff) << 24) |  
			(((rgba >> 8) & 0xff) << 16) |    
			(((rgba >> 16) & 0xff) << 8) |    
			(((rgba >> 24) & 0xff) << 0);   
	}

	uint32_t m_styleFlags;

	// color states
	uint32_t m_textColor;
	uint32_t m_backgroundColor;
	uint32_t m_overlineColor;
	uint32_t m_underlineColor;
	uint32_t m_strikeThroughColor;

	//position states	
	float m_penX;
	float m_penY;

	float m_originX;
	float m_originY;	

	float m_lineAscender;
	float m_lineDescender;
	float m_lineGap;
	
	///
	FontManager* m_fontManager;	
	
	void setVertex(size_t i, float scale, float x, float y, uint32_t rgba, uint8_t style = STYLE_NORMAL)
	{
		m_vertexBuffer[i].x = x;
		m_vertexBuffer[i].y = y;		
		m_vertexBuffer[i].rgba = rgba;
		m_styleBuffer[i] = style;
	}

	struct TextVertex
	{		
		float x,y;
		int16_t u,v,w,t;
		uint32_t rgba;		
	};

	TextVertex* m_vertexBuffer;
	uint16_t* m_indexBuffer;
	uint8_t* m_styleBuffer;
	
	size_t m_vertexCount;
	size_t m_indexCount;
	size_t m_lineStartIndex;
};

}
