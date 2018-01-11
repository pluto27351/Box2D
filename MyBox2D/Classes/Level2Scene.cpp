#include "Level2Scene.h"
#include "StartScene.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocostudio::timeline;

#define MAX_CIRCLE_OBJECTS  11
#define MAX_2(X,Y) (X)>(Y) ? (X) : (Y)

extern char ball[MAX_CIRCLE_OBJECTS][20];

extern Color3B BlockColor[3];
extern Color3B BlockColor2[3];

Level2::~Level2()
{

#ifdef BOX2D_DEBUG
	if (_DebugDraw != NULL) delete _DebugDraw;
#endif

	if (_b2World != nullptr) delete _b2World;
//  for releasing Plist&Texture
	//SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
	Director::getInstance()->getTextureCache()->removeUnusedTextures();

}

Scene* Level2::createScene()
{
    auto scene = Scene::create();
    auto layer = Level2::create();
    scene->addChild(layer);
    return scene;
}

// on "init" you need to initialize your instance
bool Level2::init()
{   
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }

//  For Loading Plist+Texture
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("UIBTN.plist");

	_visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();
   

	// �إ� Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);	//���O��V
	bool AllowSleep = true;					//���\�ε�
	_b2World = new b2World(Gravity);		//�Ыإ@��
	_b2World->SetAllowSleeping(AllowSleep);	//�]�w���󤹳\�ε�

	// Ū�J CSB ��
	_csbRoot = CSLoader::createNode("Level2.csb");
	

#ifdef BOX2D_DEBUG
	// �]�w��ܭI���ϥ�
	auto bgSprite = _csbRoot->getChildByName("bg64_2");
	bgSprite->setVisible(false);

#endif

	addChild(_csbRoot, 1);

	createStaticBoundary();
	setStaticWall();
	//setBoards();
	setPendulum();
	setFinalBox();
	setbtn();
	setUIbtn();

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

	_b2World->SetContactListener(&_colliderSeneor);
	_listener1 = EventListenerTouchOneByOne::create();	//�Ыؤ@�Ӥ@��@���ƥ��ť��
	_listener1->onTouchBegan = CC_CALLBACK_2(Level2::onTouchBegan, this);		//�[�JĲ�I�}�l�ƥ�
	_listener1->onTouchMoved = CC_CALLBACK_2(Level2::onTouchMoved, this);		//�[�JĲ�I���ʨƥ�
	_listener1->onTouchEnded = CC_CALLBACK_2(Level2::onTouchEnded, this);		//�[�JĲ�I���}�ƥ�

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//�[�J��Ыت��ƥ��ť��
	this->schedule(CC_SCHEDULE_SELECTOR(Level2::doStep));

    return true;
}

void  Level2::setbtn() {
	auto btnSprite = _csbRoot->getChildByName("redbtn");
	_redBtn = CButton::create();
	_redBtn->setButtonInfo("playBtn01_01.png", "playBtn01_02.png", btnSprite->getPosition());
	_redBtn->setScale(btnSprite->getScale());
	this->addChild(_redBtn, 5);
	btnSprite->setVisible(false);

	btnSprite = _csbRoot->getChildByName("greenbtn");
	_greenBtn = CButton::create();
	_greenBtn->setButtonInfo("playBtn02_01.png", "playBtn02_02.png", btnSprite->getPosition());
	_greenBtn->setScale(btnSprite->getScale());
	this->addChild(_greenBtn, 5);
	btnSprite->setVisible(false);

	btnSprite = _csbRoot->getChildByName("bluebtn");
	_blueBtn = CButton::create();
	_blueBtn->setButtonInfo("playBtn03_01.png", "playBtn03_02.png", btnSprite->getPosition());
	_blueBtn->setScale(btnSprite->getScale());
	this->addChild(_blueBtn, 5);
	btnSprite->setVisible(false);

	btnSprite = _csbRoot->getChildByName("penbtn");
	_penBtn = CSwitchButton::create();
	_penBtn->setButtonInfo("playBtn00_01.png", "playBtn00_02.png", btnSprite->getPosition());
	_penBtn->setScale(btnSprite->getScale());
	this->addChild(_penBtn, 5);
	btnSprite->setVisible(false);
}

