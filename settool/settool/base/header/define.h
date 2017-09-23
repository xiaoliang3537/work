#ifndef DEFINE_H
#define DEFINE_H

#include <string>
#include <list>
#include <vector>
#include "stringlist.h"

#ifdef WIN32
# ifndef I_OS_WINDOWS
# define I_OS_WINDOWS
# endif
#elif unix
# ifndef I_OS_LINUX
# define I_OS_LINUX
# endif
#endif


using namespace std;

#define CONN(a,b) a##b
#ifdef I_OS_WINDOWS
#define EXPORT_DEF(name) extern "C" __declspec(dllexport) void* getInstance##name()
#else
#define EXPORT_DEF(name) extern "C" void* getInstance##name()
#endif


// �Ӳ����б�
struct ST_Module_Param_Sub
{
    ST_Module_Param_Sub()
    {
        iValueNeed = -1;
    }

    string                              strParamName;               // ��������
    string                              strParamNameL;              // ��������1 ʵ�����
    string                              strParamDesc;               // ��������
    string                              strParamValue;              // ����ֵ
    string                              strParamDepend;             // ��������
    unsigned int                        iValueNeed;                 // �Ƿ���������
};

// �����б�
struct ST_Module_Param
{
    ST_Module_Param()
    {
        bSubParam = false;
        iDefault = -1;
        iModuleId = -1;
        iValueNeed = -1;
        iAttributes = -1;
    }
    bool                                bSubParam;                  // �Ƿ����Ӳ���
    string                              strParamName;               // ��������
    string                              strParamNameL;              // ��������1 ʵ�����
    string                              strParamDesc;               // ��������
    string                              strParamValue;              // ����ֵ
    string                              strParamDepend;             // ��������
    list<ST_Module_Param_Sub*>          listSubParam;               // �Ӳ����б�
    unsigned int                        iDefault;                   // Ĭ�ϲ���
    int                                 iModuleId;                  // ������Ӧģ����
    unsigned int                        iValueNeed;                 // �Ƿ���������
    unsigned int                        iAttributes;                // �������� 0 ͨ�� 1 ר��
};

// ģ��ṹ��Ϣ
struct ST_Module
{
    ST_Module()
    {
        iModuleID = -1;
        iType = -1;
        iToolType = -1;
    }

    int                                 iModuleID;                  // ģ��ID
    int                                 iType;                      // ģ������ 0 Ĭ���Զ���ģ�� 1 �ⲿjava��ִ���ļ� 2 ��ִ���ļ� 3 Ĭ�Ͼ���������Զ���ģ��
    int                                 iToolType;                  // �������� 1 �������� 0 ������������
    int                                 iAvailable;                 // ���ñ�ʶ 0 ������ 1 ��������(so exe jar ����������ģ��) 2 ����ֱ�ӱ�����(ֻ�ǻ������)
    string                              strModuleName;              // ģ������
    string                              strSymbol;                  // ģ���ʶ
    string                              strToolName;                // ����·�� ����·��Ϊ������Ӧ�ó���Ŀ¼
    string                              strToolGeneralName;         // ͨ�ò�����������
    string                              strListParam;               // �����б� (ΪlistModuleParam��������)
    list<ST_Module_Param*>              listModuleParam;            // �����б�
    string                              strHelp;                    // ʵ������
    string                              strCheck;                   // ִ�гɹ�У���ַ���
    string                              strVersion;                 // �汾��
};

// ����ģ����Ϣ
struct ST_Param_Module
{
    int                                 iModuleID;                  // ģ��ID
    int                                 iType;                      // ģ������ 0 Ĭ���Զ���ģ�� 1 �ⲿjava��ִ���ļ� 2 ��ִ���ļ�
    std::string                         strParam;                   // ��������ѡ��
    std::string                         strToolName;                // ����·��
    std::string                         strListParam;               // �����б�
    list<ST_Module_Param*>              listParam;                  // ������ϸ��Ϣ
};

// ��������б�
struct ST_Task_Info
{
    ST_Task_Info()
    {
        iTaskId = -1;
        iBlock = 0;
        vObject = NULL;
    }
    int                                 iTaskId;
    int                                 iBlock;                     // �Ƿ�����ִ�� 0 ��  1 ��
    void*                               vObject;                    // �������
    CStringList                         strListParam;
    string                              strApkPath;                 // ����������
    std::list<ST_Param_Module*>         listParamModule;            // �������� ��������б�
    CStringList                         strListParamCommon;         // ͨ�ò����б�
};


#ifndef MAX_DELAY_TIME
#define MAX_DELAY_TIME                      100*1000*5
#endif

# ifndef I_XML_PRINTER
# define I_XML_PRINTER "AXMLPrinter2.jar"
# endif

# ifndef I_SMALI
# define I_SMALI "smali-2.1.0.jar"
# endif

# ifndef I_SMALI_BACK
# define I_SMALI_BACK "baksmali-2.1.0.jar"
# endif


# ifndef I_XML_MODIFY
# define I_XML_MODIFY "AxmlModify.exe"
# endif

# ifndef I_ZIP_FILE
# define I_ZIP_FILE "myzip.exe"
# endif

#ifdef I_OS_WINDOWS

# ifndef I_DLL_SYMBOL
# define I_DLL_SYMBOL ".dll"
# endif

# ifndef I_SYMBOL_ENTER
# define I_SYMBOL_ENTER "\r\n"
# endif

# ifndef _SYMBOL_PATH_
# define _SYMBOL_PATH_ "\\"
# endif

# ifndef EXTRA_NAME
# define EXTRA_NAME ""
# endif

# ifndef FILE_ATTRIB
# define FILE_ATTRIB 0x20
# endif

# ifndef DIR_ATTRIB
# define DIR_ATTRIB 0x10
# endif

#else
# ifndef I_DLL_SYMBOL
# define I_DLL_SYMBOL ".so"
# endif

# ifndef _SYMBOL_PATH_
# define _SYMBOL_PATH_ "/"
# endif

# ifndef I_SYMBOL_ENTER
# define I_SYMBOL_ENTER "\n"
# endif

# ifndef EXTRA_NAME
# define EXTRA_NAME "lib"
# endif

# ifndef FILE_ATTRIB
# define FILE_ATTRIB 0x08
# endif

# ifndef DIR_ATTRIB
# define DIR_ATTRIB 0x04
# endif

#endif

#endif



