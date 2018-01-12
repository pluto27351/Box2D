#include "Level0Scene.h"
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

Level0::~Level0()
{

#ifdef BOX2D_DEBUG
	if (_DebugDraw != NULL) delete _DebugDraw;
#endif

	if (_b2World != nullptr) delete _b2World;
//  for releasing Plist&Texture
	//SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
	Director::getInstance()->getTextureCache()->removeUnusedTextures();

}

Scene* Level0::createScene()
{
    auto scene = Scene::create();
    auto layer = Level0::create();
    scene->addChild(layer);
    return scene;
}

// on "init" you need to initialize your instance
bool Level0::init()
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
	_csbRoot = CSLoader::createNode("Level0.csb");
	

#ifdef BOX2D_DEBUG
	// �]�w��ܭI���ϥ�
	auto bgSprite = _csbRoot->getChildByName("bg64_2");
	bgSprite->setVisible(false);

#endif

	addChild(_csbRoot, 1);

	createStaticBoundary();
	setStaticWall();
	//setBoards();
	//setPendulum();
	//setFinalBox();
	//setCar();
	setRope();
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
	_listener1->onTouchBegan = CC_CALLBACK_2(Level0::onTouchBegan, this);		//�[�JĲ�I�}�l�ƥ�
	_listener1->onTouchMoved = CC_CALLBACK_2(Level0::onTouchMoved, this);		//�[�JĲ�I���ʨƥ�
	_listener1->onTouchEnded = CC_CALLBACK_2(Level0::onTouchEnded, this);		//�[�JĲ�I���}�ƥ�

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//�[�J��Ыت��ƥ��ť��
	this->schedule(CC_SCHEDULE_SELECTOR(Level0::doStep));

    return true;
}

void  Level0::setbtn() {
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

void  Level0::setUIbtn() {
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


void Level0::createStaticBoundary()
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

	//// right edge
	//edgeShape.Set(b2Vec2(_visibleSize.width / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	//body->CreateFixture(&edgeFixtureDef);

	// top edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, _visibleSize.height / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);
}

void Level0::setStaticWall() {
	char tmp[20] = "";

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.userData = NULL;
	b2Body *body = _b2World->CreateBody(&bodyDef);

	b2PolygonShape polyshape;
	b2FixtureDef fixtureDef; // ���� Fixture
	fixtureDef.shape = &polyshape;

	for (int i = 0; i < 3; i++)
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
		if (i == 1)fixtureDef.density = 900;
		else fixtureDef.density = 10;
		body->CreateFixture(&fixtureDef);
	}

	auto bornSprite = (Sprite *)_csbRoot->getChildByName("born");
	Point pt = bornSprite->getPosition();
	bornpt = pt;

	/*auto sensorSprite = (Sprite *)_csbRoot->getChildByName("sensor_00");
	Size ts = sensorSprite->getContentSize();
	Point loc = sensorSprite->getPosition();
	float scaleX = sensorSprite->getScaleX();
	float scaleY = sensorSprite->getScaleY();
	b2Body *sensorbody;
	b2BodyDef sensorDef;
	sensorDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	sensorDef.type = b2_staticBody;
	sensorDef.userData = NULL;
	b2PolygonShape sensorShape;
	sensorShape.SetAsBox( (ts.width - 4) *0.5f *scaleX / PTM_RATIO, (ts.height - 4) *0.5f *scaleY / PTM_RATIO);
	sensorbody = _b2World->CreateBody(&sensorDef);
	b2FixtureDef sensorfix;
	sensorfix.shape = &sensorShape;
	sensorfix.density = 800.0f;
	sensorfix.isSensor = true;
	sensorbody->CreateFixture(&sensorfix);*/
	
}

