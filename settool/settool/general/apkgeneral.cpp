#include "apkgeneral.h"
#include "define.h"
#include "register.h"
#include "loger.h"
#include "Utils.h"
#include "FileUtils.h"

CApkGeneral::CApkGeneral()
{


}

CApkGeneral::~CApkGeneral()
{
    this->clearFileInfo(m_listFileInfo);
}

int CApkGeneral::initTask(ST_Task_Info* stTaskInfo, CPubVar* pubvar)
{
    int iRet = 0;
    iRet = CApk::initTask(stTaskInfo, pubvar);
    if(0 != iRet )
    {
        LOG(ERROR) << "��ʼ��ʧ��" << endl;
        return iRet;
    }
    // ���ý�ѹ�ļ�
    setDecompressFile("","classes*.dex");
    setDecompressFile("test","AndroidManifest.xml");

    std::list<ST_Param_Module*>::iterator it = m_stTaskInfo->listParamModule.begin();
    for(; it != m_stTaskInfo->listParamModule.end(); it++ )
    {
        ST_Param_Module* module = *it;
        if( 123 == module->iModuleID )
        {
            setDecompressDir("", "assets" );
            setDecompressDir("", "lib" );
            break;
        }
    }
    return iRet;
}


int CApkGeneral::doModule()
{
    int iRet = 0;

    LOG(INFO) << "this is CApkGeneral test " << endl ;

    if( m_iProcessType == 1 )        // ֱ�ӵ���
    {
        list<ST_Param_Module*>::iterator iter = m_stTaskInfo->listParamModule.begin();
        for(; iter != m_stTaskInfo->listParamModule.end(); iter++)
        {
            ST_Param_Module* param = *iter;
            ST_Module* module = m_pubvar->m_config->getModuleInfo( param->iModuleID );
            if(NULL == module )
            {
                LOG(ERROR) << tr("��ǰ����ģ���쳣���޷���λ��ģ�� : ") << param->iModuleID << endl;
                return -1;
            }

#ifdef _DEBUG
            LOG(INFO) << tr("ִ������: ") << param->iModuleID << endl;
            LOG(INFO) << tr("ָ��: =  ") << param->strListParam << endl;
#endif

            string strOutPut;
            mysystem(param->strListParam.c_str(), strOutPut);
            if(strOutPut.find( module->strCheck ) == -1 )
            {
                iRet = -1;
                LOG(ERROR) << tr("ģ�鴦���쳣������δ��λ���ɹ���־ : ") << module->strCheck << endl;
                return iRet;
            }
        }
    }
    else
    {
        std::list<ST_Param_Module*>::iterator it = m_stTaskInfo->listParamModule.begin();
        for( ; it != m_stTaskInfo->listParamModule.end(); it++ )
        {
            ST_Param_Module* moduleParam = *it;
            ST_Module* module = m_pubvar->m_config->getModuleInfo( moduleParam->iModuleID );
            CFileInfo fileInfo(m_stTaskInfo->strApkPath);
            string str = m_pubvar->getWorkPath() + "\\temp\\" + fileInfo.fileName();

            if( 0 != m_pubvar->getGernealToolList(m_strList) )
            {
                LOG(ERROR) << tr("��ȡ�����б����") << endl;
                return -1;
            }

//            string tempdir = "tempdir=" + m_strTempPath;
//            m_strList.append(tempdir);
            m_strConfigFile = m_strWorkPath + _SYMBOL_PATH_ + "temp" + _SYMBOL_PATH_ + m_strTempFilaName + ".ini";
            // �����ļ�����ֵ
            if( 0 != this->writeToFile(m_strConfigFile, m_strList) )
            {
                LOG(ERROR) << tr("�����������ô���") << endl;
                return -1;
            }

            // ����ģ�鴦��
            string strOutPut;
            string strparam = moduleParam->strListParam + "-f \"" + m_strTempPath + "\" -tf \"" + m_strConfigFile + "\"";
#ifdef _DEBUG
            LOG(INFO) << "strCmd = " << strparam << endl;
#endif
//            mysystem( strparam.c_str(), strOutPut);
//            if(strOutPut.find( module->strCheck ) == -1 )
//            {
//                iRet = -1;
//                LOG(ERROR) << tr("ģ�鴦���쳣������δ��λ���ɹ���־ : ") << module->strCheck << endl;
//                return iRet;
//            }

            // ����м�����
            m_listStDexFile.clear();

            // ���ûر�ذ���Ϣ
            setBackInfo();
#ifdef _DEBUG
            list<ST_FIleAddInfo>::iterator itA = m_listStAddFile.begin();
            for(; itA != m_listStAddFile.end(); itA++ )
            {
                ST_FIleAddInfo info = *itA;
                LOG(INFO) << info.strFilePath << "  "  << info.strPathInZip << endl;
            }
#endif

        }
        return iRet;
    }

}

