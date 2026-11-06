#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>
#include "SDLGL.h"
#include "ZipFile.h"

#include "CAppContainer.h"
#include "App.h"
#include "Image.h"
#include "Resource.h"
#include "Render.h"
#include "GLES.h"

#include "Canvas.h"
#include "Graphics.h"
#include "Player.h"
#include "Game.h"
#include "Graphics.h"
#include "Utils.h"
#include "TinyGL.h"
#include "Input.h"

#if TARGET_OS_IOS
#include <unistd.h>
#include <sys/stat.h>
#include <string>

// [iOS] Saves and config live in the app's Documents folder (also visible in the Files app).
// The game data is read from Documents first, then from the app bundle.
static std::string iosFindGameData() {
    const char* ipaName = "Doom 2 RPG.ipa";
    struct stat sb;

    const char* home = getenv("HOME");
    if (home) {
        std::string docs = std::string(home) + "/Documents";
        mkdir(docs.c_str(), 0755);
        chdir(docs.c_str());
        if (stat(ipaName, &sb) == 0) {
            return docs + "/" + ipaName;
        }
    }

    char* base = SDL_GetBasePath();
    if (base) {
        std::string path = std::string(base) + ipaName;
        SDL_free(base);
        if (stat(path.c_str(), &sb) == 0) {
            return path;
        }
    }

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Doom II RPG",
        "Game data not found.\n\nCopy \"Doom 2 RPG.ipa\" into this app's folder using the Files app or Finder, then relaunch.",
        nullptr);
    exit(0);
}
#endif

void drawView(SDLGL* sdlGL);

int main(int argc, char* args[]) {

    int		UpTime = 0;

	SDLGL sdlGL;
	sdlGL.Initialize();

    ZipFile zipFile;
#if TARGET_OS_IOS
    std::string ipaPath = iosFindGameData();
    zipFile.openZipFile(ipaPath.c_str());
#else
    zipFile.openZipFile("Doom 2 RPG.ipa");
#endif

    Input input;
    input.init(); // [GEC] Port: set default Binds

    CAppContainer::getInstance()->Construct(&sdlGL, &zipFile);
    sdlGL.updateVideo(); // [GEC]

    SDL_Event ev;
    

    float x = 0.0f, y = 30.0f;
    int   vp_cx = 480;
    int   vp_cy = 320;


    Uint8 state;
    int mX, mY;           /* mouse location*/
    float mousePressX = 0.f, mousePressY = 0.f;           /* mouse location float*/
    int winVidWidth = sdlGL.winVidWidth;
    int winVidHeight = sdlGL.winVidHeight;
    bool useMouse = false;
   
    

    while (CAppContainer::getInstance()->app->closeApplet != true) {

        int currentTimeMillis = CAppContainer::getInstance()->getTimeMS();

        //if (!useMouse) {
            //float ax = SDL_GameControllerGetAxis(sdlGL.accelerometer, SDL_CONTROLLER_AXIS_LEFTX);
            //float ay = SDL_GameControllerGetAxis(sdlGL.accelerometer, SDL_CONTROLLER_AXIS_LEFTY);
            //CAppContainer::getInstance()->UpdateAccelerometer((ax* (1.0f / 32767))/2, (ay* (1.0f / 32767))/2, 0.f, false);
        //}

        if (currentTimeMillis > UpTime) {
            input.handleEvents();
            UpTime = currentTimeMillis + 15;
            drawView(&sdlGL);
            input.consumeEvents();
        }
    }

    printf("APP_QUIT\n");
    CAppContainer::getInstance()->~CAppContainer();
    zipFile.closeZipFile();
    sdlGL.~SDLGL();
    input.~Input();
	return 0;
}


static uint32_t lastTimems = 0;

void drawView(SDLGL *sdlGL) {

    int w, h;

    if (lastTimems == 0) {
        lastTimems = CAppContainer::getInstance()->getTimeMS();
    }

    // Clear the whole drawable (including any letterbox bars), then draw into the game area
    sdlGL->updateDisplayRect();
    SDL_GL_GetDrawableSize(sdlGL->window, &w, &h);
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    sdlGL->setDisplayViewport();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_ALPHA_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, Applet::IOS_WIDTH, Applet::IOS_HEIGHT, 0.0, -1.0, 1.0);
    //glRotatef(90.0, 0.0, 0.0, 1.0);
    //glTranslatef(0.0, -320.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    

    uint32_t startTime = CAppContainer::getInstance()->getTimeMS();
    uint32_t passedTime = startTime - lastTimems;
    lastTimems = startTime;

    if (passedTime >= 125) {
        passedTime = 125;
    }
    //printf("passedTime %d\n", passedTime);

    CAppContainer::getInstance()->DoLoop(passedTime);

    SDL_GL_SwapWindow(sdlGL->window);  // Swap the window/pBmp to display the result.
    
}