void Level0::setBoards() {
	char tmp[20] = "";
	Sprite *gearSprite[2];
	Point loc[2];
	Size  size[2];
	float sx[2],sy[2] ;
	b2Body* staticBody[2];
	b2Body* dynamicBody[2];

	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.userData = NULL;

	b2CircleShape staticShape;
	staticShape.m_radius = 5 / PTM_RATIO;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &staticShape;

	for (int i = 0; i < 2; i++)
	{
		if(i==0)sprintf(tmp, "greenblock_%02d", i);
		else sprintf(tmp, "blueblock_%02d", i-1);

		gearSprite[i] = (Sprite *)_csbRoot->getChildByName(tmp);
		loc[i] = gearSprite[i]->getPosition();
		size[i] = gearSprite[i]->getContentSize();
		sx[i] = gearSprite[i]->getScaleX();
		sy[i] = gearSprite[i]->getScaleY();

		staticBodyDef.position.Set((loc[i].x) / PTM_RATIO, loc[i].y / PTM_RATIO);
		staticBody[i] = _b2World->CreateBody(&staticBodyDef);
		fixtureDef.filter.categoryBits = 1 << i+2;  // 0=2  1=3 blue
		staticBody[i]->CreateFixture(&fixtureDef);
	}

	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;

	b2PolygonShape polyShape;
	fixtureDef.shape = &polyShape;
	fixtureDef.density = 1000.0f;
	fixtureDef.friction = 0.2f;
	fixtureDef.restitution = 0.25f;

	for (int i = 0; i < 2; i++)
	{
		polyShape.SetAsBox((size[i].width - 4) *0.5f *sx[i] / PTM_RATIO, (size[i].height - 4) *0.5f *sy[i] / PTM_RATIO);

		dynamicBodyDef.userData = gearSprite[i];
		gearSprite[i]->setColor(BlockColor[i+1]);  //0=1  1=2
		dynamicBodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
		dynamicBody[i] = _b2World->CreateBody(&dynamicBodyDef);
		fixtureDef.filter.categoryBits = 1 << i+2;  //0=2  1=3
		dynamicBody[i]->CreateFixture(&fixtureDef);
	}

	b2RevoluteJointDef RJoint;	// �������`
	for (int i = 0; i < 2; i++)
	{
		b2RevoluteJoint*  RevJoint;
		RJoint.Initialize(staticBody[i], dynamicBody[i], dynamicBody[i]->GetWorldCenter());
		RevJoint = (b2RevoluteJoint*)_b2World->CreateJoint(&RJoint);
	}
}

