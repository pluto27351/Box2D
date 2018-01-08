#include "DrawScene.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
#define DRAW_MIN 5
#define DRAW_HEIGHT 3

USING_NS_CC;

#ifndef DRAWTEXTURE
#define DRAWTEXTURE 5
#endif
char g_DrawTexture[DRAWTEXTURE][20] = {
	"rope01.png","rope02.png","rope03.png","rope04.png","rope05.png"
};


using namespace cocostudio::timeline;

MouseDraw::~MouseDraw()
{

#ifdef BOX2D_DEBUG
	if (_DebugDraw != NULL) delete _DebugDraw;
#endif

	if (_b2World != nullptr) delete _b2World;
//  for releasing Plist&Texture
	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
	Director::getInstance()->getTextureCache()->removeUnusedTextures();

}

Scene* MouseDraw::createScene()
{
    auto scene = Scene::create();
    auto layer = MouseDraw::create();
    scene->addChild(layer);
    return scene;
}

// on "init" you need to initialize your instance
bool MouseDraw::init()
{   
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }

//  For Loading Plist+Texture
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");

	_visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
    
#ifndef BOX2D_DEBUG
	auto rootNode = CSLoader::createNode("MainScene.csb");
	this->addChild(rootNode);
#endif

	//���D : ��ܥثe BOX2D �Ҥ��Ъ��\��
	_titleLabel = Label::createWithTTF("Static & Dynamic", "fonts/Marker Felt.ttf", 32);
	_titleLabel->setPosition(_visibleSize.width / 2-200.0f, _visibleSize.height*0.95f);
	this->addChild(_titleLabel, 1);

	// �إ� Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);	//���O��V
	bool AllowSleep = true;					//���\�ε�
	_b2World = new b2World(Gravity);		//�Ыإ@��
	_b2World->SetAllowSleeping(AllowSleep);	//�]�w���󤹳\�ε�

	// �b�ù����|����ɫإ� Static Body ��������
	createStaticBoundary();
	// Ū�J CSB �ɡA�ó]�w���R�A����
//	readBlocksCSBFile("blocks.csb");
	readSceneFile("staticBlocks.csb");

#ifdef SET_GRAVITY_BUTTON
	// �إߥ|�ӭ��O��V���s
	setGravityButton();
#endif


#ifdef BOX2D_DEBUG
	//DebugDrawInit
	_DebugDraw = nullptr;
	_DebugDraw = new GLESDebugDraw(PTM_RATIO);
	//�]�wDebugDraw
	_b2World->SetDebugDraw(_DebugDraw);
	//���ø�s���O
	uint32 flags = 0;
	flags += GLESDebugDraw::e_shapeBit;						//ø�s�Ϊ�
	flags += GLESDebugDraw::e_pairBit;
	flags += GLESDebugDraw::e_jointBit;
	flags += GLESDebugDraw::e_centerOfMassBit;
	flags += GLESDebugDraw::e_aabbBit;
	//�]�wø�s����
	_DebugDraw->SetFlags(flags);
#endif

	_listener1 = EventListenerTouchOneByOne::create();	//�Ыؤ@�Ӥ@��@���ƥ��ť��
	_listener1->onTouchBegan = CC_CALLBACK_2(MouseDraw::onTouchBegan, this);		//�[�JĲ�I�}�l�ƥ�
	_listener1->onTouchMoved = CC_CALLBACK_2(MouseDraw::onTouchMoved, this);		//�[�JĲ�I���ʨƥ�
	_listener1->onTouchEnded = CC_CALLBACK_2(MouseDraw::onTouchEnded, this);		//�[�JĲ�I���}�ƥ�

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//�[�J��Ыت��ƥ��ť��
	this->schedule(CC_SCHEDULE_SELECTOR(MouseDraw::doStep));

    return true;
}

