#pragma once
#include "cocos2d.h"
#include "Box2D/Box2D.h"
#define PTM_RATIO 32.0f

class CContactListener : public b2ContactListener
{
public:
	cocos2d::Sprite *_PendulumSprite; // 用於判斷是否
	cocos2d::Sprite *_StarSprite; // 用於判斷是否
	bool star = false;
	bool _bCreateSpark;		//產生火花
	bool _bApplyImpulse;	// 產生瞬間的衝力
	b2Vec2 _createLoc;
	int  _NumOfSparks;
	bool inBoxR = false, inBoxG = false, inBoxB = false;
	bool lv3Open = false;
	bool _bp = false,_bs=false;
	CContactListener();
	~CContactListener();
	//碰撞開始
	virtual void BeginContact(b2Contact* contact);
	//碰撞結束
	virtual void EndContact(b2Contact* contact);
	void setCollisionTarget(cocos2d::Sprite &targetSprite);
	void setCollisionTarget2(cocos2d::Sprite &targetSprite);
	//void setballsensor();
	//CREATE_FUNC(CContactListener);
};