#pragma once
#include "cocos2d.h"
#include "Box2D/Box2D.h"
#define PTM_RATIO 32.0f

class CContactListener : public b2ContactListener
{
public:
	cocos2d::Sprite *_targetSprite; // 用於判斷是否
	bool _bCreateSpark;		//產生火花
	bool _bApplyImpulse;	// 產生瞬間的衝力
	b2Vec2 _createLoc;
	int  _NumOfSparks;
	bool inBox = false;
	bool lv3Open = false;
	CContactListener();
	//碰撞開始
	virtual void BeginContact(b2Contact* contact);
	//碰撞結束
	virtual void EndContact(b2Contact* contact);
	void setCollisionTarget(cocos2d::Sprite &targetSprite);

	//CREATE_FUNC(CContactListener);
};