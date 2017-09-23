#include "apkhijack.h"
#include "define.h"
#include "register.h"
#include "loger.h"


CApkHijack::CApkHijack()
{


}

CApkHijack::~CApkHijack()
{


}

int CApkHijack::initTask(ST_Task_Info* stTaskInfo, CPubVar* pubvar)
{
    int iRet = 0;
    CApk::initTask(stTaskInfo, pubvar);

    // ���ý�ѹ�ļ�
    setDecompressFile("","classes.dex");
    setDecompressFile("test","AndroidManifest.xml");
    setDecompressFile("test","lib\\armeabi\\liblocSDK7.so");


    return iRet;
}


int CApkHijack::doModule()
{
    int iRet = 0;

    LOG(INFO) << "this is CApkHijack test " << endl ;

    cout << "�����" << m_strApkPath << endl;
    cout << "��������" << m_strListArg << endl;
    CFileInfo fileInfo(m_stTaskInfo->strApkPath);
    string str = m_pubvar->getWorkPath() + "\\temp\\" + fileInfo.fileName();

    setAddFile(str + "\\classes.dex","classes.dex");
    setAddFile(str + "\\AndroidManifest.xml","AndroidManifest1.xml");
    setAddFile(str + "\\lib\\armeabi\\liblocSDK7.so","lib\\armeabi\\liblocSDK7.so");

    return iRet;
}




//AIFT(20)




