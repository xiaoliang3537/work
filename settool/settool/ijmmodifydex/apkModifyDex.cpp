﻿#include "apkModifyDex.h"
#include "Markup.h"
#include <stdlib.h>
#include <fstream>
#include <algorithm>

using namespace std;

ModifyDex::ModifyDex()
{
    m_sTempFilePath.clear();
    m_sExecFilePath.clear();
    m_sSrcsFilePath.clear();
    m_strNewApkPath.clear();
    m_sApkName.clear();
}

int ModifyDex::initTask(ST_Task_Info* stTaskInfo, CPubVar* pubvar)
{
    int iRet = 0;

    iRet = CApk::initTask(stTaskInfo, pubvar);
    if(0 != iRet )
    {
        LOG(ERROR) << "初始化失败" << endl;
        return iRet;
    }
    LOG(INFO) << "ModifyDex" << " init" << endl;
    if(!GetWorkPath())
    {
        LOG(INFO) << "not found workpath" << std::endl;
        return 1;
    }

    setDecompressFile(m_strTempPath,"classes*.dex");
    setDecompressFile(m_strTempPath,"AndroidManifest.xml");

    // -n nMaxValue -c nCopytCount
    if( m_stTaskInfo->listParamModule.size() == 0 )
    {
        LOG(ERROR) << "参数信息错误" << endl;
        return -1;
    }
    ST_Param_Module* info = *(m_stTaskInfo->listParamModule.begin());
    if( info->listParam.size() == 0 )
    {
        LOG(ERROR) << "参数列表为空" << endl;
        return -1;
    }
    list<ST_Module_Param*>::iterator it = info->listParam.begin();
    for(; it != info->listParam.end(); it++ )
    {
        ST_Module_Param* param = *it;
        if( param->strParamNameL.compare("-n") == 0 )
        {
            m_nMaxValue = atoi(param->strParamValue.c_str());
        }
        if( param->strParamNameL.compare("-c") == 0 )
        {
            m_nCopytCount = atoi(param->strParamValue.c_str());
        }
        if( param->strParamNameL.compare("-r") == 0 )
        {

        }
    }
#ifdef _DEBUG
    LOG(INFO) << " -n " << m_nMaxValue << " -c " << m_nCopytCount << endl;
#endif


    InitVariable();


    m_strNewApkPath.clear();
#ifdef  _WIN32
    m_strNewApkPath//.append(m_sExecFilePath).append(_SYMBOL_PATH_)
            .append(FileUtils::ExtractFileRemoveExt(m_strApkPath)).append("_New.apk");
#else
    m_strNewApkPath.append(FileUtils::ExtractFileRemoveExt(m_strApkPath)).append("_New.zip");
#endif

    m_strTempApkPath = m_strNewApkPath;

    return iRet;
}

int ModifyDex::doModule()
{
    int iRet = 0;
    bool bRet = true;
    bool bR = false;
    do
    {
        OUT_MAX_DEXCOUNT vcDexInfo;
        int nsize = obianDexheader(vcDexInfo, m_nMaxValue);

        if( m_nMaxValue > nsize )
        {
            LOG(INFO) << "you input \'-n\' parameter is too large!" << std::endl;
            bRet = false;
            return -1;
        }
        else if(0 >= nsize)
        {
            LOG(INFO) << "obianDexheader is Failed! parse dex file is failed!" << std::endl;
            bRet = false;
            break;
        }
        else
        {
            int nfileCount = vcDexInfo.size();
            if(nfileCount == 1)
            {
                LOG(INFO) << "The APK file is not multiDex APK!";
                LOG(INFO) << "method number is :" << nsize << std::endl;

                if(FileUtils::DirExists(m_strNewApkPath))
                {
                    FileUtils::DelFileW(m_strNewApkPath);
                }
                bRet = false;
                break;
            }

            FILES_CACHE_LIST vcOptList;
            if(!beginParseDex(vcDexInfo, vcOptList))
            {
                LOG(INFO) << "beginParseDex is Failed!" << std::endl;
                bRet = false;
                break;
            }

            int iCopyNumber = 0;
            if(!ModifyDexFile(vcOptList, m_nCopytCount, nfileCount, iCopyNumber))
            {
                LOG(INFO) << "ModifyDexFile is Failed!" << std::endl;
                bRet = false;
                break;
            }

            // 清除中间数据
            m_listStDexFile.clear();
            // 删除文件支持模糊匹配
            setPackDeleteFile("", "META-INF/*.SF");
            setPackDeleteFile("", "META-INF/*.MF");
            setPackDeleteFile("", "META-INF/*.DSA");
            setPackDeleteFile("", "META-INF/*.SF");

            // 设置回编回包信息
            setBackInfo();
        }

    }while(0);

    return iRet;
}

int ModifyDex::setBackInfo()
{
    int iRet = 0;
    // 从缓存目录寻找需要回包的文件
    // 获取目录 class 文件
    if( 0 != this->getFileInDir(m_listFileInfo) )
    {
        LOG(ERROR) << tr("获取文件列表失败") << endl;
        return -1;
    }

    std::list<I_FILE_TYPE*>::iterator it = m_listFileInfo.begin();
    for(; it != m_listFileInfo.end(); it++ )
    {
#ifdef I_OS_WINDOWS
        // 设置回编dex文件
        I_FILE_TYPE* info = *it;
        if(info->attrib &  DIR_ATTRIB/*_A_SUBDIR*/)
        {
            if(0 == strcmp(info->name, ".") || 0 == strcmp(info->name, "..") )
                continue;

            std::string p;
            std::string strFilePath;
            std::string strFileName;
            strFilePath.append(m_strTempPath).append(_SYMBOL_PATH_).append(info->name).append( ".dex" );
            strFileName.append(info->name).append(".dex");

            setBackSmaliFile(p.assign(m_strTempPath).append(_SYMBOL_PATH_).append(info->name), strFilePath );
            setAddFile( p.assign(m_strTempPath).append(_SYMBOL_PATH_).append(info->name)+".dex", strFileName);
        }
        else if(info->attrib & FILE_ATTRIB /*_A_ARCH*/ )
        {
            if( 0 == strcmp(info->name + strlen(info->name) - 4, ".dex") )
            {
                continue;
            }

            std::string p;
            setAddFile( p.assign(m_strTempPath).append(_SYMBOL_PATH_).append(info->name),info->name );
        }
#else
        // 设置回编dex文件
        I_FILE_TYPE* info = *it;
        if(info->d_type &  DIR_ATTRIB/*_A_SUBDIR*/)
        {
            if(0 == strcmp(info->d_name, ".") || 0 == strcmp(info->d_name, "..") )
                continue;

            std::string p;
            std::string strFilePath;
            std::string strFileName;
            strFilePath.append(m_strTempPath).append(_SYMBOL_PATH_).append(info->d_name).append( ".dex" );
            strFileName.append(info->d_name).append(".dex");

            setBackSmaliFile(p.assign(m_strTempPath).append(_SYMBOL_PATH_).append(info->d_name), strFilePath );
            setAddFile( p.assign(m_strTempPath).append(_SYMBOL_PATH_).append(info->d_name)+".dex", strFileName);
        }
        else if(info->d_type & FILE_ATTRIB /*_A_ARCH*/ )
        {
            if( 0 == strcmp(info->d_name + strlen(info->d_name) - 4, ".dex") )
            {
                continue;
            }

            std::string p;
            setAddFile( p.assign(m_strTempPath).append(_SYMBOL_PATH_).append(info->d_name),info->d_name );
        }
#endif
    }
    return iRet;
}

bool ModifyDex::GetWorkPath()
{
    char cBuf[256] = { 0 };

#ifdef  _WIN32
    GetModuleFileName(NULL, cBuf, 256);
#else
    FileUtils::GetModuleFileName(cBuf, 256);
#endif

    char pFind[256] = {0};
    strcpy(pFind, cBuf);
    if (!pFind)
        return false;

#ifdef _WIN32
    m_sExecFilePath = FileUtils::ExtractFileDir(cBuf);
    LOG(INFO) << "work path: " << m_sExecFilePath << endl;
#else
    m_sExecFilePath = cBuf;
#endif

    //LOG(INFO) << "m_sExecFilePath string : " << m_sExecFilePath << std::endl;

    return true;
}