void MouseDraw::readBlocksCSBFile(const char *csbfilename)
{
	// Ū���ñN�Ҧ��� block �ϧ� �]�w�� EdgeShape
	auto csbRoot = CSLoader::createNode(csbfilename);
	csbRoot->setPosition(_visibleSize.width / 2.0f, _visibleSize.height / 2.0f);
	Point pntLoc = csbRoot->getPosition();
	addChild(csbRoot, 1);
	char tmp[20] = "";

	// ���� EdgeShape �� body
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // �]�w�o�� Body �� �R�A��
	bodyDef.userData = NULL;
	// �b b2World �����͸� Body, �öǦ^���ͪ� b2Body ���󪺫���
	// ���ͤ@���A�N�i�H���᭱�Ҧ��� Shape �ϥ�
	b2Body *body = _b2World->CreateBody(&bodyDef);

	// �����R�A��ɩһݭn�� EdgeShape
	b2EdgeShape edgeShape;
	b2FixtureDef fixtureDef; // ���� Fixture
	fixtureDef.shape = &edgeShape;

	for (size_t i = 1; i <= 4; i++)
	{
		// ���ͩһݭn�� Sprite file name int plist 
		// ���B���o�����O�۹�� csbRoot �Ҧb��m���۹�y��
		// �b�p�� edgeShape ���۹����y�ЮɡA�����i���ഫ
		sprintf(tmp, "block1_%02d", i);
		auto edgeSprite = (Sprite *)csbRoot->getChildByName(tmp);
		Size ts = edgeSprite->getContentSize();
		Point loc = edgeSprite->getPosition();
		float angle = edgeSprite->getRotation();
		float scale = edgeSprite->getScaleX();	// �������u�q�ϥܰ��]���u���� X �b��j

		Point lep1, lep2, wep1, wep2; // EdgeShape ����Ӻ��I
		lep1.y = 0; lep1.x = -(ts.width - 4) / 2.0f;
		lep2.y = 0; lep2.x = (ts.width - 4) / 2.0f;

		// �Ҧ����u�q�ϥܳ��O�O�����������I�� (0,0)�A
		// �ھ��Y��B���ಣ�ͩһݭn���x�}
		// �ھڼe�׭p��X��Ӻ��I���y�СA�M��e�W�}�x�}
		// �M��i�����A
		// Step1: ��CHECK ���L����A������h�i����I���p��
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scale;  // ���]�w X �b���Y��
		cocos2d::Mat4::createRotationZ(angle*M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = pntLoc.x + loc.x; //�]�w Translation�A�ۤv���[�W���˪�
		modelMatrix.m[7] = pntLoc.y + loc.y; //�]�w Translation�A�ۤv���[�W���˪�

											 // ���ͨ�Ӻ��I
		wep1.x = lep1.x * modelMatrix.m[0] + lep1.y * modelMatrix.m[1] + modelMatrix.m[3];
		wep1.y = lep1.x * modelMatrix.m[4] + lep1.y * modelMatrix.m[5] + modelMatrix.m[7];
		wep2.x = lep2.x * modelMatrix.m[0] + lep2.y * modelMatrix.m[1] + modelMatrix.m[3];
		wep2.y = lep2.x * modelMatrix.m[4] + lep2.y * modelMatrix.m[5] + modelMatrix.m[7];

		edgeShape.Set(b2Vec2(wep1.x / PTM_RATIO, wep1.y / PTM_RATIO), b2Vec2(wep2.x / PTM_RATIO, wep2.y / PTM_RATIO));
		body->CreateFixture(&fixtureDef);
	}
}

void MouseDraw::readSceneFile(const char *csbfilename)
{
	// Ū���ñN�Ҧ��� block �ϧ� �]�w�� EdgeShape
	auto csbRoot = CSLoader::createNode(csbfilename);
	csbRoot->setPosition(_visibleSize.width / 2.0f, _visibleSize.height / 2.0f);
	Point pntLoc = csbRoot->getPosition();
	addChild(csbRoot, 1);
	char tmp[20] = "";

	// ���� EdgeShape �� body
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // �]�w�o�� Body �� �R�A��
	bodyDef.userData = NULL;
	// �b b2World �����͸� Body, �öǦ^���ͪ� b2Body ���󪺫���
	// ���ͤ@���A�N�i�H���᭱�Ҧ��� Shape �ϥ�
	b2Body *body = _b2World->CreateBody(&bodyDef);

	// �����R�A��ɩһݭn�� EdgeShape
	b2EdgeShape edgeShape;
	b2FixtureDef fixtureDef; // ���� Fixture
	fixtureDef.shape = &edgeShape;

	for (size_t i = 1; i <= 4; i++)
	{
		// ���ͩһݭn�� Sprite file name int plist 
		// ���B���o�����O�۹�� csbRoot �Ҧb��m���۹�y��
		// �b�p�� edgeShape ���۹����y�ЮɡA�����i���ഫ
		sprintf(tmp, "block1_%02d", i);
		auto edgeSprite = (Sprite *)csbRoot->getChildByName(tmp);
		Size ts = edgeSprite->getContentSize();
		Point loc = edgeSprite->getPosition();
		float angle = edgeSprite->getRotation();
		float scale = edgeSprite->getScaleX();	// �������u�q�ϥܰ��]���u���� X �b��j

		Point lep1, lep2, wep1, wep2; // EdgeShape ����Ӻ��I
		lep1.y = 0; lep1.x = -(ts.width - 4) / 2.0f;
		lep2.y = 0; lep2.x = (ts.width - 4) / 2.0f;

		// �Ҧ����u�q�ϥܳ��O�O�����������I�� (0,0)�A
		// �ھ��Y��B���ಣ�ͩһݭn���x�}
		// �ھڼe�׭p��X��Ӻ��I���y�СA�M��e�W�}�x�}
		// �M��i�����A
		// Step1: ��CHECK ���L����A������h�i����I���p��
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scale;  // ���]�w X �b���Y��
		cocos2d::Mat4::createRotationZ(angle*M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = pntLoc.x + loc.x; //�]�w Translation�A�ۤv���[�W���˪�
		modelMatrix.m[7] = pntLoc.y + loc.y; //�]�w Translation�A�ۤv���[�W���˪�

		// ���ͨ�Ӻ��I
		wep1.x = lep1.x * modelMatrix.m[0] + lep1.y * modelMatrix.m[1] + modelMatrix.m[3];
		wep1.y = lep1.x * modelMatrix.m[4] + lep1.y * modelMatrix.m[5] + modelMatrix.m[7];
		wep2.x = lep2.x * modelMatrix.m[0] + lep2.y * modelMatrix.m[1] + modelMatrix.m[3];
		wep2.y = lep2.x * modelMatrix.m[4] + lep2.y * modelMatrix.m[5] + modelMatrix.m[7];

		// bottom edge
		edgeShape.Set(b2Vec2(wep1.x / PTM_RATIO, wep1.y / PTM_RATIO), b2Vec2(wep2.x / PTM_RATIO, wep2.y / PTM_RATIO));
		body->CreateFixture(&fixtureDef);
	}


	// ���ͪ�����R�A����һݭn�� rectShape
	b2PolygonShape rectShape;
	fixtureDef.shape = &rectShape;

	for (size_t i = 1; i <= 2; i++)
	{
		// ���ͩһݭn�� Sprite file name int plist 
		// ���B���o�����O�۹�� csbRoot �Ҧb��m���۹�y��
		// �b�p�� edgeShape ���۹����y�ЮɡA�����i���ഫ
		sprintf(tmp, "ployblock1_%02d", i);
		auto rectSprite = (Sprite *)csbRoot->getChildByName(tmp);
		Size ts = rectSprite->getContentSize();
		Point loc = rectSprite->getPosition();
		float angle = rectSprite->getRotation();
		float scaleX = rectSprite->getScaleX();	// �������u�q�ϥܰ��]���u���� X �b��j
		float scaleY = rectSprite->getScaleY();	// �������u�q�ϥܰ��]���u���� X �b��j

		// rectShape ���|�Ӻ��I, 0 �k�W�B 1 ���W�B 2 ���U 3 �k�U
		Point lep[4], wep[4];
		lep[0].x = (ts.width - 4) / 2.0f;;  lep[0].y = (ts.height - 4) / 2.0f;
		lep[1].x = -(ts.width - 4) / 2.0f;; lep[1].y = (ts.height - 4) / 2.0f;
		lep[2].x = -(ts.width - 4) / 2.0f;; lep[2].y = -(ts.height - 4) / 2.0f;
		lep[3].x = (ts.width - 4) / 2.0f;;  lep[3].y = -(ts.height - 4) / 2.0f;

		// �Ҧ����u�q�ϥܳ��O�O�����������I�� (0,0)�A
		// �ھ��Y��B���ಣ�ͩһݭn���x�}
		// �ھڼe�׭p��X��Ӻ��I���y�СA�M��e�W�}�x�}
		// �M��i�����A
		// Step1: ��CHECK ���L����A������h�i����I���p��
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scaleX;  // ���]�w X �b���Y��
		modelMatrix.m[5] = scaleY;  // ���]�w Y �b���Y��
		cocos2d::Mat4::createRotationZ(angle*M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = pntLoc.x + loc.x; //�]�w Translation�A�ۤv���[�W���˪�
		modelMatrix.m[7] = pntLoc.y + loc.y; //�]�w Translation�A�ۤv���[�W���˪�
		for (size_t j = 0; j < 4; j++)
		{
			wep[j].x = lep[j].x * modelMatrix.m[0] + lep[j].y * modelMatrix.m[1] + modelMatrix.m[3];
			wep[j].y = lep[j].x * modelMatrix.m[4] + lep[j].y * modelMatrix.m[5] + modelMatrix.m[7];
		}
		b2Vec2 vecs[] = {
			b2Vec2(wep[0].x / PTM_RATIO, wep[0].y / PTM_RATIO),
			b2Vec2(wep[1].x / PTM_RATIO, wep[1].y / PTM_RATIO),
			b2Vec2(wep[2].x / PTM_RATIO, wep[2].y / PTM_RATIO),
			b2Vec2(wep[3].x / PTM_RATIO, wep[3].y / PTM_RATIO) };

		rectShape.Set(vecs, 4);
		body->CreateFixture(&fixtureDef);
	}

	// ���ͤT�����R�A����һݭn�� triShape
	b2PolygonShape triShape;
	fixtureDef.shape = &triShape;

	for (size_t i = 1; i <= 2; i++)
	{
		// ���ͩһݭn�� Sprite file name int plist 
		// ���B���o�����O�۹�� csbRoot �Ҧb��m���۹�y��
		// �b�p�� edgeShape ���۹����y�ЮɡA�����i���ഫ
		sprintf(tmp, "triangle1_%02d", i);
		auto triSprite = (Sprite *)csbRoot->getChildByName(tmp);
		Size ts = triSprite->getContentSize();
		Point loc = triSprite->getPosition();
		float angle = triSprite->getRotation();
		float scaleX = triSprite->getScaleX();	// �������u�q�ϥܰ��]���u���� X �b��j
		float scaleY = triSprite->getScaleY();	// �������u�q�ϥܰ��]���u���� X �b��j

		Point lep[3], wep[3];	// triShape ���T�ӳ��I, 0 ���I�B 1 ���U�B 2 �k�U
		lep[0].x = 0;  lep[0].y = (ts.height - 2) / 2.0f;
		lep[1].x = -(ts.width - 2) / 2.0f; lep[1].y = -(ts.height - 2) / 2.0f;
		lep[2].x = (ts.width - 2) / 2.0f; lep[2].y = -(ts.height - 2) / 2.0f;


		// �Ҧ����u�q�ϥܳ��O�O�����������I�� (0,0)�A
		// �ھ��Y��B���ಣ�ͩһݭn���x�}
		// �ھڼe�׭p��X��Ӻ��I���y�СA�M��e�W�}�x�}
		// �M��i�����A
		// Step1: ��CHECK ���L����A������h�i����I���p��
		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scaleX;  // ���]�w X �b���Y��
		modelMatrix.m[5] = scaleY;  // ���]�w Y �b���Y��
		cocos2d::Mat4::createRotationZ(angle*M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = pntLoc.x + loc.x; //�]�w Translation�A�ۤv���[�W���˪�
		modelMatrix.m[7] = pntLoc.y + loc.y; //�]�w Translation�A�ۤv���[�W���˪�
		for (size_t j = 0; j < 3; j++)
		{
			wep[j].x = lep[j].x * modelMatrix.m[0] + lep[j].y * modelMatrix.m[1] + modelMatrix.m[3];
			wep[j].y = lep[j].x * modelMatrix.m[4] + lep[j].y * modelMatrix.m[5] + modelMatrix.m[7];
		}
		b2Vec2 vecs[] = {
			b2Vec2(wep[0].x / PTM_RATIO, wep[0].y / PTM_RATIO),
			b2Vec2(wep[1].x / PTM_RATIO, wep[1].y / PTM_RATIO),
			b2Vec2(wep[2].x / PTM_RATIO, wep[2].y / PTM_RATIO) };

		triShape.Set(vecs, 3);
		body->CreateFixture(&fixtureDef);
	}


	// ���Ͷ���R�A����һݭn�� circleShape
	b2CircleShape circleShape;
	fixtureDef.shape = &circleShape;

	for (size_t i = 1; i <= 2; i++)
	{
		// ���ͩһݭn�� Sprite file name int plist 
		// ���B���o�����O�۹�� csbRoot �Ҧb��m���۹�y��
		// �b�p�� edgeShape ���۹����y�ЮɡA�����i���ഫ
		sprintf(tmp, "ball1_%02d", i);
		auto circleSprite = (Sprite *)csbRoot->getChildByName(tmp);
		Size ts = circleSprite->getContentSize();
		Point loc = circleSprite->getPosition();
		float scaleX = circleSprite->getScaleX();	// �������u�q�ϥܰ��]���u���� X �b��j
		float radius = (ts.width - 4)*scaleX *0.5f;

		Point wloc;
		wloc.x = loc.x + pntLoc.x; wloc.y = loc.y + pntLoc.y;
		circleShape.m_radius = radius / PTM_RATIO;
		bodyDef.position.Set(wloc.x / PTM_RATIO, wloc.y / PTM_RATIO);
		b2Body *body = _b2World->CreateBody(&bodyDef);
		body->CreateFixture(&fixtureDef);
	}
}

void MouseDraw::doStep(float dt)
{
	int velocityIterations = 8;	// �t�׭��N����
	int positionIterations = 1; // ��m���N���� ���N���Ƥ@��]�w��8~10 �V���V�u����Ĳv�V�t
	// Instruct the world to perform a single step of simulation.
	// It is generally best to keep the time step and iterations fixed.
	_b2World->Step(dt, velocityIterations, positionIterations);

	// ���o _b2World ���Ҧ��� body �i��B�z
	// �̥D�n�O�ھڥثe�B�⪺���G�A��s���ݦb body �� sprite ����m
	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		//body->ApplyForce(b2Vec2(10.0f, 10.0f), body->GetWorldCenter(), true);
		// �H�U�O�H Body ���]�t Sprite ��ܬ���
		if (body->GetUserData() != NULL)
		{
			Sprite *ballData = (Sprite*)body->GetUserData();
			ballData->setPosition(body->GetPosition().x*PTM_RATIO, body->GetPosition().y*PTM_RATIO);
			ballData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
		}
	}
}

bool MouseDraw::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent)//Ĳ�I�}�l�ƥ�
{
	Point touchLoc = pTouch->getLocation();
	bool bOnGravityBtn = false;

#ifdef SET_GRAVITY_BUTTON
	for (size_t i = 0; i < 4; i++) 
	{
		if ( _gravityBtn[i]->touchesBegin(touchLoc)) {
			bOnGravityBtn = true;
			break;
		}
	}
#endif

	if (!bOnGravityBtn) {
		_bDraw = true;
		_HDrawPt = new DrawPoint;
		_HDrawPt->pt = touchLoc;
		_HDrawPt->next = NULL;
		_NDrawPt = _HDrawPt;
	}
	//_drawDef = new b2BodyDef;
	//_drawDef->type = b2_dynamicBody;
	////_drawDef->position = b2Vec2(touchLoc.x/ PTM_RATIO, touchLoc.y/ PTM_RATIO);
	//_drawDef->position.Set(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO);
	//_drawBody = _b2World->CreateBody(_drawDef);
	//startpt = touchLoc;

	return true;
}

void  MouseDraw::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //Ĳ�I���ʨƥ�
{
	Point touchLoc = pTouch->getLocation();
#ifdef SET_GRAVITY_BUTTON
	for (size_t i = 0; i < 4; i++)
	{
		if (_gravityBtn[i]->touchesMoved(touchLoc)) {
			break;
		}
	}
#endif

	if (_bDraw) {
		float len = ccpDistance(_NDrawPt->pt, touchLoc);
		if (len > DRAW_MIN) {						
			struct DrawPoint *newPt;
			newPt = new DrawPoint;
			newPt->pt = touchLoc;
			float x = newPt->pt.x - _NDrawPt->pt.x;
			float y = newPt->pt.y - _NDrawPt->pt.y;
			newPt->r = atan2f(y, x);
			newPt->texture = Sprite::createWithSpriteFrameName(g_DrawTexture[rand() % DRAWTEXTURE]);
			newPt->texture->setPosition(touchLoc);
			newPt->texture->setScale(0.75f);
			newPt->texture->setRotation(-(newPt->r*180/M_PI));
			this->addChild(newPt->texture, 2);
			newPt->next = NULL;
			_NDrawPt->next = newPt;
			_NDrawPt = newPt;
		}
	}

	//float len = ccpDistance(startpt, touchLoc);
	//if (len > DRAW_MIN) {
	//	b2PolygonShape shape;
	//	//float r = atanf((touchLoc.y - startpt.y) / (touchLoc.x - startpt.x));
	//	_drawFixture = new b2FixtureDef;
	//	_drawFixture->shape = &shape;
	//	_drawFixture->density = 2.0f;
	//	_drawFixture->friction = 0.3f;
	//	_drawFixture->restitution = 0.3f;
	//	b2Vec2 vec[] = {
	//		b2Vec2(startpt.x / PTM_RATIO,startpt.y / PTM_RATIO),
	//		b2Vec2(startpt.x / PTM_RATIO,(startpt.y + DRAW_HEIGHT) / PTM_RATIO),
	//		b2Vec2(touchLoc.x / PTM_RATIO,touchLoc.y / PTM_RATIO),
	//		b2Vec2(touchLoc.x / PTM_RATIO,(touchLoc.y + DRAW_HEIGHT) / PTM_RATIO),
	//	};
	//	shape.Set(vec, 4);
	//	_drawBody->CreateFixture(_drawFixture);
	//	startpt = touchLoc;
	//}
}

void  MouseDraw::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //Ĳ�I�����ƥ� 
{
	Point touchLoc = pTouch->getLocation();


#ifdef SET_GRAVITY_BUTTON
	for (size_t i = 0; i < 4; i++)
	{
		if (_gravityBtn[i]->touchesEnded(touchLoc)) {
			// ���ܭ��O��V
			if( i == 0 ) _b2World->SetGravity(b2Vec2(0, -9.8f));
			else if(i == 1) _b2World->SetGravity(b2Vec2(-9.8f, 0));
			else if (i == 2) _b2World->SetGravity(b2Vec2(0, 9.8f));
			else  _b2World->SetGravity(b2Vec2(9.8f, 0));
			break;
		}
	}
#endif

	if (_bDraw) {
		drawLine();
		_bDraw = false;
	}
}

#ifdef BOX2D_DEBUG
//��gø�s��k
void MouseDraw::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif

#ifdef SET_GRAVITY_BUTTON
void MouseDraw::setGravityButton() {
	_gravityBtn[0] = CButton::create();
	_gravityBtn[0]->setButtonInfo("dnarrow.png", "dnarrowon.png", Point(_visibleSize.width / 2.0f, 50.0f));
	_gravityBtn[0]->setScale(0.75f);
	this->addChild(_gravityBtn[0], 10);

	_gravityBtn[1] = CButton::create();
	_gravityBtn[1]->setButtonInfo("leftarrow.png", "leftarrowon.png", Point(50.0f, _visibleSize.height / 2.0f));
	_gravityBtn[1]->setScale(0.75f);
	this->addChild(_gravityBtn[1], 10);

	_gravityBtn[2] = CButton::create();
	_gravityBtn[2]->setButtonInfo("uparrow.png", "uparrowon.png", Point(_visibleSize.width / 2.0f, _visibleSize.height - 50.0f));
	_gravityBtn[2]->setScale(0.75f);
	this->addChild(_gravityBtn[2], 10);

	_gravityBtn[3] = CButton::create();
	_gravityBtn[3]->setButtonInfo("rightarrow.png", "rightarrowon.png", Point(_visibleSize.width - 50.0f, _visibleSize.height / 2.0f));
	_gravityBtn[3]->setScale(0.75f);
	this->addChild(_gravityBtn[3], 10);
}
#endif

void MouseDraw::createStaticBoundary()
{
	// ������ Body, �]�w�������Ѽ�

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // �]�w�o�� Body �� �R�A��
	bodyDef.userData = NULL;
	// �b b2World �����͸� Body, �öǦ^���ͪ� b2Body ���󪺫���
	// ���ͤ@���A�N�i�H���᭱�Ҧ��� Shape �ϥ�
	b2Body *body = _b2World->CreateBody(&bodyDef);	

	// �����R�A��ɩһݭn�� EdgeShape
	b2EdgeShape edgeShape;
	b2FixtureDef edgeFixtureDef; // ���� Fixture
	edgeFixtureDef.shape = &edgeShape;
	// bottom edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, 0.0f / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	// left edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(0.0f / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	// right edge
	edgeShape.Set(b2Vec2(_visibleSize.width / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	// top edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, _visibleSize.height / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);
}

void MouseDraw::drawLine() {
	struct DrawPoint *DiePt;
	_NDrawPt = _HDrawPt;

	if (_HDrawPt->next) {  //�ܤ�2�I
		b2BodyDef drawDef;
		drawDef.type = b2_dynamicBody;
		//drawDef.userData = _HDrawPt->texture;
		drawDef.position.Set(0,0);
		b2Body *drawBody = _b2World->CreateBody(&drawDef);

		while (_NDrawPt->next != NULL) {
			//// �]�w�Ӫ��骺�~��
			//b2CircleShape ballShape;	//  �ŧi���骺�~�������ܼơA���B�O��Ϊ���
			//ballShape.m_radius = 0.75f*(20 - 4) *0.5f / PTM_RATIO;
			//// �H b2FixtureDef  ���c�ŧi���鵲�c�ܼơA�ó]�w���骺�������z�Y��
			//b2FixtureDef fixtureDef;
			//fixtureDef.shape = &ballShape;			// ���w���骺�~�������
			//fixtureDef.restitution = 0.75f;			// �]�w�u�ʫY��
			//fixtureDef.density = 5.0f;				// �]�w�K��
			//fixtureDef.friction = 0.15f;			// �]�w�����Y��
			////drawBody->CreateFixture(&fixtureDef);	// �b Body �W���ͳo�ӭ��骺�]�w
			//drawDef.position.Set(_NDrawPt->pt.x / PTM_RATIO, _NDrawPt->pt.y / PTM_RATIO);
			//b2Body *body = _b2World->CreateBody(&drawDef);
			//body->CreateFixture(&fixtureDef);

			float x = _NDrawPt->pt.x - _NDrawPt->next->pt.x;
			float y= _NDrawPt->pt.y - _NDrawPt->next->pt.y;
			float r = atan2f(y, x);
			cocos2d::Point pt[2],pos[4];
			pt[0].x = _NDrawPt->pt.x;  pt[0].y = _NDrawPt->pt.y;
			pt[1].x = _NDrawPt->next->pt.x;  pt[1].y = _NDrawPt->next->pt.y;
			for (int i = 0; i < 2; i++) {
				pos[i].x = pt[i].x + ( 0*cosf(r) + DRAW_MIN/2*sinf(r) );
				pos[i].y = pt[i].y + (0 * sinf(r)*(-1) + DRAW_MIN / 2 * cosf(r));
				pos[i+2].x = pt[i].x + ( 0*cosf(r) - DRAW_MIN/2*sinf(r) );
				pos[i+2].y =pt[i].y + (0 * sinf(r)*(-1) - DRAW_MIN / 2 * cosf(r));

			}
			b2PolygonShape shape;
			b2FixtureDef drawFixture;
			drawFixture.shape = &shape;
			drawFixture.density = 2.0f;
			drawFixture.friction = 0;
			drawFixture.restitution = 0.3f;
			b2Vec2 vec[] = {
				b2Vec2(pos[0].x / PTM_RATIO, pos[0].y / PTM_RATIO),
				b2Vec2(pos[1].x / PTM_RATIO, pos[1].y / PTM_RATIO),
				b2Vec2(pos[2].x / PTM_RATIO, pos[2].y / PTM_RATIO),
				b2Vec2(pos[3].x / PTM_RATIO, pos[3].y / PTM_RATIO) };
			shape.Set(vec, 4);
			drawBody->CreateFixture(&drawFixture);

			DiePt = _NDrawPt;
			_NDrawPt = DiePt->next;
			delete DiePt;
		}
	}

}