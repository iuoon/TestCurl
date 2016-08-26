#include "LoadingLayer.h"



Loading::Loading()
{
}

Loading::~Loading()
{
}

cocos2d::Scene * Loading::createScene()
{
	auto scene = Scene::create();

	auto layer = Loading::create();

	scene->addChild(layer);

	return scene;
}


bool Loading::init()
{
	if (!Layer::init())
	{
		return false;
	}

	Size visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	rootNode = CSLoader::createNode("LoadingLayer.csb");
	addChild(rootNode);
	_label2 = Label::create();
	_label2->setTextColor(Color4B::BLUE);
	_label2->setPosition(Vec2(250, 180));

	_label4 = Label::create();
	_label4->setTextColor(Color4B::BLUE);
	_label4->setPosition(Vec2(434, 180));

	_label6 = Label::create();
	_label6->setTextColor(Color4B::BLUE);
	_label6->setPosition(Vec2(666, 180));
	this->addChild(_label2);
	this->addChild(_label4);
	this->addChild(_label6);
	downStart(); //开始下载文件

	return true;
}

void Loading::onProgress(double percent, string totalSize, string downSize, string speed)
{
	auto loadingBar = static_cast<LoadingBar*>(rootNode->getChildByName("LoadingBar_1"));

	loadingBar->setPercent(percent);
	_label2->setString(totalSize);
	_label4->setString(downSize);
	_label6->setString(speed);
}

void Loading::onSuccess(bool isSuccess, string filefullPath)
{
	if (isSuccess)
	{
		MessageBox("下载成功", "提示");
	}
}

