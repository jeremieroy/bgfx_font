#include <bgfx.h>
#include <bx/bx.h>
#include <bx/timer.h>
#include <common/entry.h>
#include <common/dbg.h>
#include <common/math.h>
#include <common/processevents.h>

#include "bgfx_font.h"

#include <stdio.h>
#include <string.h>

static const char* s_shaderPath = NULL;

static void shaderFilePath(char* _out, const char* _name)
{
	strcpy(_out, s_shaderPath);
	strcat(_out, _name);
	strcat(_out, ".bin");
}

long int fsize(FILE* _file)
{
	long int pos = ftell(_file);
	fseek(_file, 0L, SEEK_END);
	long int size = ftell(_file);
	fseek(_file, pos, SEEK_SET);
	return size;
}

static const bgfx::Memory* load(const char* _filePath)
{
	FILE* file = fopen(_filePath, "rb");
	if (NULL != file)
	{
		uint32_t size = (uint32_t)fsize(file);
		const bgfx::Memory* mem = bgfx::alloc(size+1);
		size_t ignore = fread(mem->data, 1, size, file);
		BX_UNUSED(ignore);
		fclose(file);
		mem->data[mem->size-1] = '\0';
		return mem;
	}

	return NULL;
}

static const bgfx::Memory* loadShader(const char* _name)
{
	char filePath[512];
	shaderFilePath(filePath, _name);
	return load(filePath);
}