bool ModifyDex::ModifyApk(const char* pchApkName, unsigned int nMaxValue, int nCopytCount, bool bR)
{
    bool bRet = true;
    do
    {
        if(!beginDeApk(pchApkName))
        {
            LOG(INFO) << "beginDeApk is Failed!" << std::endl;
            bRet = false;
            break;
        }

        OUT_MAX_DEXCOUNT vcDexInfo;
        int nsize = obianDexheader(vcDexInfo, nMaxValue);

        if(nMaxValue > nsize)
        {
            LOG(INFO) << "you input \'-n\' parameter is too large!" << std::endl;
            bRet = false;
            break;
        }
        else if(0 >= nsize)
        {
            LOG(INFO) << "obianDexheader is Failed! parse dex file is failed!" << std::endl;
            bRet = false;
            break;
        }
        else
        {
            int nfileCount = vcDexInfo.size();
            if(nfileCount == 1)
            {
                LOG(INFO) << "The APK file is not multiDex APK!";
                LOG(INFO) << "method number is :" << nsize << std::endl;

                if(FileUtils::DirExists(m_strNewApkPath))
                {
                    FileUtils::DelFileW(m_strNewApkPath);
                }
                bRet = false;
                break;
            }

            FILES_CACHE_LIST vcOptList;
            if(!beginParseDex(vcDexInfo, vcOptList))
            {
                LOG(INFO) << "beginParseDex is Failed!" << std::endl;
                bRet = false;
                break;
            }

            int iCopyNumber = 0;
            if(!ModifyDexFile(vcOptList, nCopytCount, nfileCount, iCopyNumber))
            {
                LOG(INFO) << "ModifyDexFile is Failed!" << std::endl;
                bRet = false;
                break;
            }

            if(!startBackDex(vcOptList, iCopyNumber))
            {
                LOG(INFO) << "startBackDex is Failed!" << std::endl;
                bRet = false;
                break;
            }

            if(!startBackApk(vcDexInfo, vcOptList))
            {
                LOG(INFO) << "startBackApk is Failed!" << std::endl;
                bRet = false;
                break;
            }
        }

        if(bR)
        {
            if(FileUtils::DirExists(m_sSrcsFilePath))
            {
                FileUtils::DelFileW(m_sSrcsFilePath);

                if(!FileUtils::copyFile((char*)m_strNewApkPath.c_str(),
                                        (char*)m_sSrcsFilePath.c_str(), true))
                {
                    LOG(INFO) << "copy file is failed" << std::endl;
                    bRet = false;
                    break;
                }
            }
            LOG(INFO) << "success file dir: \n" << m_sSrcsFilePath << std::endl;
        }
        else
        {
            string strTemp = "";
            strTemp.append(FileUtils::ExtractFilePath(m_sSrcsFilePath))
                    .append(FileUtils::ExtractFileName(m_strNewApkPath));
            if(!FileUtils::DirExists(strTemp))
            {
                if(!FileUtils::copyFile((char*)m_strNewApkPath.c_str(), (char*)strTemp.c_str(), true))
                {
                    LOG(INFO) << "copy file is failed" << std::endl;
                    bRet = false;
                    break;
                }
            }
            else
            {
                if(!FileUtils::DelFileW(strTemp))
                {
                    LOG(INFO) << "Delete Apk file Failed!" << std::endl;
                }
                if(!FileUtils::copyFile((char*)m_strNewApkPath.c_str(), (char*)strTemp.c_str(), true))
                {
                    LOG(INFO) << "copy file is failed" << std::endl;
                    bRet = false;
                    break;
                }
            }
            LOG(INFO) << "success file dir: \n" << strTemp << std::endl;
        }

    }while(0);

    if(FileUtils::DirExists(m_strNewApkPath))
    {
        FileUtils::DelFileW(m_strNewApkPath);
    }

    if(FileUtils::DirExists(m_sTempFilePath))
    {
        FileUtils::DeleteDir(m_sTempFilePath);
    }

    return bRet;
}


bool ModifyDex::beginDeApk(const char* pApkFilePath)
{
    if(!pApkFilePath)
    {
        LOG(INFO) << "pApkFilePath is empty!" << std::endl;
        return false;
    }

    m_sSrcsFilePath.clear();
    m_sSrcsFilePath.append(pApkFilePath, strlen(pApkFilePath));

    if(!FileUtils::DirExists(m_sSrcsFilePath))
    {
        LOG(INFO) << "File path: " << pApkFilePath << " is not exist" << std::endl;
        LOG(INFO) << "File path: " << m_sSrcsFilePath << " is not exist" << std::endl;
        return false;
    }

    InitVariable();

    LOG(INFO) << "begin Decompie APK ...\r\n" << std::endl;

    std::string myZipPath = "";
    //myZipPath.append(m_sExecFilePath).append(_SYMBOL_PATH_).append(_myZip_);
    myZipPath = m_pubvar->getZipPath();

    std::string sCmd = "", strOutput = "";

    sCmd.append("\"").append(myZipPath).append("\" -s -z \"").append(m_sSrcsFilePath)
            .append("\" -p \"AndroidManifest.xml;classes*.dex\" -o \"").append(m_sTempFilePath).append("\"");

    LOG(INFO) << "sCmd:" << sCmd << std::endl;
    bool result = mysystem(sCmd.c_str() ,strOutput);
    if(!result)
    {
        LOG(INFO) << "执行命令异常! \r\n" << strOutput << std::endl;
        return false;
    }
    LOG(INFO) << strOutput << std::endl;

    m_strNewApkPath.clear();
#ifdef  _WIN32
    m_strNewApkPath.append(m_sExecFilePath).append(_SYMBOL_PATH_)
            .append(FileUtils::ExtractFileRemoveExt(m_sApkName)).append("_New.apk");

    LOG(INFO) << "new apk path=" << m_strNewApkPath << endl;
#else
    m_strNewApkPath.append(m_sExecFilePath).append(_SYMBOL_PATH_)
            .append(FileUtils::ExtractFileRemoveExt(m_sApkName)).append("_New.zip");
#endif
    if(!FileUtils::DirExists(m_strNewApkPath))
    {
        FileUtils::copyFile((char*)pApkFilePath, (char*)m_strNewApkPath.c_str(), true);
    }
    else
    {
        FileUtils::DelFileW(m_strNewApkPath);
        FileUtils::copyFile((char*)pApkFilePath, (char*)m_strNewApkPath.c_str(), true);
    }

    return true;
}

