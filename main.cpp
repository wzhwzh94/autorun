#include "mainwindow.h"

#include <QApplication>
#include <QTextCodec>
#include <QSettings>
#include <QFileInfo>
#include <QDir>

#include <QSystemSemaphore>
#include <QSharedMemory>
#include <windows.h>
#include <winbase.h>


//设置程序自启动 appPath程序路径
static void SetProcessAutoRunSelf(const QString &appPath)
{
    //注册表路径需要使用双反斜杠，如果是32位系统，要使用QSettings::Registry32Format
    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                       QSettings::Registry64Format);

    //以程序名称作为注册表中的键
    //根据键获取对应的值（程序路径）
    QFileInfo fInfo(appPath);
    QString name = fInfo.baseName();
    QString path = settings.value(name).toString();

    //如果注册表中的路径和当前程序路径不一样，
    //则表示没有设置自启动或自启动程序已经更换了路径
    //toNativeSeparators的意思是将"/"替换为"\"
    QString newPath = QDir::toNativeSeparators(appPath);
    if (path != newPath)
    {
        settings.setValue(name, newPath);
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //设置中文编码
    #if (QT_VERSION <= QT_VERSION_CHECK(5,0,0))
    #if _MSC_VER
        QTextCodec *codec = QTextCodec::codecForName("GBK");
    #else
        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    #endif
        QTextCodec::setCodecForLocale(codec);
        QTextCodec::setCodecForCStrings(codec);
        QTextCodec::setCodecForTr(codec);
    #else
        QTextCodec *codec = QTextCodec::codecForName("UTF-8");
        QTextCodec::setCodecForLocale(codec);
    #endif
    SetProcessAutoRunSelf(qApp->applicationFilePath());



    QSystemSemaphore semaphore("BTSemap", 1);
    semaphore.acquire();

    QSharedMemory shareMemory("BTKey");//建立共享内存对象，BTKey为改程序设置的共享内存key值，用户根据自己需要设定
    bool isRunning =false;
    if (shareMemory.attach())//判断软件是否已经打开
    {
        isRunning = true;//已经打开
    }
    else
    {
        shareMemory.create(1);//软件未打开，则创建共享内存
        isRunning = false;
    }
    semaphore.release();

    //@ 2.软件已经打开，则将软件激活，并置于桌面最前面
    if (isRunning)
    {
        //@ 将软件激活，显示在最前端
        QString wTitle = QString("Auto");//Qt创建的主MainWindow的 标题
        const char* szStr = wTitle.toLocal8Bit().toStdString().c_str();
        WCHAR wszClassName[256];
        memset(wszClassName,0,sizeof(wszClassName));
        MultiByteToWideChar(CP_ACP,0,szStr,strlen(szStr)+1,wszClassName,
            sizeof(wszClassName)/sizeof(wszClassName[0]));
        HWND handle = FindWindow(nullptr,wszClassName);//基于windows Api 获取程序窗口的句柄

        if (handle == nullptr)//判断是否为空
        {
            return -1;
        }
        ShowWindow(handle, SW_RESTORE);//激活窗口，参数：SW_RESTORE，以程序之前的大小显示，可以根据需要设置其他标识，如SW_MAXIMIZE

        SetForegroundWindow(handle);//激活窗口在桌面最前面
        return 1;
    }

    //@ 3.若程序未打开，则正常执行程序
    MainWindow w;
    w.show();
    return a.exec();
}

