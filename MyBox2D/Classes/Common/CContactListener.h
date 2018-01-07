#pragma once
#include "cocos2d.h"
#include "Box2D/Box2D.h"
#define PTM_RATIO 32.0f

class CContactListener : public b2ContactListener
{
public:
	cocos2d::Sprite *_targetSprite; // �Ω�P�_�O�_
	bool _bCreateSpark;		//���ͤ���
	bool _bApplyImpulse;	// �����������ĤO
	b2Vec2 _createLoc;
	int  _NumOfSparks;
	bool inBox = false;
	bool lv3Open = false;
	CContactListener();
	//�I���}�l
	virtual void BeginContact(b2Contact* contact);
	//�I������
	virtual void EndContact(b2Contact* contact);
	void setCollisionTarget(cocos2d::Sprite &targetSprite);

	//CREATE_FUNC(CContactListener);
};