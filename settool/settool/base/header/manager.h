#ifndef MANAGER_H
#define MANAGER_H

#ifdef WIN32
# ifndef I_OS_WINDOWS
# define I_OS_WINDOWS
# endif
#elif unix
# ifndef I_OS_LINUX
# define I_OS_LINUX
# endif
#endif

#include <string>
#include "managerthread.h"
#include "config.h"
#include "apk.h"
#include "register.h"
#include "config.h"
#include "pubvar.h"
#include "stringlist.h"
#include "library.h"


class CManager
{
public:
    CManager();
    ~CManager();

public:
    int getRunStatus(){return m_iRunStatus;}
    /**
     * @brief init ��ʼ������ģ�� �����������ã�����ҵ��ģ�飬��ʼ����������3������
     * @return 0 ���� != 0 �쳣
     */
    int init();

    /**
     * @brief execCmd ͬ������ִ��ָ��
     * @param strArg ָ����Ϣ ���ն��������������ʽ����
     * @param iModuleId ģ���� Ҫ���õľ���ҵ��ģ�飬��Ҫ��config.xml���ö�Ӧ
     * @return 0 ���� != 0 �쳣
     */
    int execCmd(int argc, char* argv[]);
    /**
     * @brief execCmdEx �첽������ִ��ָ�� ���������Ϣ�Լ���� ͨ�������ź� onStatusChanged taskFinished ��ȡ
     * @param strArg ָ����Ϣ ���ն��������������ʽ����
     * @param iModuleId ģ���� Ҫ���õľ���ҵ��ģ�飬��Ҫ��config.xml���ö�Ӧ
     * @return 0 ���� != 0 �쳣
     */
    int execCmdEx(int argc, char* argv[]);

    // ��ȡģ���б�
//    QList<ST_Module*> getModuleList()
//    {
//        if(NULL != m_conifg)
//            return m_conifg->getModuleList();
//        return QList<ST_Module*>();
//    }
//    // ��ȡģ���Ӧ�����б�
//    std::list<ST_Module_Param*> getModuleParamList(int iModuleId)
//    {
//        if(NULL != m_conifg)
//            return m_conifg->getModuleParamList(iModuleId);
//        return std::list<ST_Module_Param*>();
//    }
protected :
    int onTaskFinished(int id);
    int onApkFinished(int id);
    int onTimer();
protected:
    int loadModule(int id);
    CApk* loadModuleInner(int id);
    int unLoadModule(int id);

    // ����Բ����Ƿ����
    ST_Module_Param_Sub* checkSubParamExsit(string strParam, list<ST_Module_Param_Sub*> &list);
    bool checkParamDepend(string &strParam, list<ST_Module_Param*> &listParam);
private:
    void addModuleId(list<int> &listModuleID, int moduleID);
private:
    // ���󹤳�
    CObjectFactory *                                m_obj;
    CConfig*                                        m_conifg;
    map<int, CLibrary*>                             m_mapLib;
    CPubVar*                                        m_pubvar;
    int                                             m_iModuleId;
    int                                             m_iRunStatus;   //0 ���� 1 ��æ

    CManagerThread*                                 m_managerThread;
};

#endif //MANAGER_H
