#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "aboutdialog.h"
#include "configdialog.h"
#include <QMainWindow>
#include <QGraphicsScene>
#include <QNetworkReply>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void CheckConfigApp();
    void LoadConfigApp();

    void ListPort(QString);
    bool OpenPort(QString, QString);
    void ClosePort(QString, int);
    void TimerTxMode(QString);
    bool SendTimerSource(QString, QString);
    bool BuildData(QString, QString, QString);
    bool SendData(QString, QString, int, int);
    void ShowReadData(QString, QString, bool);

    void ListProfile();
    bool LoadProfile(QString);
    bool SaveProfile(QString, int);
    bool RenameProfile(QString, QString);
    bool DeleteProfile(QString);

    void ListPattern();

    char NibbleToHex(int, int);
    int  HexToNibble(char);
    void CalcSendData(QString);
    void CorrectionTimings(QString);

private slots:
    // ===== MAIN =====
    void on_pushAbout_clicked();
    void on_pushConfig_clicked();
    void createReport(QAction*);
    void replyVersion(QNetworkReply *reply);
    void timerOutVersion();
    // ===== PROFILE =====
    void on_comboProfile_activated(const QString &arg1);
    void on_pushSaveProfile_clicked();
    void on_pushNewProfile_clicked();
    void on_pushRenProfile_clicked();
    void on_pushDelProfile_clicked();
    void on_pushSkipProfile_clicked();
    // ===== SERIAL PORT =====
    void on_pushPortA_clicked();
    void on_pushPortB_clicked();
    void on_comboSelectPortA_activated(const QString &arg1);
    void on_comboSelectPortB_activated(const QString &arg1);
    void on_comboSpeedPortA_currentTextChanged(const QString &arg1);
    void on_comboSpeedPortB_currentTextChanged(const QString &arg1);
    void on_comboModePortA_currentTextChanged(const QString &arg1);
    void on_comboModePortB_currentTextChanged(const QString &arg1);
    void on_comboFlowPortA_currentTextChanged(const QString &arg1);
    void on_comboFlowPortB_currentTextChanged(const QString &arg1);
    void on_comboRTO_currentTextChanged(const QString &arg1);
    // ===== RECEIVE DATA =====
    void on_comboFormatRx_currentTextChanged(const QString &arg1);
    void on_comboTypeRx_currentTextChanged(const QString &arg1);
    void on_comboFrameRx_currentTextChanged(const QString &arg1);
    void on_pushSingleMode_clicked();
    void on_pushMarkerRx_clicked();
    void on_pushWordWrap_clicked();
    void on_pushClearRx_clicked();
    void timerReadPort();
    // ===== TRANSMIT DATA =====
    void on_comboFormatTx_currentTextChanged(const QString &arg1);
    void on_comboTypeTx_currentTextChanged(const QString &arg1);
    void on_comboEndTx_currentTextChanged(const QString &arg1);
    void on_pushLocalEcho_clicked();
    void on_pushMarkerTx_clicked();
    void on_comboSendPort_currentTextChanged(const QString &arg1);
    void on_lineDataTx_returnPressed();
    void on_pushSendTx_clicked();
    void on_lineDataTx_customContextMenuRequested(const QPoint &pos);
    // ===== TRANSMIT TEMPLATES =====
    void on_comboPattern_currentTextChanged(const QString &arg1);
    void on_pushSendPattern_clicked();
    void on_pushSelectFile1_clicked();
    void on_pushSelectFile2_clicked();
    void on_lineFile1_textChanged(const QString &arg1);
    void on_lineFile2_textChanged(const QString &arg1);
    void on_lineFile1_returnPressed();
    void on_lineFile2_returnPressed();
    void on_pushSendFile1_clicked();
    void on_pushSendFile2_clicked();
    void on_lineFile1_customContextMenuRequested(const QPoint &pos);
    void on_lineFile2_customContextMenuRequested(const QPoint &pos);
    void on_tableData_cellChanged(int row, int column);
    void on_tableData_itemSelectionChanged();
    void ClickRowHeaderF1F12(int);
    void on_tableData_customContextMenuRequested(const QPoint &pos);
    // ===== TRANSMIT TIMER =====
    void on_comboTmrSourceA_currentTextChanged(const QString &arg1);
    void on_comboTmrSourceB_currentTextChanged(const QString &arg1);
    void on_comboTmrTime_currentTextChanged(const QString &arg1);
    void on_pushTmrRun_clicked();
    void on_pushTmrSend_clicked();
    void timerWritePort();
    // ===== OTHER =====

private:
    Ui::MainWindow *ui;

    QTimer *timerRx;            // таймер проверки-приема данных
    QTimer *timerTx;            // таймер циклической передачи данных

    QGraphicsScene *scene;      // гафическая сцена для вывода SCOPE,CHART
    //QGraphicsItemGroup *groupA; // группа A (port A) графической сцены
    //QGraphicsItemGroup *groupB; // группа B (port B) графической сцены
    //QGraphicsItemGroup *groupG; // группа G (grid)   графической сцены

    virtual void showEvent(QShowEvent *event);
    virtual void closeEvent(QCloseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual bool eventFilter(QObject *obj, QEvent *ev);

};
#endif // MAINWINDOW_H
