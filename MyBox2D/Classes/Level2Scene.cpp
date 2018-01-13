#include "Level2Scene.h"
#include "Level3Scene.h"
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

Scene* Level2::createScene(int num[4][3],int level)
{
    auto scene = Scene::create();
    auto layer = Level2::create();
	layer->addBall(num);
	layer->maxLevel = level;
    scene->addChild(layer);
    return scene;
}

void Level2::addBall(int num[4][3]) {
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 3; j++) {
			levelball[i][j] = num[i][j];
		}
	}
	nr = 5 + num[1][0];  ng = 5 + num[1][1];  ny = 5 + num[1][2];

	CCString* sr = CCString::createWithFormat("%d", nr);
	const char* cr = sr->getCString();
	_redNum->setString(cr);

	CCString* sg = CCString::createWithFormat("%d", ng);
	const char* cg = sg->getCString();
	_greenNum->setString(cg);

	CCString* sy = CCString::createWithFormat("%d", ny);
	const char* cy = sy->getCString();
	_yellowNum->setString(cy);
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
   

	// 建立 Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);	//重力方向
	bool AllowSleep = true;					//允許睡著
	_b2World = new b2World(Gravity);		//創建世界
	_b2World->SetAllowSleeping(AllowSleep);	//設定物件允許睡著

	// 讀入 CSB 檔
	_csbRoot = CSLoader::createNode("Level2.csb");
	
	SimpleAudioEngine::getInstance()->playBackgroundMusic("./audio/bg.mp3", true);

#ifdef BOX2D_DEBUG
	// 設定顯示背景圖示
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
	setEndUi();
	setStar();
	
#ifdef BOX2D_DEBUG
	//DebugDrawInit
	_DebugDraw = nullptr;
	_DebugDraw = new GLESDebugDraw(PTM_RATIO);
	//設定DebugDraw
	_b2World->SetDebugDraw(_DebugDraw);
	//選擇繪製型別
	uint32 flags = 0;
	flags += GLESDebugDraw::e_shapeBit;						//繪製形狀
	flags += GLESDebugDraw::e_pairBit;
	flags += GLESDebugDraw::e_jointBit;
	flags += GLESDebugDraw::e_centerOfMassBit;
	flags += GLESDebugDraw::e_aabbBit;
	//設定繪製類型
	_DebugDraw->SetFlags(flags);
#endif

	_b2World->SetContactListener(&_colliderSeneor);
	_listener1 = EventListenerTouchOneByOne::create();	//創建一個一對一的事件聆聽器
	_listener1->onTouchBegan = CC_CALLBACK_2(Level2::onTouchBegan, this);		//加入觸碰開始事件
	_listener1->onTouchMoved = CC_CALLBACK_2(Level2::onTouchMoved, this);		//加入觸碰移動事件
	_listener1->onTouchEnded = CC_CALLBACK_2(Level2::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(_listener1, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(Level2::doStep));

    return true;
}

