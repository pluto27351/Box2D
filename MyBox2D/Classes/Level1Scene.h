#ifndef __LEVEL1_SCENE_H__
#define __LEVEL1_SCENE_H__

#define BOX2D_DEBUG 1

#include "cocos2d.h"
#include "Box2D/Box2D.h"
#include "Common/CButton.h"
#include "Common/CSwitchButton.h"
#include "Common/CContactListener.h"

#ifdef BOX2D_DEBUG
#include "Common/GLES-Render.h"
#include "Common/GB2ShapeCache-x.h"
#endif

#define PTM_RATIO 32.0f
#define RepeatCreateBallTime 3
#define AccelerateMaxNum 2
#define AccelerateRatio 1.5f


class Level1 : public cocos2d::Layer
{
public:

	~Level1();
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();
	Node *_csbRoot;

	// for Box2D
	b2World* _b2World;
	cocos2d::Size _visibleSize;

	
	// for MouseJoint
	b2Body *_bottomBody; // ������ edgeShape
	b2MouseJoint* _MouseJoint;

	Point bornpt;

	bool _bTouchOn;
	bool _bMouseOn = false;
	bool _bboxR = false, _bboxG = false, _bboxB = false;
	CContactListener _colliderSeneor;
	// Box2D Examples
	void setbtn();
	void createStaticBoundary();
	void setStaticWall();
	void setBoards();
	void setFinalBox();
	void setUIbtn();


	void renderball(char *, int);
	CButton *_redBtn,*_blueBtn, *_greenBtn,*_homeBtn, *_replayBtn;
	CSwitchButton *_penBtn;
#ifdef BOX2D_DEBUG
	//DebugDraw
	GLESDebugDraw* _DebugDraw;
	virtual void draw(cocos2d::Renderer* renderer, const cocos2d::Mat4& transform, uint32_t flags);
#endif

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();
	void doStep(float dt);

	cocos2d::EventListenerTouchOneByOne *_listener1;
	bool onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //Ĳ�I�}�l�ƥ�
	void onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //Ĳ�I���ʨƥ�
	void onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent); //Ĳ�I�����ƥ� 

	
    // implement the "static create()" method manually
    CREATE_FUNC(Level1);
};

#endif // __LEVEL1_SCENE_H__
