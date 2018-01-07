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
// �u�n�O��� body �� fixtures �I���A�N�|�I�s�o�Ө禡
//
void CContactListener::BeginContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();

	auto DensityA = BodyA->GetFixtureList()->GetDensity();
	auto DensityB = BodyB->GetFixtureList()->GetDensity();

	// check �O�_�����U���y�g�L sensor1 �A�u�n�g�L�N�ߨ����L�u�X�h
	if (BodyA->GetFixtureList()->GetDensity() == 10000.0f) { // �N�� sensor1
		BodyB->ApplyLinearImpulse(b2Vec2(0, 50 + rand() % 101), BodyB->GetWorldCenter(), true);
		_bApplyImpulse = true;
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 10000.0f) {// �N�� sensor1
		BodyA->ApplyLinearImpulse(b2Vec2(0, 50 + rand() % 101), BodyB->GetWorldCenter(), true);
		_bApplyImpulse = true;
	}

	if (BodyA->GetUserData() == _targetSprite) {
		float lengthV = BodyB->GetLinearVelocity().Length();
		if (lengthV >= 4.25f) { // ��Ĳ�ɪ��t�׶W�L�@�w���Ȥ~�Q�X����
			_bCreateSpark = true;
			_createLoc = BodyA->GetWorldCenter() + b2Vec2(0, -30 / PTM_RATIO);
		}
	}
	else if (BodyB->GetUserData() == _targetSprite) {
		float lengthV = BodyB->GetLinearVelocity().Length();
		if (lengthV >= 4.25f) { // ��Ĳ�ɪ��t�׶W�L�@�w���Ȥ~�Q�X����
			_bCreateSpark = true;
			_createLoc = BodyB->GetWorldCenter() + b2Vec2(0, -30 / PTM_RATIO);
		}
	}

	if (DensityA == 500.0f || DensityA == 600.0f || DensityA == 700.0f) { // �N�� r.g.b box
		inBox = true;
		
	}
	else if (DensityB == 500.0f || DensityB == 600.0f || DensityB == 700.0f) { // �N�� r.g.b box
		inBox = true;
	}

	if (DensityA == 800.0f ) { // �N�� lv3
		CCLOG("open");
		lv3Open = true;
	}
	else if (DensityB == 800.0f) { // �N�� lv3
		CCLOG("open");
		lv3Open = true;
	}
	
}

//�I������
void CContactListener::EndContact(b2Contact* contact)
{
	b2Body* BodyA = contact->GetFixtureA()->GetBody();
	b2Body* BodyB = contact->GetFixtureB()->GetBody();

	if (BodyA->GetFixtureList()->GetDensity() == 10001.0f && _bApplyImpulse) { // �N�� sensor2
		BodyA->GetFixtureList()->SetDensity(10002);
		_bApplyImpulse = false;
	}
	else if (BodyB->GetFixtureList()->GetDensity() == 10001.0f && _bApplyImpulse) {	// �N�� sensor2
		BodyB->GetFixtureList()->SetDensity(10002);
		_bApplyImpulse = false;
	}
}