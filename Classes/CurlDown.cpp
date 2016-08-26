#include "CurlDown.h"
#include "io.h"

CurlDown::CurlDown()
{
	mFileName = "res.temp";
	mFilePath = FileUtils::getInstance()->getSearchPaths()[0]+"download/res.temp";
	mDownloadUrl = "http://127.0.0.1:8080/download";
	mFileLenth=0;
}

CurlDown::~CurlDown()
{
}

static string getSize(double size)
{
	string unit = "";
	char tsize[10]="";
	if (size > 1024 * 1024 * 1024)
	{
		unit = "G";
		size /= 1024 * 1024 * 1024;
	}
	else if (size > 1024 * 1024)
	{
		unit = "M";
		size /= 1024 * 1024;
	}
	else if (size > 1024)
	{
		unit = "KB";
		size /= 1024;
	}
	sprintf(tsize, "%.1f", size);//保留1位小数(可自行设置)
	string res = tsize + unit;
	return res;
}

static size_t write_func(void *ptr, size_t size, size_t nmemb, void *userdata) {
	FILE *fp = (FILE*)userdata;
	size_t written = fwrite(ptr, size, nmemb, fp);
	return written;
}

static size_t getcontentlengthfunc(void *ptr, size_t size, size_t nmemb, void *data)
{
	return (size_t)(size * nmemb);
}


/************************************************************************/
/*  
这个回调函数可以告诉我们有多少数据需要传输以及传输了多少数据，单位是字节。
totalToDownload是需要下载的总字节数(这里不包括本地已下载的一部分)，nowDownloaded是已经下载的字节数(指的是totalToDownload中已下载多少)。
totalToUpLoad是将要上传的字节数，nowUpLoaded是已经上传的字节数。如果你仅仅下载数据的话，那么ultotal，ulnow将会是0，反之，
如果你仅仅上传的话，那么dltotal和dlnow也会是0。clientp为用户自定义参数，
通过设置CURLOPT_XFERINFODATA属性来传递。此函数返回非0值将会中断传输，错误代码是CURLE_ABORTED_BY_CALLBACK 
*/
/************************************************************************/
static int progress_func(void *ptr, double totalToDownload, double nowDownloaded, double totalToUpLoad, double nowUpLoaded) {
	CurlDown* curlDown = (CurlDown*)ptr;
	CURL *easy_handle = curlDown->libcurl;

	char timeFormat[12] = "";
	double speed; //下载速度
	string unit = "B";
	curl_easy_getinfo(easy_handle, CURLINFO_SPEED_DOWNLOAD, &speed); // curl_get_info必须在curl_easy_perform之后调用
	if (speed != 0)
	{
		// Time remaining  
		double leftTime = (curlDown->mFileLenth - nowDownloaded - curlDown->getLocalFileLength()) / speed; //（总大小-已下载大小-本地存储大小）/速度
		int hours = leftTime / 3600;
		int minutes = (leftTime - hours * 3600) / 60;
		int seconds = leftTime - hours * 3600 - minutes * 60;
		sprintf(timeFormat, "%02d:%02d:%02d", hours, minutes, seconds);//剩余时间格式化
	}

	if (speed > 1024 * 1024 * 1024)
	{
		unit = "G";
		speed /= 1024 * 1024 * 1024;
	}
	else if (speed > 1024 * 1024)
	{
		unit = "M";
		speed /= 1024 * 1024;
	}
	else if (speed > 1024)
	{
		unit = "KB";
		speed /= 1024;
	}	
	log("speed:%.2f%s/s", speed, unit.c_str());
	char fspeed[20]="";
	sprintf(fspeed, "%.2f%s/s", speed, unit.c_str());
	if (!curlDown || curlDown->mFileLenth == 0 || nowDownloaded == 0) return 0;
	double nowDown = (curlDown->mFileLenth - totalToDownload + nowDownloaded);
	double curpercent = nowDown / curlDown->mFileLenth * 100; 
	curlDown->onProgress(curpercent, getSize(curlDown->mFileLenth), getSize(curlDown->mFileLenth - totalToDownload + nowDownloaded), fspeed);
	log("nowDd:%d; totalDown:%d; downProgress:%.2f%%",(int)(curlDown->mFileLenth - totalToDownload + nowDownloaded),(int)curlDown->mFileLenth , curpercent);
		
	return 0;
}




