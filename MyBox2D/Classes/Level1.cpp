#include "Level1.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

USING_NS_CC;

#define MAX_CIRCLE_OBJECTS  11
#define MAX_2(X,Y) (X)>(Y) ? (X) : (Y)

char ball[MAX_CIRCLE_OBJECTS][20] = {
	"clock01.png","clock02.png","clock03.png","clock04.png",
	"dount01.png","dount02.png","dount03.png","dount04.png",
	"orange01.png","orange02.png","orange03.png" };

using namespace cocostudio::timeline;
Color3B BlockColor[3] = { Color3B(208,45,45), Color3B(77,204,42), Color3B(14,201,220) };

Level1::~Level1()
{

#ifdef BOX2D_DEBUG
	if (_DebugDraw != NULL) delete _DebugDraw;
#endif

	if (_b2World != nullptr) delete _b2World;
//  for releasing Plist&Texture
	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
	Director::getInstance()->getTextureCache()->removeUnusedTextures();

}

Scene* Level1::createScene()
{
    auto scene = Scene::create();
    auto layer = Level1::create();
    scene->addChild(layer);
    return scene;
}

// on "init" you need to initialize your instance
bool Level1::init()
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
   

	// �إ� Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);	//���O��V
	bool AllowSleep = true;					//���\�ε�
	_b2World = new b2World(Gravity);		//�Ыإ@��
	_b2World->SetAllowSleeping(AllowSleep);	//�]�w���󤹳\�ε�

	// Ū�J CSB ��
	_csbRoot = CSLoader::createNode("Level1.csb");
	

#ifdef BOX2D_DEBUG
	// �]�w��ܭI���ϥ�
	auto bgSprite = _csbRoot->getChildByName("bg64_2");
	bgSprite->setVisible(true);

#endif
	addChild(_csbRoot, 1);
	// �b�ù����|����ɫإ� Static Body ��������
	createStaticBoundary();
	setStaticWalls();
	setbtn();
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
	_listener1->onTouchBegan = CC_CALLBACK_2(Level1::onTouchBegan, this);		//�[�JĲ�I�}�l�ƥ�
	_listener1->onTouchMoved = CC_CALLBACK_2(Level1::onTouchMoved, this);		//�[�JĲ�I���ʨƥ�
	_listener1->onTouchEnded = CC_CALLBACK_2(Level1::onTouchEnded, this);		//�[�JĲ�I���}�ƥ�

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//�[�J��Ыت��ƥ��ť��
	this->schedule(CC_SCHEDULE_SELECTOR(Level1::doStep));

    return true;
}
void  Level1::setbtn() {
	auto btnSprite = _csbRoot->getChildByName("redbtn");
	_redBtn = CButton::create();
	_redBtn->setButtonInfo("clock03.png", "clock01.png", btnSprite->getPosition());
	_redBtn->setScale(btnSprite->getScale());
	this->addChild(_redBtn, 5);
	btnSprite->setVisible(false);

	btnSprite = _csbRoot->getChildByName("greenbtn");
	_greenBtn = CButton::create();
	_greenBtn->setButtonInfo("clock02.png", "clock01.png", btnSprite->getPosition());
	_greenBtn->setScale(btnSprite->getScale());
	this->addChild(_greenBtn, 5);
	btnSprite->setVisible(false);

	btnSprite = _csbRoot->getChildByName("bluebtn");
	_blueBtn = CButton::create();
	_blueBtn->setButtonInfo("clock04.png", "clock01.png", btnSprite->getPosition());
	_blueBtn->setScale(btnSprite->getScale());
	this->addChild(_blueBtn, 5);
	btnSprite->setVisible(false);
}
void  Level1::setupMouseJoint()
{
	// ���o�ó]�w frame01 �e�عϥܬ��ʺA����
	auto frameSprite = _csbRoot->getChildByName("frame01");
	Point loc = frameSprite->getPosition();

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	bodyDef.userData = frameSprite;
	b2Body *body = _b2World->CreateBody(&bodyDef);

	// Define poly shape for our dynamic body.
	b2PolygonShape rectShape;
	Size frameSize = frameSprite->getContentSize();
	rectShape.SetAsBox((frameSize.width - 4)*0.5f / PTM_RATIO, (frameSize.height - 4)*0.5f / PTM_RATIO);
	// Define the dynamic body fixture.
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &rectShape;
	fixtureDef.restitution = 0.1f;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.1f;
	body->CreateFixture(&fixtureDef);
	_bTouchOn = false;
}