void  Level2::setbtn() {
	//red
	auto btnSprite = _csbRoot->getChildByName("redbtn");
	_redBtn = CButton::create();
	_redBtn->setButtonInfo("playBtn01_01.png", "playBtn01_02.png", btnSprite->getPosition());
	_redBtn->setScale(btnSprite->getScale());
	this->addChild(_redBtn, 5);
	_csbRoot->removeChildByName("redbtn");

	_redNum = cocos2d::ui::Text::create("0", "Marker Felt.ttf", 36);
	_redNum->setPosition(btnSprite->getPosition());
	this->addChild(_redNum, 6);

	//green
	btnSprite = _csbRoot->getChildByName("greenbtn");
	_greenBtn = CButton::create();
	_greenBtn->setButtonInfo("playBtn02_01.png", "playBtn02_02.png", btnSprite->getPosition());
	_greenBtn->setScale(btnSprite->getScale());
	this->addChild(_greenBtn, 5);
	_csbRoot->removeChildByName("greenbtn");

	_greenNum = cocos2d::ui::Text::create("0", "Marker Felt.ttf", 36);
	_greenNum->setPosition(btnSprite->getPosition());
	this->addChild(_greenNum, 6);

	//yellow
	btnSprite = _csbRoot->getChildByName("bluebtn");
	_blueBtn = CButton::create();
	_blueBtn->setButtonInfo("playBtn03_01.png", "playBtn03_02.png", btnSprite->getPosition());
	_blueBtn->setScale(btnSprite->getScale());
	this->addChild(_blueBtn, 5);
	_csbRoot->removeChildByName("bluebtn");

	_yellowNum = cocos2d::ui::Text::create("0", "Marker Felt.ttf", 36);
	_yellowNum->setPosition(btnSprite->getPosition());
	this->addChild(_yellowNum, 6);

	//pen
	btnSprite = _csbRoot->getChildByName("penbtn");
	_penBtn = CSwitchButton::create();
	_penBtn->setButtonInfo("playBtn00_01.png", "playBtn00_02.png", btnSprite->getPosition());
	_penBtn->setScale(btnSprite->getScale());
	this->addChild(_penBtn, 5);
	_csbRoot->removeChildByName("penbtn");
}

void  Level2::setUIbtn() {
	auto btnSprite = _csbRoot->getChildByName("homebtn");
	_homeBtn = CButton::create();
	_homeBtn->setButtonInfo("uiBtn01_01.png", "uiBtn01_02.png", btnSprite->getPosition());
	_homeBtn->setScale(btnSprite->getScale());
	this->addChild(_homeBtn, 5);
	_csbRoot->removeChildByName("homebtn");

	btnSprite = _csbRoot->getChildByName("replaybtn");
	_replayBtn = CButton::create();
	_replayBtn->setButtonInfo("uiBtn02_01.png", "uiBtn02_02.png", btnSprite->getPosition());
	_replayBtn->setScale(btnSprite->getScale());
	this->addChild(_replayBtn, 5);
	_csbRoot->removeChildByName("replaybtn");

}

void Level2::setEndUi() {
	_endUi = new Node;
	this->addChild(_endUi, 20);

	auto bg = Sprite::createWithSpriteFrameName("bgwhite.png");
	bg->setPosition(640, 360);
	bg->setScale(20);
	_endUi->addChild(bg, 1);

	auto level = Sprite::createWithSpriteFrameName("levelup.png");
	level->setPosition(640, 360);
	level->setScale(2);
	_endUi->addChild(level, 2);

	auto btnSprite = _csbRoot->getChildByName("endhome");
	_homeBtn2 = CButton::create();
	_homeBtn2->setButtonInfo("uiBtn01_01.png", "uiBtn01_02.png", btnSprite->getPosition());
	_homeBtn2->setScale(btnSprite->getScale());
	_endUi->addChild(_homeBtn2, 5);
	_csbRoot->removeChildByName("endhome");

	btnSprite = _csbRoot->getChildByName("endreplay");
	_replayBtn2 = CButton::create();
	_replayBtn2->setButtonInfo("uiBtn02_01.png", "uiBtn02_02.png", btnSprite->getPosition());
	_replayBtn2->setScale(btnSprite->getScale());
	_endUi->addChild(_replayBtn2, 5);
	_csbRoot->removeChildByName("endreplay");

	btnSprite = _csbRoot->getChildByName("endplay");
	_nextBtn = CButton::create();
	_nextBtn->setButtonInfo("uiBtn03_01.png", "uiBtn03_02.png", btnSprite->getPosition());
	_nextBtn->setScale(btnSprite->getScale());
	_endUi->addChild(_nextBtn, 5);
	_csbRoot->removeChildByName("endplay");

	for (int i = 0; i < 3; i++) {
		_score[i] = cocos2d::ui::Text::create("0", "Marker Felt.ttf", 36);
		_score[i]->setPosition(Vec2(495 + 145 * i, 337));  //(495,337),(640,337),(785,337)
		_endUi->addChild(_score[i], 6);
	}

	_endUi->setVisible(false);
}