void  Level2::setUIbtn() {
	auto btnSprite = _csbRoot->getChildByName("homebtn");
	_homeBtn = CButton::create();
	_homeBtn->setButtonInfo("uiBtn01_01.png", "uiBtn01_02.png", btnSprite->getPosition());
	_homeBtn->setScale(btnSprite->getScale());
	this->addChild(_homeBtn, 5);
	btnSprite->setVisible(false);


	btnSprite = _csbRoot->getChildByName("replaybtn");
	_replayBtn = CButton::create();
	_replayBtn->setButtonInfo("uiBtn02_01.png", "uiBtn02_02.png", btnSprite->getPosition());
	_replayBtn->setScale(btnSprite->getScale());
	this->addChild(_replayBtn, 5);
	btnSprite->setVisible(false);
}


void Level2::createStaticBoundary()
{
	// ������ Body, �]�w�������Ѽ�

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // �]�w�o�� Body �� �R�A��
	bodyDef.userData = NULL;
	// �b b2World �����͸� Body, �öǦ^���ͪ� b2Body ���󪺫���
	// ���ͤ@���A�N�i�H���᭱�Ҧ��� Shape �ϥ�
	b2Body *body = _b2World->CreateBody(&bodyDef);
	_bottomBody = body;
	// �����R�A��ɩһݭn�� EdgeShape
	b2EdgeShape edgeShape;
	b2FixtureDef edgeFixtureDef; // ���� Fixture
	edgeFixtureDef.shape = &edgeShape;
	// bottom edge
	/*edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, 0.0f / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);
	*/
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

void Level2::setStaticWall() {
	char tmp[20] = "";

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.userData = NULL;
	b2Body *body = _b2World->CreateBody(&bodyDef);

	b2PolygonShape polyshape;
	b2FixtureDef fixtureDef; // ���� Fixture
	fixtureDef.shape = &polyshape;

	for (int i = 0; i < 2; i++)
	{
		sprintf(tmp, "wall_%02d", i);
		auto wallSprite = (Sprite *)_csbRoot->getChildByName(tmp);
		Size ts = wallSprite->getContentSize();
		Point loc = wallSprite->getPosition();
		float angle = wallSprite->getRotation();
		float scaleX = wallSprite->getScaleX();
		float scaleY = wallSprite->getScaleY();

		Point lep[4], wep[4];
		lep[0].x = (ts.width - 4) / 2.0f;;  lep[0].y = (ts.height - 4) / 2.0f;
		lep[1].x = -(ts.width - 4) / 2.0f;; lep[1].y = (ts.height - 4) / 2.0f;
		lep[2].x = -(ts.width - 4) / 2.0f;; lep[2].y = -(ts.height - 4) / 2.0f;
		lep[3].x = (ts.width - 4) / 2.0f;;  lep[3].y = -(ts.height - 4) / 2.0f;

		cocos2d::Mat4 modelMatrix, rotMatrix;
		modelMatrix.m[0] = scaleX;  // ���]�w X �b���Y��
		modelMatrix.m[5] = scaleY;  // ���]�w Y �b���Y��
		cocos2d::Mat4::createRotationZ(angle*M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = loc.x; //�]�w Translation�A�ۤv���[�W���˪�
		modelMatrix.m[7] = loc.y; //�]�w Translation�A�ۤv���[�W���˪�
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

		polyshape.Set(vecs, 4);
		body->CreateFixture(&fixtureDef);
	}

	auto bornSprite = (Sprite *)_csbRoot->getChildByName("born");
	Point loc = bornSprite->getPosition();
	bornpt = loc;

}

//void Level2::setBoards() {
//	char tmp[20] = "";
//	Sprite *gearSprite[2];
//	Point loc[2];
//	Size  size[2];
//	float sx,sy ;
//	b2Body* staticBody[2];
//	b2Body* dynamicBody[2];
//
//	b2BodyDef staticBodyDef;
//	staticBodyDef.type = b2_staticBody;
//	staticBodyDef.userData = NULL;
//
//	b2CircleShape staticShape;
//	staticShape.m_radius = 5 / PTM_RATIO;
//	b2FixtureDef fixtureDef;
//	fixtureDef.shape = &staticShape;
//
//	for (int i = 0; i < 2; i++)
//	{
//		if (i<1)sprintf(tmp, "redblock2_%02d", i);
//		else sprintf(tmp, "greenblock2_%02d", i-1);
//
//		gearSprite[i] = (Sprite *)_csbRoot->getChildByName(tmp);
//		loc[i] = gearSprite[i]->getPosition();
//		size[i] = gearSprite[i]->getContentSize();
//		sx = gearSprite[i]->getScaleX();
//		sy = gearSprite[i]->getScaleY();
//
//		if(i==0)staticBodyDef.position.Set((loc[i].x - 65) / PTM_RATIO, loc[i].y / PTM_RATIO);
//		else staticBodyDef.position.Set((loc[i].x + 65) / PTM_RATIO, loc[i].y / PTM_RATIO);
//		staticBody[i] = _b2World->CreateBody(&staticBodyDef);
//		fixtureDef.filter.categoryBits = 1 << i+1;  // 0=1  1=2 
//		staticBody[i]->CreateFixture(&fixtureDef);
//	}
//
//	b2BodyDef dynamicBodyDef;
//	dynamicBodyDef.type = b2_dynamicBody;
//
//	b2PolygonShape polyShape;
//	fixtureDef.shape = &polyShape;
//	fixtureDef.density = 1000.0f;
//	fixtureDef.friction = 0.2f;
//	fixtureDef.restitution = 0.25f;
//
//	for (int i = 0; i < 2; i++)
//	{
//		polyShape.SetAsBox((size[i].width - 4) *0.5f *sx / PTM_RATIO, (size[i].height - 4) *0.5f *sy / PTM_RATIO);
//
//		dynamicBodyDef.userData = gearSprite[i];
//		gearSprite[i]->setColor(BlockColor[i]);
//		dynamicBodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
//		dynamicBody[i] = _b2World->CreateBody(&dynamicBodyDef);
//		fixtureDef.filter.categoryBits = 1 << i + 1;  // 0=1  1=2 
//		dynamicBody[i]->CreateFixture(&fixtureDef);
//	}
//
//	b2RevoluteJointDef RJoint;	// �������`
//	for (int i = 0; i < 2; i++)
//	{
//		b2RevoluteJoint*  RevJoint;
//		RJoint.Initialize(staticBody[i], dynamicBody[i], staticBody[i]->GetWorldCenter());
//		RevJoint = (b2RevoluteJoint*)_b2World->CreateJoint(&RJoint);
//	}
//}

void Level2::setPendulum() {
	//��
	auto *basic = _csbRoot->getChildByName("Pendulum_basic_00");
	Point loc = basic->getPosition();
	Size size = basic->getContentSize();
	float scale = basic->getScale();
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	staticBodyDef.userData = basic;
	b2Body* basicBody = _b2World->CreateBody(&staticBodyDef);

	b2PolygonShape boxShape;
	boxShape.SetAsBox(size.width*0.5f*scale / PTM_RATIO, size.height*0.5f*scale / PTM_RATIO);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &boxShape;
	basicBody->CreateFixture(&fixtureDef);

	// ���\
	auto circleSprite = _csbRoot->getChildByName("Pendulum_cir_00");
	loc = circleSprite->getPosition();
	size = circleSprite->getContentSize();
	b2CircleShape circleShape;
	circleShape.m_radius = size.width*0.5f / PTM_RATIO;

	b2BodyDef circleBodyDef;
	circleBodyDef.type = b2_dynamicBody;
	circleBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	circleBodyDef.userData = circleSprite;
	b2Body* circleBody = _b2World->CreateBody(&circleBodyDef);
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 100000;
	circleBody->CreateFixture(&fixtureDef);

	//���ͶZ�����`
	b2DistanceJointDef JointDef;
	JointDef.Initialize(basicBody, circleBody, basicBody->GetPosition(), circleBody->GetPosition());
	_b2World->CreateJoint(&JointDef);

	//���o�ó]�w circle01_weld ���i�ʺA����j
	auto ropeSprite = _csbRoot->getChildByName("Pendulum_rope_00");
	loc = ropeSprite->getPosition();
	size = ropeSprite->getContentSize();
	float scaleX = ropeSprite->getScaleX();
	float scaleY = ropeSprite->getScaleY();

	b2BodyDef rpoeBodyDef;
	rpoeBodyDef.type = b2_dynamicBody;
	rpoeBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	rpoeBodyDef.userData = ropeSprite;
	b2Body* ropeBody = _b2World->CreateBody(&rpoeBodyDef);
	b2PolygonShape ropeShape;
	ropeShape.SetAsBox(size.width*0.5f*scaleX / PTM_RATIO, size.height*0.5f*scaleY / PTM_RATIO);
	fixtureDef.shape = &ropeShape;
	fixtureDef.density = 1.0f;  fixtureDef.friction = 0.25f; fixtureDef.restitution = 0.25f;
	ropeBody->CreateFixture(&fixtureDef);

	//b2WeldJointDef ropeJointDef;
	//ropeJointDef.Initialize(circleBody, ropeBody, circleBody->GetPosition());
	//_b2World->CreateJoint(&ropeJointDef); // �ϥιw�]�Ȳk��

	b2RevoluteJointDef revolutrJointDef;
	revolutrJointDef.Initialize(basicBody, ropeBody, basicBody->GetPosition());
	_b2World->CreateJoint(&revolutrJointDef);

	revolutrJointDef.Initialize(circleBody, ropeBody, circleBody->GetPosition());
	_b2World->CreateJoint(&revolutrJointDef);

}

void Level2::setFinalBox() {
	//���I�c�l
	char tmp[20] = "";
	Sprite *boxSprite[12];
	Point loc[12];
	Size  size[12];
	b2Body* boxBody[12];
	b2FixtureDef fixtureDef;
	b2BodyDef boxBodyDef;
	boxBodyDef.type = b2_staticBody;

	b2PolygonShape polyShape;
	fixtureDef.friction = 0.2f;
	fixtureDef.restitution = 0.0f;
	fixtureDef.shape = &polyShape;
	fixtureDef.density = 1.0f;
	for (int i = 0; i < 9; i++)
	{
		sprintf(tmp, "finbox_%02d", i);
		boxSprite[i] = (Sprite *)_csbRoot->getChildByName(tmp);
		loc[i] = boxSprite[i]->getPosition();
		size[i] = boxSprite[i]->getContentSize();
		float sx = boxSprite[i]->getScaleX();
		float sy = boxSprite[i]->getScaleY();
		polyShape.SetAsBox((size[i].width - 4) *0.5f *sx / PTM_RATIO, (size[i].height - 4) *0.5f *sy / PTM_RATIO);

		boxBodyDef.userData = boxSprite[i];
		boxSprite[i]->setColor(BlockColor[(i / 3)]);	// �ϥ� filterColor �w�g�إߪ��C��
		boxBodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
		boxBody[i] = _b2World->CreateBody(&boxBodyDef);
		fixtureDef.filter.categoryBits = 1 << (i / 3 + 1);   // 0.1.2 =1  3.4.2 =2  6.7.8 =3
		boxBody[i]->CreateFixture(&fixtureDef);
	}

	//���I�P����
	fixtureDef.friction = 0.2f;
	fixtureDef.restitution = 0.0f;
	fixtureDef.shape = &polyShape;
	for (int i = 9; i < 12; i++)
	{
		sprintf(tmp, "boxSensor_%02d", i-8);
		boxSprite[i] = (Sprite *)_csbRoot->getChildByName(tmp);
		loc[i] = boxSprite[i]->getPosition();
		size[i] = boxSprite[i]->getContentSize();
		float sx = boxSprite[i]->getScaleX();
		float sy = boxSprite[i]->getScaleY();
		polyShape.SetAsBox((size[i].width - 4) *0.5f *sx / PTM_RATIO, (size[i].height - 4) *0.5f *sy / PTM_RATIO);

		boxBodyDef.userData = boxSprite[i];
		boxSprite[i]->setColor(BlockColor2[(i % 3)]);	// �ϥ� filterColor �w�g�إߪ��C��
		boxBodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
		boxBody[i] = _b2World->CreateBody(&boxBodyDef);
		fixtureDef.filter.categoryBits = 1 << ((i % 3 + 1));   // 0.1.2 =1  3.4.2 =2  6.7.8 =3
		fixtureDef.density = 500.0f + 100*(i-9);  //500 600 700
		fixtureDef.isSensor = true;
		boxBody[i]->CreateFixture(&fixtureDef);
	}
	
}


void Level2::doStep(float dt)
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

	if (_bboxR && _bboxG && _bboxB) {
		CCLOG("LEVEL UP!");
	}
	if (!_bboxR) {
		if (_colliderSeneor.inBoxR == true) _bboxR = true;
	}
	if (!_bboxG) {
		if (_colliderSeneor.inBoxG == true) _bboxG = true;
	}
	if (!_bboxB) {
		if (_colliderSeneor.inBoxB == true) _bboxB = true;
	}
}

bool Level2::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent)//Ĳ�I�}�l�ƥ�
{
	Point touchLoc = pTouch->getLocation();

	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		if (body->GetUserData() != NULL) {// �R�A���餣�B�z	
			//�P�_�I����m�O�_���b�ʺA����@�w���d��
			Sprite *spriteObj = (Sprite*)body->GetUserData();
			Size objSize = spriteObj->getContentSize();
			Point pt = spriteObj->getPosition();
			float scaleX = spriteObj->getScaleX();
			float scaleY = spriteObj->getScaleY();
			/*objSize.width *= scaleX;   objSize.height *= scaleY;
			Rect collider(pt.x- objSize.width/2 , pt.y - objSize.height/2, objSize.width, objSize.height);*/
			float fdist = MAX_2(objSize.width*scaleX, objSize.height*scaleY) / 2.0f;
			float x = body->GetPosition().x*PTM_RATIO - touchLoc.x;
			float y = body->GetPosition().y*PTM_RATIO - touchLoc.y;
			float tpdist = x*x + y*y;
			if (tpdist < fdist*fdist)
			
			{
				_bTouchOn = true;
				b2MouseJointDef mouseJointDef;
				mouseJointDef.bodyA = _bottomBody;
				mouseJointDef.bodyB = body;
				mouseJointDef.target = b2Vec2(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO);
				mouseJointDef.collideConnected = true;
				mouseJointDef.maxForce = 1000.0f * body->GetMass();
				_MouseJoint = (b2MouseJoint*)_b2World->CreateJoint(&mouseJointDef); // �s�W Mouse Joint
				body->SetAwake(true);
				_bMouseOn = true;
				break;
			}
		}
	}

	_redBtn->touchesBegin(touchLoc);
	_greenBtn->touchesBegin(touchLoc);
	_blueBtn->touchesBegin(touchLoc);
	_homeBtn->touchesBegin(touchLoc);
	_replayBtn->touchesBegin(touchLoc);
	return true;
}

