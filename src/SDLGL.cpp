
#include <stdexcept>

#include "SDLGL.h"
#include "App.h"


SDLResVidModes sdlResVideoModes[18] = {
	{480, 320},
	{640, 480},
	{720, 400},
	{720, 480},
	{720, 576},
	{800, 600},
	{832, 624},
	{960, 640},
	{1024, 768},
	{1152, 864},
	{1152, 872},
	{1280, 720},
	{1280, 800},
	{1280, 1024},
	{1440, 900},
	{1600, 1000},
	{1680, 1050},
	{1920, 1080}
};

SDLGL::SDLGL() {
	initialized = false;
}

SDLGL::~SDLGL() {
	if (this) {
		if (initialized) {
			if (this->window) {
				SDL_SetWindowFullscreen(this->window, 0);
			}
			SDL_GL_DeleteContext(glcontext);
			SDL_DestroyWindow(window);
			SDL_Quit();
		}
	}
}

bool SDLGL::Initialize() {
	Uint32 flags;

	if (!this->initialized) {
		
		SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
		if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
			printf("Could not initialize SDL: %s", SDL_GetError());
		}

#if TARGET_OS_IOS
		SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
		SDL_SetHint(SDL_HINT_IOS_HIDE_HOME_INDICATOR, "2");
		SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0"); // touches are handled as SDL_FINGER* events
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS;
		this->keepAspect = true;
#else
		flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN /* | SDL_WINDOW_RESIZABLE*/;
		this->keepAspect = false;
#endif
		// Set the highdpi flags - this makes a big difference on Macs with
		// retina displays, especially when using small window sizes.
		flags |= SDL_WINDOW_ALLOW_HIGHDPI;

		this->oldResolutionIndex = -1;
		this->resolutionIndex = 0;
		this->winVidWidth = sdlResVideoModes[this->resolutionIndex].width;//Applet::IOS_WIDTH*2;
		this->winVidHeight = sdlResVideoModes[this->resolutionIndex].height;//Applet::IOS_HEIGHT*2;

		//this->winVidWidth = 1440;
		//this->winVidHeight = 900;

		this->window = SDL_CreateWindow("Doom II RPG By [GEC] Version 0.1", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, winVidWidth, winVidHeight, flags);
		if (!this->window) {
			printf("Could not set %dx%d video mode: %s", winVidWidth, winVidHeight, SDL_GetError());
		}

		this->vidWidth = Applet::IOS_WIDTH;
		this->vidHeight = Applet::IOS_HEIGHT;
		this->oldWindowMode = -1;
		this->windowMode = 0;
		this->oldVSync = false;
		this->vSync = true;

		//SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
		//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
		//SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");//sdlVideo.vSync ? "1" : "0");
		this->glcontext = SDL_GL_CreateContext(window);
		if (!this->glcontext) {
			printf("Could not create GL context: %s\n", SDL_GetError());
		}
		this->updateVideo();

		// now you can make GL calls.
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		SDL_GL_SwapWindow(this->window);

		// Check for joysticks
		SDL_SetHint(SDL_HINT_JOYSTICK_RAWINPUT, "0");
		this->initialized = true;
	}

	return this->initialized;
}


#include <stdarg.h> //va_list|va_start|va_end

