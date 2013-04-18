/* Copyright 2013 Jeremie Roy. All rights reserved.
 * License: http://www.opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#include <stdint.h> // uint32_t

namespace bgfx_font
{	
	const uint16_t INVALID_HANDLE_ID = UINT16_MAX;
	#define BGFX_FONT_HANDLE(_name) struct _name { uint16_t idx; explicit _name(uint16_t _idx = INVALID_HANDLE_ID):idx(_idx){} bool isValid(){return idx != INVALID_HANDLE_ID;} }
	// @ note to myself
	// would be better if I didn't get issues with Initializer lists :  if (handle != { UINT16_MAX }) ...

	BGFX_FONT_HANDLE(TrueTypeHandle);
	BGFX_FONT_HANDLE(FontHandle);	
	BGFX_FONT_HANDLE(TextBufferHandle);	
		
	/// Type of rendering used for the font (determine texture format and shader type)
	/// @remark Encode texture compatibility on the low bits
	enum FontType
	{
		FONT_TYPE_ALPHA    = 0x00000100 , // L8
		FONT_TYPE_LCD      = 0x00000200,  // BGRA8
		FONT_TYPE_RGBA     = 0x00000300,  // BGRA8
		FONT_TYPE_DISTANCE = 0x00000400,   // L8
		FONT_TYPE_DISTANCE_SUBPIXEL = 0x00000500   // L8
	};
	
	/// type of vertex and index buffer to use with a TextBuffer
	enum BufferType
	{
		STATIC,
		DYNAMIC ,
		TRANSIENT
	};

	/// special style effect (can be combined)
	enum TextStyleFlags
	{
		STYLE_NORMAL           = 0,
		STYLE_OVERLINE         = 1,
		STYLE_UNDERLINE        = 1<<1,
		STYLE_STRIKE_THROUGH   = 1<<2,
		STYLE_BACKGROUND       = 1<<3,
	};
	
	struct FontInfo
	{
		//the font height in pixel 
		uint16_t pixelSize;
		/// Rendering type used for the font
		int16_t fontType;

		/// The pixel extents above the baseline in pixels (typically positive)
		float ascender;
		/// The extents below the baseline in pixels (typically negative)
		float descender;
		/// The spacing in pixels between one row's descent and the next row's ascent
		float lineGap;
		/// The thickness of the under/hover/striketrough line in pixels
		float underline_thickness;
		/// The position of the underline relatively to the baseline
		float underline_position;
				
		//scale to apply to glyph data
		float scale;
	};

// Glyph metrics:
// --------------
//
//                       xmin                     xmax
//                        |                         |
//                        |<-------- width -------->|
//                        |                         |    
//              |         +-------------------------+----------------- ymax
//              |         |    ggggggggg   ggggg    |     ^        ^
//              |         |   g:::::::::ggg::::g    |     |        | 
//              |         |  g:::::::::::::::::g    |     |        | 
//              |         | g::::::ggggg::::::gg    |     |        | 
//              |         | g:::::g     g:::::g     |     |        | 
//    offset_x -|-------->| g:::::g     g:::::g     |  offset_y    | 
//              |         | g:::::g     g:::::g     |     |        | 
//              |         | g::::::g    g:::::g     |     |        | 
//              |         | g:::::::ggggg:::::g     |     |        |  
//              |         |  g::::::::::::::::g     |     |      height
//              |         |   gg::::::::::::::g     |     |        | 
//  baseline ---*---------|---- gggggggg::::::g-----*--------      |
//            / |         |             g:::::g     |              | 
//     origin   |         | gggggg      g:::::g     |              | 
//              |         | g:::::gg   gg:::::g     |              | 
//              |         |  g::::::ggg:::::::g     |              | 
//              |         |   gg:::::::::::::g      |              | 
//              |         |     ggg::::::ggg        |              | 
//              |         |         gggggg          |              v
//              |         +-------------------------+----------------- ymin
//              |                                   |
//              |------------- advance_x ---------->|

	/// Unicode value of a character
	typedef int32_t CodePoint_t;

	/// A structure that describe a glyph.	
	struct GlyphInfo
	{
		/// Index for faster retrieval
		int32_t glyphIndex;
	
		/// Glyph's width in pixels.
		float width;

		/// Glyph's height in pixels.
		float height;
	
		/// Glyph's left offset in pixels
		float offset_x;

		/// Glyph's top offset in pixels
		/// Remember that this is the distance from the baseline to the top-most
		/// glyph scan line, upwards y coordinates being positive.
		float offset_y;

		/// For horizontal text layouts, this is the unscaled horizontal distance in pixels
		/// used to increment the pen position when the glyph is drawn as part of a string of text.
		float advance_x;
	
		/// For vertical text layouts, this is the unscaled vertical distance in pixels
		/// used to increment the pen position when the glyph is drawn as part of a string of text.
		float advance_y;
		
		/// region index in the atlas storing textures
		uint16_t regionIndex;
		///32 bits alignment
		int16_t padding;		
	};
	
	
}