void  Level2::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //Ĳ�I���ʨƥ�
{
	Point touchLoc = pTouch->getLocation();
	if (_bTouchOn)
	{
		_MouseJoint->SetTarget(b2Vec2(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO));
	}

	_redBtn->touchesBegin(touchLoc);
	_greenBtn->touchesBegin(touchLoc);
	_blueBtn->touchesBegin(touchLoc);
	_homeBtn->touchesBegin(touchLoc);
	_replayBtn->touchesBegin(touchLoc);
}

void  Level2::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //Ĳ�I�����ƥ� 
{
	Point touchLoc = pTouch->getLocation();
	if (_bTouchOn)
	{
		_bTouchOn = false;
		if (_MouseJoint != NULL)
		{
			_b2World->DestroyJoint(_MouseJoint);
			_MouseJoint = NULL;
			_bMouseOn = false;

		}
	}

	if (_redBtn->touchesEnded(touchLoc))   renderball("ball_01.png", 1);
	if (_greenBtn->touchesEnded(touchLoc)) renderball("ball_02.png", 2);
	if (_blueBtn->touchesEnded(touchLoc))  renderball("ball_03.png", 3);
	if (_homeBtn->touchesEnded(touchLoc)) {
		this->unschedule(schedule_selector(Level2::doStep));
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("UIBTN.plist");
		TransitionFade *pageTurn;
		pageTurn = TransitionFade::create(1.0F, StartScene::createScene());
		Director::getInstance()->replaceScene(pageTurn);
	}
	if (_replayBtn->touchesEnded(touchLoc)) {
		this->unschedule(schedule_selector(Level2::doStep));
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("UIBTN.plist");
		TransitionFade *pageTurn;
		pageTurn = TransitionFade::create(1.0F, Level2::createScene());
		Director::getInstance()->replaceScene(pageTurn);
	}
}
void Level2::renderball(char *name, int mask) {
	auto ballSprite = Sprite::createWithSpriteFrameName(name);
	this->addChild(ballSprite, 2);

	// �إߤ@��²�檺�ʺA�y��
	b2BodyDef bodyDef;	// ���H���c b2BodyDef �ŧi�@�� Body ���ܼ�
	bodyDef.type = b2_dynamicBody; // �]�w���ʺA����
	bodyDef.userData = ballSprite;	// �]�w Sprite ���ʺA���骺��ܹϥ�
	bodyDef.position.Set(bornpt.x / PTM_RATIO, bornpt.y / PTM_RATIO);
	// �H bodyDef �b b2World  ���إ߹���öǦ^�ӹ��骺����
	b2Body *ballBody = _b2World->CreateBody(&bodyDef);

	// �]�w�Ӫ��骺�~��
	b2CircleShape ballShape;	//  �ŧi���骺�~�������ܼơA���B�O��Ϊ���
	Size ballsize = ballSprite->getContentSize();	// �ھ� Sprite �ϧΪ��j�p�ӳ]�w��Ϊ��b�|
	ballShape.m_radius = 0.5f*(ballsize.width - 4) / PTM_RATIO;
	// �H b2FixtureDef  ���c�ŧi���鵲�c�ܼơA�ó]�w���骺�������z�Y��
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &ballShape;			// ���w���骺�~�������
	fixtureDef.restitution = 0.15f;			// �]�w�u�ʫY��
	fixtureDef.density = 1.0f;				// �]�w�K��
	fixtureDef.friction = 0.15f;			// �]�w�����Y��
	fixtureDef.filter.maskBits = 1 << mask | 1; //�]�w�s��
	ballBody->CreateFixture(&fixtureDef);	// �b Body �W���ͳo�ӭ��骺�]�w
}


#ifdef BOX2D_DEBUG
//��gø�s��k
void Level2::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif
