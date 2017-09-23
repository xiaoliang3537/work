#ifndef APK_H
#define APK_H

#ifdef WIN32
# ifndef I_OS_WINDOWS
# define I_OS_WINDOWS
# endif
#elif unix
# ifndef I_OS_LINUX
# define I_OS_LINUX
# endif
#endif

#include <iostream>
#include <string>
#include <string>
#include <list>
#include "stringlist.h"
#include "pubvar.h"
#include "define.h"
#include "loger.h"
#include "fileinfo.h"



#ifdef I_OS_WINDOWS
#include <direct.h>
#else
#include <unistd.h>
#endif

#ifdef I_OS_WINDOWS
#define I_FILE_TYPE _finddata_t
#else
#define I_FILE_TYPE dirent
#endif

extern loger *g_loger;

using namespace std;

class CApk
{
public:
    CApk();
    virtual ~CApk();

    virtual int initTask(ST_Task_Info* stTaskInfo, CPubVar* pubva);
public:
    /**
     * @brief start ����ģ��������
     * @param pubvar ����������������������Ҫ��һЩ���ݣ������ǰģ�鲻��Ҫ�ⲿ���ݴ��������
     * @return 0 ���� != �쳣
     */
    int start( );

    /**
     * @brief setApkInfo �ṩ����ģ�������������ݽӿ�
     * @param apkInfo �������ݵĵ�ַ
     */
    virtual void setApkInfo(void* apkInfo){return;}

    /**
     * @brief ִ��ָ�����
     */
    int execCmd(std::string strCmd);
protected:
    /**
     * @brief checkParam ��μ�飬�������Ƿ����Ϸ�
     * @param strList �������
     * @return 0 ���� != �쳣
     */
    virtual int checkParam(CStringList& strList);
    /**
     * @brief init ������ν���apk����Ϣ��ʼ������ Ŀǰ�����ѹ�ļ��б� �������ļ�
     * @return 0 ���� != �쳣
     */
    virtual int initApk();
    /**
     * @brief doModule ����ӿڣ�������ģ��ҵ���������,���������ʵ�ָýӿڽ���ҵ����
     * @return 0 ���� != �쳣
     */
    virtual int doModule() = 0;

    /**
     * @brief doEnd;
     * @return 0 ���� != �쳣
     */
    virtual int doEnd();

    /**
     * @brief ��ѹ�ļ� �����б��е��ļ���ָ��Ŀ¼(m_strTempPath+�ļ����·��)
     * @param strListFile ����ѹ�ļ��б�
     * @return 0 �ɹ� > 0 ��Ӧ�ڼ����ļ�����ʧ�� < 0 �쳣
     */
    virtual int zipDeCompress(CStringList strListFile);

    /**
     * @brief decompileDexFile ������dex�ļ�
     * @return 0 ���� != �쳣
     */
    virtual int decompileDexFile();
    /**
     * @brief backPackDexFile �ر�dex�ļ�
     * @return 0 ���� != �쳣
     */
    virtual int backPackDexFile();
    /**
     * @brief backPack �ذ�
     * @return 0 ���� != �쳣
     */
    virtual int backPack();

protected:
    /**
     * @brief zipDeCompress ��ѹ�ļ�
     * @return 0 �ɹ� < 0 �쳣 > 0 ��������
     */
    int zipDeCompress();

    /**
     * @brief zipCompress ѹ���ļ�
     * @param strZipFile ������zip�ļ�
     * @param strFile ����ӵ�ѹ���ļ�
     * @return
     */
    int zipCompress(string  strZipFile, string  strFile);

    /**
     * @brief zipRemoveFile ɾ���ļ�
     * @param strZipFile ������zip�ļ�
     * @param strFile ��ɾ�����ļ� ���·��
     * @return
     */
    int zipRemoveFile(string  strZipFile,string  strFile);