int ModifyDex::obianDexheader(OUT_MAX_DEXCOUNT &vcMethodCountOutMax, unsigned int nMaxCount)
{
    FILES_CACHE_LIST listFile_Dex;
    SearchDexFile(listFile_Dex);
    if(listFile_Dex.size() == 0)
    {
        LOG(INFO) << "get dex file is zero!" << std::endl;
        return 0;
    }

    int nMethodSize = 0;
    for(FILES_CACHE_LIST::iterator iter = listFile_Dex.begin(); iter != listFile_Dex.end(); iter++)
    {
        std::string strDexFileName = "";
        //strDexFileName = strDexFileName.append(m_sTempFilePath).append(_SYMBOL_PATH_).append(m_strFilaName).append(*iter);
        strDexFileName = *iter;
        LOG(INFO) << "strDexFileName:" << strDexFileName << std::endl;
        do
        {
            FILE* fp;
            fp = fopen(strDexFileName.c_str(), "rb+");

            if(!fp)
            {
                LOG(INFO) << "classes.dex file isn't exist:" << strDexFileName.c_str() << std::endl;
                return 0;
            }

            fseek(fp, 0, SEEK_END);
            int nLen = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            //            LOG(INFO) << "nLen:" << nLen << std::endl;
            int nsize = sizeof(DexHeader);

            char *pRead_buff = new char[nsize+1];
            if(!pRead_buff)
            {
                LOG(INFO) << "read dexfile is failed!" << std::endl;
                return 0;
            }
            memset(pRead_buff, 0, nsize);
            fread(pRead_buff, nsize, 1, fp);
            dexData = (DexHeader*)pRead_buff;
            fclose(fp);

            int nSize = dexData->methodIdsSize;
            LOG(INFO) << "methodIdsSize:" << dexData->methodIdsSize << std::endl;
            if(nSize > nMethodSize)
                nMethodSize = nSize;

            std::string sTmpName = *iter;
            MAX_DEXCOUNT stInfo;
            stInfo.strFileName = sTmpName;
            if(nSize > nMaxCount)
            {
                stInfo.bMax = 1;
                vcMethodCountOutMax.push_back(stInfo);
            }
            else
            {
                stInfo.bMax = 0;
                vcMethodCountOutMax.push_back(stInfo);
            }

            if(pRead_buff)
            {
                delete [] pRead_buff;
                pRead_buff = NULL;
            }

            // 设置要处理的dex文件
            m_strListDex.append(strDexFileName);
        }while(0);
    }


    std::sort(vcMethodCountOutMax.begin(), vcMethodCountOutMax.end(), mySort);


    return nMethodSize;
}

bool ModifyDex::parseMaxDexFile(OUT_MAX_DEXCOUNT vcDexFile, FILES_CACHE_LIST &vcModifyFile)
{
    for(OUT_MAX_DEXCOUNT::iterator iter = vcDexFile.begin(); iter != vcDexFile.end(); iter++)
    {
        MAX_DEXCOUNT dexInfo;
        dexInfo = *iter;
        if(dexInfo.bMax)
        {
            std::string strFileName = "";
            //strFileName = m_sTempFilePath + _SYMBOL_PATH_ + dexInfo.strFileName;
            strFileName = dexInfo.strFileName;
            LOG(INFO) << "strFileName:" << strFileName << std::endl;
            strFileName = dexInfo.strFileName;
            vcModifyFile.push_back(strFileName);
        }
    }
    return true;
}


bool ModifyDex::decomDexFileToSmali(FILES_CACHE_LIST vcModifyFile, FILES_CACHE_LIST &vcOptList)
{
    bool bRet = true;
    do
    {
        if(vcModifyFile.size() < 1)
        {
            bRet = false;
            LOG(INFO) << "vcModifyFile size is zero!" << std::endl;
            break;
        }

        std::string strcmd = "";
        for(FILES_CACHE_LIST::iterator iter = vcModifyFile.begin(); iter != vcModifyFile.end(); iter++)
        {
            std::string sFile_L = "", sFile_R = "";
            strcmd.clear();
            sFile_L.append(*iter);

            int value = ExtraNumFrStr(sFile_L);

            sFile_L.clear();
            sFile_L = sFile_L.append(m_sTempFilePath).append(_SYMBOL_PATH_).append(*iter);
            if(value == 0)
            {
                sFile_R = sFile_R.append(m_sTempFilePath).append(_SYMBOL_PATH_).append("smali");
            }
            else
            {
                stringstream ss;
                std::string strId;
                ss << value;
                ss >> strId;

                sFile_R = sFile_R.append(m_sTempFilePath).append(_SYMBOL_PATH_).append("smali_").append(strId);
            }

            strcmd.append("java -jar \"").append(m_sExecFilePath).append(_SYMBOL_PATH_)
                    .append(_baksmali2).append("\" \"").append(sFile_L)
                    .append("\" -o \"").append(sFile_R).append("\"");

            LOG(INFO) << "strcmd : " << strcmd << std::endl;
            std::string strOpt = "";
            int nResult = mysystem(strcmd.c_str(), strOpt);
            if(nResult)
            {
                LOG(INFO) << strOpt << std::endl;
            }

            vcOptList.push_back(sFile_R);
        }

    }while(0);

    std::sort(vcOptList.begin(), vcOptList.end(), std::less<std::string>());

    return bRet;
}

bool ModifyDex::beginParseDex(OUT_MAX_DEXCOUNT vcDexFile, FILES_CACHE_LIST &vcOptList)
{
    FILES_CACHE_LIST vcModifyFile;
    if(!parseMaxDexFile(vcDexFile, vcModifyFile))
    {
        LOG(INFO) << "parseMaxDexFile func is error!" << std::endl;
        return false;
    }
    if(vcModifyFile.size() == 0)
    {
        LOG(INFO) << "The DEX file method has no more than -n parameters." << std::endl;
        return false;
    }

    list<ST_FIleAddInfo>::iterator it = m_listStDexFile.begin();
    for(; it != m_listStDexFile.end(); it++ )
    {
        ST_FIleAddInfo info = *it;
        vcOptList.push_back(info.strFilePath);
    }
    return true;
}

bool ModifyDex::ModifyDexFile(FILES_CACHE_LIST &vcOptList, int nCopytCount, int nDexCount, int& iCopyNumber)
{
    std::string strActiveName = "";
    strActiveName = obianMainActivity();
    LOG(INFO) << "mainActivity : " << strActiveName << "    " << vcOptList.size() << std::endl;
    if(!ModifyActiveFile(vcOptList, nCopytCount, nDexCount, strActiveName, iCopyNumber))
    {
        LOG(INFO) << "ModifyActivityFile if Failed!" << std::endl;
        return false;
    }
    return true;
}

bool ModifyDex::startBackDex(FILES_CACHE_LIST vcOptList, int iCopyNumber)
{
    std::string strcmd = "";
    int nIndex = 0;
    for (FILES_CACHE_LIST::iterator iter = vcOptList.begin(); iter != vcOptList.end(); iter++)
    {
        nIndex++;
        std::string sFile_L = "", sFile_R = "";
        strcmd.clear();
        sFile_L = sFile_L.append(*iter);
        LOG(INFO) << "sFile_L:" << sFile_L << std::endl;
        if(!FileUtils::DirExists(sFile_L))
            continue;

        if(nIndex == vcOptList.size())
        {
            if(iCopyNumber < 1)
                continue;
        }

        int nPos = sFile_L.rfind(_SYMBOL_PATH_);
        std::string sSrcFoder = sFile_L.substr(nPos+strlen(_SYMBOL_PATH_));
        LOG(INFO) << "sSrcFoder:" << sSrcFoder << std::endl;

        int value = ExtraNumFrStr(sSrcFoder);

        if(value == 0)
        {
            sFile_R = sFile_R.append(m_sTempFilePath).append(_SYMBOL_PATH_).append("classes.dex");
        }
        else
        {
            stringstream ss;
            std::string strId;
            ss << value;
            ss >> strId;
            sFile_R = sFile_R.append(m_sTempFilePath).append(_SYMBOL_PATH_).append("classes").append(strId).append(".dex");
        }

//        strcmd.append("java -jar \"").append(m_sExecFilePath).append(_SYMBOL_PATH_)
//                .append(_smali2).append("\" \"").append(sFile_L)
//                .append("\" -o \"").append(sFile_R).append("\"");

        strcmd.append("java -jar \"").append(m_pubvar->getModulePath(TOOL_SMALI2))
                .append("\" \"").append(sFile_L).append("\" -o \"").append(sFile_R).append("\"");

        LOG(INFO) << "strcmd : \r\n" << strcmd << std::endl;
        std::string stroutput;
        stroutput.clear();
        int nResult = mysystem(strcmd.c_str(), stroutput);
        if(nResult)
            LOG(INFO) << stroutput << std::endl;
        else
            return false;
    }

    return true;
}