void Level2::createStaticBoundary()
{
	// 先產生 Body, 設定相關的參數

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // 設定這個 Body 為 靜態的
	bodyDef.userData = NULL;
	// 在 b2World 中產生該 Body, 並傳回產生的 b2Body 物件的指標
	// 產生一次，就可以讓後面所有的 Shape 使用
	b2Body *body = _b2World->CreateBody(&bodyDef);
	_bottomBody = body;
	// 產生靜態邊界所需要的 EdgeShape
	b2EdgeShape edgeShape;
	b2FixtureDef edgeFixtureDef; // 產生 Fixture
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
	b2FixtureDef fixtureDef; // 產生 Fixture
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
		modelMatrix.m[0] = scaleX;  // 先設定 X 軸的縮放
		modelMatrix.m[5] = scaleY;  // 先設定 Y 軸的縮放
		cocos2d::Mat4::createRotationZ(angle*M_PI / 180.0f, &rotMatrix);
		modelMatrix.multiply(rotMatrix);
		modelMatrix.m[3] = loc.x; //設定 Translation，自己的加上父親的
		modelMatrix.m[7] = loc.y; //設定 Translation，自己的加上父親的
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
	Point pt = bornSprite->getPosition();
	bornpt = pt;
	_born = Sprite::createWithSpriteFrameName("pipe01.png");
	_born->setPosition(pt);
	this->addChild(_born, 10);
	_csbRoot->removeChildByName("born");

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
//	b2RevoluteJointDef RJoint;	// 旋轉關節
//	for (int i = 0; i < 2; i++)
//	{
//		b2RevoluteJoint*  RevJoint;
//		RJoint.Initialize(staticBody[i], dynamicBody[i], staticBody[i]->GetWorldCenter());
//		RevJoint = (b2RevoluteJoint*)_b2World->CreateJoint(&RJoint);
//	}
//}

void Level2::setPendulum() {
	//基底
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

	// 單擺
	auto circleSprite = _csbRoot->getChildByName("Pendulum_cir_00");
	loc = circleSprite->getPosition();
	size = circleSprite->getContentSize();
	b2CircleShape circleShape;
	circleShape.m_radius = size.width*0.5f / PTM_RATIO;
	   //感應器
	auto _collisionSprite = (Sprite *)_csbRoot->getChildByName("Pendulum_cir_00");
	_colliderSeneor.setCollisionTarget(*_collisionSprite);

	b2BodyDef circleBodyDef;
	circleBodyDef.type = b2_dynamicBody;
	circleBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	circleBodyDef.userData = circleSprite;
	b2Body* circleBody = _b2World->CreateBody(&circleBodyDef);
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 100000;
	circleBody->CreateFixture(&fixtureDef);

	//產生距離關節
	b2DistanceJointDef JointDef;
	JointDef.Initialize(basicBody, circleBody, basicBody->GetPosition(), circleBody->GetPosition());
	_b2World->CreateJoint(&JointDef);

	//取得並設定 circle01_weld 為【動態物體】
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
	//_b2World->CreateJoint(&ropeJointDef); // 使用預設值焊接

	b2RevoluteJointDef revolutrJointDef;
	revolutrJointDef.Initialize(basicBody, ropeBody, basicBody->GetPosition());
	_b2World->CreateJoint(&revolutrJointDef);

	revolutrJointDef.Initialize(circleBody, ropeBody, circleBody->GetPosition());
	_b2World->CreateJoint(&revolutrJointDef);

}

void Level2::setFinalBox() {
	//終點箱子
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
		boxSprite[i]->setColor(BlockColor[(i / 3)]);	// 使用 filterColor 已經建立的顏色
		boxBodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
		boxBody[i] = _b2World->CreateBody(&boxBodyDef);
		fixtureDef.filter.categoryBits = 1 << (i / 3 + 1);   // 0.1.2 =1  3.4.2 =2  6.7.8 =3
		boxBody[i]->CreateFixture(&fixtureDef);
	}

	//終點感應器
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
		boxSprite[i]->setColor(BlockColor2[(i % 3)]);	// 使用 filterColor 已經建立的顏色
		boxBodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
		boxBody[i] = _b2World->CreateBody(&boxBodyDef);
		fixtureDef.filter.categoryBits = 1 << ((i % 3 + 1));   // 0.1.2 =1  3.4.2 =2  6.7.8 =3
		fixtureDef.density = 500.0f + 100*(i-9);  //500 600 700
		fixtureDef.isSensor = true;
		boxBody[i]->CreateFixture(&fixtureDef);
	}
	
}
void Level2::setStar() {
	auto circleSprite = _csbRoot->getChildByName("star");
	auto loc = circleSprite->getPosition();
	auto size = circleSprite->getContentSize();
	b2CircleShape circleShape;
	circleShape.m_radius = size.width*0.5f / PTM_RATIO;
	//感應器
	auto _collisionSprite = (Sprite *)_csbRoot->getChildByName("star");
	_colliderSeneor.setCollisionTarget2(*_collisionSprite);

	b2BodyDef circleBodyDef;
	b2FixtureDef fixtureDef;
	circleBodyDef.type = b2_staticBody;
	circleBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	circleBodyDef.userData = circleSprite;
	_star = _b2World->CreateBody(&circleBodyDef);
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 1.0f;
	fixtureDef.isSensor = true;
	_star->CreateFixture(&fixtureDef);
}

