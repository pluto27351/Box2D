#include "CSwitchButton.h"

USING_NS_CC;

CSwitchButton* CSwitchButton::create()
{
	CSwitchButton *btnobj = new (std::nothrow) CSwitchButton();
	if (btnobj)
	{
		btnobj->autorelease();
		return btnobj;
	}
	CC_SAFE_DELETE(btnobj);
	return nullptr;
}


CSwitchButton::CSwitchButton()
{
	_SwitchOffPic = nullptr;
	_SwitchOnPic = nullptr;
}

void CSwitchButton::setButtonInfo(const char *offImg, const char *onImg, const cocos2d::Point locPt)
{
	_SwitchOffPic = (Sprite *)Sprite::createWithSpriteFrameName(offImg);
	_SwitchOnPic = (Sprite *)Sprite::createWithSpriteFrameName(onImg);

	_BtnLoc = locPt;
	_SwitchOffPic->setPosition(_BtnLoc); // 設定位置
	_SwitchOnPic->setPosition(_BtnLoc); // 設定位置
	_SwitchOffPic->setVisible(true);
	_SwitchOnPic->setVisible(false);

	_ShowBtn = _SwitchOffPic;

	this->addChild(_SwitchOffPic,1);
	this->addChild(_SwitchOnPic, 1);

	// 取得大小
	_BtnSize = _SwitchOffPic->getContentSize();
	// 設定判斷區域
	_BtnRect.size = _BtnSize;
	_BtnRect.origin.x = _BtnLoc.x - _BtnSize.width*0.5f;
	_BtnRect.origin.y = _BtnLoc.y - _BtnSize.height*0.5f;
	_bTouched = false;
	_fScale = 1.0f;
	_bVisible = true;
	_bSwitchOn = false;
}


bool CSwitchButton::touchesBegan(cocos2d::Point inPos)
{
	if( _BtnRect.containsPoint(inPos) && _bVisible )
	{
		_bTouched = true;
		_ShowBtn->setScale(_fScale+0.05f);
		return(true); // 有按在上面
	}
	return(false);
}

bool CSwitchButton::touchesMoved(cocos2d::Point inPos)
{
	if( _bTouched ) { // 只有被按住的時候才處理
		if( !_BtnRect.containsPoint(inPos) ) { // 手指頭位置離開按鈕
			_bTouched = false;
			_ShowBtn->setScale(_fScale);
			return(false);
		}
		else return(true);
	}
	return(false); // 事後再移到按鈕上將被忽略
}

bool CSwitchButton::touchesEnded(cocos2d::Point inPos)
{
	if( _bTouched ) {
		_bSwitchOn = !_bSwitchOn;
		_ShowBtn->setScale(_fScale);
		_ShowBtn->setVisible(false);
		if (_bSwitchOn) _ShowBtn = _SwitchOnPic;
		else _ShowBtn = _SwitchOffPic;
		_ShowBtn->setVisible(true);

		_bTouched = false;
		if( _BtnRect.containsPoint(inPos) ) return(true);  // 手指頭位置按鈕時，還在該按鈕上
	}
	return false;
}

void CSwitchButton::setVisible(bool bVisible)
{
	_bVisible = bVisible;
	if (!_bVisible) {
		_SwitchOffPic->setVisible(false);
	}
	else {
		_SwitchOffPic->setVisible(true);
	}
}

void CSwitchButton::setScale(float scale)
{
	_fScale = scale;
	_SwitchOnPic->setScale(_fScale);
	_SwitchOffPic->setScale(_fScale);
}

bool CSwitchButton::getStatus()
{
	return(_bSwitchOn); // 傳回目前按鈕的狀態為開或是關
}

void CSwitchButton::setStatus(bool status) {
	_SwitchOnPic->setVisible(status);
	_SwitchOffPic->setVisible(!status);
	_bSwitchOn = status;
}

void CSwitchButton::setUneable() {
	_Unable = (Sprite *)Sprite::createWithSpriteFrameName("levelBtn00.png");

	_BtnLoc = _SwitchOffPic->getPosition();
	_Unable->setPosition(_BtnLoc); // 設定位置

	this->addChild(_Unable, 10);

}