bool ModifyDex::startBackApk(OUT_MAX_DEXCOUNT vcDexInfo, FILES_CACHE_LIST vcOptList)
{

    if(!deleteRepeatFile(vcDexInfo, vcOptList))
    {
        LOG(INFO) << "delete Repeat files is failed!!" << std::endl;
        return false;
    }

    std::string strCmd = "", strOutput = "";
    //strPath = strPath.append(m_sTempFilePath).append(_SYMBOL_PATH_).append(_SMALI_).append("*");

    // Comparess that Dex and xml into apk
    //strCmd.append("\"").append(m_sExecFilePath).append(_SYMBOL_PATH_).append(_myZip_).
    strCmd.append("\"").append(m_pubvar->getZipPath()).
            append("\" -a -z \"").append(m_strTempApkPath).append("\" -f \"").append(m_sTempFilePath).append("\" -d");
			
    strCmd.append("\"").append(m_sExecFilePath).append(_SYMBOL_PATH_)
            .append(_myZip_).append("\" -a -z \"").append(m_strNewApkPath)
            .append("\" -f \"").append(m_sTempFilePath).append("\" -d");
			
    LOG(INFO) << "using myzip command ...\r\n" << strCmd << std::endl;
    bool result = mysystem(strCmd.c_str(), strOutput);
    if (!result)
    {
        LOG(INFO) << "执行命令(" << strCmd.c_str() << ")异常��? \r\n" << strOutput << std::endl;
        return false;
    }

    LOG(INFO)<<strOutput<<std::endl;

#ifndef  _WIN32
    if(FileUtils::DirExists(m_strNewApkPath))
    {
        string str = FileUtils::ChangeFileExt(m_strNewApkPath, "apk");
        if(!FileUtils::copyFile((char*)m_strNewApkPath.c_str(), (char*)str.c_str()))
        {
            LOG(INFO) << "obain Apk file failed!" << std::endl;
            FileUtils::DelFileW(m_strNewApkPath);
        }
        else
        {
            LOG(INFO) << "packing package success!" << std::endl;
            FileUtils::DelFileW(m_strNewApkPath);
        }

        m_strNewApkPath.clear();
        m_strNewApkPath = str;
    }

#endif

    FileUtils::DeleteDir(m_sTempFilePath);

    return true;
}


bool ModifyDex::deleteRepeatFile(OUT_MAX_DEXCOUNT vcDexInfo, FILES_CACHE_LIST vcOptList)
{
    bool bret = true;

    do
    {
        // Delete tempDir Dex files
        for(OUT_MAX_DEXCOUNT::iterator itDex = vcDexInfo.begin(); itDex != vcDexInfo.end(); itDex++)
        {
            MAX_DEXCOUNT stDex;
            stDex = *itDex;
            if(stDex.bMax == false)
            {
                std::string strFileName = "";
                strFileName = m_sTempFilePath + _SYMBOL_PATH_ + stDex.strFileName;
                LOG(INFO) << "strFileName:" << strFileName << std::endl;
                if(FileUtils::DirExists(strFileName))
                    FileUtils::DelFileW(strFileName);
            }
        }

        // Delete Smali folders
        for(FILES_CACHE_LIST::iterator it = vcOptList.begin(); it != vcOptList.end(); it++)
        {
            std::string strPath = "";
            strPath = strPath.append(*it);
            if(FileUtils::DirExists(strPath))
                FileUtils::DeleteDir(strPath);
        }

        std::string strCmd = "", strOutput = "";
        // Delete Signature file
//        strCmd = strCmd.append(m_sExecFilePath).append(_SYMBOL_PATH_).append(_myZip_)
//                .append(" -r -z ").append(m_strNewApkPath).append(" -p \"META-INF/*.SF\"");
        strCmd = strCmd.append(m_pubvar->getZipPath())
                .append(" -r -z ").append(m_strNewApkPath).append(" -p \"META-INF/*.SF\"");
        LOG(INFO) << "strCmd: " << strCmd << std::endl;
        bret = mysystem(strCmd.c_str(), strOutput);
        LOG(INFO) << strOutput << std::endl;
        if(!bret) break;

        strCmd.clear();
        //strCmd = strCmd.append(m_sExecFilePath).append(_SYMBOL_PATH_).append(_myZip_)
        strCmd = strCmd.append(m_pubvar->getZipPath())
                .append(" -r -z ").append(m_strNewApkPath).append(" -p \"META-INF/*.RSA\"");
        LOG(INFO) << "strCmd: " << strCmd << std::endl;
        bret = mysystem(strCmd.c_str(), strOutput);
        LOG(INFO) << strOutput << std::endl;
        if(!bret) break;

        strCmd.clear();
        strCmd = strCmd.append(m_pubvar->getZipPath())
        //strCmd = strCmd.append(m_sExecFilePath).append(_SYMBOL_PATH_).append(_myZip_)
                .append(" -r -z ").append(m_strNewApkPath).append(" -p \"META-INF/*.MF\"");
        LOG(INFO) << "strCmd: " << strCmd << std::endl;
        bret = mysystem(strCmd.c_str(), strOutput);
        LOG(INFO) << strOutput << std::endl;
        if(!bret) break;

        strCmd.clear();
        //strCmd = strCmd.append(m_sExecFilePath).append(_SYMBOL_PATH_).append(_myZip_)
        strCmd = strCmd.append(m_pubvar->getZipPath())
                .append(" -r -z ").append(m_strNewApkPath).append(" -p \"META-INF/*.DSA\"");
        LOG(INFO) << "strCmd: " << strCmd << std::endl;
        bret = mysystem(strCmd.c_str(), strOutput);
        LOG(INFO) << strOutput << std::endl;
        if(!bret) break;

        // Delete AndroidMinifest.xml file
        strCmd.clear();
        //strCmd.append("\"").append(m_sExecFilePath).append(_SYMBOL_PATH_).append(_myZip_)
        strCmd = strCmd.append("\"").append(m_pubvar->getZipPath())
                .append("\" -r -z \"").append(m_strNewApkPath).append("\" -p \"AndroidManifest.xml\"");
        LOG(INFO) << "strCmd: " << strCmd << std::endl;
        bret = mysystem(strCmd.c_str() ,strOutput);
        LOG(INFO) << strOutput << std::endl;
        if(!bret) break;

        // Delete duplicate Dex files
        for(FILES_CACHE_LIST::iterator iter = vcOptList.begin(); iter != vcOptList.end(); iter++)
        {
            std::string sFile_L = "", sFile_DEX = "";
            sFile_L = sFile_L.append(*iter);
            int nPos = sFile_L.rfind(_SYMBOL_PATH_);
            std::string sfile_folder = sFile_L.substr(nPos + strlen(_SYMBOL_PATH_));

            int value = ExtraNumFrStr(sfile_folder);

            if(0 == value)
            {
                sFile_DEX = sFile_DEX.append("classes.dex");
            }
            else
            {
                stringstream ss;
                std::string strId = "";
                ss << value;
                ss >> strId;
                sFile_DEX = sFile_DEX.append("smali").append(strId).append(".dex");
            }
            strCmd.clear();
            //strCmd.append("\"").append(m_sExecFilePath).append(_SYMBOL_PATH_).append(_myZip_).append("\" -r -z \"")
            strCmd.append("\"").append(m_pubvar->getZipPath()).append("\" -r -z \"")
                    .append(m_strNewApkPath).append("\" -p \"").append(sFile_DEX).append("\"");
            LOG(INFO) << "strCmd: " << strCmd << std::endl;
            bret = mysystem(strCmd.c_str() ,strOutput);
            LOG(INFO) << strOutput << std::endl;
            if(!bret) break;
        }

    }while(0);

    return bret;
}


void ModifyDex::InitVariable()
{
    std::string strApkNamepath = m_sSrcsFilePath;
    m_sApkName = FileUtils::ExtractFileName(strApkNamepath);

//    std::string strOutput;
//    m_sTempFilePath.clear();
//    strOutput = strApkNamepath;
//    CFileInfo file(m_strApkPath);

//    m_sTempFilePath.append(m_sExecFilePath).append(_SYMBOL_PATH_).append("temp").append(_SYMBOL_PATH_).append(file.fileName());

//    if(!FileUtils::DirExists(m_sTempFilePath))
//    {
//        FileUtils::MakeSureDirExsits(m_sTempFilePath);
//    }
//    else
//    {
//        FileUtils::DeleteDir(m_sTempFilePath);
//        FileUtils::MakeSureDirExsits(m_sTempFilePath);
//    }

//    if(!strstr((char*)strApkNamepath.c_str(), _SYMBOL_PATH_))
//    {
//        string strTemp = "";
//        strTemp.append(m_sTempFilePath.c_str(), m_sTempFilePath.length());
//        m_sTempFilePath.clear();
//        m_sTempFilePath = FileUtils::ExcludeTrailingPathDelimiter(strTemp);
//    }
    m_sTempFilePath = m_strTempPath;
}