void Level2::doStep(float dt)
{
	int velocityIterations = 8;	// 速度迭代次數
	int positionIterations = 1; // 位置迭代次數 迭代次數一般設定為8~10 越高越真實但效率越差
	_b2World->Step(dt, velocityIterations, positionIterations);

	for (b2Body* body = _b2World->GetBodyList(); body;)
	{
		//body->ApplyForce(b2Vec2(10.0f, 10.0f), body->GetWorldCenter(), true);
		// 以下是以 Body 有包含 Sprite 顯示為例
		if (body->GetUserData() != NULL)
		{
			Sprite *ballData = (Sprite*)body->GetUserData();
			ballData->setPosition(body->GetPosition().x*PTM_RATIO, body->GetPosition().y*PTM_RATIO);
			ballData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));

			int d = body->GetFixtureList()->GetDensity() * 100.0;
			if (d == 111 && _balltime == 0.0f) {
				_balltime += dt;
				auto sparkSprite = Sprite::createWithSpriteFrameName("flare.png");
				sparkSprite->setScale(0.5);
				sparkSprite->setBlendFunc(BlendFunc::ADDITIVE);
				this->addChild(sparkSprite, 5);
				//產生小方塊資料
				b2BodyDef RectBodyDef;
				RectBodyDef.position.Set(body->GetPosition().x, body->GetPosition().y);
				RectBodyDef.type = b2_dynamicBody;
				RectBodyDef.userData = sparkSprite;
				b2PolygonShape RectShape;
				RectShape.SetAsBox(5 / PTM_RATIO, 5 / PTM_RATIO);
				b2Body* RectBody = _b2World->CreateBody(&RectBodyDef);
				b2FixtureDef RectFixtureDef;
				RectFixtureDef.shape = &RectShape;
				RectFixtureDef.density = 1.0f;
				RectFixtureDef.filter.maskBits = 0;
				b2Fixture*RectFixture = RectBody->CreateFixture(&RectFixtureDef);
			}

		}

		if (_balltime > 1.0f)      _balltime = 0.0f;
		else if(_balltime > 0.0f)  _balltime += dt;
		

		// 跑出螢幕外面就讓物體從 b2World 中移除
		if (body->GetType() == b2BodyType::b2_dynamicBody) {
			float x = body->GetPosition().x * PTM_RATIO;
			float y = body->GetPosition().y * PTM_RATIO;
			if (x > _visibleSize.width || x < 0 || y >  _visibleSize.height || y < 0) {
				if (body->GetUserData() != NULL) {
					Sprite* spriteData = (Sprite *)body->GetUserData();
					this->removeChild(spriteData);
				}
				_b2World->DestroyBody(body);
				body = NULL;
			}
			else body = body->GetNext(); //否則就繼續更新下一個Body
		}
		else body = body->GetNext(); //否則就繼續更新下一個Body
	}

	// 產生火花
	if (_colliderSeneor._bCreateSpark) {
		_colliderSeneor._bCreateSpark = false;	//產生完關閉
												// 判斷延遲的時間是否滿足
		if (_bSparking) { //可以噴發，實現這次的噴發
			_tdelayTime = 0; // 時間重新設定，
			_bSparking = false; // 開始計時
			for (int i = 0; i < 20; i++) {
				// 建立 Spark Sprite 並與目前的物體結合
				auto sparkSprite = Sprite::createWithSpriteFrameName("spark.png");
				sparkSprite->setColor(Color3B(rand() % 256, rand() % 256, rand() % 156));
				sparkSprite->setBlendFunc(BlendFunc::ADDITIVE);
				this->addChild(sparkSprite, 5);
				//產生小方塊資料
				b2BodyDef RectBodyDef;
				Vec2 pt;
				float r = i*M_PI / 10;  // 18*i*PI/180
				pt.x =cosf(r);  pt.y =sinf(r);
				RectBodyDef.position.Set(pt.x + _colliderSeneor._createLoc.x , pt.y + _colliderSeneor._createLoc.y);
				RectBodyDef.type = b2_dynamicBody;
				RectBodyDef.userData = sparkSprite;
				b2PolygonShape RectShape;
				RectShape.SetAsBox(5 / PTM_RATIO, 5 / PTM_RATIO);
				b2Body* RectBody = _b2World->CreateBody(&RectBodyDef);
				b2FixtureDef RectFixtureDef;
				RectFixtureDef.shape = &RectShape;
				RectFixtureDef.density = 1.0f;
				RectFixtureDef.filter.maskBits = 0;
				b2Fixture*RectFixture = RectBody->CreateFixture(&RectFixtureDef);

				//給力量
				RectBody->ApplyForce(b2Vec2(pt.x*10,pt.y*10), _colliderSeneor._createLoc, true);
			}
		}
	}
	if (!_bSparking) {
		_tdelayTime += dt;
		if (_tdelayTime >= 0.075f) {
			_tdelayTime = 0; // 歸零
			_bSparking = true; // 可進行下一次的噴發
		}
	}

	//成就
	if (_colliderSeneor.star == true) {
		for (int i = 0; i < 20; i++) {
			// 建立 Spark Sprite 並與目前的物體結合
			auto sparkSprite = Sprite::createWithSpriteFrameName("star.png");
			sparkSprite->setScale(0.5);
			sparkSprite->setBlendFunc(BlendFunc::ADDITIVE);
			this->addChild(sparkSprite, 5);
			//產生小方塊資料
			b2BodyDef RectBodyDef;
			Vec2 pt;
			float r = i*M_PI / 10;  // 18*i*PI/180
			pt.x = cosf(r);  pt.y = sinf(r);
			RectBodyDef.position.Set(pt.x + _star->GetPosition().x, pt.y + _star->GetPosition().y);
			RectBodyDef.type = b2_dynamicBody;
			RectBodyDef.userData = sparkSprite;
			b2PolygonShape RectShape;
			RectShape.SetAsBox(5 / PTM_RATIO, 5 / PTM_RATIO);
			b2Body* RectBody = _b2World->CreateBody(&RectBodyDef);
			b2FixtureDef RectFixtureDef;
			RectFixtureDef.shape = &RectShape;
			RectFixtureDef.density = 1.0f;
			RectFixtureDef.filter.maskBits = 0;
			b2Fixture*RectFixture = RectBody->CreateFixture(&RectFixtureDef);

			//給力量
			RectBody->ApplyForce(b2Vec2(pt.x * 10, pt.y * 10), _colliderSeneor._createLoc, true);
		}
		_colliderSeneor.star = false;
		_bstar = true;
		Sprite* spriteData = (Sprite *)_star->GetUserData();
		_csbRoot->removeChild(spriteData);
		_b2World->DestroyBody(_star);
	}

	//終點判斷
	if (_bboxR && _bboxG && _bboxB) {
		CCLOG("LEVEL UP!");
		startGame = false;
		_endUi->setVisible(true);
		levelball[2][0] = nr;
		levelball[2][1] = ng;
		levelball[2][2] = ny;
		if (maxLevel < 3)maxLevel = 3;
		for (int i = 0; i < 3; i++) {
			CCString* s = CCString::createWithFormat("+%d", levelball[2][i]);
			const char* c = s->getCString();
			_score[i]->setString(c);
		}
		if (_bstar) {
			auto starsprite = Sprite::createWithSpriteFrameName("star.png");
			starsprite->setPosition(Vec2(834, 461));
			starsprite->setScale(1.6);
			starsprite->setRotation(15);
			this->addChild(starsprite, 30);
		}
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

bool Level2::onTouchBegan(cocos2d::Touch *pTouch, cocos2d::Event *pEvent)//觸碰開始事件
{
	Point touchLoc = pTouch->getLocation();
	if (startGame) {
		for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
		{
			if (body->GetUserData() != NULL) {// 靜態物體不處理	
				float d = body->GetFixtureList()->GetDensity();
				if (d == 100000) {
					//判斷點的位置是否落在動態物體一定的範圍
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
						_MouseJoint = (b2MouseJoint*)_b2World->CreateJoint(&mouseJointDef); // 新增 Mouse Joint
						body->SetAwake(true);
						_bMouseOn = true;
						break;
					}
				}

			}
		}

		_redBtn->touchesBegin(touchLoc);
		_greenBtn->touchesBegin(touchLoc);
		_blueBtn->touchesBegin(touchLoc);

		_homeBtn->touchesBegin(touchLoc);
		_replayBtn->touchesBegin(touchLoc);
	}
	else {
		_homeBtn2->touchesBegin(touchLoc);
		_replayBtn2->touchesBegin(touchLoc);
		_nextBtn->touchesBegin(touchLoc);
	}
	return true;
}