    /**
     * @brief zipAddFile ���ѹ���ļ�
     * @param strZipFile ������zip�ļ�
     * @param strFile ������ļ�·��
     * @param strFileInZip  ������ļ���zip�е�·��
     * @return
     */
    int zipAddFile(string &strZipFile, string &strFile, string &strFileInZip);

    /**
     * @brief xmlPrinter �Խ�ѹ��xml�ļ����н���
     * @param strXml ������xml�ļ�
     * @param strXmlTag �����ļ����·��
     * @return
     */
    int xmlPrinter(string  strXml, string  strXmlTag);
    /**
     * @brief xmlModefy ���Ľ�ѹ���xml�ļ�
     * @param strXml ������xml�ļ�
     * @param strParam �������
     * @return
     */
    int xmlModefy(string  strXml, string  strParam);
    int smali();
    int backSmali();
    bool checkFileExsit(const char* file);
    // ��ȡĿ¼�������ļ�
    int getFileInDir(std::list<I_FILE_TYPE*> &listFileInfo);
    int clearFileInfo(std::list<I_FILE_TYPE*> &listFileInfo);
protected:
    // ��ѹ ѹ���ļ��б�����
    int setAddFile(string strFilePath, string strPathInZip);
    int setDecompressFile(string strFilePath, string strPathInZip);
    int setDecompressDir(string strFilePath, string strPathInZip);
    // ���ý��лر���ļ�
    int setBackSmaliFile(string strFilePath, string strPathInZip);
    // ���ûذ�����Ҫɾ�����ļ�
    int setPackDeleteFile(string strFilePath, string strPathInZip);
    // ���������н��
    int checkOutPut(string &strOut, string su = "SUCESS");
protected:
    int                                         m_iProgramId;               // ģ��ID
    CPubVar*                                    m_pubvar;                   // ϵͳ��������
    CConfig*                                    m_config;                   // ������Ϣ
    int                                         m_iParamID;                 // ҵ����ID ��Ծ���ҵ�����
    int                                         m_iApkCount;
    int                                         m_iProcessRet;              // ������ 0 ���� !=0�쳣
    int                                         m_iProcessType;             // ִ������ 1 ��ֱͨ�ӵ��� 2 ģ�龭����ֺ����
    string                                      m_strWorkPath;              // ����·��
    string                                      m_strArg;                   // ���ܲ���
    string                                      m_strApkPath;               // ������apk·��
    string                                      m_strTempApkPath;           // ��ʱapk·��
    string                                      m_strTempPath;
    string                                      m_strListArg;               // ��������
    string                                      m_strFilaName;              // apk �ļ���
    string                                      m_strTempFilaName;          // apk ����ļ�����
    CStringList                                 m_strFileList;              // ����ѹ�ļ��б�
    CStringList                                 m_strListDex;               // dex�ļ��б� ��Ҫ��ѹdex�ļ� �Ż����
    CStringList                                 m_strListAdd;               // �����ļ��б�
    ST_Module*                                  m_stModuleInfo;             // ģ����Ϣ
    ST_Task_Info*                               m_stTaskInfo;               // ������Ϣ
protected:
    typedef struct
    {
        string strFilePath;
        string strPathInZip;
    } ST_FIleAddInfo;
    list<ST_FIleAddInfo>                        m_listStAddFile;            // ����ӵ��ļ��б�
    list<ST_FIleAddInfo>                        m_listStDecompressFile;     // ����ѹ�ļ��б�
    list<ST_FIleAddInfo>                        m_listStDecompressDir;      // ����ѹ�ļ��� ��ѹ���ļ���
    list<ST_FIleAddInfo>                        m_listStDexFile;            // Dex�ļ��б�
    list<ST_FIleAddInfo>                        m_listStDeleteFile;         // ��ɾ�����ļ��б�

    std::map<std::string, std::string>          m_mapDexToSmali;            // �������ļ�Ŀ¼
    std::map<std::string, std::string>          m_mapSmaliToDex;            // �������ļ�Ŀ¼
};

#endif      // APK_H
