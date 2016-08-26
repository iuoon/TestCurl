#pragma once
#ifndef _LOADING_LAYER_H_
#define _LOADING_LAYER_H_

#include "cocos2d.h"
#include "CurlDown.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"
USING_NS_CC;
using namespace cocos2d::ui;


class Loading :public Layer,public CurlDown
{
public:
	Loading();
	~Loading();

	static cocos2d::Scene* createScene();

	virtual bool init();
	CREATE_FUNC(Loading);

	virtual void onProgress(double percent, string totalSize, string downSize, string speed) override;
	virtual void onSuccess(bool isSuccess, string filefullPath) override;

private:
	int _count;
	Node* rootNode;
	Label* _label2;
	Label* _label4;
	Label* _label6;
};	

#endif