void  Level2::onTouchMoved(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //觸碰移動事件
{
	Point touchLoc = pTouch->getLocation();
	if (startGame) {
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
	else {
		_homeBtn2->touchesBegin(touchLoc);
		_replayBtn2->touchesBegin(touchLoc);
		_nextBtn->touchesBegin(touchLoc);
	}
}

void  Level2::onTouchEnded(cocos2d::Touch *pTouch, cocos2d::Event *pEvent) //觸碰結束事件 
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
		if (nr > 0) {
			nr -= 1;
			renderball("ball_01.png", 1);
		}
	}
	if (_greenBtn->touchesEnded(touchLoc)) {
		if (ng > 0) {
			ng -= 1;
			renderball("ball_02.png", 2);
		}
	}
	if (_blueBtn->touchesEnded(touchLoc)) {
		if (ny > 0) {
			ny -= 1;
			renderball("ball_03.png", 3);
		}
	}

	if (_homeBtn->touchesEnded(touchLoc)) {
		this->unschedule(schedule_selector(Level2::doStep));
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("UIBTN.plist");
		SimpleAudioEngine::getInstance()->stopBackgroundMusic();
		TransitionFade *pageTurn;
		pageTurn = TransitionFade::create(1.0F, StartScene::createScene(levelball, maxLevel));
		Director::getInstance()->replaceScene(pageTurn);
	}
	if (_replayBtn->touchesEnded(touchLoc)) {
		this->unschedule(schedule_selector(Level2::doStep));
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("UIBTN.plist");
		SimpleAudioEngine::getInstance()->stopBackgroundMusic();
		TransitionFade *pageTurn;
		pageTurn = TransitionFade::create(1.0F, Level2::createScene(levelball, maxLevel));
		Director::getInstance()->replaceScene(pageTurn);
	}

	if (_homeBtn2->touchesEnded(touchLoc)) {
		this->unschedule(schedule_selector(Level2::doStep));
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("UIBTN.plist");
		SimpleAudioEngine::getInstance()->stopBackgroundMusic();
		TransitionFade *pageTurn;
		pageTurn = TransitionFade::create(1.0F, StartScene::createScene(levelball, maxLevel));
		Director::getInstance()->replaceScene(pageTurn);
	}
	if (_replayBtn2->touchesEnded(touchLoc)) {
		this->unschedule(schedule_selector(Level2::doStep));
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("UIBTN.plist");
		SimpleAudioEngine::getInstance()->stopBackgroundMusic();
		TransitionFade *pageTurn;
		pageTurn = TransitionFade::create(1.0F, Level2::createScene(levelball, maxLevel));
		Director::getInstance()->replaceScene(pageTurn);
	}
	if (_nextBtn->touchesEnded(touchLoc)) {
		this->unschedule(schedule_selector(Level2::doStep));
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("UIBTN.plist");
		SimpleAudioEngine::getInstance()->stopBackgroundMusic();
		TransitionFade *pageTurn;
		pageTurn = TransitionFade::create(1.0F, Level3::createScene(levelball, maxLevel));
		Director::getInstance()->replaceScene(pageTurn);
	}
}

