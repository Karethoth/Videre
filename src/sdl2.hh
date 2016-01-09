#pragma once
#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>



// Wrapping SDL2 stuff and some helpers
namespace sdl2
{
	struct Deleter
	{
		void operator()( SDL_Surface  *ptr ) { if( ptr ) SDL_FreeSurface( ptr ); }
		void operator()( SDL_Window   *ptr ) { if( ptr ) SDL_DestroyWindow( ptr ); }
		void operator()( SDL_Texture  *ptr ) { if( ptr ) SDL_DestroyTexture( ptr ); }
		void operator()( SDL_Renderer *ptr ) { if( ptr ) SDL_DestroyRenderer( ptr ); }
	};

	using SurfacePtr  = std::unique_ptr<SDL_Surface, Deleter>;
	using WindowPtr   = std::unique_ptr<SDL_Window, Deleter>;
	using TexturePtr  = std::unique_ptr<SDL_Texture, Deleter>;
	using RendererPtr = std::unique_ptr<SDL_Renderer, Deleter>;


	Uint32 event_window_id( const SDL_Event &e );
}