void CurlDown::downStart()
{
	thread _st_d(&CurlDown::downloadControler, this);//创建一个分支线程
	_st_d.detach();
}

void CurlDown::downloadControler()
{
	/*mFileName = mDownloadUrl.substr(mDownloadUrl.rfind('/') + 1);*/
	mFileLenth = getDownloadFileLenth();
	log("file size================%f", mFileLenth);
	if (mFileLenth <= 0) {		
		return;
	}	
	bool ret = false;
	int timeout = 30;
	while (true) { // 循环下载 每30秒进行下载 避免断网情况
		ret = download(timeout); //直接下载
		log("----ret--------->%d",ret);
		if (ret) { //下载完成
			break;
		}
		Sleep(500); //每次下载间隔500ms
	}

	if (ret)
	{		
		onSuccess(ret, mFilePath);
	}
}

bool CurlDown::download(long timeout)
{
	FILE *fp = NULL;
	if (_access(mFilePath.c_str(), 0) == 0) { // 以二进制形式追加
		fp = fopen(mFilePath.c_str(), "ab+");
	}
	else { // 二进制写
		fp = fopen(mFilePath.c_str(), "wb");
	}

	if (fp == NULL) {// 如果文件初始化失败进行返回
		return false;
	}

	// 读取本地文件下载大小
	int use_resume = 0;
	long localFileLenth = getLocalFileLength(); //已经下载的大小
	if (localFileLenth>0)
	{
		use_resume = 1;
	}
	log("filePath:%s  ; leng:%ld", mFilePath.c_str(), localFileLenth);
	CURL *handle = curl_easy_init();
	libcurl = handle;
	curl_easy_setopt(handle, CURLOPT_URL, mDownloadUrl.c_str()); 
//	curl_easy_setopt(handle, CURLOPT_TIMEOUT, timeout);//设置下载时间，超过时间断开下载，测试模式下把这行注释可以无时间限制下载
//	curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, 0);//感觉没什么卵用,得看下
	curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, getcontentlengthfunc);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_func);   //写文件回调方法
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, fp); // 写入文件对象
    curl_easy_setopt(handle, CURLOPT_RESUME_FROM, use_resume ? localFileLenth : 0);  // 从本地大小位置进行请求数据
//	curl_easy_setopt(handle, CURLOPT_RESUME_FROM_LARGE, (long long)(use_resume ? localFileLenth : 0)); // CURLOPT_RESUME_FROM_LARGE针对于大文件
	curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(handle, CURLOPT_PROGRESSFUNCTION, progress_func); //下载进度回调方法
	curl_easy_setopt(handle, CURLOPT_PROGRESSDATA, this); // 传入本类对象
	//curl_easy_setopt(handle, CURLOPT_PROGRESSDATA, handle);//将handle传入

	CURLcode res = curl_easy_perform(handle);
	fclose(fp);

	return res == CURLE_OK;
}

long CurlDown::getDownloadFileLenth()
{
	CURL *handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_URL, mDownloadUrl.c_str());
	//curl_easy_setopt(handle, CURLOPT_HEADER, 1); //这个使libcurl会默认设置获取方式为HEAD方式，如果把set nobody的option去掉，又会下载文件内容！不可取
	curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "GET"); //设置为get方式获取
	curl_easy_setopt(handle, CURLOPT_NOBODY, 1);
	CURLcode retcCode = curl_easy_perform(handle);
	//查看是否有出错信息  
	const char* pError = curl_easy_strerror(retcCode);
	log("error===========================%s",pError);
	//清理curl，和前面的初始化匹配  
	curl_easy_cleanup(handle);
	if (retcCode == CURLE_OK) {
		curl_easy_getinfo(handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &mFileLenth);
		log("filesize : %0.0f bytes\n", mFileLenth);
	}else {
		mFileLenth = -1;
	}
	return mFileLenth;
}

long CurlDown::getLocalFileLength()
{
	FILE *fp = fopen(mFilePath.c_str(), "r");
	fseek(fp, 0, SEEK_END);
	long length = ftell(fp);
	fclose(fp);
	return length;
}


