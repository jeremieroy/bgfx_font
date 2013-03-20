#include "TrueTypeFont.h"
/*
#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       { 0, 0 } };
const struct {
    int          code;
    const char*  message;
} FT_Errors[] =
*/
#include "FreeType.h"
#include "edtaa3func.h"
//#include "SEDT.h"
#include <assert.h>
#include <math.h>


struct FTHolder
{
	FT_Library library;
	FT_Face face;
};

namespace bgfx_font
{

TrueTypeFont::TrueTypeFont(): m_font(NULL)
{	
}

TrueTypeFont::~TrueTypeFont()
{
	if(m_font!=NULL)
	{
		FTHolder* holder = (FTHolder*) m_font;
		FT_Done_Face( holder->face );
        FT_Done_FreeType( holder->library );
		delete m_font;
		m_font = NULL;
	}
}

bool TrueTypeFont::init(const uint8_t* buffer, uint32_t bufferSize, int32_t fontIndex, uint32_t pixelHeight)
{
	assert((bufferSize > 256 && bufferSize < 100000000) && "TrueType buffer size is suspicious");
	assert((pixelHeight > 4 && pixelHeight < 128) && "TrueType buffer size is suspicious");
	
	assert(m_font == NULL && "TrueTypeFont already initialized" );
	
	FTHolder* holder = new FTHolder();	

	// Initialize Freetype library
	FT_Error error = FT_Init_FreeType( &holder->library );
	if( error)
	{
		delete holder;
		return false;
	}

	error = FT_New_Memory_Face( holder->library, buffer, bufferSize, fontIndex, &holder->face );
	if ( error == FT_Err_Unknown_File_Format )
	{		
		// the font file could be opened and read, but it appears
		//that its font format is unsupported
		FT_Done_FreeType( holder->library );
		delete holder;
		return false;
	}
	else if ( error )
	{
		// another error code means that the font file could not
		// be opened or read, or simply that it is broken...
		FT_Done_FreeType( holder->library );
		delete holder;
		return false;
	}

    // Select unicode charmap 
    error = FT_Select_Charmap( holder->face, FT_ENCODING_UNICODE );
    if( error )
    {
		FT_Done_Face( holder->face );
		FT_Done_FreeType( holder->library );
        return false;
    }
	//set size in pixels
	error = FT_Set_Pixel_Sizes( holder->face, 0, pixelHeight );  
	if( error )
    {
		FT_Done_Face( holder->face );
		FT_Done_FreeType( holder->library );
        return false;
    }

	m_font = holder;
	return true;
}

FontInfo TrueTypeFont::getFontInfo()
{
	assert(m_font != NULL && "TrueTypeFont not initialized" );
	FTHolder* holder = (FTHolder*) m_font;
	
	assert(FT_IS_SCALABLE (holder->face));

	FT_Size_Metrics metrics = holder->face->size->metrics;
	//todo manage unscalable font
	FontInfo outFontInfo;
	outFontInfo.scale = 1.0f;
	outFontInfo.ascender = (int16_t)(metrics.ascender >> 6);
	outFontInfo.descender = (int16_t)(metrics.descender >> 6);
	outFontInfo.lineGap = (int16_t)((metrics.height - metrics.ascender + metrics.descender)>>6);
	
	outFontInfo.underline_position =(int16_t) (FT_MulFix(holder->face->underline_position, metrics.y_scale)>>6);
	outFontInfo.underline_thickness=(int16_t)(FT_MulFix(holder->face->underline_thickness,metrics.y_scale)>>6);
	return outFontInfo;
}

/*

bool TrueTypeFont::getGlyphInfo(const FontInfo& fontInfo, CodePoint_t codePoint, GlyphInfo& outGlyphInfo)
{
	assert(m_font != NULL && "TrueTypeFont not initialized" );
	FTHolder* holder = (FTHolder*) m_font;

	uint32_t glyph_index = FT_Get_Char_Index( holder->face, codePoint );


	int32_t load_flags = FT_LOAD_DEFAULT;
	FT_Error error = FT_Load_Glyph(  holder->face, glyph_index, load_flags );
	if(error) { return false; }

	FT_GlyphSlot slot = holder->face->glyph;

	FT_Glyph glyph;
	error = FT_Get_Glyph( slot, &glyph );
	if ( error ) { return false; }
	
	uint32_t bbox_mode = FT_GLYPH_BBOX_PIXELS;
	FT_BBox  bbox;
	FT_Glyph_Get_CBox( glyph, bbox_mode, &bbox );


	outGlyphInfo.glyphIndex = glyph_index;
	outGlyphInfo.width =(uint16_t) (bbox.xMax - bbox.xMin);
	outGlyphInfo.height = (uint16_t)(bbox.yMax - bbox.yMin);
	outGlyphInfo.offset_x = (uint16_t)bbox.xMin;
	outGlyphInfo.offset_y = (uint16_t)bbox.yMin;
	outGlyphInfo.advance_x = (uint16_t)(slot->advance.x >>6);
	outGlyphInfo.advance_y = 16;

	if(fontInfo.fontType == FONT_TYPE_DISTANCE)
	{
		uint32_t w= outGlyphInfo.width;
		uint32_t h= outGlyphInfo.height;
		uint32_t dw = 6;
		uint32_t dh = 6;
		if(dw<2) dw = 2;
		if(dh<2) dh = 2;
		outGlyphInfo.width = w + dw*2;
		outGlyphInfo.height = h + dh*2;
		outGlyphInfo.offset_x -= dw;
		outGlyphInfo.offset_y -= dh;
	}

	outGlyphInfo.texture_x0 = 0;
	outGlyphInfo.texture_y0 = 0;
	outGlyphInfo.texture_x1 = 0;
	outGlyphInfo.texture_y1 = 0;	
	return true;
}
*/

bool TrueTypeFont::bakeGlyphAlpha(const FontInfo& fontInfo,CodePoint_t codePoint, GlyphInfo& glyphInfo, uint8_t* outBuffer)
{	
	assert(m_font != NULL && "TrueTypeFont not initialized" );
	FTHolder* holder = (FTHolder*) m_font;
	
	glyphInfo.glyphIndex = FT_Get_Char_Index( holder->face, codePoint );
	
	FT_GlyphSlot slot = holder->face->glyph;
	FT_Error error = FT_Load_Glyph(  holder->face, glyphInfo.glyphIndex, FT_LOAD_DEFAULT );
	if(error) { return false; }
	
	FT_Glyph glyph;
	error = FT_Get_Glyph( slot, &glyph );
	if ( error ) { return false; }
		
	error = FT_Glyph_To_Bitmap( &glyph, FT_RENDER_MODE_NORMAL, 0, 1 );
	if(error){ return false; }
	
	FT_BitmapGlyph bitmap = (FT_BitmapGlyph)glyph;
		
	glyphInfo.offset_x = bitmap->left;
	glyphInfo.offset_y = -bitmap->top;	
	glyphInfo.width = bitmap->bitmap.width;
	glyphInfo.height = bitmap->bitmap.rows;
	glyphInfo.advance_x = (uint16_t)(slot->advance.x >>6);
	glyphInfo.advance_y = (uint16_t)(slot->advance.y >>6);
	 
	int x = glyphInfo.offset_x;
	int y = glyphInfo.offset_y;
	int w = glyphInfo.width;
	int h = glyphInfo.height;
	int charsize = 1;
	int depth=1;
	int stride = bitmap->bitmap.pitch;
	for( int i=0; i<h; ++i )
    {
        memcpy(outBuffer+(i*w) * charsize * depth, 
			bitmap->bitmap.buffer + (i*stride) * charsize, w * charsize * depth  );
    }
	FT_Done_Glyph(glyph);
	return true;
}

bool TrueTypeFont::bakeGlyphSubpixel(const FontInfo& fontInfo,CodePoint_t codePoint, GlyphInfo& glyphInfo, uint8_t* outBuffer)
{
	assert(m_font != NULL && "TrueTypeFont not initialized" );
	FTHolder* holder = (FTHolder*) m_font;
	
	glyphInfo.glyphIndex = FT_Get_Char_Index( holder->face, codePoint );
	
	FT_GlyphSlot slot = holder->face->glyph;
	FT_Error error = FT_Load_Glyph(  holder->face, glyphInfo.glyphIndex, FT_LOAD_DEFAULT );
	if(error) { return false; }
	
	FT_Glyph glyph;
	error = FT_Get_Glyph( slot, &glyph );
	if ( error ) { return false; }
		
	error = FT_Glyph_To_Bitmap( &glyph, FT_RENDER_MODE_LCD, 0, 1 );
	if(error){ return false; }
	
	FT_BitmapGlyph bitmap = (FT_BitmapGlyph)glyph;
		
	glyphInfo.offset_x = bitmap->left;
	glyphInfo.offset_y = -bitmap->top;	
	glyphInfo.width = bitmap->bitmap.width;
	glyphInfo.height = bitmap->bitmap.rows;
	glyphInfo.advance_x = (uint16_t)(slot->advance.x >>6);
	glyphInfo.advance_y = (uint16_t)(slot->advance.y >>6);
	 
	int x = glyphInfo.offset_x;
	int y = glyphInfo.offset_y;
	int w = glyphInfo.width;
	int h = glyphInfo.height;
	int charsize = 1;
	int depth=3;
	int stride = bitmap->bitmap.pitch;
	for( int i=0; i<h; ++i )
    {
        memcpy(outBuffer+(i*w) * charsize * depth, 
			bitmap->bitmap.buffer + (i*stride) * charsize, w * charsize * depth  );
    }
	FT_Done_Glyph(glyph);
	return true;
}

void make_distance_map( unsigned char *img, unsigned char *outImg, unsigned int width, unsigned int height )
{
    short * xdist = (short *)  malloc( width * height * sizeof(short) );
    short * ydist = (short *)  malloc( width * height * sizeof(short) );
    double * gx   = (double *) calloc( width * height, sizeof(double) );
    double * gy      = (double *) calloc( width * height, sizeof(double) );
    double * data    = (double *) calloc( width * height, sizeof(double) );
    double * outside = (double *) calloc( width * height, sizeof(double) );
    double * inside  = (double *) calloc( width * height, sizeof(double) );
    uint32_t i;

    // Convert img into double (data)
    double img_min = 255, img_max = -255;
    for( i=0; i<width*height; ++i)
    {
        double v = img[i];
        data[i] = v;
        if (v > img_max) img_max = v;
        if (v < img_min) img_min = v;
    }
    // Rescale image levels between 0 and 1
    for( i=0; i<width*height; ++i)
    {
        data[i] = (img[i]-img_min)/img_max;
    }

    // Compute outside = edtaa3(bitmap); % Transform background (0's)
    computegradient( data, width, height, gx, gy);
    edtaa3(data, gx, gy, width, height, xdist, ydist, outside);
    for( i=0; i<width*height; ++i)
        if( outside[i] < 0 )
            outside[i] = 0.0;

    // Compute inside = edtaa3(1-bitmap); % Transform foreground (1's)
    memset(gx, 0, sizeof(double)*width*height );
    memset(gy, 0, sizeof(double)*width*height );
    for( i=0; i<width*height; ++i)
        data[i] = 1 - data[i];
    computegradient( data, width, height, gx, gy);
    edtaa3(data, gx, gy, height, width, xdist, ydist, inside);
    for( i=0; i<width*height; ++i)
        if( inside[i] < 0 )
            inside[i] = 0.0;

    // distmap = outside - inside; % Bipolar distance field
    unsigned char *out = outImg;//(unsigned char *) malloc( width * height * sizeof(unsigned char) );
    for( i=0; i<width*height; ++i)
    {
        outside[i] -= inside[i];
        outside[i] = 128+outside[i]*16;
        if( outside[i] < 0 ) outside[i] = 0;
        if( outside[i] > 255 ) outside[i] = 255;
        out[i] = 255 - (unsigned char) outside[i];
        //out[i] = (unsigned char) outside[i];
    }

    free( xdist );
    free( ydist );
    free( gx );
    free( gy );
    free( data );
    free( outside );
    free( inside );
}


bool TrueTypeFont::bakeGlyphDistance(const FontInfo& fontInfo, CodePoint_t codePoint, GlyphInfo& glyphInfo, uint8_t* outBuffer)
{	
	assert(m_font != NULL && "TrueTypeFont not initialized" );
	FTHolder* holder = (FTHolder*) m_font;
	
	glyphInfo.glyphIndex = FT_Get_Char_Index( holder->face, codePoint );
	
	FT_Int32 loadMode = FT_LOAD_DEFAULT; //FT_LOAD_TARGET_MONO;
	FT_Render_Mode renderMode = FT_RENDER_MODE_NORMAL;//FT_RENDER_MODE_MONO

	FT_GlyphSlot slot = holder->face->glyph;
	FT_Error error = FT_Load_Glyph(  holder->face, glyphInfo.glyphIndex, loadMode );
	if(error) { return false; }
	
	FT_Glyph glyph;
	error = FT_Get_Glyph( slot, &glyph );
	if ( error ) { return false; }
	
	error = FT_Glyph_To_Bitmap( &glyph, renderMode, 0, 1 );
	if(error){ return false; }
	
	FT_BitmapGlyph bitmap = (FT_BitmapGlyph)glyph;
		
	glyphInfo.offset_x = bitmap->left;
	glyphInfo.offset_y = -bitmap->top;	
	glyphInfo.width = bitmap->bitmap.width;
	glyphInfo.height = bitmap->bitmap.rows;
	glyphInfo.advance_x = (uint16_t)(slot->advance.x >>6);
	glyphInfo.advance_y = (uint16_t)(slot->advance.y >>6);
	 
	int x = glyphInfo.offset_x;
	int y = glyphInfo.offset_y;
	int w = glyphInfo.width;
	int h = glyphInfo.height;
	int charsize = 1;
	int depth=1;
	int stride = bitmap->bitmap.pitch;
	
	/*
	const uint8_t* src = bitmap->bitmap.buffer;
	for( int y=0; y<h; ++y )
	{		
		for( int x=0; x<w; ++x )
		{
			uint32_t idx = y*w+x;
			outBuffer[idx] = ((src[x / 8]) & (1 << (7 - (x % 8)))) ? 255 : 0;
		}
		src+=stride;
	}*/
	
	
	for( int i=0; i<h; ++i )
    {	

        memcpy(outBuffer+(i*w) * charsize * depth, 
			bitmap->bitmap.buffer + (i*stride) * charsize, w * charsize * depth  );
    }
	FT_Done_Glyph(glyph);
		
	if(w*h >0)
	{
		uint32_t dw = 6;
		uint32_t dh = 6;	
		if(dw<2) dw = 2;
		if(dh<2) dh = 2;
	
		uint32_t nw = w + dw*2;
		uint32_t nh = h + dh*2;
		assert(nw*nh < 128*128);
		uint32_t buffSize = nw*nh*sizeof(uint8_t);
	
		uint8_t * alphaImg = (uint8_t *)  malloc( buffSize );
		memset(alphaImg, 0, nw*nh*sizeof(uint8_t));

		//copy the original buffer to the temp one
		for(uint32_t  i= dh; i< nh-dh; ++i)
		{
			memcpy(alphaImg+i*nw+dw, outBuffer+(i-dh)*w, w);
		}
	
		make_distance_map(alphaImg, outBuffer, nw, nh);
		free(alphaImg);	
		
		glyphInfo.offset_x -= dw;
		glyphInfo.offset_y -= dh;
		glyphInfo.width = nw ;
		glyphInfo.height = nh;
	}
	
	return true;	
}

}
