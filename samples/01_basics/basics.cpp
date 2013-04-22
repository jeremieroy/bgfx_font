#include <bgfx.h>
#include <bx/bx.h>
#include <bx/timer.h>
#include <common/entry.h>
#include <common/dbg.h>
#include <common/math.h>
#include <common/processevents.h>

#include "../src/font_manager.h"
#include "../src/text_buffer_manager.h"

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
	bgfx_font::FontManager* fontManager = new bgfx_font::FontManager(512);
	bgfx_font::TextBufferManager* textBufferManager = new bgfx_font::TextBufferManager(fontManager);
	textBufferManager->init(s_shaderPath);

	
	//load some truetype files
	bgfx_font::TrueTypeHandle times_tt = fontManager->loadTrueTypeFromFile("c:/windows/fonts/times.ttf");
	bgfx_font::TrueTypeHandle consola_tt = fontManager->loadTrueTypeFromFile("c:/windows/fonts/consola.ttf");

	//create some usable font with of a specific size
	bgfx_font::FontHandle times_24 = fontManager->createFontByPixelSize(times_tt, 0, 24);
		
	//preload glyphs and blit them to atlas
	fontManager->preloadGlyph(times_24, L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ. \n");
		
	//You can unload the truetype files at this stage, but in that case, the set of glyph's will be limited to the set of preloaded glyph
	fontManager->unLoadTrueType(times_tt);

	//this font doesn't have any preloaded glyph's but the truetype file is loaded
	//so glyph will be generated as needed
	bgfx_font::FontHandle consola_16 = fontManager->createFontByPixelSize(consola_tt, 0, 16);
	
	//create a static text buffer compatible with alpha font
	//a static text buffer content cannot be modified after its first submit.
	bgfx_font::TextBufferHandle staticText = textBufferManager->createTextBuffer(bgfx_font::FONT_TYPE_ALPHA, bgfx_font::STATIC);
	
	//the pen position represent the top left of the box of the first line of text
	textBufferManager->setPenPosition(staticText, 20.0f, 100.0f);
	
	//add some text to the buffer
	textBufferManager->appendText(staticText, times_24, L"The quick brown fox jumps over the lazy dog\n");
	//the position of the pen is adjusted when there is an endline
	
	//setup style colors 
	textBufferManager->setBackgroundColor(staticText, 0x551111FF);
	textBufferManager->setUnderlineColor(staticText, 0xFF2222FF);
	textBufferManager->setOverlineColor(staticText, 0x2222FFFF);
	textBufferManager->setStrikeThroughColor(staticText, 0x22FF22FF);
	

	//text + bkg
	textBufferManager->setStyle(staticText, bgfx_font::STYLE_BACKGROUND);
	textBufferManager->appendText(staticText, times_24, L"The quick brown fox jumps over the lazy dog\n");

	//text + strike-through
	textBufferManager->setStyle(staticText, bgfx_font::STYLE_STRIKE_THROUGH);
	textBufferManager->appendText(staticText, times_24, L"The quick brown fox jumps over the lazy dog\n");

	//text + overline
	textBufferManager->setStyle(staticText, bgfx_font::STYLE_OVERLINE);
	textBufferManager->appendText(staticText, times_24, L"The quick brown fox jumps over the lazy dog\n");
	
	//text + underline
	textBufferManager->setStyle(staticText, bgfx_font::STYLE_UNDERLINE);
	textBufferManager->appendText(staticText, times_24, L"The quick brown fox jumps over the lazy dog\n");

	
	//text + bkg + strike-through
	textBufferManager->setStyle(staticText, bgfx_font::STYLE_BACKGROUND|bgfx_font::STYLE_STRIKE_THROUGH);
	textBufferManager->appendText(staticText, times_24, L"The quick brown fox jumps over the lazy dog\n");

	//create a transient buffer for realtime data	
	bgfx_font::TextBufferHandle transientText = textBufferManager->createTextBuffer(bgfx_font::FONT_TYPE_ALPHA, bgfx_font::TRANSIENT);	
				
	float ra = 0, rb=0, rg=0;
	uint32_t w = 0,h = 0;
    while (!processEvents(width, height, debug, reset) )
	{
		
		if(w!=width|| h!=height)
		{
			w=width;
			h= height;
			printf("ri: %d,%d\n",width,height);
		}
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
		textBufferManager->submitTextBuffer(staticText, 0);	
		
		
		//submit some realtime text
		wchar_t fpsText[64];
		swprintf(fpsText,L"Frame: % 7.3f[ms]", double(frameTime)*toMs);
		
		textBufferManager->clearTextBuffer(transientText);
		textBufferManager->setPenPosition(transientText, 20.0, 4.0f);
		
		textBufferManager->appendText(transientText, consola_16, L"bgfx_font\\sample\\01_basics\n");		
		textBufferManager->appendText(transientText, consola_16, L"Description: truetype, font, text and style\n");
		textBufferManager->appendText(transientText, consola_16, fpsText);
		
		textBufferManager->submitTextBuffer(transientText, 0);
		
        // Advance to next frame. Rendering thread will be kicked to 
		// process submitted rendering primitives.
		bgfx::frame();
		//just to prevent my CG Fan to howl
		Sleep(2);
	}
	
	
	fontManager->unLoadTrueType(consola_tt);
	fontManager->destroyFont(consola_16);
	fontManager->destroyFont(times_24);

	textBufferManager->destroyTextBuffer(staticText);
	textBufferManager->destroyTextBuffer(transientText);	

	delete textBufferManager;
	delete fontManager;	
	
	// Shutdown bgfx.
    bgfx::shutdown();

	return 0;
}
