#ifndef APK_HIJACK_H
#define APK_HIJACK_H

#include <string>
#include "apk.h"
#include "pubvar.h"
#include "define.h"

class CApkHijack : public CApk
{
public:
    CApkHijack();
    ~CApkHijack();

public:
    virtual int initTask(ST_Task_Info* stTaskInfo, CPubVar* pubvar);
    int doModule();
protected:

};

// 导出函数声明
EXPORT_DEF(CApkHijack)
{
    return (void*)(new CApkHijack);
}

#endif // APH_HIJACK_H