void Level2::renderball(char *name, int mask) {
	auto ballSprite = Sprite::createWithSpriteFrameName(name);
	this->addChild(ballSprite, 2);

	// 建立一個簡單的動態球體
	b2BodyDef bodyDef;	// 先以結構 b2BodyDef 宣告一個 Body 的變數
	bodyDef.type = b2_dynamicBody; // 設定為動態物體
	bodyDef.userData = ballSprite;	// 設定 Sprite 為動態物體的顯示圖示
	bodyDef.position.Set(bornpt.x / PTM_RATIO, bornpt.y / PTM_RATIO);
	// 以 bodyDef 在 b2World  中建立實體並傳回該實體的指標
	b2Body *ballBody = _b2World->CreateBody(&bodyDef);

	// 設定該物體的外型
	b2CircleShape ballShape;	//  宣告物體的外型物件變數，此處是圓形物體
	Size ballsize = ballSprite->getContentSize();	// 根據 Sprite 圖形的大小來設定圓形的半徑
	ballShape.m_radius = 0.5f*(ballsize.width - 4) / PTM_RATIO;
	// 以 b2FixtureDef  結構宣告剛體結構變數，並設定剛體的相關物理係數
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &ballShape;			// 指定剛體的外型為圓形
	fixtureDef.restitution = 0.15f;			// 設定彈性係數
	fixtureDef.density = 1.1f;				// 設定密度
	fixtureDef.friction = 0.15f;			// 設定摩擦係數
	fixtureDef.filter.maskBits = 1 << mask | 1; //設定群組
	ballBody->CreateFixture(&fixtureDef);	// 在 Body 上產生這個剛體的設定

	if (mask == 1) {
		CCString* s = CCString::createWithFormat("%d", nr);
		const char* c = s->getCString();
		_redNum->setString(c);
	}
	else if (mask == 2) {
		CCString* s = CCString::createWithFormat("%d", ng);
		const char* c = s->getCString();
		_greenNum->setString(c);
	}
	else if (mask == 3) {
		CCString* s = CCString::createWithFormat("%d", ny);
		const char* c = s->getCString();
		_yellowNum->setString(c);
	}
}


#ifdef BOX2D_DEBUG
//改寫繪製方法
void Level2::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif
