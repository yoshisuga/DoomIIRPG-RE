// [iOS] Apps linked against the iOS 27 SDK must adopt the scene lifecycle, which SDL2 predates.
// This delegate records the window scene so SDLGL can attach SDL's UIWindow to it. SDL's own
// background/foreground handling uses UIApplication notifications, which still fire with scenes.

#import <UIKit/UIKit.h>

#include <SDL.h>
#include <SDL_syswm.h>

@interface DoomSceneDelegate : UIResponder <UIWindowSceneDelegate>
@property (nonatomic, strong) UIWindow *window;
@end

static UIWindowScene *gWindowScene = nil;

@implementation DoomSceneDelegate

- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)connectionOptions {
	if ([scene isKindOfClass:[UIWindowScene class]]) {
		gWindowScene = (UIWindowScene *)scene;
	}
}

- (void)sceneDidDisconnect:(UIScene *)scene {
	if (scene == gWindowScene) {
		gWindowScene = nil;
	}
}

@end

// Runs the main run loop until UIKit has connected the scene
void iosWaitForWindowScene(void) {
	const CFAbsoluteTime deadline = CFAbsoluteTimeGetCurrent() + 5.0;
	while (gWindowScene == nil && CFAbsoluteTimeGetCurrent() < deadline) {
		CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.01, false);
	}
	if (gWindowScene == nil) {
		printf("iosWaitForWindowScene: no window scene connected\n");
	}
}

// A UIWindow without a scene is never displayed
void iosAttachWindowToScene(SDL_Window *window) {
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (gWindowScene == nil || !SDL_GetWindowWMInfo(window, &info)) {
		printf("iosAttachWindowToScene: failed: %s\n", SDL_GetError());
		return;
	}

	UIWindow *uiwindow = info.info.uikit.window;
	uiwindow.windowScene = gWindowScene;
	[uiwindow makeKeyAndVisible];
}