void ModifyDex::SearchDexFile(FILES_CACHE_LIST &listFile_Dex)
{
#ifdef  _WIN32
    std::string strDexFile = "*.dex";
#else
    std::string strDexFile = ".dex";
#endif
    if(!FileSearch(strDexFile, listFile_Dex))
    {
        LOG(INFO) << "can not found dex file:" << strDexFile << std::endl;
        return;
    }
}

bool ModifyDex::FileSearch(std::string strFileName, FILES_CACHE_LIST &listFile_Dex)
{
#ifdef  _WIN32
    struct _finddata_t files;
    int File_Handle;
    int i=0;
    std::string strfile = "";
    strfile.append(m_sTempFilePath).append(_SYMBOL_PATH_).append(strFileName);

#ifdef _DEBUG
    LOG(INFO) << "search Dir info:" << strfile << " dex size :" << m_strListDex.length()  << std::endl;
#endif

//    File_Handle = _findfirst(strfile.c_str(), &files);
//    if( File_Handle == -1L )
//    {
//        LOG(INFO) << "search Dir error:" << strfile  << std::endl;
//        return 0;
//    }
//    listFile_Dex.clear();
//    do
//    {
//        std::string strFile = "";
//        strFile.append(files.name);
//        listFile_Dex.push_back(strFile);
//        i++;
//    }while(0==_findnext(File_Handle,&files));
//    _findclose(File_Handle);
    for(int i = 0; i < m_strListDex.length(); i++ )
    {
        listFile_Dex.push_back(m_strListDex.at(i));
    }
#else
    DIR              *pDir = NULL;
    struct dirent    *ent = NULL ;
    char              childpath[MAX_PATH_LEN] = {'\0'} ;

    pDir = opendir(m_sTempFilePath.c_str());

    if(!pDir)
    {
        printf("(ldistDir)open dir failed! error\n ");
        return -1;
    }

    memset(childpath,0,sizeof(childpath));
    sprintf(childpath, "%s", m_sTempFilePath.c_str());

    while((ent=readdir(pDir))!=NULL)
    {
        if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)
            continue;

        LOG(INFO)<<"after opendir :"<<ent->d_name << ":"<< strFileName<< std::endl;
        if(strstr(ent->d_name, strFileName.c_str()))
        {
            std::string strFile = m_sTempFilePath + "/" + ent->d_name;
            listFile_Dex.push_back(strFile.c_str());
        }

    }
    closedir(pDir);

#endif
    return true;
}

bool ModifyDex::ModifyActiveFile(FILES_CACHE_LIST &vcOptList, int nCopytCount,
                                 int nDexCount, std::string strMainActivity, int &iCopyNumber)
{
    int nsize = nDexCount;
    if(nsize > 1)
    {
        nsize += 1;
        stringstream ss;
        std::string strId;
        ss << nsize;
        ss >> strId;

        std::string newSmaliFoder = "";
//        newSmaliFoder.append(m_sTempFilePath).append(_SYMBOL_PATH_)
//                .append("smali").append("_").append(strId);
        newSmaliFoder.append(m_sTempFilePath).append(_SYMBOL_PATH_)
                .append("smali_").append(strId);

        if(access(newSmaliFoder.c_str(), 0) == -1)
        {
#ifdef _WIN32
            CreateDirectoryA(newSmaliFoder.c_str(), NULL);
#else
            FileUtils::MakeSureDirExsits(newSmaliFoder);
#endif
        }

        for(FILES_CACHE_LIST::iterator iter = vcOptList.begin(); iter != vcOptList.end(); iter++)
        {
            std::string sSrcFoder = "";
            sSrcFoder = sSrcFoder.append(*iter);
            LOG(INFO) << "sSrcFoder:" << sSrcFoder << std::endl;
            if(!FileUtils::DirExists(sSrcFoder))
                continue;

            char *pszName = (char*)malloc(MAX_PATH_LEN*4*sizeof(char*));
            if(pszName == NULL)
            {
                LOG(INFO) << "create pszName Failed" << std::endl;
                return false;
            }
            memset(pszName, 0, MAX_PATH_LEN*4);
            sprintf(pszName, "%s", sSrcFoder.c_str());

            FILES_CACHE_LIST listFile_Foder;
            if(!getFileList(pszName, listFile_Foder))
            {
                LOG(INFO) << "getFileList Failed: " << pszName << std::endl;
                return false;
            }
            if(pszName)
            {
                free(pszName);
                pszName = NULL;
            }

            if(!copyActiveToNewSmali(newSmaliFoder, listFile_Foder, nCopytCount, strMainActivity, iCopyNumber, sSrcFoder))
            {
                LOG(INFO) << "sSrcFoder copy activity is failed!" << std::endl;
                continue;
            }
        }

        // new smali add to vcOptList
        vcOptList.push_back(newSmaliFoder);
    }

    return true;
}

bool ModifyDex::copyActiveToNewSmali(string newSmaliFoder, FILES_CACHE_LIST listFile_Foder, int nCopytCount,
                                     std::string strMainActivity, int & iCopyNumber, std::string sSrcFoder)
{
    bool bRet = true;

    do
    {
        std::string strSmaliPath = newSmaliFoder;
        if(!FileUtils::DirExists(strSmaliPath))
        {
            FileUtils::MakeSureDirExsits(strSmaliPath);
        }

        FILES_CACHE_LIST listFile_Activity;

        // Markup parse XML activity file path
        if(!parseXmlActivity(strMainActivity, listFile_Activity, sSrcFoder))
        {
            bRet = false;
            LOG(INFO) << "parse Activity failed" << std::endl;
            break;
        }

        if( listFile_Activity.size() > 1 )
        {
            int nSize = 0;
            for(FILES_CACHE_LIST::iterator iter = listFile_Activity.begin();
                iter != listFile_Activity.end(); iter++)
            {
                if(nSize >= nCopytCount)
                {
                    bRet = true;
                    LOG(INFO) << " copy activity over" << std::endl;
                    break;
                }
                string strData = "";
                strData.append(*iter).append(".smali");

                for(FILES_CACHE_LIST::iterator itNeed = listFile_Foder.begin();
                    itNeed != listFile_Foder.end(); itNeed++)
                {
                    std::string sFile_Smali = "";
                    sFile_Smali.append(*itNeed);

                    int nlen = m_sTempFilePath.length();
                    std::string sTmpStr = sFile_Smali.substr(nlen + strlen(_SYMBOL_PATH_));

                    nlen = sTmpStr.find(_SYMBOL_PATH_);
                    sFile_Smali.clear();
                    sFile_Smali = sTmpStr.substr(nlen + strlen(_SYMBOL_PATH_));

                    char* pszData = NULL;
#ifdef _WIN32
                    pszData = (char*)malloc((sFile_Smali.length() + 1)*sizeof(char*));
                    if(!pszData)
                        continue;
                    memset(pszData, 0, sFile_Smali.length()+1);
                    memcpy(pszData, sFile_Smali.c_str(), sFile_Smali.length());
                    if(strstr(pszData, _SYMBOL_PATH_))
                        ReplaceStr(pszData, _SYMBOL_PATH_, "/");

                    sFile_Smali.clear();
                    sFile_Smali = pszData;
#endif

                    if (sFile_Smali.compare(strData) != 0)
                    {
                        continue;
                    }
                    else
                    {
                        string strNewData = "";
                        strNewData.append(strSmaliPath.c_str()).append(_SYMBOL_PATH_)
                                .append(*iter).append(".smali");
                        if(!FileUtils::DirExists(strNewData))
                        {
                            FileUtils::MakeSureDirExsits(FileUtils::ExtractFilePath(strNewData).c_str());
                        }
                        sFile_Smali.clear();
                        sFile_Smali.append(*itNeed);
                        char* pszData1 = NULL;
#ifdef _WIN32
                        pszData1 = (char*)malloc((strNewData.length() + 1)*sizeof(char*));
                        if(!pszData1)
                            continue;
                        memset(pszData1, 0, strNewData.length()+1);
                        memcpy(pszData1, strNewData.c_str(), strNewData.length());
                        if(strstr(pszData1, "/"))
                            ReplaceStr(pszData1, "/", _SYMBOL_PATH_);

                        strNewData.clear();
                        strNewData = pszData1;
#endif
                        if(FileUtils::DirExists(sFile_Smali))
                        {
                            FileUtils::MakeSureDirExsits(FileUtils::ExtractFilePath(strNewData).c_str());
                            LOG(INFO) << sFile_Smali << "\n--To--\n" << strNewData << std::endl;
                            if(!FileUtils::copyFile((char*)sFile_Smali.c_str(), (char*)strNewData.c_str(), true))
                            {
                                bRet = false;
                                LOG(INFO) << "copy file error" << std::endl;
//                                LOG(INFO) << sFile_Smali << "\n--To--\n" << strNewData << std::endl;
                            }
                            else
                            {
                                if(FileUtils::DirExists(sFile_Smali))
                                {
                                    FileUtils::DelFileW(sFile_Smali);
                                }
                                iCopyNumber += 1;
                                nSize++;
                            }
                        }

                        if(pszData)
                        {
                            free(pszData);
                            pszData = NULL;
                        }

                        if(pszData1)
                        {
                            free(pszData1);
                            pszData1 = NULL;
                        }

                        break;
                    }
                }
            }
        }
    }while(0);

    return bRet;
}

