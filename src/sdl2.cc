#include "sdl2.hh"


Uint32 sdl2::event_window_id( const SDL_Event &e )
{
	switch( e.type )
	{
		case SDL_WINDOWEVENT:
			return e.window.windowID;

		case SDL_KEYUP:
		case SDL_KEYDOWN:
			return e.key.windowID;

		case SDL_TEXTEDITING:
			return e.edit.windowID;

		case SDL_TEXTINPUT:
			return e.text.windowID;

		case SDL_MOUSEMOTION:
			return e.motion.windowID;

		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEBUTTONDOWN:
			return e.button.windowID;

		case SDL_USEREVENT:
			return e.user.windowID;

		case SDL_JOYAXISMOTION:
		case SDL_JOYBALLMOTION:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYDEVICEADDED:
		case SDL_JOYDEVICEREMOVED:
		case SDL_CONTROLLERAXISMOTION:
		case SDL_CONTROLLERBUTTONUP:
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERDEVICEADDED:
		case SDL_CONTROLLERDEVICEREMOVED:
		case SDL_CONTROLLERDEVICEREMAPPED:
		case SDL_AUDIODEVICEADDED:
		case SDL_AUDIODEVICEREMOVED:
		case SDL_QUIT:
		case SDL_SYSWMEVENT:
		case SDL_FINGERUP:
		case SDL_FINGERDOWN:
		case SDL_FINGERMOTION:
		case SDL_MULTIGESTURE:
		case SDL_DROPFILE:
		default:
			return 0;
	}
}