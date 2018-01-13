#ifndef __START_SCENE_H__
#define __START_SCENE_H__


#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "Common/CButton.h"
#include "Common/CSwitchButton.h"
#include "SimpleAudioEngine.h"

#include "Level1Scene.h"
#include "Level2Scene.h"
#include "Level3Scene.h"
#include "Level4Scene.h"

#define PTM_RATIO 32.0f
#define RepeatCreateBallTime 3
#define AccelerateMaxNum 2
#define AccelerateRatio 1.5f
#define DRAW_MIN 5
#define DRAW_HEIGHT 3

using namespace CocosDenshion;

class StartScene : public cocos2d::Layer
{
public:

	~StartScene();
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene(int [4][3],int);
	Node *_csbRoot;
	// for Box2D
	b2World* _b2World;
	cocos2d::Size _visibleSize;
	cocos2d::Point bornpt;
	int LV = 1;
	
	int levelball[4][3];
	int maxLevel=0;

	void setballNum(int [4][3],int);
	void setStaticWall();
	void setbtn();
	void setUIbtn();
	void renderball(char *, int);
	CButton *_redBtn,*_blueBtn, *_greenBtn;
	CButton *_startBtn;
	CSwitchButton *_LevelBtn[4], *_NowLevel;

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();
	void doStep(float dt);

	cocos2d::EventListenerTouchOneByOne *_listener1;
	bool onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰開始事件
	void onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰移動事件
	void onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //觸碰結束事件 

	
    // implement the "static create()" method manually
    CREATE_FUNC(StartScene);
};

#endif // __START_SCENE_H__