void SDLGL::Error(const char* fmt, ...)
{
	char errMsg[256];
	va_list ap;
	va_start(ap, fmt);
#ifdef _WIN32
	vsprintf_s(errMsg, sizeof(errMsg), fmt, ap);
#else
	vsnprintf(errMsg, sizeof(errMsg), fmt, ap);
#endif
	va_end(ap);

	printf("%s", errMsg);

	const SDL_MessageBoxButtonData buttons[] = {
		{ /* .flags, .buttonid, .text */        0, 0, "Ok" },
	};
	const SDL_MessageBoxColorScheme colorScheme = {
		{ /* .colors (.r, .g, .b) */
			/* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
			{ 255,   0,   0 },
			/* [SDL_MESSAGEBOX_COLOR_TEXT] */
			{   0, 255,   0 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
			{ 255, 255,   0 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
			{   0,   0, 255 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
			{ 255,   0, 255 }
		}
	};
	const SDL_MessageBoxData messageboxdata = {
		SDL_MESSAGEBOX_ERROR, /* .flags */
		NULL, /* .window */
		"Doom II RPG Error", /* .title */
		errMsg, /* .message */
		SDL_arraysize(buttons), /* .numbuttons */
		buttons, /* .buttons */
		&colorScheme /* .colorScheme */
	};

	SDL_ShowMessageBox(&messageboxdata, NULL);
	//closeZipFile(&zipFile);
	//DoomRPG_FreeAppData(doomRpg);
	//SDL_CloseAudio();
	//SDL_Close();
	SDLGL::~SDLGL();
	exit(0);
}


void SDLGL::transformCoord2f(float* x, float* y) {
	*x *= (float)this->dispW / (float)Applet::IOS_WIDTH;
	*y *= (float)this->dispH / (float)Applet::IOS_HEIGHT;
}

// Maps a GL viewport/scissor rect {x, y, w, h} in game coordinates to drawable pixels
void SDLGL::transformViewport(int* rect) {
	float sx = (float)this->dispW / (float)Applet::IOS_WIDTH;
	float sy = (float)this->dispH / (float)Applet::IOS_HEIGHT;
	rect[0] = this->dispX + (int)((float)rect[0] * sx);
	rect[1] = this->dispY + (int)((float)rect[1] * sy);
	rect[2] = (int)((float)rect[2] * sx);
	rect[3] = (int)((float)rect[3] * sy);
}

void SDLGL::updateDisplayRect() {
	int w, h;
	SDL_GL_GetDrawableSize(this->window, &w, &h);
	if (w <= 0 || h <= 0) {
		SDL_GetWindowSize(this->window, &w, &h);
	}

	this->dispX = 0;
	this->dispY = 0;
	this->dispW = w;
	this->dispH = h;

	if (this->keepAspect) {
		// Letterbox / pillarbox to the original 3:2 aspect ratio
		if (w * Applet::IOS_HEIGHT > h * Applet::IOS_WIDTH) {
			this->dispW = (h * Applet::IOS_WIDTH) / Applet::IOS_HEIGHT;
			this->dispX = (w - this->dispW) / 2;
		}
		else {
			this->dispH = (w * Applet::IOS_HEIGHT) / Applet::IOS_WIDTH;
			this->dispY = (h - this->dispH) / 2;
		}
	}
}

void SDLGL::setDisplayViewport() {
	glViewport(this->dispX, this->dispY, this->dispW, this->dispH);
	glScissor(this->dispX, this->dispY, this->dispW, this->dispH);
}

// Converts window coordinates (points) to normalized [0, 1] game coordinates
void SDLGL::windowToGame(int wx, int wy, float* x, float* y) {
	int ww, wh, pw, ph;
	SDL_GetWindowSize(this->window, &ww, &wh);
	SDL_GL_GetDrawableSize(this->window, &pw, &ph);
	float px = (ww > 0) ? (float)wx * ((float)pw / (float)ww) : (float)wx;
	float py = (wh > 0) ? (float)wy * ((float)ph / (float)wh) : (float)wy;
	// drawable y is top-down here; dispY is symmetric so it works for either origin
	*x = (px - (float)this->dispX) / (float)this->dispW;
	*y = (py - (float)this->dispY) / (float)this->dispH;
}

void SDLGL::centerMouse(int x, int y) {
	float X = (float)((Applet::IOS_WIDTH / 2) + x);
	float Y = (float)((Applet::IOS_HEIGHT / 2) + y);
	this->transformCoord2f(&X, &Y);
	SDL_WarpMouseInWindow(this->window, (int)X, (int)Y);
}

void SDLGL::updateWinVid(int w, int h) {
	this->winVidWidth = w;
	this->winVidHeight = h;
}

void SDLGL::updateVideo() {
	if (this->vSync != this->oldVSync) {
		SDL_GL_SetSwapInterval(this->vSync ? SDL_TRUE : SDL_FALSE);
		this->oldVSync = this->vSync;
	}

#if TARGET_OS_IOS
	// The window always covers the whole screen on iOS
	this->oldWindowMode = this->windowMode;
	this->oldResolutionIndex = this->resolutionIndex;
#endif

	if (this->windowMode != this->oldWindowMode) {
		SDL_SetWindowFullscreen(this->window, 0);
		SDL_SetWindowBordered(this->window, SDL_TRUE);

		if (this->windowMode == 1) {
			SDL_SetWindowBordered(this->window, SDL_FALSE);
		}
		else if (this->windowMode == 2) {
			SDL_SetWindowFullscreen(this->window, SDL_WINDOW_FULLSCREEN);
		}
		this->oldWindowMode = this->windowMode;
	}

	if (this->resolutionIndex != this->oldResolutionIndex) {
		SDL_SetWindowSize(this->window, sdlResVideoModes[this->resolutionIndex].width, sdlResVideoModes[this->resolutionIndex].height);
		SDL_SetWindowPosition(this->window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
		this->updateWinVid(sdlResVideoModes[this->resolutionIndex].width, sdlResVideoModes[this->resolutionIndex].height);
		this->oldResolutionIndex = this->resolutionIndex;
	}

	int winVidWidth, winVidHeight;
	SDL_GetWindowSize(this->window, &winVidWidth, &winVidHeight);
	this->updateWinVid(winVidWidth, winVidHeight);
	this->updateDisplayRect();
}

void SDLGL::restore() {
	if (this->vSync != this->oldVSync) {
		this->vSync = this->oldVSync;
	}

	if (this->windowMode != this->oldWindowMode) {
		this->windowMode = this->oldWindowMode;
	}

	if (this->resolutionIndex != this->oldResolutionIndex) {
		this->resolutionIndex = this->oldResolutionIndex;
	}
}