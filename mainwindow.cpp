#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QRegExp>
#include <QSettings>
#include <QProcess>
#include <QTime>
#include <QMessageBox>

#define WINVER 0x0500
#include <windows.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    CreatTrayIcon();
    active();

    QRegExp in("([0-9])([0-9])?");
    validatorin = new QRegExpValidator(in, this);
    ui->lineEdit_hour->setValidator(validatorin);
    ui->lineEdit_min->setValidator(validatorin);
    ui->lineEdit_sec->setValidator(validatorin);

    QRegExp fl("([0-9]+)(\.[0-9]+)?");
    validatorfl = new QRegExpValidator(fl, this);
    ui->lineEdit_waitFile->setValidator(validatorfl);
    ui->lineEdit_waitAccount->setValidator(validatorfl);
    ui->lineEdit_waitPassword->setValidator(validatorfl);

    loadSettings();

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkTime()));
    timer->start(100);
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete validatorin;
    delete validatorfl;
    delete timer;
    delete myTrayIcon;
    delete myMenu;
    delete miniSizeAction;
    delete restoreWinAction;
    delete quitAction;
    delete ui;
}

void MainWindow::loadSettings()
{
    QString path = QCoreApplication::applicationDirPath();
    QSettings settings(path+"/settings.ini", QSettings::IniFormat);
    ui->lineEdit_file->setText(settings.value("file").toString());
    ui->lineEdit_hour->setText(settings.value("hour").toString());
    ui->lineEdit_min->setText(settings.value("min").toString());
    ui->lineEdit_sec->setText(settings.value("sec").toString());
    ui->lineEdit_account->setText(settings.value("account").toString());
    ui->lineEdit_password->setText(settings.value("password").toString());
    ui->lineEdit_waitFile->setText(settings.value("waitFile").toString());
    ui->lineEdit_waitAccount->setText(settings.value("waitAccount").toString());
    ui->lineEdit_waitPassword->setText(settings.value("waitPassword").toString());
}

void MainWindow::saveSettings()
{
    QString path = QCoreApplication::applicationDirPath();
    QSettings settings(path+"/settings.ini", QSettings::IniFormat);
    settings.setValue("file", ui->lineEdit_file->text());
    settings.setValue("hour", ui->lineEdit_hour->text());
    settings.setValue("min", ui->lineEdit_min->text());
    settings.setValue("sec", ui->lineEdit_sec->text());
    settings.setValue("account", ui->lineEdit_account->text());
    settings.setValue("password", ui->lineEdit_password->text());
    settings.setValue("waitFile", ui->lineEdit_waitFile->text());
    settings.setValue("waitAccount", ui->lineEdit_waitAccount->text());
    settings.setValue("waitPassword", ui->lineEdit_waitPassword->text());
}

void MainWindow::on_pushButton_file_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("程序路径"),"/", "所有文件 (*.*);; ");
    ui->lineEdit_file->setText(fileName);
}


void MainWindow::active()
{
    activateWindow();
    setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
    raise();
    #ifdef Q_OS_WIN32 //windows必须加这个，不然windows10 会不起作用，具体参看activateWindow 函数的文档
    HWND hForgroundWnd = GetForegroundWindow();
    DWORD dwForeID = ::GetWindowThreadProcessId(hForgroundWnd, NULL);
    DWORD dwCurID = ::GetCurrentThreadId();

    ::AttachThreadInput(dwCurID, dwForeID, TRUE);
    ::SetForegroundWindow((HWND)winId());
    ::AttachThreadInput(dwCurID, dwForeID, FALSE);
    #endif // MAC_OS
}