void Level1::setStaticWalls() {
	char tmp[20] = "";
	Sprite *gearSprite[6];
	Point loc[6];
	Size  size[6];
	b2Body* staticBody[6];
	b2Body* dynamicBody[6];

	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.userData = NULL;

	b2CircleShape staticShape;
	staticShape.m_radius = 5 / PTM_RATIO;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &staticShape;

	for (int i = 0; i < 6; i++)
	{
		if(i<2)sprintf(tmp, "redblock_%02d", i + 1);
		else if (i < 4) sprintf(tmp, "greenblock_%02d", i -1);
		else sprintf(tmp, "blueblock_%02d", i - 3);

		gearSprite[i] = (Sprite *)_csbRoot->getChildByName(tmp);
		loc[i] = gearSprite[i]->getPosition();
		size[i] = gearSprite[i]->getContentSize();


		staticBodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
		staticBody[i] = _b2World->CreateBody(&staticBodyDef);
		staticBody[i]->CreateFixture(&fixtureDef);
	}

	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;
	//dynamicBodyDef.type = b2_staticBody;

	b2PolygonShape polyShape;
	fixtureDef.shape = &polyShape;
	fixtureDef.density = 1000.0f;
	fixtureDef.friction = 0.2f;
	fixtureDef.restitution = 0.25f;

	for (int i = 0; i < 6; i++)
	{
		float sx = gearSprite[i]->getScaleX();
		float sy = gearSprite[i]->getScaleY();
		polyShape.SetAsBox((size[i].width - 4) *0.5f *sx / PTM_RATIO, (size[i].height - 4) *0.5f *sy / PTM_RATIO);

		dynamicBodyDef.userData = gearSprite[i];
		gearSprite[i]->setColor(BlockColor[(i/2)]);	// �ϥ� filterColor �w�g�إߪ��C��
		dynamicBodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
		dynamicBody[i] = _b2World->CreateBody(&dynamicBodyDef);
		dynamicBody[i]->CreateFixture(&fixtureDef);
	}

	b2RevoluteJointDef RJoint;	// �������`
	for (int i = 0; i < 6; i++)
	{		
		b2RevoluteJoint*  RevJoint;
		RJoint.Initialize(staticBody[i], dynamicBody[i], dynamicBody[i]->GetWorldCenter());
		RevJoint = (b2RevoluteJoint*)_b2World->CreateJoint(&RJoint);
	}
}




void Level1::doStep(float dt)
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