void Level0::setPendulum() {
	//��
	auto *basic = _csbRoot->getChildByName("Pendulum_basic_00");
	basic->setVisible(true);
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
	circleSprite->setVisible(true);
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
	ropeSprite->setVisible(true);
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

void Level0::setFinalBox() {
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

void Level0::setCar() {
	//���l
	b2Body *wheelbody[2];
	char tmp[20] = "";

	for (int i = 0; i < 2; i++) {
		b2BodyDef wheelDef;
		wheelDef.type = b2_dynamicBody;
		sprintf(tmp, "wheel_%02d", i);
		auto carSprite = _csbRoot->getChildByName(tmp);
		Point locTail = carSprite->getPosition();
		Size sizeTail = carSprite->getContentSize();
		float scale = carSprite->getScale();
		wheelDef.position.Set(locTail.x / PTM_RATIO, locTail.y / PTM_RATIO);
		wheelDef.userData = carSprite;

		b2CircleShape circleShape;
		circleShape.m_radius = (sizeTail.width*scale - 4)*0.5f / PTM_RATIO;
		b2FixtureDef wheelfix;
		wheelfix.shape = &circleShape;
		wheelfix.density = 1.0f;
		wheelfix.friction = 0.2f;
		wheelfix.restitution = 0.25f;

		wheelbody[i] = _b2World->CreateBody(&wheelDef);
		wheelbody[i]->CreateFixture(&wheelfix);
	}

	// ����
	auto carSprite = _csbRoot->getChildByName("carengine");
	Point locTail = carSprite->getPosition();
	Size sizeTail = carSprite->getContentSize();
	float scale = carSprite->getScale();

	b2Body* staticBody;
	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.userData = NULL;
	b2CircleShape staticShape;
	staticShape.m_radius = 5 / PTM_RATIO;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &staticShape;
	staticBodyDef.position.Set(locTail.x / PTM_RATIO, locTail.y / PTM_RATIO);
	staticBody = _b2World->CreateBody(&staticBodyDef);
	staticBody->CreateFixture(&fixtureDef);

	b2Body *enginebody;
	b2FixtureDef enginefix;
	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(locTail.x / PTM_RATIO, locTail.y / PTM_RATIO);
	bodyDef.userData = carSprite;
	b2CircleShape circleShape;
	circleShape.m_radius = (sizeTail.width*scale - 4)*0.5f / PTM_RATIO;
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.2f;
	fixtureDef.restitution = 0.25f;
	enginebody = _b2World->CreateBody(&bodyDef);
	enginebody->CreateFixture(&fixtureDef);
	
	//���l
	carSprite = _csbRoot->getChildByName("car");
	Point loc = carSprite->getPosition();
	scale = carSprite->getScale()*-1;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	bodyDef.userData = carSprite;
	b2Body *carbody = _b2World->CreateBody(&bodyDef);
	Size frameSize = carSprite->getContentSize();
	b2PolygonShape rectShape;
	rectShape.SetAsBox((frameSize.width - 5)*0.5f*scale / PTM_RATIO, (frameSize.height - 5)*0.5f*scale / PTM_RATIO);
	fixtureDef.shape = &rectShape;
	fixtureDef.restitution = 0.1f;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.1f;
	carbody->CreateFixture(&fixtureDef);

	//�ᨮ�c
	b2Body *boxbody[3];
	for (int i = 0; i < 3; i++) {
		sprintf(tmp, "linkbox_%02d", i);
		auto boxSprite = _csbRoot->getChildByName(tmp);
		loc = boxSprite->getPosition();
		float sx = boxSprite->getScaleX();
		float sy = boxSprite->getScaleY();
		frameSize = boxSprite->getContentSize();
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
		bodyDef.userData = boxSprite;
		rectShape.SetAsBox((frameSize.width - 4) *0.5f *sx / PTM_RATIO, (frameSize.height - 4) *0.5f *sy / PTM_RATIO);
		fixtureDef.shape = &rectShape;
		fixtureDef.restitution = 0.1f;
		fixtureDef.density = 1.0f;
		fixtureDef.friction = 0.1f;
		boxbody[i] = _b2World->CreateBody(&bodyDef);
		boxbody[i]->CreateFixture(&fixtureDef);
	}

	//�s��
	b2RevoluteJoint *RevJoint[3];// �������` Gear��
	b2RevoluteJointDef RJoint;

	//�s�� - ����P����
	RJoint.Initialize(carbody, wheelbody[0], wheelbody[0]->GetWorldCenter());
	RevJoint[0] = (b2RevoluteJoint*)_b2World->CreateJoint(&RJoint);
	RJoint.Initialize(carbody, wheelbody[1], wheelbody[1]->GetWorldCenter());
	RevJoint[1] = (b2RevoluteJoint*)_b2World->CreateJoint(&RJoint);
	RJoint.Initialize(staticBody, enginebody, enginebody->GetWorldCenter());
	RevJoint[2] = (b2RevoluteJoint*)_b2World->CreateJoint(&RJoint);

	//�s�� - ���D�P���[
	RJoint.Initialize(carbody, boxbody[0], boxbody[0]->GetWorldCenter()+b2Vec2(-2 / PTM_RATIO,0));
	_b2World->CreateJoint(&RJoint);
	RJoint.Initialize(boxbody[0], boxbody[1], boxbody[1]->GetWorldCenter());
	_b2World->CreateJoint(&RJoint);

	//�k�����`
	b2WeldJointDef JointDef;
	JointDef.Initialize(boxbody[1], boxbody[2], boxbody[1]->GetPosition() + b2Vec2(45 / PTM_RATIO, 0));
	_b2World->CreateJoint(&JointDef); // �ϥιw�]�Ȳk��


	//�s�� - �����P����
	b2GearJointDef GJoint;
	GJoint.bodyA = wheelbody[1];
	GJoint.bodyB = enginebody;
	GJoint.joint1 = RevJoint[1];
	GJoint.joint2 = RevJoint[2];
	GJoint.ratio = -2;
	_b2World->CreateJoint(&GJoint);

}

void Level0::setRope() {
	//��
	char tmp[20] = "";
	b2Body *ropecir[3],*ropecirstatic[3];
	Point locHead[3];
	Size sizeHead[3];
	float scale[3];
	for (int i = 0; i < 3; i++) {
		sprintf(tmp, "ropecir_%02d", i);
		auto cirSprite = _csbRoot->getChildByName(tmp);
		locHead[i] = cirSprite->getPosition();
		sizeHead[i] = cirSprite->getContentSize();
		scale[i] = cirSprite->getScale();

		b2BodyDef bodyDef;
		b2CircleShape staticShape;
		b2FixtureDef fixtureDef;
		bodyDef.type = b2_staticBody;
		bodyDef.userData = NULL;
		staticShape.m_radius = (5 / PTM_RATIO);
		fixtureDef.shape = &staticShape;
		bodyDef.position.Set(locHead[i].x / PTM_RATIO, locHead[i].y / PTM_RATIO);
		ropecirstatic[i] = _b2World->CreateBody(&bodyDef);
		ropecirstatic[i]->CreateFixture(&fixtureDef);

		b2CircleShape circleShape;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position.Set(locHead[i].x / PTM_RATIO, locHead[i].y / PTM_RATIO);
		bodyDef.userData = cirSprite;
		ropecir[i] = _b2World->CreateBody(&bodyDef);
		fixtureDef.density = 0.3f;  fixtureDef.friction = 50.0f; fixtureDef.restitution = 0.25f;
		circleShape.m_radius = (sizeHead[i].width - 4)*0.5f*scale[i] / PTM_RATIO;
		fixtureDef.shape = &circleShape;
		ropecir[i]->CreateFixture(&fixtureDef);
	}
	
	b2RevoluteJoint *RevJoint[3];
	b2RevoluteJointDef RJoint;	// �������`
	for (int i = 0; i < 3; i++) {
		RJoint.Initialize(ropecirstatic[i], ropecir[i], ropecir[i]->GetWorldCenter());
		RevJoint[i] = (b2RevoluteJoint*)_b2World->CreateJoint(&RJoint);
	}

	b2GearJointDef GJoint;
	GJoint.bodyA = ropecir[2];
	GJoint.bodyB = ropecir[0];
	GJoint.joint1 = RevJoint[2];
	GJoint.joint2 = RevJoint[0];
	GJoint.ratio = -3;
	_b2World->CreateJoint(&GJoint);

	//GJoint.bodyA = ropecir[2];
	//GJoint.bodyB = ropecir[1];
	//GJoint.joint1 = RevJoint[2];
	//GJoint.joint2 = RevJoint[1];
	//GJoint.ratio = -3;
	//_b2World->CreateJoint(&GJoint);

	//÷�l����
	Sprite *ropeSprite[74];  //37*2  24+13 = 37
	Point loc[74];
	Size  size[75];
	b2Body* ropeBody[74];

	b2BodyDef  bodyDef;
	b2FixtureDef fixtureDef;
	b2PolygonShape boxShape;
	bodyDef.type = b2_dynamicBody;
	fixtureDef.density = 0.0f;  fixtureDef.friction = 1.0f; fixtureDef.restitution = 0.0f;
	fixtureDef.shape = &boxShape;
	// ���ͤ@�t�C��÷�l�q�� rope03_7_00 ~ rope03_7_23�A�P�ɱ��_��
	for (int i = 0; i < 74; i++)
	{
		sprintf(tmp, "rope03_7_%02d", i);
		ropeSprite[i] = (Sprite *)_csbRoot->getChildByName(tmp);
		loc[i] = ropeSprite[i]->getPosition();
		size[i] = ropeSprite[i]->getContentSize();

		bodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
		bodyDef.userData = ropeSprite[i];
		ropeBody[i] = _b2World->CreateBody(&bodyDef);
		boxShape.SetAsBox((size[i].width - 4)*0.5f / PTM_RATIO, (size[i].height - 4)*0.5f / PTM_RATIO);
		ropeBody[i]->CreateFixture(&fixtureDef);
	}

	float locAnchor = 0.5f*(size[0].width) / PTM_RATIO;
	b2RevoluteJointDef revJoint;
	//revJoint.bodyA = ropebox;
	//revJoint.localAnchorA.Set(0, 0);
	//revJoint.bodyB = ropeBody[0];
	//revJoint.localAnchorB.Set(0,locAnchor);
//	_b2World->CreateJoint(&revJoint);
	for (int i = 0; i < 73; i++) {
		revJoint.bodyA = ropeBody[i];
		revJoint.localAnchorA.Set(-locAnchor, 0);
		revJoint.bodyB = ropeBody[i + 1];
		revJoint.localAnchorB.Set(locAnchor,0);
		_b2World->CreateJoint(&revJoint);
	}
	revJoint.bodyA = ropeBody[73];
	revJoint.localAnchorA.Set(-locAnchor,0);
	revJoint.bodyB = ropeBody[0];
	revJoint.localAnchorB.Set(locAnchor,0);
	_b2World->CreateJoint(&revJoint);

	
}


void Level0::doStep(float dt)
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

bool Level0::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent)//Ĳ�I�}�l�ƥ�
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

	if (drawOn) {
		_bDraw = true;
		_HDrawPt = new DrawPoint;
		_HDrawPt->pt = touchLoc;
		_HDrawPt->next = NULL;
		_NDrawPt = _HDrawPt;
	}

	_redBtn->touchesBegin(touchLoc);
	_greenBtn->touchesBegin(touchLoc);
	_blueBtn->touchesBegin(touchLoc);
	_penBtn->touchesBegan(touchLoc);
	_homeBtn->touchesBegin(touchLoc);
	_replayBtn->touchesBegin(touchLoc);
	return true;
}

