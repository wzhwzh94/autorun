#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QMenu>
#include <QRegExpValidator>
#include <QTimer>

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_pushButton_file_clicked();
    void loadSettings();
    void saveSettings();
    void checkTime();
    void input(char in);
    void wait(int msec);
    void active();
    void on_pushButton_run_clicked();

    void CreatTrayMenu();
    void CreatTrayIcon();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);

    void on_pushButton_save_clicked();

private:
    Ui::MainWindow *ui;
    QRegExpValidator *validatorin;
    QRegExpValidator *validatorfl;
    QTimer *timer;
    QSystemTrayIcon *myTrayIcon;
    QMenu *myMenu;
    QAction *miniSizeAction;
    QAction *restoreWinAction;
    QAction *quitAction;
};
#endif // MAINWINDOW_H
