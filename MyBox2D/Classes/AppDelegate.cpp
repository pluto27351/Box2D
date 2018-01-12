#include <cstdlib>
#include <ctime>
#include "AppDelegate.h"

//#define BOX2D_DEBUG 1

//#include "1_DebugDrawScene.h"
#define BOX2D_EXAMPLE 9

#if  BOX2D_EXAMPLE == 5
#include "Level1Scene.h"
#elif  BOX2D_EXAMPLE == 6
#include "Level2Scene.h"
#elif  BOX2D_EXAMPLE == 7
#include "Level3Scene.h"
#elif  BOX2D_EXAMPLE == 8
#include "Level4Scene.h"
#elif  BOX2D_EXAMPLE == 9
#include "StartScene.h"
#endif

USING_NS_CC;

AppDelegate::AppDelegate() {

}

AppDelegate::~AppDelegate() 
{
}

//if you want a different context,just modify the value of glContextAttrs
//it will takes effect on all platforms
void AppDelegate::initGLContextAttrs()
{
    //set OpenGL context attributions,now can only set six attributions:
    //red,green,blue,alpha,depth,stencil
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8};

    GLView::setGLContextAttrs(glContextAttrs);
}

bool AppDelegate::applicationDidFinishLaunching() {
    // initialize director
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
	if (!glview) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
		glview = GLViewImpl::createWithRect("MyBox2D", Rect(0, 0, 1920, 1080), 0.5f);
#else
		glview = GLViewImpl::create("MyBox2D");
#endif
		director->setOpenGLView(glview);
	}

	director->getOpenGLView()->setDesignResolutionSize(1280, 720, ResolutionPolicy::SHOW_ALL);

    // turn on display FPS
    director->setDisplayStats(true);

    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0f / 60.0f);

    FileUtils::getInstance()->addSearchPath("res");

	srand(time(NULL));

    // create a scene. it's an autorelease object

#if  BOX2D_EXAMPLE == 5
	auto scene = Level1::createScene();
#elif  BOX2D_EXAMPLE == 6
	auto scene = Level2::createScene();
#elif  BOX2D_EXAMPLE == 7
	auto scene = Level3::createScene();
#elif  BOX2D_EXAMPLE == 8
	auto scene = Level4::createScene();
#elif  BOX2D_EXAMPLE == 9
	int num[4][3] = { 0 };
	auto scene = StartScene::createScene(num,1);
#endif

    // run
    director->runWithScene(scene);

    return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

    // if you use SimpleAudioEngine, it must be pause
    // SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

    // if you use SimpleAudioEngine, it must resume here
    // SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}