int CApkGeneral::doEnd()
{
    int iRet = 0;
    // ɾ����ʱ�ļ�
    FileUtils::DeleteDir(m_strTempPath);
    FileUtils::DelFileW(m_strConfigFile);
    return iRet;
}

int CApkGeneral::setBackInfo()
{
    int iRet = 0;

    // ���ô�ѹ���ļ��е�ʱ�����ԭ���ļ�¼
    m_listStDecompressDir.clear();

    // ��ȡĿ¼�� class �ļ���
    if( 0 != this->getFileInDir(m_listFileInfo) )
    {
        LOG(ERROR) << tr("��ȡ�ļ��б�ʧ��") << endl;
        return -1;
    }

    std::list<I_FILE_TYPE*>::iterator it = m_listFileInfo.begin();
    for(; it != m_listFileInfo.end(); it++ )
    {
#ifdef I_OS_WINDOWS
        // ���ûر�dex�ļ�
        I_FILE_TYPE* info = *it;

        if(info->attrib &  DIR_ATTRIB /*_A_SUBDIR*/)
        {
            if(0 == strcmp(info->name, ".") || 0 == strcmp(info->name, "..") )
                continue;

            // ���ƥ�䵽class�ļ��� ��Ϊ��Ҫ���лر��dex�ļ��� ����Ϊ��ѹ���ļ�
            if( NULL != strstr(info->name, "classes") )
            {
                std::string p;
                std::string strFilePath;
                std::string strFileName;
                strFilePath.append(m_strTempPath).append(_SYMBOL_PATH_).append(info->name).append( ".dex" );
                strFileName.append(info->name).append(".dex");

                setBackSmaliFile(p.assign(m_strTempPath).append(_SYMBOL_PATH_).append(info->name), strFilePath );
                setAddFile( p.assign(m_strTempPath).append(_SYMBOL_PATH_).append(info->name)+".dex", strFileName);
            }
            else
            {
                std::string p;
                setDecompressDir( p.assign(m_strTempPath).append(_SYMBOL_PATH_).append(info->name),info->name );
            }
        }
        else if(info->attrib & FILE_ATTRIB /*_A_ARCH*/ )
        {
            if(strlen(info->name) > 4 )
            {
                if( 0 == strcmp(info->name + strlen(info->name) - 4, ".dex") )
                {
                    continue;
                }
            }

            std::string p;
            setAddFile( p.assign(m_strTempPath).append(_SYMBOL_PATH_).append(info->name),info->name );
        }
#else
#endif
    }
    return iRet;
}

int CApkGeneral::writeToFile(std::string &strFileName, CStringList &strList)
{
    int iRet = 0;
    FILE* f = fopen(strFileName.c_str(), "wb");
    if(NULL == f )
        return -1;
    for(int i = 0; i < strList.length(); i++ )
    {
        fprintf(f, "%s%s", strList.at(i).c_str(), I_SYMBOL_ENTER);
    }
    if(f)
        fclose(f);
    return iRet;
}



