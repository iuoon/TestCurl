#pragma once
#ifndef __download__CurlDown__
#define __download__CurlDown__
#include <string>
#include "cocos2d.h"
#include <thread>
#include "../external/curl/include/win32/curl/curl.h"
using namespace std;
USING_NS_CC;
#pragma comment(lib, "libcurl_imp.lib")

/************************************************************************/
/* libcurl download file                                                */
/************************************************************************/
class CurlDown 
{
public:
	CurlDown();
	~CurlDown();
	//入口函数
	void downStart();
	//下载控制
	void downloadControler();
	//下载
	bool download(long timeout);
	// 获取远程下载文件的大小
	long getDownloadFileLenth();
	// 读取本地文件已下载大小
	long getLocalFileLength();

	virtual void onProgress(double percent, string totalSize, string downSize,string speed) {};
	virtual void onSuccess(bool isSuccess, string filefullPath) { rename(filefullPath.c_str(), (mFilePath.substr(0, mFilePath.find(mFileName))+"res.zip").c_str()); };
	
public:
	string mFilePath; // 本地存储地址
	double mFileLenth; // 下载文件大小
	string mDownloadUrl; // 下载URL
	CURL *libcurl;

private:
	string mFileName; // 下载文件名称
	bool isStop;
	
};

#endif