void MainWindow::on_pushButton_run_clicked()
{
    active();
    QString strPath = ui->lineEdit_file->text();
    QFile file(strPath);
    if (!file.exists())
    {
        QMessageBox::information(this, "错误", "文件不存在！");
        return;
    }

    QProcess pro;
    pro.startDetached(strPath);
    wait(ui->lineEdit_waitFile->text().toDouble()*1000);
    char* account = ui->lineEdit_account->text().toLatin1().data();
    for (int i=0; i<ui->lineEdit_account->text().length(); i++)
    {
        input(account[i]);
    }
    input('\r\n');
    wait(ui->lineEdit_waitAccount->text().toDouble()*1000);

    char* password = ui->lineEdit_password->text().toLatin1().data();
    for (int i=0; i<ui->lineEdit_password->text().length(); i++)
    {
        input(password[i]);
    }
    input('\r\n');
    wait(ui->lineEdit_waitPassword->text().toDouble()*1000);
    saveSettings();
}

void MainWindow::input(char in)
{
    INPUT ip;
    ip.type = INPUT_KEYBOARD;
    ip.ki.time = 0;
    ip.ki.dwFlags = KEYEVENTF_UNICODE;// 将键指定为 unicode 字符
    ip.ki.wScan = in; // 模拟哪个按键
    ip.ki.wVk = 0;
    ip.ki.dwExtraInfo = 0;
    SendInput(1, &ip, sizeof(INPUT));
}


void MainWindow::wait(int msec)
{
    QTime t;
    t.start();
    while(t.elapsed() < msec)
    {
        QCoreApplication::processEvents();
    }
}

void MainWindow::checkTime()
{
    static double lastLeft = -9999999;

    int h = ui->lineEdit_hour->text().toInt();
    int m = ui->lineEdit_min->text().toInt();
    int s = ui->lineEdit_sec->text().toInt();
    QTime current_time = QTime::currentTime();
    int hour = current_time.hour();
    int minute = current_time.minute();
    int second = current_time.second();
    int msec = current_time.msec();
    double left = (h-hour)*3600 + (m-minute)*60 + (s-second) + (double)msec*0.001;

    if (lastLeft == -9999999)
    {
        lastLeft = left;
    }
    if (left<=0 && lastLeft>=0)
    {
        on_pushButton_run_clicked();
    }
    lastLeft = left;

    double show = left;
    if (show < 0)
    {
        show = show + 86400;
    }
    ui->lineEdit_left->setText(QString::number(show));
}


void MainWindow::CreatTrayMenu()
{
    miniSizeAction = new QAction("隐藏(&N)",this);
    restoreWinAction = new QAction("还 原(&R)",this);
    quitAction = new QAction("退出(&Q)",this);

    this->connect(miniSizeAction,SIGNAL(triggered()),this,SLOT(hide()));
    this->connect(restoreWinAction,SIGNAL(triggered()),this,SLOT(showNormal()));
    this->connect(quitAction,SIGNAL(triggered()),qApp,SLOT(quit()));

    myMenu = new QMenu((QWidget*)QApplication::desktop());

    myMenu->addAction(miniSizeAction);
    myMenu->addAction(restoreWinAction);
    myMenu->addSeparator();     //加入一个分离符
    myMenu->addAction(quitAction);
}

void MainWindow::CreatTrayIcon()
{
    CreatTrayMenu();

    if (!QSystemTrayIcon::isSystemTrayAvailable())      //判断系统是否支持系统托盘图标
    {
        return;
    }

    myTrayIcon = new QSystemTrayIcon(this);
    myTrayIcon->setIcon(QIcon(":/auto.ico"));   //设置图标图片
    setWindowIcon(QIcon(":/auto.ico"));  //把图片设置到窗口上
    myTrayIcon->setToolTip("自动运行程序");    //托盘时，鼠标放上去的提示信息
    myTrayIcon->setContextMenu(myMenu);     //设置托盘上下文菜单
    myTrayIcon->show();
    this->connect(myTrayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
    case QSystemTrayIcon::Trigger:

    case QSystemTrayIcon::DoubleClick:
        showNormal();
        break;
    case QSystemTrayIcon::MiddleClick:
        break;

    default:
        break;
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (myTrayIcon->isVisible())
    {
        hide();     //最小化
        saveSettings();
        event->ignore();
    }
    else
        event->accept();
}

void MainWindow::on_pushButton_save_clicked()
{
    saveSettings();
}