bool Level1::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent)//Ĳ�I�}�l�ƥ�
{
	Point touchLoc = pTouch->getLocation();
	bool bOnGravityBtn = false;

	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		if (body->GetUserData() != NULL) {// �R�A���餣�B�z
												   // �P�_�I����m�O�_���b�ʺA����@�w���d��
			Sprite *spriteObj = (Sprite*)body->GetUserData();
			Size objSize = spriteObj->getContentSize();
			float scaleX = spriteObj->getScaleX();
			float scaleY = spriteObj->getScaleY();
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

	if( !bOnGravityBtn && !_bMouseOn) {
		// ���إ� ballSprite �� Sprite �å[�J������
		auto ballSprite = Sprite::createWithSpriteFrameName(ball[rand()% MAX_CIRCLE_OBJECTS]);
		ballSprite->setScale(0.75f);
	//	ballSprite->setPosition(touchLoc);
		this->addChild(ballSprite, 2);

		// �إߤ@��²�檺�ʺA�y��
		b2BodyDef bodyDef;	// ���H���c b2BodyDef �ŧi�@�� Body ���ܼ�
		bodyDef.type = b2_dynamicBody; // �]�w���ʺA����
		bodyDef.userData = ballSprite;	// �]�w Sprite ���ʺA���骺��ܹϥ�
		bodyDef.position.Set(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO);
		// �H bodyDef �b b2World  ���إ߹���öǦ^�ӹ��骺����
		b2Body *ballBody = _b2World->CreateBody(&bodyDef);
		// �]�w�Ӫ��骺�~��
		b2CircleShape ballShape;	//  �ŧi���骺�~�������ܼơA���B�O��Ϊ���
		Size ballsize = ballSprite->getContentSize();	// �ھ� Sprite �ϧΪ��j�p�ӳ]�w��Ϊ��b�|
		ballShape.m_radius = 0.75f*(ballsize.width - 4) *0.5f / PTM_RATIO;
		// �H b2FixtureDef  ���c�ŧi���鵲�c�ܼơA�ó]�w���骺�������z�Y��
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &ballShape;			// ���w���骺�~�������
		fixtureDef.restitution = 0.15f;			// �]�w�u�ʫY��
		fixtureDef.density = 5.0f;				// �]�w�K��
		fixtureDef.friction = 0.15f;			// �]�w�����Y��
		ballBody->CreateFixture(&fixtureDef);	// �b Body �W���ͳo�ӭ��骺�]�w
		//ballBody->ApplyLinearImpulse(b2Vec2(0, 250), ballBody->GetWorldCenter(), true);
		// GetWorldCenter():Get the world position of the center of mass
	}

	_redBtn->touchesBegin(touchLoc);
	_greenBtn->touchesBegin(touchLoc);
	_blueBtn->touchesBegin(touchLoc);
	return true;
}

void  Level1::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //Ĳ�I���ʨƥ�
{
	Point touchLoc = pTouch->getLocation();
	if (_bTouchOn)
	{
		_MouseJoint->SetTarget(b2Vec2(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO));
	}

	_redBtn->touchesBegin(touchLoc);
	_greenBtn->touchesBegin(touchLoc);
	_blueBtn->touchesBegin(touchLoc);
}

void  Level1::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //Ĳ�I�����ƥ� 
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
	if (_redBtn->touchesEnded(touchLoc)) {
		// �H����ܤ@���y�A�q (260,700) �B����ۥѸ��U �A�j�p�Y��50%
		auto ballSprite = Sprite::createWithSpriteFrameName("clock03.png");
		ballSprite->setScale(0.5f);
		this->addChild(ballSprite, 2);

		// �إߤ@��²�檺�ʺA�y��
		b2BodyDef bodyDef;	// ���H���c b2BodyDef �ŧi�@�� Body ���ܼ�
		bodyDef.type = b2_dynamicBody; // �]�w���ʺA����
		bodyDef.userData = ballSprite;	// �]�w Sprite ���ʺA���骺��ܹϥ�
		bodyDef.position.Set(260.0f / PTM_RATIO, 700.0f / PTM_RATIO);
		// �H bodyDef �b b2World  ���إ߹���öǦ^�ӹ��骺����
		b2Body *ballBody = _b2World->CreateBody(&bodyDef);

		// �]�w�Ӫ��骺�~��
		b2CircleShape ballShape;	//  �ŧi���骺�~�������ܼơA���B�O��Ϊ���
		Size ballsize = ballSprite->getContentSize();	// �ھ� Sprite �ϧΪ��j�p�ӳ]�w��Ϊ��b�|
		ballShape.m_radius = 0.5f*(ballsize.width - 4) *0.5f / PTM_RATIO;
		// �H b2FixtureDef  ���c�ŧi���鵲�c�ܼơA�ó]�w���骺�������z�Y��
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &ballShape;			// ���w���骺�~�������
		fixtureDef.restitution = 0.75f;			// �]�w�u�ʫY��
		fixtureDef.density = 1.0f;				// �]�w�K��
		fixtureDef.friction = 0.15f;			// �]�w�����Y��
		ballBody->CreateFixture(&fixtureDef);	// �b Body �W���ͳo�ӭ��骺�]�w
	}
	if (_greenBtn->touchesEnded(touchLoc)) {
		// �H����ܤ@���y�A�q (260,700) �B����ۥѸ��U �A�j�p�Y��50%
		auto ballSprite = Sprite::createWithSpriteFrameName("clock02.png");
		ballSprite->setScale(0.5f);
		this->addChild(ballSprite, 2);

		// �إߤ@��²�檺�ʺA�y��
		b2BodyDef bodyDef;	// ���H���c b2BodyDef �ŧi�@�� Body ���ܼ�
		bodyDef.type = b2_dynamicBody; // �]�w���ʺA����
		bodyDef.userData = ballSprite;	// �]�w Sprite ���ʺA���骺��ܹϥ�
		bodyDef.position.Set(260.0f / PTM_RATIO, 700.0f / PTM_RATIO);
		// �H bodyDef �b b2World  ���إ߹���öǦ^�ӹ��骺����
		b2Body *ballBody = _b2World->CreateBody(&bodyDef);

		// �]�w�Ӫ��骺�~��
		b2CircleShape ballShape;	//  �ŧi���骺�~�������ܼơA���B�O��Ϊ���
		Size ballsize = ballSprite->getContentSize();	// �ھ� Sprite �ϧΪ��j�p�ӳ]�w��Ϊ��b�|
		ballShape.m_radius = 0.5f*(ballsize.width - 4) *0.5f / PTM_RATIO;
		// �H b2FixtureDef  ���c�ŧi���鵲�c�ܼơA�ó]�w���骺�������z�Y��
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &ballShape;			// ���w���骺�~�������
		fixtureDef.restitution = 0.75f;			// �]�w�u�ʫY��
		fixtureDef.density = 1.0f;				// �]�w�K��
		fixtureDef.friction = 0.15f;			// �]�w�����Y��
		ballBody->CreateFixture(&fixtureDef);	// �b Body �W���ͳo�ӭ��骺�]�w
	}
	if (_blueBtn->touchesEnded(touchLoc)) {
		// �H����ܤ@���y�A�q (260,700) �B����ۥѸ��U �A�j�p�Y��50%
		auto ballSprite = Sprite::createWithSpriteFrameName("clock04.png");
		ballSprite->setScale(0.5f);
		this->addChild(ballSprite, 2);

		// �إߤ@��²�檺�ʺA�y��
		b2BodyDef bodyDef;	// ���H���c b2BodyDef �ŧi�@�� Body ���ܼ�
		bodyDef.type = b2_dynamicBody; // �]�w���ʺA����
		bodyDef.userData = ballSprite;	// �]�w Sprite ���ʺA���骺��ܹϥ�
		bodyDef.position.Set(260.0f / PTM_RATIO, 700.0f / PTM_RATIO);
		// �H bodyDef �b b2World  ���إ߹���öǦ^�ӹ��骺����
		b2Body *ballBody = _b2World->CreateBody(&bodyDef);

		// �]�w�Ӫ��骺�~��
		b2CircleShape ballShape;	//  �ŧi���骺�~�������ܼơA���B�O��Ϊ���
		Size ballsize = ballSprite->getContentSize();	// �ھ� Sprite �ϧΪ��j�p�ӳ]�w��Ϊ��b�|
		ballShape.m_radius = 0.5f*(ballsize.width - 4) *0.5f / PTM_RATIO;
		// �H b2FixtureDef  ���c�ŧi���鵲�c�ܼơA�ó]�w���骺�������z�Y��
		b2FixtureDef fixtureDef;
		fixtureDef.shape = &ballShape;			// ���w���骺�~�������
		fixtureDef.restitution = 0.75f;			// �]�w�u�ʫY��
		fixtureDef.density = 1.0f;				// �]�w�K��
		fixtureDef.friction = 0.15f;			// �]�w�����Y��
		ballBody->CreateFixture(&fixtureDef);	// �b Body �W���ͳo�ӭ��骺�]�w
	}
}

void Level1::createStaticBoundary()
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

#ifdef BOX2D_DEBUG
//��gø�s��k
void Level1::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif