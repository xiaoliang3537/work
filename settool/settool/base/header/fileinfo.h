#ifndef FILEINFO_H
#define FILEINFO_H

#include <string>
#include <stdio.h>

#include "PathUtils.h"
#include "FileUtils.h"

using namespace std;

// �ļ�����
class CFileInfo
{
public:
    CFileInfo(std::string &path);
    ~CFileInfo();

public:
    bool exists();
    std::string fileDir();
    std::string fileName();
    std::string fileFullName();
    std::string filePath();
private:
    std::string m_strFilePath;
};


// Ŀ¼����
class CDir
{
public:
    CDir(std::string &path);
    ~CDir();

public:
    bool exists();
    static bool mkdir(const char* str);
    static bool deleteDir(const char* str);
private:
    std::string m_strPath;
};

#endif // FILEINFO_H