bool ModifyDex::parseXmlActivity(std::string strMainActivity, FILES_CACHE_LIST &listFile_Activity, std::string sSrcFoder)
{
    bool bRet = true;
    std::string strTempXml = "";
    do
    {
        FILES_CACHE_LIST vcXmlRegisterFiles;

        std::string strCmd = "", strOutput = "", strXmlPath = "";
        strXmlPath.append(m_sTempFilePath).append(_SYMBOL_PATH_)
                .append("AndroidManifest.xml");
        strTempXml.append(m_sExecFilePath).append(_SYMBOL_PATH_)
                .append("AndroidManifestTemp.xml");

//        strCmd.append("java -jar \"").append(m_pubvar->getAXMLPrinter2Path())
//                .append("\" \"").append(strXmlPath).append("\" > \"").append(strTempXml).append("\"");


        strCmd = "\"" + m_pubvar->getAxmlModifyPath() + "\" -f \""
                + strXmlPath + "\" -t 16 > \"" + strTempXml + "\"";

        LOG(INFO) << "strCmd = " << strCmd << endl;

        mysystem(strCmd.c_str(), strOutput);
        if(!FileUtils::DirExists(strTempXml))
        {
            LOG(INFO) << "Temp XML File not exist" << std::endl;
            return false;
        }

        CMarkup objMark;
        if(!objMark.Load(strTempXml))
        {
            bRet = false;
            LOG(INFO) << "Load Xml failed" << std::endl;
            break;
        }
        if(!objMark.FindElem("manifest"))
        {
            bRet = false;
            LOG(INFO) << "Find manifest failed" << std::endl;
            break;
        }
        objMark.IntoElem();
        if(!objMark.FindElem("application"))
        {
            bRet = false;
            LOG(INFO) << "Find application failed" << std::endl;
            break;
        }
        objMark.IntoElem();
        std::string strActivity = "";
        while(1)
        {
            if(!objMark.FindElem("activity"))
            {
                LOG(INFO) << "Find activity over" << std::endl;
                break;
            }
            strActivity.clear();
            strActivity = objMark.GetAttrib("android:name");
            if(strMainActivity.compare(strActivity) == 0)
            {
                continue;
            }

            char* pszData = new char[strActivity.size()+1];
            if(pszData == NULL)
            {
                LOG(INFO) << "Modify Active File is failed!" << std::endl;
                bRet = false;
                break;
            }
            memset(pszData, 0, strActivity.size() + 1);
            strcpy(pszData, strActivity.c_str());
            if(strstr(pszData, "."))
                ReplaceStr(pszData, ".", "/");

            strActivity.clear();
            strActivity = pszData;

            vcXmlRegisterFiles.push_back(strActivity);

            if(pszData)
            {
                delete[] pszData;
                pszData = NULL;
            }
        }

        objMark.OutOfElem();
        objMark.OutOfElem();

        FILES_CACHE_LIST vcApplicationCallFiles;
        std::string strAppFile = "";
        if(!getMainApplication(strAppFile))
        {
            LOG(INFO) << "get mainApplication is Failed!" << std::endl;
            bRet = false;
            break;
        }
        else
        {
            if(!getMainApplicationOrMainActivity(strAppFile, vcApplicationCallFiles, vcXmlRegisterFiles))
            {
                LOG(INFO) << "get Application call activity is failed!" << std::endl;
                bRet = false;
                break;
            }
            if(!getMainSmaliBaseClass(sSrcFoder, strAppFile, vcApplicationCallFiles, true)) {
                LOG(INFO) << "get Application base class call activity is failed!" << std::endl;
                bRet = false;
                break;
            }
        }

        FILES_CACHE_LIST vcActivityCallFiles;
        std::string strMainActivity = obianMainActivity();
        if(strMainActivity.empty() || strMainActivity == "")
        {
            LOG(INFO) << "get main activity is failed!!" << std::endl;
            bRet = false;
            break;
        }
        else
        {
            char* pszData = (char*)malloc((strMainActivity.length() + 1)*sizeof(char*));
            memset(pszData, 0, strMainActivity.length() + 1);
            memcpy(pszData, strMainActivity.c_str(), strMainActivity.length());
            if(strstr(pszData, "."))
                ReplaceStr(pszData, ".", _SYMBOL_PATH_);
            else if(strstr(pszData, "/"))
                ReplaceStr(pszData, "/", _SYMBOL_PATH_);

            std::string strFiles = "";
            strFiles = pszData;
            strMainActivity.clear();

            strMainActivity = strMainActivity.append(m_sTempFilePath).append(_SYMBOL_PATH_)
                    .append("smali_").append(_SYMBOL_PATH_)
                    .append(strFiles.c_str(), strFiles.length()).append(".smali");

            if(pszData)
            {
                free(pszData);
                pszData = NULL;
            }

            if(!getMainApplicationOrMainActivity(strMainActivity, vcActivityCallFiles, vcXmlRegisterFiles))
            {
                LOG(INFO) << "get mainActivity call activity is failed!" << std::endl;
                bRet = false;
                break;
            }
            if(!getMainSmaliBaseClass(sSrcFoder, strMainActivity, vcActivityCallFiles, false)) {
                std::cout << "get mainActivity base class call activity is failed!" << std::endl;
                bRet = false;
                break;
            }
        }

        std::sort(vcXmlRegisterFiles.begin(), vcXmlRegisterFiles.end(), std::less<std::string>());
        std::sort(vcActivityCallFiles.begin(), vcActivityCallFiles.end(), std::less<std::string>());
        std::sort(vcApplicationCallFiles.begin(), vcApplicationCallFiles.end(), std::less<std::string>());

        int nxml = vcXmlRegisterFiles.size();
        int nact = vcActivityCallFiles.size();
        int napp = vcApplicationCallFiles.size();

        for(FILES_CACHE_LIST::iterator itXml = vcXmlRegisterFiles.begin();
            itXml != vcXmlRegisterFiles.end(); itXml++)
        {
            std::string strXml = "";
            strXml = *itXml;

            bool bSame = false;
            for(FILES_CACHE_LIST::iterator itApp = vcApplicationCallFiles.begin();
                itApp != vcApplicationCallFiles.end(); itApp++)
            {
                std::string strApp = "";
                strApp = *itApp;
                if(strcmp(strXml.c_str(), strApp.c_str()) == 0)
                {
                    bSame = true;
                    break;
                }
            }

            if(bSame)
                continue;

            for(FILES_CACHE_LIST::iterator itAct = vcActivityCallFiles.begin();
                itAct != vcActivityCallFiles.end(); itAct++)
            {
                std::string strAct = "";
                strAct = *itAct;

                if(strcmp(strXml.c_str(), strAct.c_str()) == 0)
                {
                    bSame = true;
                    break;
                }
            }

            if(!bSame)
            {
                listFile_Activity.push_back(strXml);
            }
        }

        int nlist = listFile_Activity.size();

        if(nlist == 0)
        {
            listFile_Activity.resize(vcXmlRegisterFiles.size());
            listFile_Activity.swap(vcXmlRegisterFiles);
        }

        printf("%d\n", listFile_Activity.size());

    }while(0);

    if(FileUtils::DirExists(strTempXml))
    {
        if(!FileUtils::DelFileW(strTempXml))
        {
            LOG(INFO) << "delete Temp xml error" << std::endl;
        }
    }

    return bRet;
}