void  Level0::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //Ĳ�I���ʨƥ�
{
	Point touchLoc = pTouch->getLocation();
	if (_bTouchOn)
	{
		_MouseJoint->SetTarget(b2Vec2(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO));
	}

	if (_bDraw) {
		float len = ccpDistance(_NDrawPt->pt, touchLoc);
		if (len > DRAW_MIN) {
			struct DrawPoint *newPt;
			newPt = new DrawPoint;
			newPt->pt = touchLoc;
			float x = newPt->pt.x - _NDrawPt->pt.x;
			float y = newPt->pt.y - _NDrawPt->pt.y;
			newPt->r = atan2f(y, x);
			newPt->texture = Sprite::createWithSpriteFrameName("square06.png");
			newPt->texture->setPosition(touchLoc);
			newPt->texture->setScale(0.3f,0.07f);
			newPt->texture->setColor(BlockColor[pencolor]);
			newPt->texture->setRotation(-(newPt->r * 180 / M_PI));
			this->addChild(newPt->texture, 2);
			newPt->next = NULL;
			_NDrawPt->next = newPt;
			_NDrawPt = newPt;
		}
	}

	_redBtn->touchesBegin(touchLoc);
	_greenBtn->touchesBegin(touchLoc);
	_blueBtn->touchesBegin(touchLoc);
	_penBtn->touchesMoved(touchLoc);
	_homeBtn->touchesBegin(touchLoc);
	_replayBtn->touchesBegin(touchLoc);
}

