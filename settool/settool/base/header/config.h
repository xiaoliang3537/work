#ifndef CONFIG_H
#define CONFIG_H

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
#include <iostream>
#include <map>
#include <list>
#include "register.h"
#include "stringlist.h"

using namespace std;

struct ST_Module;
struct ST_Module_Param;

class CConfig
{
public:
    CConfig();
    ~CConfig();

public:
    int init();

    // ͨ��ģ��id����ģ����Ϣ
    ST_Module* getModuleInfo(int moduleId);

    // ͨ���������Ҷ�Ӧģ��
    ST_Module* getParamModule(string strParam);

    // ��ȡģ�������Ӧ������Ϣ
    ST_Module_Param* getParamInModule(int moduleId, string strParam);

    // ��ȡͨ��ģ�������б�
    int getGernealToolList(CStringList& strlist);

    // ��ӡ������Ϣ
    int showHelp();
    std::map <std::string, int>                                       m_mapParamModule;               // ����ģ����ձ�
    std::map<std::string, ST_Module_Param*>                           m_mapParam;                     // ������
private:
    int loadConfig();
private:
    std::map<int, ST_Module*>                                         m_mapModule;                    // ģ���б�
    std::map<int, ST_Module*>                                         m_mapGeneralModule;             // ģ���б�
};

#endif // CONFIG_H