int ModifyDex::ReplaceStr(char* sSrc, const char* sMatchStr, const char* sReplaceStr)
{
    int StringLen;
    std::string strTempData;

    char* FindPos = (char*)strstr(sSrc, sMatchStr);
    if( (!FindPos) || (!sMatchStr) )
        return -1;

    while( FindPos )
    {
        strTempData.clear();
        StringLen = FindPos - sSrc;

        strTempData.append(sSrc, StringLen).append(sReplaceStr).append(FindPos+strlen(sMatchStr));

        strcpy(sSrc, strTempData.c_str());

        FindPos = strstr(sSrc, sMatchStr);

    }

    return 0;
}


bool ModifyDex::getFileList(char* pszDestPath, FILES_CACHE_LIST &listFile_Foder)
{
#ifdef  _WIN32
    WIN32_FIND_DATA FindFileData;
    HANDLE hListFile;
    char szFilePath[MAX_PATH_LEN];
    sprintf(szFilePath, "%s\\*", pszDestPath);
    hListFile = FindFirstFile(szFilePath, &FindFileData);
    if(hListFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    do
    {
        if( lstrcmp(FindFileData.cFileName, TEXT(".")) == 0 ||
                lstrcmp(FindFileData.cFileName, TEXT("..")) == 0 )
        {
            continue;
        }

        sprintf(szFilePath, "%s\\%s", pszDestPath, FindFileData.cFileName);

        if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            std::string strFoder = "";
            strFoder.append(szFilePath);

            //            listFile_Foder.push_back(strFoder);

            getFileList((char*)strFoder.c_str(), listFile_Foder);
        }
        else
        {
            listFile_Foder.push_back(szFilePath);
        }

    }while(FindNextFile(hListFile, &FindFileData));

    FindClose(hListFile);

#else
    //打开指定目录
    std::string strTempName = "";
    strTempName.append(pszDestPath);

    struct stat s;
    DIR     *dir;
    struct dirent *dt;
    char dirname[500] = {0};

    strcpy(dirname, strTempName.c_str());

    if(lstat(dirname, &s) < 0)
    {
        printf("lstat error\n");
        return false;
    }
    if(S_ISDIR(s.st_mode))
    {
        if((dir = opendir(dirname)) == NULL)
        {
            printf("opendir %s/%s error\n");
            return false;
        }
        while((dt = readdir(dir)) != NULL)
        {
            if(strcmp(dt->d_name,".") == 0 || strcmp(dt->d_name,"..") == 0)
            {
                continue;
            }

            char szBuf[500] = {0};
            sprintf(szBuf,"%s/%s",dirname, dt->d_name);

            string strPath = "";
            strPath.append(szBuf);
            //            listFile_Foder.push_back(strPath);

            getFileList(szBuf, listFile_Foder);
        }

        closedir(dir);
    }
    else
    {
        listFile_Foder.push_back(dirname);
    }
#endif
    return true;
}


std::string ModifyDex::obianMainActivity()
{
    bool bRet = true;
    std::string strManiActivity = "";
    do
    {
        std::string strCmd = "", strOutput = "";

        //strCmd.append("\"").append(m_sExecFilePath).append(_SYMBOL_PATH_).append(_AXMNAME_)
        strCmd.append("\"").append(m_pubvar->getAxmlModifyPath())
                .append("\" -f \"").append(m_strTempPath).append(_SYMBOL_PATH_)
                .append("AndroidManifest.xml").append("\" -t 9 -a");
        LOG(INFO) << "strCmd: " << strCmd << std::endl;
        if(!mysystem(strCmd.c_str(),strOutput))
        {
            bRet = false;
            LOG(INFO) << "obtain MainActivityName command failed!" << std::endl;
            break;
        }
        LOG(INFO) << strOutput << std::endl;
        if(strOutput.empty())
        {
            bRet = false;
            LOG(INFO) << "obtain MainActivityName Failed!" << std::endl;
            break;
        }
        else
        {
            int nlen = strOutput.length();
            int npos = strOutput.find("MainActivityName:", 0);
            if(npos == string::npos)
            {
                bRet = false;
                LOG(INFO) << "paser MainActivityName-1 failed" << std::endl;
                break;
            }

            npos = strOutput.find(":", npos);
            if(npos == string::npos)
            {
                bRet = false;
                LOG(INFO) << "paser MainActivityName-2 failed" << std::endl;
                break;
            }

            strManiActivity = strOutput.substr(npos+1, nlen - npos - 1);
        }
    }while(0);

    return strManiActivity;
}

int ModifyDex::ExtraNumFrStr(string str)
{
    char *p = (char*)malloc(32*sizeof(char*));
    if(!p)
    {
        LOG(INFO) << "malloc point is error!" << std::endl;
        return -1;
    }
    memset(p, 0, 32);
    int nsize = str.length();
    memcpy(p, str.c_str(), nsize);

    char* tmp = p;
    int  value = 0;
    for (value = 0; nsize--; tmp++)
    {
        if (*tmp < '0' || *tmp > '9')
        {
            continue;
        }

        value = value * 10 + (*tmp - '0');
    }

    if(p)
    {
        memset(tmp, 0, strlen(tmp));
        memset(p, 0, 32);
        free(p);
        p = NULL;
    }

    return value;
}