int _main_(int _argc, char** _argv)
{
    uint32_t width = 1280;
	uint32_t height = 720;
	uint32_t debug = BGFX_DEBUG_TEXT;
	uint32_t reset = 0;

	bgfx::init();
	

	bgfx::reset(width, height);

	// Enable debug text.
	bgfx::setDebug(debug);

	// Set view 0 clear state.
	bgfx::setViewClear(0
		, BGFX_CLEAR_COLOR_BIT|BGFX_CLEAR_DEPTH_BIT
		, 0x303030ff		
		, 1.0f
		, 0
		);

    // Setup root path for binary shaders. Shader binaries are different 
	// for each renderer.
	switch (bgfx::getRendererType() )
	{
	default:
	case bgfx::RendererType::Direct3D9:
		s_shaderPath = "shaders/dx9/";
		break;

	case bgfx::RendererType::Direct3D11:
		s_shaderPath = "shaders/dx11/";
		break;

	case bgfx::RendererType::OpenGL:
		s_shaderPath = "shaders/glsl/";
		break;

	case bgfx::RendererType::OpenGLES2:
	case bgfx::RendererType::OpenGLES3:
		s_shaderPath = "shaders/gles/";
		break;
	}

	//init the text rendering system
	bgfx_font::init(s_shaderPath);

	
	//load some truetype files
	bgfx_font::TrueTypeHandle times_tt = bgfx_font::loadTrueTypeFont("c:/windows/fonts/times.ttf");
	bgfx_font::TrueTypeHandle consola_tt = bgfx_font::loadTrueTypeFont("c:/windows/fonts/consola.ttf");

	//create some usable font with of a specific size
	bgfx_font::FontHandle times_24 = bgfx_font::createFont(times_tt, 0, 24);
		
	//preload glyphs and blit them to atlas
	bgfx_font::preloadGlyph(times_24, L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ. \n");
		
	//You can unload the truetype files at this stage, but in that case, the set of glyph's will be limited to the set of preloaded glyph
	bgfx_font::unloadTrueTypeFont(times_tt);

	//this font doesn't have any preloaded glyph's but the truetype file is loaded
	//so glyph will be generated as needed
	bgfx_font::FontHandle consola_16 = bgfx_font::createFont(consola_tt, 0, 16);
	
	//create a static text buffer compatible with alpha font
	//a static text buffer content cannot be modified after its first submit.
	bgfx_font::TextBufferHandle staticText = bgfx_font::createTextBuffer(bgfx_font::FONT_TYPE_ALPHA, bgfx_font::STATIC);
	
	//the pen position represent the top left of the box of the first line of text
	bgfx_font::setPenPosition(staticText, 20.0f, 100.0f);
	
	//add some text to the buffer
	bgfx_font::appendText(staticText, times_24, L"The quick brown fox jumps over the lazy dog\n");
	//the position of the pen is adjusted when there is an endline
	
	//setup style colors 
	bgfx_font::setTextBackgroundColor(staticText, 0x551111FF);
	bgfx_font::setUnderlineColor(staticText, 0xFF2222FF);
	bgfx_font::setOverlineColor(staticText, 0x2222FFFF);
	bgfx_font::setStrikeThroughColor(staticText, 0x22FF22FF);

	

	//text + bkg
	bgfx_font::setTextStyle(staticText, bgfx_font::STYLE_BACKGROUND);
	bgfx_font::appendText(staticText, times_24, L"The quick brown fox jumps over the lazy dog\n");

	//text + strike-through
	bgfx_font::setTextStyle(staticText, bgfx_font::STYLE_STRIKE_THROUGH);
	bgfx_font::appendText(staticText, times_24, L"The quick brown fox jumps over the lazy dog\n");

	//text + overline
	bgfx_font::setTextStyle(staticText, bgfx_font::STYLE_OVERLINE);
	bgfx_font::appendText(staticText, times_24, L"The quick brown fox jumps over the lazy dog\n");
	
	//text + underline
	bgfx_font::setTextStyle(staticText, bgfx_font::STYLE_UNDERLINE);
	bgfx_font::appendText(staticText, times_24, L"The quick brown fox jumps over the lazy dog\n");

	
	//text + bkg + strike-through
	bgfx_font::setTextStyle(staticText, bgfx_font::STYLE_BACKGROUND|bgfx_font::STYLE_STRIKE_THROUGH);
	bgfx_font::appendText(staticText, times_24, L"The quick brown fox jumps over the lazy dog\n");

	//create a transient buffer for realtime data	
	bgfx_font::TextBufferHandle transientText = bgfx_font::createTextBuffer(bgfx_font::FONT_TYPE_ALPHA, bgfx_font::TRANSIENT);	
				
	float ra = 0, rb=0, rg=0;
    while (!processEvents(width, height, debug, reset) )
	{
		// Set view 0 default viewport.
		bgfx::setViewRect(0, 0, 0, width, height);

		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		bgfx::submit(0);

		int64_t now = bx::getHPCounter();
		static int64_t last = now;
		const int64_t frameTime = now - last;
		last = now;
		const double freq = double(bx::getHPFrequency() );
		const double toMs = 1000.0/freq;
		float time = (float)(bx::getHPCounter()/double(bx::getHPFrequency() ) );
		
		float at[3] = { 0, 0, 0.0f };
		float eye[3] = {0, 0, -1.0f };
		
		float view[16];
		float proj[16];
		mtxLookAt(view, eye, at);		
		//setup a top-left ortho matrix for screen space drawing		
		float centering = 0.5f;
		mtxOrtho(proj, centering, width+centering,height+centering, centering,-1.0f, 1.0f);

		// Set view and projection matrix for view 0.
		bgfx::setViewTransform(0, view, proj);

		//submit the static text
		bgfx_font::submitTextBuffer(staticText, 0);	
		
		
		//submit some realtime text
		wchar_t fpsText[64];
		swprintf(fpsText,L"Frame: % 7.3f[ms]", double(frameTime)*toMs);
		
		bgfx_font::clearTextBuffer(transientText);
		bgfx_font::setPenPosition(transientText, 20.0, 4.0f);
		
		bgfx_font::appendText(transientText, consola_16, L"bgfx_font\\sample\\01_basics\n");		
		bgfx_font::appendText(transientText, consola_16, L"Description: truetype, font, text and style\n");
		bgfx_font::appendText(transientText, consola_16, fpsText);
		
		bgfx_font::submitTextBuffer(transientText, 0);		
		
        // Advance to next frame. Rendering thread will be kicked to 
		// process submitted rendering primitives.
		bgfx::frame();
		//just to prevent my CG Fan to howl
		Sleep(2);
	}
	
	
	bgfx_font::unloadTrueTypeFont(consola_tt);
	bgfx_font::destroyFont(consola_16);
	bgfx_font::destroyFont(times_24);

	bgfx_font::destroyTextBuffer(staticText);
	bgfx_font::destroyTextBuffer(transientText);	
	bgfx_font::shutdown();
	// Shutdown bgfx.
    bgfx::shutdown();

	return 0;
}
