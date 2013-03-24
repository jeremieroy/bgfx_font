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
		//, 0x303030ff
		, 0x000000ff
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

	//load a truetype files
	bgfx_font::TrueTypeHandle times_tt = bgfx_font::loadTrueTypeFont("c:/windows/fonts/times.ttf");	
	bgfx_font::FontHandle distance_font = bgfx_font::createFont(times_tt, 0, 48, bgfx_font::FONT_TYPE_DISTANCE);
	//preload glyph and generate (generate bitmap's)
	bgfx_font::preloadGlyph(distance_font, L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ. \n");

	uint32_t fontsCount = 0;
	bgfx_font::FontHandle fonts[64];
	fonts[fontsCount++] = distance_font;
	//generate various sub distance field fonts at various size
	int step=4;
	for(int i = 64; i>1 ; i-=step)
	{		
		if(i<32) step = 2;
		//instantiate a usable font
		bgfx_font::FontHandle font = bgfx_font::createScaledFont(distance_font, i);
		fonts[fontsCount++] = font;
	}
	//You can unload the truetype files at this stage, but in that case, the set of glyph's will be limited to the set of preloaded glyph
	bgfx_font::unloadTrueTypeFont(times_tt);
			
	bgfx_font::TextBufferHandle staticText = bgfx_font::createTextBuffer(bgfx_font::FONT_TYPE_DISTANCE, bgfx_font::STATIC);
	
	bgfx_font::setPenPosition(staticText, 10.0f, 10.0f);	
	for(size_t i = 0; i< fontsCount; ++i)
	{
		bgfx_font::appendText(staticText, fonts[i], L"The quick brown fox jumps over the lazy dog\n");
		//bgfx_font::appendText(staticText, fonts[i], L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ\n");
		
	}	
		
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
		
		// Use debug font to print information about this example.
		bgfx::dbgTextClear();
		//bgfx::dbgTextPrintf(0, 1, 0x4f, "bgfx/examples/00-helloworld");
		//bgfx::dbgTextPrintf(0, 2, 0x6f, "Description: Initialization and debug text.");
		//bgfx::dbgTextPrintf(0, 3, 0x0f, "Frame: % 7.3f[ms]", double(frameTime)*toMs);
				
		float at[3] = { 0, 0, 0.0f };
		float eye[3] = {0, 0, -1.0f };
		
		float view[16];
		float proj[16];
		mtxLookAt(view, eye, at);		
		float centering = 0.5f;
		//setup a top-left ortho matrix for screen space drawing
		mtxOrtho(proj, centering, width+centering,height+centering, centering,-1.0f, 1.0f);		
		
		// Set view and projection matrix for view 0.
		bgfx::setViewTransform(0, view, proj);				
		
		//draw your text
		bgfx_font::submitTextBuffer(staticText, 0);	

        // Advance to next frame. Rendering thread will be kicked to 
		// process submitted rendering primitives.
		bgfx::frame();
		//just to prevent my CG Fan to howl
		Sleep(2);
	}

	//destroy the fonts
	for(size_t i=0; i<fontsCount;++i)
	{
		bgfx_font::destroyFont(fonts[i]);
	}
	
	bgfx_font::destroyTextBuffer(staticText);
	bgfx_font::shutdown();
	// Shutdown bgfx.
    bgfx::shutdown();

	return 0;
}
