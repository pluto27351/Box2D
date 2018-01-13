#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "CContactListener.h"


CContactListener::~CContactListener()
{
	_bApplyImpulse = false;
	_bCreateSpark = false;
	_NumOfSparks = 5;

}

CContactListener::CContactListener()
{
	_bApplyImpulse = false;
	_bCreateSpark = false;
	_NumOfSparks = 5;

}

void CContactListener::setCollisionTarget(cocos2d::Sprite &targetSprite)
{
	_PendulumSprite = &targetSprite;
	_bp = true;
}

void CContactListener::setCollisionTarget2(cocos2d::Sprite &targetSprite)
{
	_StarSprite = &targetSprite;
	_bs = true;
}

//
// 只要是兩個 body 的 fixtures 碰撞，就會呼叫這個函式
//
void CContactListener::BeginContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();

	auto DensityA = BodyA->GetFixtureList()->GetDensity();
	auto DensityB = BodyB->GetFixtureList()->GetDensity();


	if (_bp && BodyA->GetUserData() == _PendulumSprite) {
		float lengthV = BodyB->GetLinearVelocity().Length();
		if (lengthV >= 4.25f) { // 接觸時的速度超過一定的值才噴出火花
			_bCreateSpark = true;
			_createLoc = BodyA->GetWorldCenter();
		}
	}
	else if (_bp && BodyB->GetUserData() == _PendulumSprite) {
		float lengthV = BodyA->GetLinearVelocity().Length();
		if (lengthV >= 4.25f) { // 接觸時的速度超過一定的值才噴出火花
			_bCreateSpark = true;
			_createLoc = BodyB->GetWorldCenter();
		}
	}

	if ((int)(DensityA*100.0f) == 111) {
		BodyA->GetFixtureList()->SetDensity(1.0f);
	}
	else if ((int)(DensityB*100.0f) == 111) {
		BodyB->GetFixtureList()->SetDensity(1.0f);
	}
	if (_bp && BodyA->GetUserData() == _PendulumSprite && (int)DensityB == 1) {
		float lengthV = BodyB->GetLinearVelocity().Length();
		if (lengthV >= 4.25f) {
			BodyB->GetFixtureList()->SetDensity(1.11f);
		}

	}else if (_bp && BodyB->GetUserData() == _PendulumSprite && (int)DensityA == 1) {
		float lengthV = BodyB->GetLinearVelocity().Length();
		if (lengthV >= 4.25f) {
			BodyA->GetFixtureList()->SetDensity(1.11f);
		}
	}


	if (_bs && BodyA->GetUserData() == _StarSprite) {
		star = true;

	}
	else if (_bs && BodyB->GetUserData() == _StarSprite) {
		star = true;
	}


	if      (DensityA == 500.0f)  inBoxR = true;  //RED
	else if (DensityB == 500.0f)  inBoxR = true;
	if      (DensityA == 600.0f)  inBoxG = true;  //GREEN
	else if (DensityB == 600.0f)  inBoxG = true;
	if      (DensityA == 700.0f)  inBoxB = true;  //BLUE
	else if (DensityB == 700.0f)  inBoxB = true;
	
	if (DensityA == 800.0f ) { // 代表 lv3
		CCLOG("open");
		lv3Open = true;
	}
	else if (DensityB == 800.0f) { // 代表 lv3
		CCLOG("open");
		lv3Open = true;
	}
	
}

//碰撞結束
void CContactListener::EndContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();

	if (BodyA->GetFixtureList()->GetDensity() == 10001.0f && _bApplyImpulse) { // 代表 sensor2
		BodyA->GetFixtureList()->SetDensity(10002);
		_bApplyImpulse = false;
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 10001.0f && _bApplyImpulse) {	// 代表 sensor2
		BodyB->GetFixtureList()->SetDensity(10002);
		_bApplyImpulse = false;
	}
}