bool ModifyDex::getMainApplicationOrMainActivity(std::string strXmlFile, FILES_CACHE_LIST &vcCallActivity, FILES_CACHE_LIST vcXmlRegisterFiles)
{
    bool bRet = true;
    do
    {
        std::string sFilePath = strXmlFile;
        if(!FileUtils::DirExists(sFilePath))
        {
            LOG(INFO) << sFilePath << " is not exists!!";
            bRet = true;
            break;
        }

        FILE *fp = fopen(sFilePath.c_str(), "rb+");
        if(!fp)
        {
            LOG(INFO) << sFilePath << " file open failed!!";
            bRet = true;
            break;
        }
        fseek(fp, 0L, SEEK_END);
        int nlen = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        if(0 == nlen)
        {
            LOG(INFO) << sFilePath << " file size is 0 KB!!";
            bRet = true;
            break;
        }

        const int MAX_LINE_SIZE = 1024;
        char buf[MAX_LINE_SIZE];
        memset(buf, 0, MAX_LINE_SIZE);
        while (fgets(buf, MAX_LINE_SIZE, fp) != NULL )
        {
            std::string stmpbuf = "", stmpClass = "", sSavaClass = "";
            stmpbuf.append(buf, strlen(buf));

            int nPos = stmpbuf.find("L");
            if(std::string::npos == nPos)
            {
                memset(buf, 0, MAX_LINE_SIZE);
                continue;
            }
            if(nPos < stmpbuf.length() - 1)
                stmpClass = stmpbuf.substr(nPos+1);
            else
            {
                memset(buf, 0, MAX_LINE_SIZE);
                continue;
            }

            nPos = stmpClass.rfind(";");
            if(std::string::npos == nPos)
            {
                memset(buf, 0, MAX_LINE_SIZE);
                continue;
            }

            stmpbuf.clear();
            stmpbuf = stmpClass.substr(0, nPos);
            sSavaClass = stmpbuf;

            bool bSame = false;
            for(FILES_CACHE_LIST::iterator itXml = vcXmlRegisterFiles.begin();
                itXml != vcXmlRegisterFiles.end(); itXml++)
            {
                std::string sTempXml = "";
                sTempXml = *itXml;

                if(strcmp(sTempXml.c_str(), sSavaClass.c_str()) == 0)
                {
                    bSame = true;
                    break;
                }
            }

            if(!bSame)
            {
                bool bSave = true;
                for(FILES_CACHE_LIST::iterator itCall = vcCallActivity.begin();
                    itCall != vcCallActivity.end(); itCall++)
                {
                    std::string sTempCall = "";
                    sTempCall = *itCall;
                    if(strcmp(sSavaClass.c_str(), sTempCall.c_str()) == 0)
                    {
                        bSave = false;
                        break;
                    }
                }
                if(bSave)
                    vcCallActivity.push_back(sSavaClass);
            }

            memset(buf, 0, MAX_LINE_SIZE);
        }

        fclose(fp);
    }while(0);

    return bRet;
}

bool ModifyDex::getMainSmaliBaseClass(std::string sSrcFoder, string strSmaliPath, FILES_CACHE_LIST &vcBaseClassSmali, bool bApp)
{
    bool bRet = true;
    do
    {
        std::string sFilePath = strSmaliPath;
        if(!FileUtils::DirExists(sFilePath))
        {
            std::cout << sFilePath << " is not exists!!";
            bRet = true;
            break;
        }

        FILE *fp = fopen(sFilePath.c_str(), "rb+");
        if(!fp)
        {
            std::cout << sFilePath << " file open failed!!";
            bRet = true;
            break;
        }
        fseek(fp, 0L, SEEK_END);
        int nlen = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        if(0 == nlen)
        {
            std::cout << sFilePath << " file size is 0 KB!!";
            bRet = true;
            break;
        }

        const int MAX_LINE_SIZE = 1024;
        char buf[MAX_LINE_SIZE];
        memset(buf, 0, MAX_LINE_SIZE);
        bool bfirstSuper = true;
        while (fgets(buf, MAX_LINE_SIZE, fp) != NULL )
        {
            std::string stmpbuf = "", stmpClass = "", sSavaClass = "";
            stmpbuf.append(buf, strlen(buf));

            if(!bApp) {
                if(stmpbuf.find("Landroid/support/v7") != std::string::npos
                        || stmpbuf.find("Landroid/support/v4") != std::string::npos
                        || stmpbuf.find("Landroid/support/v4/app/FragmentActivity") != std::string::npos
                        || stmpbuf.find("Landroid/app/Activity") != std::string::npos ) {
                    break;
                }
            } else {
                if(stmpbuf.find("Landroid/support/v7") != std::string::npos
                        || stmpbuf.find("Landroid/support/v4") != std::string::npos
                        || stmpbuf.find("Landroid/app/Application") != std::string::npos
                        || stmpbuf.find("android/support/multidex/MultiDexApplication") != std::string::npos) {
                    break;
                }
            }


            int nPos = stmpbuf.find(".super");
            if(std::string::npos == nPos)
            {
                memset(buf, 0, MAX_LINE_SIZE);
                continue;
            }
            if(nPos < stmpbuf.length() - 1){
                bfirstSuper = false;
                nPos = stmpbuf.find("L");
                stmpClass = stmpbuf.substr(nPos+1);
            } else {
                memset(buf, 0, MAX_LINE_SIZE);
                continue;
            }

            nPos = stmpClass.rfind(";");
            if(std::string::npos == nPos)
            {
                memset(buf, 0, MAX_LINE_SIZE);
                continue;
            }

            stmpbuf.clear();
            stmpbuf = stmpClass.substr(0, nPos);
            sSavaClass = stmpbuf;

            if(bfirstSuper) {
                continue;
            }

            bool bSave = true;
            for(FILES_CACHE_LIST::iterator itCall = vcBaseClassSmali.begin();
                itCall != vcBaseClassSmali.end(); itCall++)
            {
                std::string sTempCall = "";
                sTempCall = *itCall;
                if(strcmp(sSavaClass.c_str(), sTempCall.c_str()) == 0)
                {
                    bSave = false;
                    break;
                }
            }
            if(bSave) {
                vcBaseClassSmali.push_back(sSavaClass);

                std::string strBaseClass = "";
                char* pszData = new char[sSavaClass.size()+1];
                if(pszData == NULL)
                {
                    std::cout << "Modify Active File is failed!" << std::endl;
                    bRet = false;
                    break;
                }
                memset(pszData, 0, sSavaClass.size() + 1);
                strcpy(pszData, sSavaClass.c_str());
                if(strstr(pszData, "."))
                    ReplaceStr(pszData, ".", _SYMBOL_PATH_);

                std::string strActivity = "";
                strActivity = pszData;
                strBaseClass = sSrcFoder + _SYMBOL_PATH_ + strActivity + ".smali";

                getMainSmaliBaseClass(sSrcFoder, strBaseClass, vcBaseClassSmali, bApp);

            }

            memset(buf, 0, MAX_LINE_SIZE);
        }
        if(fp)
            fclose(fp);

    }while(0);

    return bRet;
}

bool ModifyDex::getMainApplication(std::string &strMainApplication)
{
    bool bRet = true;
    do
    {
        std::string strCmd = "", strOutput = "";

        //strCmd.append("\"").append(m_sExecFilePath).append(_SYMBOL_PATH_).append(_AXMNAME_)
        strCmd.append("\"").append(m_pubvar->getAxmlModifyPath())
                .append("\" -f \"").append(m_strTempPath).append(_SYMBOL_PATH_)
                .append("AndroidManifest.xml").append("\" -t 2 -a");
        LOG(INFO) << "strCmd: " << strCmd << std::endl;
        if(!mysystem(strCmd.c_str(),strOutput))
        {
            bRet = false;
            LOG(INFO) << "get MainApplication command failed!" << std::endl;
            break;
        }
        LOG(INFO) << strOutput << std::endl;
        if(strOutput.empty())
        {
            bRet = false;
            LOG(INFO) << "get MainApplication Failed!" << std::endl;
            break;
        }
        else
        {
            int nlen = strOutput.length();
            int npos = strOutput.find("ApplicationName:", 0);
            if(npos == std::string::npos)
            {
                bRet = false;
                LOG(INFO) << "paser MainApplication-1 failed" << std::endl;
                break;
            }

            npos = strOutput.find(":", npos);
            if(npos == std::string::npos)
            {
                bRet = false;
                LOG(INFO) << "paser MainApplication-2 failed" << std::endl;
                break;
            }

            std::string strApp = strOutput.substr(npos+1, nlen - npos - 1);

            int nSize = strApp.length();
            char* pszData = (char*)malloc((nSize + 1)*sizeof(char*));
            memset(pszData, 0, nSize + 1);
            memcpy(pszData, strApp.c_str(), nSize);
            if(strstr(pszData, "."))
                ReplaceStr(pszData, ".", _SYMBOL_PATH_);
            else if(strstr(pszData, "/"))
                ReplaceStr(pszData, "/", _SYMBOL_PATH_);

            strApp.clear();
            strApp = pszData;
            strMainApplication = strMainApplication.append(m_sTempFilePath).append(_SYMBOL_PATH_)
                    .append("smali_").append(_SYMBOL_PATH_).append(strApp.c_str(), strApp.length()).append(".smali");

            if(pszData)
            {
                free(pszData);
                pszData = NULL;
            }
        }
    }while(0);

    return bRet;
}
