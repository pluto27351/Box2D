#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#include "CContactListener.h"

CContactListener::CContactListener()
{
	_bApplyImpulse = false;
	_bCreateSpark = false;
	_NumOfSparks = 5;
}
void CContactListener::setCollisionTarget(cocos2d::Sprite &targetSprite)
{
	_targetSprite = &targetSprite;
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

	// check 是否為落下的球經過 sensor1 ，只要經過就立刻讓他彈出去
	if (BodyA->GetFixtureList()->GetDensity() == 10000.0f) { // 代表 sensor1
		BodyB->ApplyLinearImpulse(b2Vec2(0, 50 + rand() % 101), BodyB->GetWorldCenter(), true);
		_bApplyImpulse = true;
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 10000.0f) {// 代表 sensor1
		BodyA->ApplyLinearImpulse(b2Vec2(0, 50 + rand() % 101), BodyB->GetWorldCenter(), true);
		_bApplyImpulse = true;
	}

	if (BodyA->GetUserData() == _targetSprite) {
		float lengthV = BodyB->GetLinearVelocity().Length();
		if (lengthV >= 4.25f) { // 接觸時的速度超過一定的值才噴出火花
			_bCreateSpark = true;
			_createLoc = BodyA->GetWorldCenter() + b2Vec2(0, -30 / PTM_RATIO);
		}
	}
	else if (BodyB->GetUserData() == _targetSprite) {
		float lengthV = BodyB->GetLinearVelocity().Length();
		if (lengthV >= 4.25f) { // 接觸時的速度超過一定的值才噴出火花
			_bCreateSpark = true;
			_createLoc = BodyB->GetWorldCenter() + b2Vec2(0, -30 / PTM_RATIO);
		}
	}

	if (DensityA == 500.0f || DensityA == 600.0f || DensityA == 700.0f) { // 代表 r.g.b box
		inBox = true;
		
	}
	else if (DensityB == 500.0f || DensityB == 600.0f || DensityB == 700.0f) { // 代表 r.g.b box
		inBox = true;
	}

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