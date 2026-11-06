#ifndef __SDLGL_H__
#define __SDLGL_H__

#include <SDL.h>
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IOS
#define GLES_SILENCE_DEPRECATION 1
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#ifndef APIENTRYP
#define APIENTRYP *
#endif
typedef void (APIENTRYP PFNGLACTIVETEXTUREPROC) (GLenum texture);
typedef void (APIENTRYP PFNGLCLIENTACTIVETEXTUREPROC) (GLenum texture);
// Desktop GL names for the ES 1.1 texture combiner enums
#define GL_SOURCE0_RGB   GL_SRC0_RGB
#define GL_SOURCE1_RGB   GL_SRC1_RGB
#define GL_SOURCE2_RGB   GL_SRC2_RGB
#define GL_SOURCE0_ALPHA GL_SRC0_ALPHA
#define GL_SOURCE1_ALPHA GL_SRC1_ALPHA
#define GL_SOURCE2_ALPHA GL_SRC2_ALPHA
// OpenGL ES 1.1 only has the float variants
static inline void glOrtho(double l, double r, double b, double t, double n, double f) {
	glOrthof((GLfloat)l, (GLfloat)r, (GLfloat)b, (GLfloat)t, (GLfloat)n, (GLfloat)f);
}
static inline void glFrustum(double l, double r, double b, double t, double n, double f) {
	glFrustumf((GLfloat)l, (GLfloat)r, (GLfloat)b, (GLfloat)t, (GLfloat)n, (GLfloat)f);
}
static inline void glClearDepth(double d) { glClearDepthf((GLfloat)d); }
static inline void glDepthRange(double n, double f) { glDepthRangef((GLfloat)n, (GLfloat)f); }
#else
#include <SDL_opengl.h>
#endif

typedef struct SDLResVidModes_s {
	int width, height;
} SDLResVidModes;
extern SDLResVidModes sdlResVideoModes[18];

class SDLGL
{
private:
	bool initialized;
	SDL_GLContext glcontext;

public:
	SDL_Window* window;
	int resolutionIndex;
	int oldResolutionIndex;
	int winVidWidth, winVidHeight;
	int vidWidth, vidHeight;
	int windowMode, oldWindowMode;
	bool vSync, oldVSync;
	// [iOS] Area of the drawable (in pixels) the 480x320 game is drawn into
	int dispX, dispY, dispW, dispH;
	bool keepAspect;

	SDLGL();
	// Destructor
	~SDLGL();
	bool Initialize();
	void Error(const char* fmt, ...);

	void transformCoord2f(float* x, float* y);
	void transformViewport(int* rect);
	void updateDisplayRect();
	void setDisplayViewport();
	void windowToGame(int wx, int wy, float* x, float* y);
	void centerMouse(int x, int y);
	void updateWinVid(int w, int h);
	void updateVideo();
	void restore();
};

#endif