void  Level0::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //Ĳ�I�����ƥ� 
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

	if (_bDraw) {
		drawLine();
		_bDraw = false;
	}

	if (_redBtn->touchesEnded(touchLoc)) {
		if (!drawOn) renderball("ball_01.png", 1);
		else pencolor = 0;
	}
	if (_greenBtn->touchesEnded(touchLoc)) {
		if (!drawOn)renderball("ball_02.png", 2);
		else pencolor = 1;
	}
	if (_blueBtn->touchesEnded(touchLoc)) {
		if (!drawOn)  renderball("ball_03.png", 3);
		else pencolor = 2;
	}
	if (_penBtn->touchesEnded(touchLoc)) {
		drawOn = !drawOn;
		auto sprite = _csbRoot->getChildByName("pencolor");
		if (drawOn) sprite->setVisible(true);
		else        sprite->setVisible(false);
	}
	auto sprite = _csbRoot->getChildByName("pencolor");
	sprite->setColor(BlockColor[pencolor]);
	
	if (_homeBtn->touchesEnded(touchLoc)) {
		this->unschedule(schedule_selector(Level0::doStep));
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("UIBTN.plist");
		TransitionFade *pageTurn;
		pageTurn = TransitionFade::create(1.0F, StartScene::createScene());
		Director::getInstance()->replaceScene(pageTurn);
	}
	if (_replayBtn->touchesEnded(touchLoc)) {
		this->unschedule(schedule_selector(Level0::doStep));
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("UIBTN.plist");
		TransitionFade *pageTurn;
		pageTurn = TransitionFade::create(1.0F, Level0::createScene());
		Director::getInstance()->replaceScene(pageTurn);
	}

}
void Level0::renderball(char *name, int mask) {
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

void Level0::drawLine() {
	struct DrawPoint *DiePt;
	_NDrawPt = _HDrawPt;

	if (_HDrawPt->next) {  //�ܤ�2�I
		b2BodyDef drawDef;
		drawDef.type = b2_staticBody;
		//drawDef.userData = _HDrawPt->texture;
		drawDef.position.Set(0, 0);
		b2Body *drawBody = _b2World->CreateBody(&drawDef);

		while (_NDrawPt->next != NULL) {
			float x = _NDrawPt->pt.x - _NDrawPt->next->pt.x;
			float y = _NDrawPt->pt.y - _NDrawPt->next->pt.y;
			float r = atan2f(y, x);
			cocos2d::Point pt[2], pos[4];
			pt[0].x = _NDrawPt->pt.x;  pt[0].y = _NDrawPt->pt.y;
			pt[1].x = _NDrawPt->next->pt.x;  pt[1].y = _NDrawPt->next->pt.y;
			for (int i = 0; i < 2; i++) {
				pos[i].x = pt[i].x + (0 * cosf(r) + DRAW_MIN / 2 * sinf(r));
				pos[i].y = pt[i].y + (0 * sinf(r)*(-1) + DRAW_MIN / 2 * cosf(r));
				pos[i + 2].x = pt[i].x + (0 * cosf(r) - DRAW_MIN / 2 * sinf(r));
				pos[i + 2].y = pt[i].y + (0 * sinf(r)*(-1) - DRAW_MIN / 2 * cosf(r));

			}
			b2PolygonShape shape;
			b2FixtureDef drawFixture;
			drawFixture.shape = &shape;
			drawFixture.density = 2.0f;
			drawFixture.friction = 0;
			drawFixture.restitution = 0.3f;
			drawFixture.filter.categoryBits = 1 << pencolor+1;  // 123
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


#ifdef BOX2D_DEBUG
//��gø�s��k
void Level0::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif