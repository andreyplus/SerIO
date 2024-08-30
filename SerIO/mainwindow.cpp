// ******************************************************
// ***  Application: "SerIO"                          ***
// ***      License: GNU General Public License v3    ***
// ***        Autor: Kozhevnikov Y.V.  ©2020  RUSSIA  ***
// ***       e-Mail: tvm-system@yandex.ru             ***
// ***          Web: tvm-system.ru                    ***
// ******************************************************

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMenu>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QTextStream>
#include <QString>
#include <QMessageBox>
#include <QKeyEvent>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QThread>
#include <QTimer>
#include <QPixmap>
#include <QWidget>
#include <QTime>
#include <QDateTime>
#include <QDesktopServices>
#include <QClipboard>
#include <QNetworkAccessManager>


// ******************
// *** ПЕРЕМЕННЫЕ ***
// ******************
QString programName    = "SerIO";
QString programVersion = "3.0";
QString programEdition = "";
QString programQtComp  = "(Qt 5.15.2)";
QString programPath = "";       // абсолютный путь программы
//QString projectPath = "";       // полный путь текущего проекта
//QString projectName = "";       // полный путь и имя файла текущего проекта
bool flagRunProg = false;       // флаг "программа запущена"
bool checkUpdates = true;       // разрешение проверки обновлений
QString linkUpdates = "";       // http адрес проверки обновлений
//----------------------------------------
QFont fontGroup;                // шрифты программы
QFont fontLabel;
QFont fontContr;
QFont fontMonoD;
QFont fontMonoT;
//----------------------------------------
QSerialPort serialA;            // объект SerialPort-A
QSerialPort serialB;            // объект SerialPort-B
QString lastTxLineF = "";       // стек переданых данных Line
QString lastTxLineD = "";
QString lastTxLineE = "";
int countRTOA = 0;              // счетчик значения ReadTimeOut A
int countRTOB = 0;              // счетчик значения ReadTimeOut B
int flagRxPortA = 0;            // флаг состояния приема данных A
int flagRxPortB = 0;            // флаг состояния приема данных B
QString    markerRxDataAS;      // маркер начала поступления данных A
QString    markerRxDataBS;      // маркер начала поступления данных B
QString    markerRxDataAE;      // маркер окончания поступления данных A
QString    markerRxDataBE;      // маркер окончания поступления данных B
QByteArray bufferTxData;        // буфер передачи данных AB
//QByteArray bufferTxDataB;       // буфер передачи данных B
QByteArray bufferRxDataA;       // буфер приёма данных A
QByteArray bufferRxDataB;       // буфер приёма данных B
QByteArray bufferShowFrameA;    // буфер отрисованных граф.данных A
QByteArray bufferShowFrameB;    // буфер отрисованных граф.данных B
QByteArray bufferReportA;       // буфер отчета A
QByteArray bufferReportB;       // буфер отчета B
//----------------------------------------
QString currentProfile = "";    // текущий профиль настроек
int  currentProfileIndex = -1;  // индекс текущего профиля настроек
bool flagNewProfile = false;    // флаг создания нового профиля
bool flagRenameProfile = false; // флаг переименования профиля
//----------------------------------------
int  TimerSendCount = 0;        // счётчик срабатываний таймера
bool TimerInitFlagA = false;    // флаг перезапуска таймера-счётчика A
bool TimerInitFlagB = false;    // флаг перезапуска таймера-счётчика B
int  TimerCounterByteA = 0;     // байт-счётчик A таймера-счётчика
int  TimerCounterByteB = 0;     // байт-счётчик B таймера-счётчика
//----------------------------------------
QString Error = "";             // буфер вывода ошибок
//----------------------------------------
QString ClipFormat = "";        // буферы обмена Line-File-Table
QString ClipData = "";
QString ClipEnd = "";
//----------------------------------------
QAction *reportView;            // действия меню кнопки REPORT
QAction *reportBinA;
QAction *reportBinB;
QAction *reportText;
QAction *reportFrame;
QAction *reportBlock;
QAction *reportWindow;


// Процедуры конструктора-деструктора главного окна
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // настройка таймера проверки-приема данных
    timerRx = new QTimer(this);
    connect(timerRx, SIGNAL(timeout()), this, SLOT(timerReadPort()));
    // настройка таймера циклической передачи данных
    timerTx = new QTimer(this);
    connect(timerTx, SIGNAL(timeout()), this, SLOT(timerWritePort()));
    // настройка гафической сцены вывода SCOPE,CHART
    scene = new QGraphicsScene(ui->graphicsDataRx);
    ui->graphicsDataRx->setScene(scene);
}
MainWindow::~MainWindow()
{
    delete scene;
    delete timerRx;
    delete timerTx;
    delete ui;
}


// ********************
// *** ПОДПРОГРАММЫ ***
// ********************
// Подпрограммы настройки приложения
void MainWindow::CheckConfigApp()
{
    QFile file;
    // проверить-создать файл конфигурации "config"
    file.setFileName(programPath + "/serio.conf");
    if (file.exists() == false) {
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream fileStream(&file);
            // если ОС "WINDOWS"
            if (QFile::exists(programPath + "/SerIO.exe") == true) {
                fileStream << "Arial\n";    fileStream << "9\n";  fileStream << "1\n"; fileStream << "0\n"; // шрифт групп
                fileStream << "Arial\n";    fileStream << "8\n";  fileStream << "0\n"; fileStream << "1\n"; // шрифт меток
                fileStream << "Arial\n";    fileStream << "9\n";  fileStream << "0\n"; fileStream << "0\n"; // щрифт контролов
                fileStream << "Consolas\n"; fileStream << "11\n"; fileStream << "0\n"; fileStream << "0\n"; // шрифт данных
                fileStream << "Consolas\n"; fileStream << "9\n";  fileStream << "0\n"; fileStream << "0\n"; // шрифт заготовок
            }
            // если ОС "MACOS"
            else if (QFile::exists(programPath + "/SerIO.app") == true) {
                fileStream << "Arial\n";       fileStream << "9\n";  fileStream << "1\n"; fileStream << "0\n";
                fileStream << "Arial\n";       fileStream << "8\n";  fileStream << "0\n"; fileStream << "1\n";
                fileStream << "Arial\n";       fileStream << "9\n";  fileStream << "0\n"; fileStream << "0\n";
                fileStream << "Courier New\n"; fileStream << "11\n"; fileStream << "0\n"; fileStream << "0\n";
                fileStream << "Courier New\n"; fileStream << "9\n";  fileStream << "0\n"; fileStream << "0\n";
            }
            // иначе ОС "LINUX"
            else {
                fileStream << "Ubuntu\n";      fileStream << "9\n";  fileStream << "1\n"; fileStream << "0\n";
                fileStream << "Ubuntu\n";      fileStream << "8\n";  fileStream << "0\n"; fileStream << "1\n";
                fileStream << "Ubuntu\n";      fileStream << "9\n";  fileStream << "0\n"; fileStream << "0\n";
                fileStream << "Ubuntu Mono\n"; fileStream << "12\n"; fileStream << "0\n"; fileStream << "0\n";
                fileStream << "Ubuntu Mono\n"; fileStream << "10\n"; fileStream << "0\n"; fileStream << "0\n";
            }
            // для всех ОС
            fileStream << "CheckUpdatesON\n"; // разрешение и адрес проверки обновлений
            fileStream << "http://tvm-system.ru/SerIO/History.txt\n";
            fileStream << "1200\n";     // скорости порта
            fileStream << "2400\n";
            fileStream << "4800\n";
            fileStream << "9600\n";
            fileStream << "19200\n";
            fileStream << "38400\n";
            fileStream << "57600\n";
            fileStream << "115200\n";
            fileStream << "230400\n";
            fileStream << "460800\n";
            fileStream << "921600\n";
            file.close();
        }
        else {
            QMessageBox::warning(this, programName, "Error create \"serio.conf\" file !");
        }
    }
    // проверить-создать папку "Profiles"
    if (QDir(programPath + "/Profiles").exists() == false) {
        if (QDir().mkdir(programPath + "/Profiles") == false)
        {
            QMessageBox::warning(this, programName, "Error create folder \"Profiles\" !");
        }
    }
    // проверить-создать файл профиля "default"
    file.setFileName(programPath + "/Profiles/default");
    if (file.exists() == false)
    {
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream fileStream(&file);
            fileStream << "9600\n";         // скорость порта A
            fileStream << "8-N-1\n";        // режим порта A
            fileStream << "NONE\n";         // управление потоком порта A
            fileStream << "Clone A\n";      // скорость порта B
            fileStream << "Clone A\n";      // режим порта B
            fileStream << "Clone A\n";      // управление потоком порта B
            fileStream << "50\n";           // таймаут приема паекета данных RTO
            fileStream << "CHAR\n";         // формат приема данных
            fileStream << "Unsigned\n";     // тип принятых данных
            fileStream << "AUTO\n";         // размер графического фрэйма
            fileStream << "SingleOFF\n";    // однократный режим отображения
            fileStream << "MarkerRxOFF\n";  // маркеры приема данных
            fileStream << "WordWrapOFF\n";  // перенос на новую строку
            fileStream << "CHAR\n";         // формат передаваемых данных
            fileStream << "ASCII\n";        // тип передаваемых данных
            fileStream << "NONE\n";         // терминатор передаваемых данных
            fileStream << "LocalEchoOFF\n"; // локальное эхо
            fileStream << "MarkerTxOFF\n";  // маркер передачи данных
            fileStream << "AB\n";           // порт передачи данных
            fileStream << "\n";             // строка передаваемых данных
            fileStream << "\n";             // паттерн
            fileStream << "\n";             // файл 1
            fileStream << "\n";             // файл 2
            fileStream << "CHAR\n"; fileStream << "\n"; fileStream << "NONE\n"; // данные таблицы
            fileStream << "CHAR\n"; fileStream << "\n"; fileStream << "NONE\n";
            fileStream << "CHAR\n"; fileStream << "\n"; fileStream << "NONE\n";
            fileStream << "CHAR\n"; fileStream << "\n"; fileStream << "NONE\n";
            fileStream << "CHAR\n"; fileStream << "\n"; fileStream << "NONE\n";
            fileStream << "CHAR\n"; fileStream << "\n"; fileStream << "NONE\n";
            fileStream << "CHAR\n"; fileStream << "\n"; fileStream << "NONE\n";
            fileStream << "CHAR\n"; fileStream << "\n"; fileStream << "NONE\n";
            fileStream << "CHAR\n"; fileStream << "\n"; fileStream << "NONE\n";
            fileStream << "CHAR\n"; fileStream << "\n"; fileStream << "NONE\n";
            fileStream << "CHAR\n"; fileStream << "\n"; fileStream << "NONE\n";
            fileStream << "CHAR\n"; fileStream << "\n"; fileStream << "NONE\n";
            fileStream << "Line\n";         // источник таймера передачи порта A
            fileStream << "Clone A\n";      // источник таймера передачи порта B
            fileStream << "1000\n";         // время таймера передачи
            file.close();
        }
        else {
            QMessageBox::warning(this, programName, "Error create \"default\" profile file !");
        }
    }
    // проверить-создать папку "Patterns"
    if (QDir(programPath + "/Patterns").exists() == false) {
        if (QDir().mkdir(programPath + "/Patterns") == false)
        {
            QMessageBox::warning(this, programName, "Error create folder \"Patterns\" !");
        }
    }
    // проверить-создать папку "Reports"
    if (QDir(programPath + "/Reports").exists() == false) {
        if (QDir().mkdir(programPath + "/Reports") == false)
        {
            QMessageBox::warning(this, programName, "Error create folder \"Reports\" !");
        }
    }
}
void MainWindow::LoadConfigApp()
{
    QFile file(programPath + "/serio.conf");
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream fileStream(&file);
            // шрифты программы
            fontGroup.setFamily(fileStream.readLine());
                fontGroup.setPointSize(fileStream.readLine().toInt());
                fontGroup.setBold(fileStream.readLine().toInt());
                fontGroup.setItalic(fileStream.readLine().toInt());
            fontLabel.setFamily(fileStream.readLine());
                fontLabel.setPointSize(fileStream.readLine().toInt());
                fontLabel.setBold(fileStream.readLine().toInt());
                fontLabel.setItalic(fileStream.readLine().toInt());
            fontContr.setFamily(fileStream.readLine());
                fontContr.setPointSize(fileStream.readLine().toInt());
                fontContr.setBold(fileStream.readLine().toInt());
                fontContr.setItalic(fileStream.readLine().toInt());
            fontMonoD.setFamily(fileStream.readLine());
                fontMonoD.setPointSize(fileStream.readLine().toInt());
                fontMonoD.setBold(fileStream.readLine().toInt());
                fontMonoD.setItalic(fileStream.readLine().toInt());
            fontMonoT.setFamily(fileStream.readLine());
                fontMonoT.setPointSize(fileStream.readLine().toInt());
                fontMonoT.setBold(fileStream.readLine().toInt());
                fontMonoT.setItalic(fileStream.readLine().toInt());
            // проверка обновлений
            if (fileStream.readLine() == "CheckUpdatesON") checkUpdates = true;
            else checkUpdates = false;
            linkUpdates = fileStream.readLine();
            // список скоростей портов
            ui->comboSpeedPortA->clear();
            ui->comboSpeedPortB->clear(); ui->comboSpeedPortB->addItem("Clone A");
            while (!fileStream.atEnd()) {
                QString tmp = fileStream.readLine();
                if (tmp != "") { ui->comboSpeedPortA->addItem(tmp); ui->comboSpeedPortB->addItem(tmp); }
            }
            file.close();

            // ***** примененить шрифты
            // MAIN WINDOW
            MainWindow::setFont(fontContr);
            ui->pushAbout->setFont(fontLabel);
            ui->pushConfig->setFont(fontLabel);
            ui->pushReport->setFont(fontLabel);
            // PROFILE
            ui->groupProfile->setFont(fontGroup);
            ui->labelProfile->setFont(fontLabel); ui->comboProfile->setFont(fontContr);
            ui->pushSaveProfile->setFont(fontContr);
            ui->pushNewProfile->setFont(fontContr);
            ui->pushRenProfile->setFont(fontContr);
            ui->pushDelProfile->setFont(fontContr);
            ui->pushSkipProfile->setFont(fontContr);
            // SERIAL PORT
            ui->groupPort->setFont(fontGroup);
            ui->pushPortA->setFont(fontContr); ui->pushPortB->setFont(fontContr);
            ui->labelSelectPort->setFont(fontLabel); ui->comboSelectPortA->setFont(fontContr); ui->comboSelectPortB->setFont(fontContr);
            ui->labelSpeedPort->setFont(fontLabel); ui->comboSpeedPortA->setFont(fontContr); ui->comboSpeedPortB->setFont(fontContr);
            ui->labelModePort->setFont(fontLabel); ui->comboModePortA->setFont(fontContr); ui->comboModePortB->setFont(fontContr);
            ui->labelFlowPort->setFont(fontLabel); ui->comboFlowPortA->setFont(fontContr); ui->comboFlowPortB->setFont(fontContr);
            ui->labelRTO->setFont(fontLabel); ui->comboRTO->setFont(fontContr);
            // RECEIVE DATA
            ui->groupRxData->setFont(fontGroup);
            ui->labelFormatRx->setFont(fontLabel); ui->comboFormatRx->setFont(fontContr);
            ui->labelTypeRx->setFont(fontLabel); ui->comboTypeRx->setFont(fontContr);
            ui->labelFrameRx->setFont(fontLabel); ui->comboFrameRx->setFont(fontContr);
            ui->pushSingleMode->setFont(fontContr);
            ui->pushMarkerRx->setFont(fontContr);
            ui->pushWordWrap->setFont(fontContr);
            ui->labelTotalRxA->setFont(fontLabel); ui->lineTotalRxA->setFont(fontContr);
            ui->labelTotalRxB->setFont(fontLabel); ui->lineTotalRxB->setFont(fontContr);
            ui->pushClearRx->setFont(fontContr);
            ui->textDataRx->setFont(fontMonoD);
            // TRANSMIT DATA
            ui->groupTxData->setFont(fontGroup);
            ui->labelFormatTx->setFont(fontLabel); ui->comboFormatTx->setFont(fontContr);
            ui->labelTypeTx->setFont(fontLabel); ui->comboTypeTx->setFont(fontContr);
            ui->labelEndTx->setFont(fontLabel); ui->comboEndTx->setFont(fontContr);
            ui->pushLocalEcho->setFont(fontContr);
            ui->pushMarkerTx->setFont(fontContr);
            ui->labelTotalTxA->setFont(fontLabel); ui->lineTotalTxA->setFont(fontContr);
            ui->labelTotalTxB->setFont(fontLabel); ui->lineTotalTxB->setFont(fontContr);
            ui->labelSendPort->setFont(fontLabel); ui->comboSendPort->setFont(fontContr);
            ui->lineDataTx->setFont(fontMonoD); ui->pushSendTx->setFont(fontContr);
            // TRANSMIT TEMPLATE
            ui->groupTxTemplates->setFont(fontGroup);
            ui->labelPattern->setFont(fontLabel); ui->labelPatternInfo->setFont(fontLabel);
                ui->comboPattern->setFont(fontContr); ui->pushSendPattern->setFont(fontContr);
            ui->labelFile1->setFont(fontLabel); ui->labelFileInfo1->setFont(fontLabel);
                ui->pushSelectFile1->setFont(fontContr); ui->lineFile1->setFont(fontMonoT); ui->pushSendFile1->setFont(fontContr);
            ui->labelFile2->setFont(fontLabel); ui->labelFileInfo2->setFont(fontLabel);
                ui->pushSelectFile2->setFont(fontContr); ui->lineFile2->setFont(fontMonoT); ui->pushSendFile2->setFont(fontContr);
            ui->labelTable->setFont(fontLabel); ui->labelTableInfo->setFont(fontLabel); ui->tableData->setFont(fontMonoT);
            // TRANSMIT TMER
            ui->groupTxTimer->setFont(fontGroup);
            ui->labelTmrPortA->setFont(fontContr); ui->labelTmrPortB->setFont(fontContr);
            ui->labelTmrSource->setFont(fontLabel); ui->comboTmrSourceA->setFont(fontContr); ui->comboTmrSourceB->setFont(fontContr);
            ui->labelTmrTime->setFont(fontLabel); ui->comboTmrTime->setFont(fontContr);
            ui->labelTmrCount->setFont(fontLabel); ui->pushTmrRun->setFont(fontContr); ui->pushTmrSend->setFont(fontContr);
        }
        else { QMessageBox::warning(this, programName, "Error open \"serio.conf\" file !"); }
    }
    else {
        QMessageBox::warning(this, programName, "Not found \"serio.conf\" file !");
    }
}
// Подпрограммы работы с SerialPort
void MainWindow::ListPort(QString port)
{
    if (port == "A" || port == "ALL") { ui->comboSelectPortA->clear(); ui->comboSelectPortA->addItem("CLOSE"); }
    if (port == "B" || port == "ALL") { ui->comboSelectPortB->clear(); ui->comboSelectPortB->addItem("CLOSE"); }
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        if (port == "A" || port == "ALL") ui->comboSelectPortA->addItem(info.portName() + "  " + info.manufacturer() + "  " + info.description());
        if (port == "B" || port == "ALL") ui->comboSelectPortB->addItem(info.portName() + "  " + info.manufacturer() + "  " + info.description());
    }
}
bool MainWindow::OpenPort(QString port, QString name)
{
    QString param;
    int SPBoudRate=9600, SPDataBit=8, SPParity=0, SPStopBits=1, SPFlowControl=0;

    if (timerTx->isActive()) TimerTxMode("STOP");

    if (port == "A") {
        // подготовка параметров порта
        param = ui->comboSpeedPortA->currentText();
        SPBoudRate = param.toInt();
        param = ui->comboModePortA->currentText();
        if      (param == "8-N-1") { SPDataBit=8; SPParity=0; SPStopBits=1; }
        else if (param == "8-N-2") { SPDataBit=8; SPParity=0; SPStopBits=2; }
        else if (param == "8-E-1") { SPDataBit=8; SPParity=2; SPStopBits=1; }
        else if (param == "8-E-2") { SPDataBit=8; SPParity=2; SPStopBits=2; }
        else if (param == "8-O-1") { SPDataBit=8; SPParity=3; SPStopBits=1; }
        else if (param == "8-O-2") { SPDataBit=8; SPParity=3; SPStopBits=2; }
        param = ui->comboFlowPortA->currentText();
        if      (param == "NONE")     SPFlowControl=0;
        else if (param == "RTS/CTS")  SPFlowControl=1;
        else if (param == "XON/XOFF") SPFlowControl=2;
        // применение параметров порта
        int cntCharsName = 0;
        while (name[cntCharsName] != ' ' && cntCharsName <= name.length()) cntCharsName++;
        serialA.setPortName(name.left(cntCharsName));
        serialA.setBaudRate(QSerialPort::BaudRate(SPBoudRate));
        serialA.setDataBits(QSerialPort::DataBits(SPDataBit));              // 5, 6, 7, 8
        serialA.setParity(QSerialPort::Parity(SPParity));                   // 0=No, 2=Even, 3=Odd
        serialA.setStopBits(QSerialPort::StopBits(SPStopBits));             // 1=1, 3=1.5, 2=2
        serialA.setFlowControl(QSerialPort::FlowControl(SPFlowControl));    // 0=No, 1=RTS/CTS, 2=XON/XOFF
        //serialA.setReadBufferSize(0); // 0=бесконечный
        // сброс буфферов приема
        bufferRxDataA.clear(); bufferShowFrameA.clear(); bufferReportA.clear();
        // открытие порта
        if (serialA.open(QSerialPort::ReadWrite) == true) {
            if (checkUpdates == true) { ui->textDataRx->clear(); checkUpdates = false; }
            ui->labelTotalRxA->setText("Rx A:");
            ui->lineTotalRxA->setText("");
            ui->lineTotalTxA->setText("");
            ui->pushSendTx->setEnabled(true);
            ui->pushSendPattern->setEnabled(true);
            ui->pushSendFile1->setEnabled(true);
            ui->pushSendFile2->setEnabled(true);
            ui->pushTmrRun->setEnabled(true);
            ui->pushTmrSend->setEnabled(true);
            timerRx->start(10);
            ui->pushPortA->setChecked(true); ui->pushPortA->setEnabled(true);
            return true;
        }
    }
    else if (port == "B") {
        // подготовка параметров порта
        if (ui->comboSpeedPortB->currentText() == "Clone A") param = ui->comboSpeedPortA->currentText(); else param = ui->comboSpeedPortB->currentText();
        SPBoudRate = param.toInt();
        if (ui->comboModePortB->currentText() == "Clone A") param = ui->comboModePortA->currentText(); else ui->comboModePortB->currentText();
        if      (param == "8-N-1") { SPDataBit=8; SPParity=0; SPStopBits=1; }
        else if (param == "8-N-2") { SPDataBit=8; SPParity=0; SPStopBits=2; }
        else if (param == "8-E-1") { SPDataBit=8; SPParity=2; SPStopBits=1; }
        else if (param == "8-E-2") { SPDataBit=8; SPParity=2; SPStopBits=2; }
        else if (param == "8-O-1") { SPDataBit=8; SPParity=3; SPStopBits=1; }
        else if (param == "8-O-2") { SPDataBit=8; SPParity=3; SPStopBits=2; }
        if (ui->comboFlowPortB->currentText() == "Clone A") param = ui->comboFlowPortA->currentText(); else param = ui->comboFlowPortB->currentText();
        if      (param == "NONE")     SPFlowControl=0;
        else if (param == "RTS/CTS")  SPFlowControl=1;
        else if (param == "XON/XOFF") SPFlowControl=2;
        // применение параметров порта
        int cntCharsName = 0;
        while (name[cntCharsName] != ' ' && cntCharsName <= name.length()) cntCharsName++;
        serialB.setPortName(name.left(cntCharsName));
        serialB.setBaudRate(QSerialPort::BaudRate(SPBoudRate));
        serialB.setDataBits(QSerialPort::DataBits(SPDataBit));              // 5, 6, 7, 8
        serialB.setParity(QSerialPort::Parity(SPParity));                   // 0=No, 2=Even, 3=Odd
        serialB.setStopBits(QSerialPort::StopBits(SPStopBits));             // 1=1, 3=1.5, 2=2
        serialB.setFlowControl(QSerialPort::FlowControl(SPFlowControl));    // 0=No, 1=RTS/CTS, 2=XON/XOFF
        //serialB.setReadBufferSize(0); // 0=бесконечный
        // сброс буфферов приема
        bufferRxDataB.clear(); bufferShowFrameB.clear(); bufferReportB.clear();
        // открытие порта
        if (serialB.open(QSerialPort::ReadWrite) == true) {
            if (checkUpdates == true) { ui->textDataRx->clear(); checkUpdates = false; }
            ui->labelTotalRxB->setText("Rx B:");
            ui->lineTotalRxB->setText("");
            ui->lineTotalTxB->setText("");
            ui->pushSendTx->setEnabled(true);
            ui->pushSendPattern->setEnabled(true);
            ui->pushSendFile1->setEnabled(true);
            ui->pushSendFile2->setEnabled(true);
            ui->pushTmrRun->setEnabled(true);
            ui->pushTmrSend->setEnabled(true);
            timerRx->start(10);
            ui->pushPortB->setChecked(true); ui->pushPortB->setEnabled(true);
            return true;
        }
    }
    return false;
}
void MainWindow::ClosePort(QString port, int mode)
{
    if (timerTx->isActive()) TimerTxMode("STOP");

    if (port == "A" || port == "ALL") {
        if (serialA.isOpen()) {
            serialA.waitForBytesWritten(1000);
            serialA.close();
        }
        ui->labelTotalRxA->setText("Rx A:");
        ui->lineTotalRxA->clear();
        ui->lineTotalTxA->clear();
        if (mode == 0) {
            ui->comboSelectPortA->setCurrentText("CLOSE");
            ui->pushPortA->setChecked(false); ui->pushPortA->setEnabled(false);
        }
        bufferRxDataA.clear(); bufferShowFrameA.clear(); bufferReportA.clear();
    }
    if (port == "B" || port == "ALL") {
        if (serialB.isOpen()) {
            serialB.waitForBytesWritten(1000);
            serialB.close();
        }
        ui->labelTotalRxB->setText("Rx B:");
        ui->lineTotalRxB->clear();
        ui->lineTotalTxB->clear();
        if (mode == 0) {
            ui->comboSelectPortB->setCurrentText("CLOSE");
            ui->pushPortB->setChecked(false); ui->pushPortB->setEnabled(false);
        }
        bufferRxDataB.clear(); bufferShowFrameB.clear(); bufferReportB.clear();
    }

    if ((ui->comboSelectPortA->currentText() == "CLOSE" && ui->comboSelectPortB->currentText() == "CLOSE") ||
            (ui->pushPortA->isChecked() == false && ui->pushPortB->isChecked() == false)) {
        timerRx->stop();
        ui->pushSendTx->setEnabled(false);
        ui->pushSendPattern->setEnabled(false);
        ui->pushSendFile1->setEnabled(false);
        ui->pushSendFile2->setEnabled(false);
        ui->pushTmrRun->setEnabled(false);
        ui->pushTmrSend->setEnabled(false);
    }
}
void MainWindow::TimerTxMode(QString setMode)
{
    if (setMode == "START") {
        TimerInitFlagA = true;
        TimerInitFlagB = true;
        timerTx->start(ui->comboTmrTime->currentText().toInt());
        ui->pushTmrRun->setChecked(true);
    }
    else if (setMode == "STOP") {
        timerTx->stop();
        ui->pushTmrRun->setChecked(false);
        ui->labelTmrCount->setText("");
        TimerSendCount = 0;
    }
}
bool MainWindow::SendTimerSource(QString port, QString source)
{
    if      (source == "Line")              return SendData(port, "LINE", 0, 0);
    else if (source == "Pattern")           return SendData(port, "PATTERN", 0, 0);
    else if (source == "File 1")            return SendData(port, "FILE1", 0, 0);
    else if (source == "File 2")            return SendData(port, "FILE2", 0, 0);
    else if (source == "Table F1")          return SendData(port, "TABLE", 0, 0);
    else if (source == "Table F2")          return SendData(port, "TABLE", 1, 0);
    else if (source == "Table F3")          return SendData(port, "TABLE", 2, 0);
    else if (source == "Table F4")          return SendData(port, "TABLE", 3, 0);
    else if (source == "Table F5")          return SendData(port, "TABLE", 4, 0);
    else if (source == "Table F6")          return SendData(port, "TABLE", 5, 0);
    else if (source == "Table F7")          return SendData(port, "TABLE", 6, 0);
    else if (source == "Table F8")          return SendData(port, "TABLE", 7, 0);
    else if (source == "Table F9")          return SendData(port, "TABLE", 8, 0);
    else if (source == "Table F10")         return SendData(port, "TABLE", 9, 0);
    else if (source == "Table F11")         return SendData(port, "TABLE", 10, 0);
    else if (source == "Table F12")         return SendData(port, "TABLE", 11, 0);
    else if (source == "Counter CHAR 0-9")  return SendData(port, "COUNTER", 0x30, 0x39);
    else if (source == "Counter CHAR A-Z")  return SendData(port, "COUNTER", 0x41, 0x5A);
    else if (source == "Counter BIN 0-15")  return SendData(port, "COUNTER", 0, 15);
    else if (source == "Counter BIN 0-255") return SendData(port, "COUNTER", 0, 255);
    else return false;
}
bool MainWindow::BuildData(QString dataFormat, QString dataSource, QString endChars) {
    QByteArray tID = dataSource.toLocal8Bit();
    QByteArray tOD = "";
    bufferTxData = "";
    int i, j, nb;

    if (dataFormat == "CHAR") {
        //bufferTxData = dataSource.toLocal8Bit();
        i = 0;
        while (i < tID.length()) { // скопировать символы
            if (tID[i] == '\\') { // если управляющий символ
                i++;
                if      (tID[i] == '0') { tOD += char(0x00); i++; } // NUL (пустой символ 0x00)
                else if (tID[i] == 'a') { tOD += char(0x07); i++; } // BEL (звуковой сигнал)
                else if (tID[i] == 'b') { tOD += char(0x08); i++; } // BSP (забой)
                else if (tID[i] == 't') { tOD += char(0x09); i++; } // TAB (горизонтальная табуляция)
                else if (tID[i] == 'n') { tOD += char(0x0A); i++; } // LF  (новая строка)
                else if (tID[i] == 'v') { tOD += char(0x0B); i++; } // VT  (вертикальная табуляция)
                else if (tID[i] == 'f') { tOD += char(0x0C); i++; } // FF  (новая страница)
                else if (tID[i] == 'r') { tOD += char(0x0D); i++; } // CR  (начало строки)
                else if (tID[i] == 's') { tOD += char(0x1A); i++; } // SUB (замена)
                else if (tID[i] == 'e') { tOD += char(0x1B); i++; } // ESC (отмена)
                else if (tID[i] == 'd') { tOD += char(0x7F); i++; } // DEL (удаление)
                else tOD += tID[i++];
            }
            else tOD += tID[i++];
        }
        bufferTxData += tOD;
        tOD = "";
    }
    else if (dataFormat == "DEC") {
        i = 0;
        while (i < tID.length()) {
            while (i < tID.length() && tID[i] == ' ') i++; // пропустить пробелы
            if (tID[i] == '+' || tID[i] == '-') tOD += tID[i++]; // если знак "+","-"
            while (i < (tID.length()-1) && tID[i] == '0' && (tID[i+1] == '0' || tID[i+1] != ' ')) i++; // пропустить ведущие ноли
            while (i < tID.length() && (tID[i] >= '0' && tID[i] <= '9')) { // скопировать цифры
                tOD += tID[i++];
            }
            if (tID[i] == ' ' || i >= tID.length()) {
                // добавление данных в буфер передачи
                int32_t d = tOD.toInt();
                if (d > 255 || d < -128) { Error = "Overflow DEC data !\r\nValid data range: -128...127 or 0...255"; return false; }
                bufferTxData += uint8_t(d & 0x000000FF);
                tOD = "";
            }
            else { Error = "Invalid DEC data symbol(s) !\r\nValid symbols: +-0123456789"; return false; }
        }
    }
    else if (dataFormat == "HEX") {
        i = 0;
        while (i < tID.length()) {
            while (i < tID.length() && tID[i] == ' ') i++; // пропустить пробелы
            while (i < (tID.length()-1) && tID[i] == '0' && (tID[i+1] == '0' || tID[i+1] != ' ')) i++; // пропустить ведущие ноли
            tOD += '0'; // дополнение для нечетного количества hex-символов
            while (i < tID.length() && ((tID[i] >= '0' && tID[i] <= '9') || (tID[i] >= 'A' && tID[i] <= 'F') || (tID[i] >= 'a' && tID[i] <= 'f'))) {
                tOD += tID[i++];
            }
            if (tID[i] == ' ' || i >= tID.length()) {
                // добавление данных в буффер передачи
                if (tOD.length() & 1) j = 1; else j = 0;
                while (j < tOD.length()) {
                    uchar d = HexToNibble(tOD[j++])<<4;
                    d |= HexToNibble(tOD[j++]);
                    bufferTxData += d;
                }
                tOD = "";
            }
            else { Error = "Invalid HEX data symbol(s) !\r\nValid symbols: 0123456789AaBbCcDdEeFf"; return false; }
        }
    }
    else if (dataFormat == "BIN") {
        i = 0;
        while (i < tID.length()) {
            while (i < tID.length() && tID[i] == ' ') i++; // пропустить пробелы
            while (i < (tID.length()-1) && tID[i] == '0' && (tID[i+1] == '0' || tID[i+1] != ' ')) i++; // пропустить ведущие ноли
            nb = 0; // сброс счетчика bin-символов
            tOD += "00000000"; // дополнение для некратного 8 количества bin-символов
            while (i < tID.length() && (tID[i] == '0' || tID[i] == '1')) { // скопировать и подсчитать bin-символы
                tOD += tID[i++];
                nb++;
            }
            if (tID[i] == ' ' || i >= tID.length()) {
                // добавление данных в буффер передачи
                j = 8;
                if (nb == 0) j = tOD.length();
                if (nb > 0 && nb < 8) j = nb;
                if (nb > 8 && (nb % 8) != 0) j = nb % 8;
                while (j < tOD.length()) {
                    uchar d = 0;
                    if (tOD[j++] == '0') d = (d<<1)|0; else d = (d<<1)|1;
                    if (tOD[j++] == '0') d = (d<<1)|0; else d = (d<<1)|1;
                    if (tOD[j++] == '0') d = (d<<1)|0; else d = (d<<1)|1;
                    if (tOD[j++] == '0') d = (d<<1)|0; else d = (d<<1)|1;
                    if (tOD[j++] == '0') d = (d<<1)|0; else d = (d<<1)|1;
                    if (tOD[j++] == '0') d = (d<<1)|0; else d = (d<<1)|1;
                    if (tOD[j++] == '0') d = (d<<1)|0; else d = (d<<1)|1;
                    if (tOD[j++] == '0') d = (d<<1)|0; else d = (d<<1)|1;
                    bufferTxData += d;
                }
                tOD = "";
                nb = 0;
            }
            else { Error = "Invalid BIN data symbol(s) !\r\nValid symbols: 0 and 1"; return false; }
        }
    }
    else if (dataFormat == "MIXED") {
        i = 0;
        while (i < tID.length()) {
            while (i < tID.length() && tID[i] == ' ') i++; // пропустить пробелы
            if (i < tID.length()) {
                // CHAR
                if (tID[i] == '"') {
                    i++;
                    while (i < tID.length() && tID[i] != '"') { // скопировать символы
                        if (tID[i] == '\\') { // если управляющий символ
                            i++;
                            if      (tID[i] == '0') { tOD += char(0x00); i++; } // NUL (пустой символ 0x00)
                            else if (tID[i] == 'a') { tOD += char(0x07); i++; } // BEL (звуковой сигнал)
                            else if (tID[i] == 'b') { tOD += char(0x08); i++; } // BSP (забой)
                            else if (tID[i] == 't') { tOD += char(0x09); i++; } // TAB (горизонтальная табуляция)
                            else if (tID[i] == 'n') { tOD += char(0x0A); i++; } // LF  (новая строка)
                            else if (tID[i] == 'v') { tOD += char(0x0B); i++; } // VT  (вертикальная табуляция)
                            else if (tID[i] == 'f') { tOD += char(0x0C); i++; } // FF  (новая страница)
                            else if (tID[i] == 'r') { tOD += char(0x0D); i++; } // CR  (начало строки)
                            else if (tID[i] == 's') { tOD += char(0x1A); i++; } // SUB (замена)
                            else if (tID[i] == 'e') { tOD += char(0x1B); i++; } // ESC (отмена)
                            else if (tID[i] == 'd') { tOD += char(0x7F); i++; } // DEL (удаление)
                            else tOD += tID[i++];
                        }
                        else tOD += tID[i++];
                    }
                    if (tID[i] == '"') i++;
                    bufferTxData += tOD;
                    tOD = "";
                }
                // DEC
                else if (tID[i] == '+' || tID[i] == '-' || (tID[i] >= '0' && tID[i] <= '9')) {
                    if (tID[i] == '+' || tID[i] == '-') tOD += tID[i++]; // скопировать префикс "плюс" или "минус"
                    while (i < (tID.length()-1) && tID[i] == '0' && (tID[i+1] == '0' || tID[i+1] != ' ')) i++; // пропустить ведущие ноли
                    while (i < tID.length() && (tID[i] >= '0' && tID[i] <= '9')) { // скопировать значение
                        tOD += tID[i++];
                    }
                    if (tID[i] == ' ' || tID[i] == '\\' || i >= tID.length()) {
                        // добавление данных в буффер передачи
                        int32_t d = tOD.toInt();
                        if (d > 255 || d < -128) { Error = "Overflow DEC data !\r\nValid data range: -128...127 or 0...255"; return false; }
                        bufferTxData += uint8_t(d & 0x000000FF);
                        tOD = "";
                    }
                    else { Error = "Invalid DEC data symbol(s) !\r\nValid symbols: +-0123456789"; return false; }
                }
                // HEX
                else if (tID[i] == 'x') {
                    i++;
                    while (i < (tID.length()-1) && tID[i] == '0' && (tID[i+1] == '0' || tID[i+1] != ' ')) i++; // пропустить ведущие ноли
                    tOD += '0'; // дополнение для нечетного количества hex-символов
                    while (i < tID.length() && ((tID[i] >= '0' && tID[i] <= '9') || (tID[i] >= 'A' && tID[i] <= 'F') || (tID[i] >= 'a' && tID[i] <= 'f'))) { // скопировать значение
                        tOD += tID[i++];
                    }
                    if (tID[i] == ' ' || tID[i] == '\\' || i >= tID.length()) {
                        // добавление данных в буффер передачи
                        if (tOD.length() & 1) j = 1; else j = 0;
                        while (j < tOD.length()) {
                            uchar d = HexToNibble(tOD[j++])<<4;
                            d |= HexToNibble(tOD[j++]);
                            bufferTxData += d;
                        }
                        tOD = "";
                    }
                    else { Error = "Invalid HEX data symbol(s) !\r\nValid symbols: 0123456789AaBbCcDdEeFf"; return false; }
                }
                // BIN
                else if (tID[i] == 'b') {
                    i++;
                    while (i < (tID.length()-1) && tID[i] == '0' && (tID[i+1] == '0' || tID[i+1] != ' ')) i++; // пропустить ведущие ноли
                    nb = 0; // сброс счетчика bin-символов
                    tOD += "00000000"; // дополнение для некратного 8 количества bin-символов
                    while (i < tID.length() && (tID[i] == '0' || tID[i] == '1')) { // скопировать и подсчитать bin-символы
                        tOD += tID[i++];
                        nb++;
                    }
                    if (tID[i] == ' ' || tID[i] == '\\' || i >= tID.length()) {
                        // добавление данных в буффер передачи
                        j = 8;
                        if (nb == 0) j = tOD.length();
                        if (nb > 0 && nb < 8) j = nb;
                        if (nb > 8 && (nb % 8) != 0) j = nb % 8;
                        while (j < tOD.length()) {
                            uchar d = 0;
                            if (tOD[j++] == '0') d = (d<<1)|0; else d = (d<<1)|1;
                            if (tOD[j++] == '0') d = (d<<1)|0; else d = (d<<1)|1;
                            if (tOD[j++] == '0') d = (d<<1)|0; else d = (d<<1)|1;
                            if (tOD[j++] == '0') d = (d<<1)|0; else d = (d<<1)|1;
                            if (tOD[j++] == '0') d = (d<<1)|0; else d = (d<<1)|1;
                            if (tOD[j++] == '0') d = (d<<1)|0; else d = (d<<1)|1;
                            if (tOD[j++] == '0') d = (d<<1)|0; else d = (d<<1)|1;
                            if (tOD[j++] == '0') d = (d<<1)|0; else d = (d<<1)|1;
                            bufferTxData += d;
                        }
                        tOD = "";
                    }
                    else { Error = "Invalid BIN data symbol(s) !\r\nValid symbols: 0 and 1"; return false; }
                }
                // управляющий символ
                else if (tID[i] == '\\') {
                    i++;
                    if      (tID[i] == '0') { tOD += char(0x00); i++; } // NUL (пустой символ 0x00)
                    else if (tID[i] == 'a') { tOD += char(0x07); i++; } // BEL (звуковой сигнал)
                    else if (tID[i] == 'b') { tOD += char(0x08); i++; } // BSP (забой)
                    else if (tID[i] == 't') { tOD += char(0x09); i++; } // TAB (горизонтальная табуляция)
                    else if (tID[i] == 'n') { tOD += char(0x0A); i++; } // LF  (новая строка)
                    else if (tID[i] == 'v') { tOD += char(0x0B); i++; } // VT  (вертикальная табуляция)
                    else if (tID[i] == 'f') { tOD += char(0x0C); i++; } // FF  (новая страница)
                    else if (tID[i] == 'r') { tOD += char(0x0D); i++; } // CR  (начало строки)
                    else if (tID[i] == 's') { tOD += char(0x1A); i++; } // SUB (замена)
                    else if (tID[i] == 'e') { tOD += char(0x1B); i++; } // ESC (отмена)
                    else if (tID[i] == 'd') { tOD += char(0x7F); i++; } // DEL (удаление)
                    else tOD += tID[i++];
                    bufferTxData += tOD;
                    tOD = "";
                }
                else { Error = "MIXED mode syntax error !\r\nUnknown data prefix."; return false; }
            }
        }
    }
    else if (dataFormat == "FILE") {
        QFile file(dataSource);
        if (file.exists()) {
            if (file.open(QIODevice::ReadOnly)) {
                bufferTxData = file.readAll();
                file.close();
            }
            else { Error = "Error open file:  " + dataSource; return false; }
        }
        else { Error = "Not found file:  " + dataSource; return false; }
    }
    // символ-код завершения строки-данных
    if      (endChars == "CR") bufferTxData += char(0x0D);
    else if (endChars == "LF") bufferTxData += char(0x0A);
    else if (endChars == "CR+LF") { bufferTxData += char(0x0D); bufferTxData += char(0x0A); }
    else if (endChars == "SUB") bufferTxData += char(0x1A);
    else if (endChars == "ESC") bufferTxData += char(0x1B);
    else if (endChars == "TAB") bufferTxData += char(0x09);
    else if (endChars == "NUL") bufferTxData += char(0x00);
    return true;
}
bool MainWindow::SendData(QString port, QString typeData, int Param1, int Param2)
{
    if (port == "A" || port == "AB") ui->lineTotalTxA->clear();
    if (port == "B" || port == "AB") ui->lineTotalTxB->clear();

    if (ui->pushSingleMode->isChecked()) ui->textDataRx->clear();

    if      (typeData == "LINE") {
        if (BuildData(ui->comboFormatTx->currentText(), ui->lineDataTx->text(), ui->comboEndTx->currentText())) {
            if (ui->lineDataTx->text() != "") {
                lastTxLineF = ui->comboFormatTx->currentText();
                lastTxLineD = ui->lineDataTx->text();
                lastTxLineE = ui->comboEndTx->currentText();
            }
            if (ui->pushLocalEcho->isChecked() == true) {
                ui->textDataRx->setTextColor(QColor(176,48,48)); ui->textDataRx->append("<< ");
                if (ui->pushMarkerTx->isChecked()) ui->textDataRx->insertPlainText(port + " " + QTime::currentTime().toString("hh:mm:ss.zzz") + " ");
                ui->textDataRx->insertPlainText("LINE: ");
                ui->textDataRx->setTextColor(QColor(112,56,40)); ui->textDataRx->insertPlainText(ui->lineDataTx->text());
                if (ui->comboEndTx->currentText() != "NONE") {
                    ui->textDataRx->setTextColor(QColor(112,56,40)); ui->textDataRx->insertPlainText(" [" + ui->comboEndTx->currentText() + "]");
                }
            }
        }
        else { QMessageBox::warning(this, programName, Error); return false; }
    }
    else if (typeData == "PATTERN") {
        if (BuildData("FILE", programPath + "/Patterns/" + ui->comboPattern->currentText(), "NONE")) {
            if (ui->pushLocalEcho->isChecked() == true) {
                ui->textDataRx->setTextColor(QColor(176,48,48)); ui->textDataRx->append("<< ");
                if (ui->pushMarkerTx->isChecked()) ui->textDataRx->insertPlainText(port + " " + QTime::currentTime().toString("hh:mm:ss.zzz") + " ");
                ui->textDataRx->insertPlainText("PATTERN: ");
                ui->textDataRx->setTextColor(QColor(112,56,40)); ui->textDataRx->insertPlainText(ui->comboPattern->currentText());
            }
        }
        else { QMessageBox::warning(this, programName, Error); return false; }
    }
    else if (typeData == "FILE1") {
        if (BuildData("FILE", ui->lineFile1->text(), "NONE")) {
            if (ui->pushLocalEcho->isChecked() == true) {
                ui->textDataRx->setTextColor(QColor(176,48,48)); ui->textDataRx->append("<< ");
                if (ui->pushMarkerTx->isChecked()) ui->textDataRx->insertPlainText(port + " " + QTime::currentTime().toString("hh:mm:ss.zzz") + " ");
                ui->textDataRx->insertPlainText("FILE: ");
                ui->textDataRx->setTextColor(QColor(112,56,40)); ui->textDataRx->insertPlainText(ui->lineFile1->text());
                //ui->textDataRx->setTextColor(QColor(112,56,40)); ui->textDataRx->insertPlainText(QFileInfo(ui->lineFile->text()).fileName());
            }
        }
        else { QMessageBox::warning(this, programName, Error); return false; }
    }
    else if (typeData == "FILE2") {
        if (BuildData("FILE", ui->lineFile2->text(), "NONE")) {
            if (ui->pushLocalEcho->isChecked() == true) {
                ui->textDataRx->setTextColor(QColor(176,48,48)); ui->textDataRx->append("<< ");
                if (ui->pushMarkerTx->isChecked()) ui->textDataRx->insertPlainText(port + " " + QTime::currentTime().toString("hh:mm:ss.zzz") + " ");
                ui->textDataRx->insertPlainText("FILE: ");
                ui->textDataRx->setTextColor(QColor(112,56,40)); ui->textDataRx->insertPlainText(ui->lineFile2->text());
                //ui->textDataRx->setTextColor(QColor(112,56,40)); ui->textDataRx->insertPlainText(QFileInfo(ui->lineFile->text()).fileName());
            }
        }
        else { QMessageBox::warning(this, programName, Error); return false; }
    }
    else if (typeData == "TABLE") {
        if (BuildData(ui->tableData->item(Param1, 0)->text(), ui->tableData->item(Param1, 1)->text(), ui->tableData->item(Param1, 2)->text())) {
            if (ui->pushLocalEcho->isChecked() == true) {
                ui->textDataRx->setTextColor(QColor(176,48,48)); ui->textDataRx->append("<< ");
                if (ui->pushMarkerTx->isChecked()) ui->textDataRx->insertPlainText(port + " " + QTime::currentTime().toString("hh:mm:ss.zzz") + " ");
                ui->textDataRx->insertPlainText("TABLE F" + QString::number(Param1+1) + ": ");
                ui->textDataRx->setTextColor(QColor(112,56,40)); ui->textDataRx->insertPlainText(ui->tableData->item(Param1, 1)->text());
                if (ui->tableData->item(Param1, 2)->text() != "NONE") {
                    ui->textDataRx->setTextColor(QColor(112,56,40)); ui->textDataRx->insertPlainText(" [" + ui->tableData->item(Param1, 2)->text() + "]");
                }
            }
        }
        else { QMessageBox::warning(this, programName, Error); return false; }
    }
    else if (typeData == "COUNTER") {
        if ((port == "A" || port == "AB") && (TimerInitFlagA == true || (TimerCounterByteA < Param1 || TimerCounterByteA > Param2))) {
            TimerCounterByteA = Param1;
            TimerInitFlagA = false;
        }
        if (port == "B" && (TimerInitFlagB == true || (TimerCounterByteB < Param1 || TimerCounterByteB > Param2))) {
            TimerCounterByteB = Param1;
            TimerInitFlagB = false;
        }
        if (ui->pushLocalEcho->isChecked() == true) {
            ui->textDataRx->setTextColor(QColor(176,48,48)); ui->textDataRx->append("<< ");
            if (ui->pushMarkerTx->isChecked()) ui->textDataRx->insertPlainText(port + " " + QTime::currentTime().toString("hh:mm:ss.zzz") + " ");
            ui->textDataRx->insertPlainText("TIMER: ");
            ui->textDataRx->setTextColor(QColor(112,56,40));
            if (port == "A" || port == "AB") ui->textDataRx->insertPlainText(ui->comboTmrSourceA->currentText() + " [" + TimerCounterByteA + "]");
            if (port == "B")                 ui->textDataRx->insertPlainText(ui->comboTmrSourceB->currentText() + " [" + TimerCounterByteB + "]");
        }
        bufferTxData = "";
        if (port == "A" || port == "AB") { bufferTxData += uint8_t(TimerCounterByteA); TimerCounterByteA++; }
        if (port == "B")  { bufferTxData += uint8_t(TimerCounterByteB); TimerCounterByteB++; }

    }
    else { QMessageBox::warning(this, programName, "Unknown data transmission source !"); return false; }

    // передача буфера данных
    if (port == "A" || port == "AB") {
        if (serialA.isOpen()) {
            serialA.write(bufferTxData); serialA.flush();
            ui->lineTotalRxA->clear();
            ui->lineTotalTxA->setText(QString::number(bufferTxData.length()));
        }
        else ui->lineTotalTxA->setText("0");
    }
    if (port == "B" || port == "AB") {
        if (serialB.isOpen()) {
            serialB.write(bufferTxData); serialB.flush();
            ui->lineTotalRxB->clear();
            ui->lineTotalTxB->setText(QString::number(bufferTxData.length()));
        }
        else ui->lineTotalTxB->setText("0");
    }
    bufferTxData = "";

    return true;
}
void MainWindow::ShowReadData(QString port, QString typeData, bool Redraw)
{
    int gDataSizeA = 0;
    int gDataSizeB = 0;
    QByteArray binString  = "";
    QByteArray hexString  = "";
    QByteArray decString  = "";
    QByteArray charString = "";
    QByteArray temp = "";

    // добавление в буфферы отчетов + размеры для граф.данных
    if (Redraw == false) {
        if (ui->pushSingleMode->isChecked()) {
            if      (port == "A") bufferReportA = bufferRxDataA;
            else if (port == "B") bufferReportB = bufferRxDataB;
        }
        else {
            if      (port == "A") bufferReportA += bufferRxDataA;
            else if (port == "B") bufferReportB += bufferRxDataB;
        }
        gDataSizeA = bufferReportA.length();
        gDataSizeB = bufferReportB.length();
    }
    else {
        gDataSizeA = bufferShowFrameA.length();
        gDataSizeB = bufferShowFrameB.length();
    }
    // визуальный вывод данных
    if      (typeData == "CHAR") {
        if (ui->pushSingleMode->isChecked()) {
            if      (port == "A") { ui->labelTotalRxA->setText("Rx A:"); ui->lineTotalRxA->setText(QString::number(bufferRxDataA.length())); }
            else if (port == "B") { ui->labelTotalRxB->setText("Rx B:"); ui->lineTotalRxB->setText(QString::number(bufferRxDataB.length())); }
        }
        else {
            if      (port == "A") { ui->labelTotalRxA->setText("Rx A: " + QString::number(bufferRxDataA.length())); ui->lineTotalRxA->setText(QString::number(bufferReportA.length())); }
            else if (port == "B") { ui->labelTotalRxB->setText("Rx B: " + QString::number(bufferRxDataB.length())); ui->lineTotalRxB->setText(QString::number(bufferReportB.length())); }
        }
        ui->textDataRx->moveCursor(QTextCursor::End);
        if (ui->pushMarkerRx->isChecked() == true) {
            ui->textDataRx->setTextColor(QColor(112,0,112));
            if      (port == "A") ui->textDataRx->append(">> A " + markerRxDataAS + " " + markerRxDataAE);
            else if (port == "B") ui->textDataRx->append(">> B " + markerRxDataBS + " " + markerRxDataBE);
        }
        ui->textDataRx->setTextColor(QColor(0,112,64));
        if      (port == "A") ui->textDataRx->append(bufferRxDataA);
        else if (port == "B") ui->textDataRx->append(bufferRxDataB);
        ui->textDataRx->moveCursor(QTextCursor::End);
    }
    else if (typeData == "DEC") {
        if (ui->pushSingleMode->isChecked()) {
            if      (port == "A") { ui->labelTotalRxA->setText("Rx A:"); ui->lineTotalRxA->setText(QString::number(bufferRxDataA.length())); }
            else if (port == "B") { ui->labelTotalRxB->setText("Rx B:"); ui->lineTotalRxB->setText(QString::number(bufferRxDataB.length())); }
        }
        else {
            if      (port == "A") { ui->labelTotalRxA->setText("Rx A: " + QString::number(bufferRxDataA.length())); ui->lineTotalRxA->setText(QString::number(bufferReportA.length())); }
            else if (port == "B") { ui->labelTotalRxB->setText("Rx B: " + QString::number(bufferRxDataB.length())); ui->lineTotalRxB->setText(QString::number(bufferReportB.length())); }
        }
        ui->textDataRx->moveCursor(QTextCursor::End);
        if (ui->pushMarkerRx->isChecked() == true) {
            ui->textDataRx->setTextColor(QColor(112,0,112));
            if      (port == "A") ui->textDataRx->append(">> A " + markerRxDataAS + " " + markerRxDataAE);
            else if (port == "B") ui->textDataRx->append(">> B " + markerRxDataBS + " " + markerRxDataBE);
        }
        int dc = 0;
        int ds = 0, dsAlign = 0;
        int data = 0;
        if      (port == "A") ds = bufferRxDataA.length();
        else if (port == "B") ds = bufferRxDataB.length();
        if (ds % 8) dsAlign = ds + (8 - (ds % 8)); else dsAlign = ds;
        for (int i = 0; i < dsAlign; i++) {
            if (i < ds) {
                if      (port == "A") data = bufferRxDataA[i];
                else if (port == "B") data = bufferRxDataB[i];
            }
            else data = ' ';

            temp = "";
            if (i < ds) {
                if      (ui->comboTypeRx->currentText() == "Unsigned") {
                    temp += QString::number(uchar(data)).toLocal8Bit();
                }
                else if (ui->comboTypeRx->currentText() == "Signed") {
                    temp += QString::number(char(data)).toLocal8Bit();
                }
            }

            if      (temp.length() == 0) decString += "    ";
            else if (temp.length() == 1) decString += "   ";
            else if (temp.length() == 2) decString += "  ";
            else if (temp.length() == 3) decString += " ";
            decString += temp;

            decString += " "; dc++;
            if (data >= 0x20 && data <= 0x7E) charString += data; else charString += ".";
            if (dc >= 8) {
                ui->textDataRx->setTextColor(QColor(0,80,112)); ui->textDataRx->append(decString + " ");
                ui->textDataRx->setTextColor(QColor(0,112,64)); ui->textDataRx->insertPlainText(charString);
                decString = ""; charString = ""; dc = 0; }
        }
        if (decString.length()) {
            ui->textDataRx->setTextColor(QColor(0,80,112)); ui->textDataRx->append(decString + " ");
            ui->textDataRx->setTextColor(QColor(0,112,64)); ui->textDataRx->insertPlainText(charString);
        }
        ui->textDataRx->moveCursor(QTextCursor::End);
    }
    else if (typeData == "HEX") {
        if (ui->pushSingleMode->isChecked()) {
            if      (port == "A") { ui->labelTotalRxA->setText("Rx A:"); ui->lineTotalRxA->setText(QString::number(bufferRxDataA.length())); }
            else if (port == "B") { ui->labelTotalRxB->setText("Rx B:"); ui->lineTotalRxB->setText(QString::number(bufferRxDataB.length())); }
        }
        else {
            if      (port == "A") { ui->labelTotalRxA->setText("Rx A: " + QString::number(bufferRxDataA.length())); ui->lineTotalRxA->setText(QString::number(bufferReportA.length())); }
            else if (port == "B") { ui->labelTotalRxB->setText("Rx B: " + QString::number(bufferRxDataB.length())); ui->lineTotalRxB->setText(QString::number(bufferReportB.length())); }
        }
        ui->textDataRx->moveCursor(QTextCursor::End);
        if (ui->pushMarkerRx->isChecked() == true) {
            ui->textDataRx->setTextColor(QColor(112,0,112));
            if      (port == "A") ui->textDataRx->append(">> A " + markerRxDataAS + " " + markerRxDataAE);
            else if (port == "B") ui->textDataRx->append(">> B " + markerRxDataBS + " " + markerRxDataBE);
        }
        int hc = 0;
        int hs = 0, hsAlign = 0;
        int data = 0;
        if      (port == "A") hs = bufferRxDataA.length();
        else if (port == "B") hs = bufferRxDataB.length();
        if (hs % 16) hsAlign = hs + (16 - (hs % 16)); else hsAlign = hs;
        for (int i = 0; i < hsAlign; i++) {
            if (i < hs) {
                if      (port == "A") data = bufferRxDataA[i];
                else if (port == "B") data = bufferRxDataB[i];
            }
            else data = ' ';

            if (i < hs) {
                hexString += NibbleToHex(data,1);
                hexString += NibbleToHex(data,0);
            }
            else hexString += "  ";

            hexString += ' '; hc++;
            if (data >= 0x20 && data <= 0x7E) charString +=data; else charString += ".";
            if (hc >= 16) {
                ui->textDataRx->setTextColor(QColor(0,64,160)); ui->textDataRx->append(hexString + " ");
                ui->textDataRx->setTextColor(QColor(0,112,64)); ui->textDataRx->insertPlainText(charString);
                hexString = ""; charString = ""; hc = 0; }
        }
        if (hexString.length()) {
            ui->textDataRx->setTextColor(QColor(0,64,160)); ui->textDataRx->append(hexString + " ");
            ui->textDataRx->setTextColor(QColor(0,112,64)); ui->textDataRx->insertPlainText(charString);
        }
        ui->textDataRx->moveCursor(QTextCursor::End);
    }
    else if (typeData == "BIN") {
        if (ui->pushSingleMode->isChecked()) {
            if      (port == "A") { ui->labelTotalRxA->setText("Rx A:"); ui->lineTotalRxA->setText(QString::number(bufferRxDataA.length())); }
            else if (port == "B") { ui->labelTotalRxB->setText("Rx B:"); ui->lineTotalRxB->setText(QString::number(bufferRxDataB.length())); }
        }
        else {
            if      (port == "A") { ui->labelTotalRxA->setText("Rx A: " + QString::number(bufferRxDataA.length())); ui->lineTotalRxA->setText(QString::number(bufferReportA.length())); }
            else if (port == "B") { ui->labelTotalRxB->setText("Rx B: " + QString::number(bufferRxDataB.length())); ui->lineTotalRxB->setText(QString::number(bufferReportB.length())); }
        }
        ui->textDataRx->moveCursor(QTextCursor::End);
        if (ui->pushMarkerRx->isChecked() == true) {
            ui->textDataRx->setTextColor(QColor(112,0,112));
            if      (port == "A") ui->textDataRx->append(">> A " + markerRxDataAS + " " + markerRxDataAE);
            else if (port == "B") ui->textDataRx->append(">> B " + markerRxDataBS + " " + markerRxDataBE);
        }
        int bhc = 0;
        int bhs = 0, bhsAlign = 0;
        int data = 0;
        if      (port == "A") bhs = bufferRxDataA.length();
        else if (port == "B") bhs = bufferRxDataB.length();
        if (bhs % 4) bhsAlign = bhs + (4 - (bhs % 4)); else bhsAlign = bhs;
        for (int i = 0; i < bhsAlign; i++) {
            if (i < bhs) {
                if      (port == "A") data = bufferRxDataA[i];
                else if (port == "B") data = bufferRxDataB[i];
            }
            else data = ' ';

            if (i < bhs) {
                hexString += NibbleToHex(data,1);
                hexString += NibbleToHex(data,0);
            }
            else hexString += "  ";

            hexString += ' '; bhc++;

            if (i < bhs) {
                for (int j = 0; j < 8; j++) {
                    if ((data << j) & 0x80) binString += "1";
                    else binString += "0";
                }
            }
            else binString += "        ";
            binString += " ";

            if (data >= 0x20 && data <= 0x7E) charString +=data; else charString += ".";
            if (bhc >= 4) {
                ui->textDataRx->setTextColor(QColor(48,60,96)); ui->textDataRx->append(binString + " ");
                ui->textDataRx->setTextColor(QColor(0,64,160)); ui->textDataRx->insertPlainText(hexString + " ");
                ui->textDataRx->setTextColor(QColor(0,112,64)); ui->textDataRx->insertPlainText(charString);
                binString = ""; hexString = ""; charString = ""; bhc = 0;
            }
        }
        if (hexString.length()) {
            ui->textDataRx->setTextColor(QColor(48,60,96)); ui->textDataRx->append(binString + " ");
            ui->textDataRx->setTextColor(QColor(0,64,160)); ui->textDataRx->insertPlainText(hexString + " ");
            ui->textDataRx->setTextColor(QColor(0,112,64)); ui->textDataRx->insertPlainText(charString);
        }
        ui->textDataRx->moveCursor(QTextCursor::End);
    }
    else if (typeData == "SCOPE" || typeData == "CHART") {
        int flagApprox = 0;     // флаг применения аппроксимации
        int sizeFRAME  = 512;   // размер кадра графических данных (16-512 байт)
        int frameSub   = 0;     // значение коррекции sizeFRAME при SCOPE
        int widthPOINT = 1;     // ширина отсчета графика
        int widthGRAF  = 1;     // ширина линии графика

        int gs = 0;
        if (gDataSizeA >= gDataSizeB) gs = gDataSizeA; else gs = gDataSizeB;
        // определить параметры вывода
        if      (ui->comboFrameRx->currentText() == "AUTO") {
            if      (gs >= 0  && gs <= 16)  { flagApprox = 0; sizeFRAME = 16;  widthPOINT = 32; widthGRAF = 31; }
            else if (gs > 16  && gs <= 32)  { flagApprox = 0; sizeFRAME = 32;  widthPOINT = 16; widthGRAF = 15; }
            else if (gs > 32  && gs <= 64)  { flagApprox = 0; sizeFRAME = 64;  widthPOINT = 8;  widthGRAF = 7; }
            else if (gs > 64  && gs <= 128) { flagApprox = 0; sizeFRAME = 128; widthPOINT = 4;  widthGRAF = 3; }
            else if (gs > 128 && gs <= 256) { flagApprox = 0; sizeFRAME = 256; widthPOINT = 2;  widthGRAF = 1; }
            else if (gs > 256 && gs <= 512) { flagApprox = 0; sizeFRAME = 512; widthPOINT = 1;  widthGRAF = 1; }
            else if (gs > 512)              { flagApprox = 1; sizeFRAME = 512; widthPOINT = 1;  widthGRAF = 1; }
        }
        else if (ui->comboFrameRx->currentText() ==  "16") { flagApprox = 0; sizeFRAME = 16;  widthPOINT = 32; widthGRAF = 31; }
        else if (ui->comboFrameRx->currentText() ==  "32") { flagApprox = 0; sizeFRAME = 32;  widthPOINT = 16; widthGRAF = 15; }
        else if (ui->comboFrameRx->currentText() ==  "64") { flagApprox = 0; sizeFRAME = 64;  widthPOINT = 8;  widthGRAF = 7; }
        else if (ui->comboFrameRx->currentText() == "128") { flagApprox = 0; sizeFRAME = 128; widthPOINT = 4;  widthGRAF = 3; }
        else if (ui->comboFrameRx->currentText() == "256") { flagApprox = 0; sizeFRAME = 256; widthPOINT = 2;  widthGRAF = 1; }
        else if (ui->comboFrameRx->currentText() == "512") { flagApprox = 0; sizeFRAME = 512; widthPOINT = 1;  widthGRAF = 1; }
        else if (ui->comboFrameRx->currentText() == "FIT") { flagApprox = 1; sizeFRAME = 512; widthPOINT = 1;  widthGRAF = 1; }
        // вывод разрешения фрэйма
        if (ui->comboFrameRx->currentText() == "AUTO") {
            if (flagApprox == 1) ui->labelFrameRx->setText("Frame: FIT");
            else ui->labelFrameRx->setText("Frame: " + QString::number(sizeFRAME));
        }
        else ui->labelFrameRx->setText("Frame:");
        // вывод количества принятых байт данных
        if (gs) {
            if (ui->pushSingleMode->isChecked()) {
                if      (port == "A") { ui->labelTotalRxA->setText("Rx A:"); ui->lineTotalRxA->setText(QString::number(gDataSizeA)); }
                else if (port == "B") { ui->labelTotalRxB->setText("Rx B:"); ui->lineTotalRxB->setText(QString::number(gDataSizeB)); }
            }
            else {
                if      (port == "A") { ui->labelTotalRxA->setText("Rx A: " + QString::number(bufferRxDataA.length())); ui->lineTotalRxA->setText(QString::number(bufferReportA.length())); }
                else if (port == "B") { ui->labelTotalRxB->setText("Rx B: " + QString::number(bufferRxDataB.length())); ui->lineTotalRxB->setText(QString::number(bufferReportB.length())); }
            }
            ui->lineTotalRxA->repaint();
            ui->lineTotalRxB->repaint();
        }
        // копия данных для перерисовки
        bufferShowFrameA = bufferReportA;
        bufferShowFrameB = bufferReportB;
        // если формат SCOPE, уменшить кадр на одно значение
        if (typeData == "SCOPE") frameSub = 1;
        // создать ресурсы рисования
        QPen penGridS(QColor(128,128,128),1,Qt::SolidLine);
        QPen penGridD(QColor(80,80,80),1,Qt::DashLine);
        QPen penScopeA(QColor(232,232,128,200),1,Qt::SolidLine);
        QPen penScopeB(QColor(128,128,255,200),1,Qt::SolidLine);
        QPen penChartA(QColor(232,232,128,200),1,Qt::SolidLine);
        QPen penChartB(QColor(128,128,255,200),1,Qt::SolidLine);
//        QPen penScopeA(QColor(232,232,128,200),1,Qt::SolidLine);
//        QPen penScopeB(QColor(128,128,255,200),1,Qt::SolidLine);
//        QPen penChartA(QColor(255,128,128,200),1,Qt::SolidLine);
//        QPen penChartB(QColor(128,255,128,200),1,Qt::SolidLine);
        // начало вывода SCOPE,CHART
        ui->graphicsDataRx->setUpdatesEnabled(false); // приостановить обновление сцен
        scene->setSceneRect(0, 0, 512, 256); // задать область сцены
        scene->clear(); // очистить сцену
        // отобразить координатную сетку
        if      (ui->comboTypeRx->currentText() == "Unsigned") {
            scene->addLine(0,0,0,255,penGridS);
            scene->addLine(511,0,511,255,penGridD);
            scene->addLine(0,0,512,0,penGridD);
            scene->addLine(0,64,512,64,penGridD);
            scene->addLine(0,128,512,128,penGridD);
            scene->addLine(0,192,512,192,penGridD);
            scene->addLine(0,255,512,255,penGridS);
        }
        else if (ui->comboTypeRx->currentText() == "Signed") {
            scene->addLine(0,0,0,255,penGridS);
            scene->addLine(511,0,511,255,penGridD);
            scene->addLine(0,0,512,0,penGridD);
            scene->addLine(0,64,512,64,penGridD);
            scene->addLine(0,128,512,128,penGridS);
            scene->addLine(0,192,512,192,penGridD);
            scene->addLine(0,255,512,255,penGridD);
        }
        // отобразить данные буфера
        int ib, ibn = 1; // инициализация индексов данных буфера
        float aprx = 1;  // инициализация множителя аппроксимации
        if (flagApprox == 1) aprx = (float)gs/sizeFRAME; // вычисление множителя аппроксимации
        // вывод данных буферов на сцену
        if      (ui->comboTypeRx->currentText() == "Unsigned") {
            for (int i = 0; i < (sizeFRAME-frameSub) && (i < (gs/aprx)); i++)
            {
                ib = i * aprx;
                ibn = (i + 1) * aprx;
                if (typeData == "SCOPE") {
                    if (gDataSizeA >= ib) scene->addLine(i * widthPOINT, 255 - uint8_t(bufferShowFrameA[ib]), i * widthPOINT + widthPOINT, 255 - uint8_t(bufferShowFrameA[ibn]), penScopeA);
                    if (gDataSizeB >= ib) scene->addLine(i * widthPOINT, 255 - uint8_t(bufferShowFrameB[ib]), i * widthPOINT + widthPOINT, 255 - uint8_t(bufferShowFrameB[ibn]), penScopeB);
                }
                if (typeData == "CHART") {
                    if (gDataSizeA >= ib) { for (int j = 0; j < widthGRAF; j++)  scene->addLine(i * widthPOINT + j, 255, i * widthPOINT + j, 255 - uint8_t(bufferShowFrameA[ib]), penChartA); }
                    if (gDataSizeB >= ib) { for (int j = 0; j < widthGRAF; j++)  scene->addLine(i * widthPOINT + j, 255, i * widthPOINT + j, 255 - uint8_t(bufferShowFrameB[ib]), penChartB); }
                }
            }
        }
        else if (ui->comboTypeRx->currentText() == "Signed") {
            for (int i = 0; i < (sizeFRAME-frameSub) && (i < (gs/aprx)); i++)
            {
                ib = i * aprx;
                ibn = (i + 1) * aprx;
                if (typeData == "SCOPE") {
                    if (gDataSizeA >= ib) scene->addLine(i * widthPOINT, 255 - uint8_t(bufferShowFrameA[ib]+128), i * widthPOINT + widthPOINT, 255 - uint8_t(bufferShowFrameA[ibn]+128), penScopeA);
                    if (gDataSizeB >= ib) scene->addLine(i * widthPOINT, 255 - uint8_t(bufferShowFrameB[ib]+128), i * widthPOINT + widthPOINT, 255 - uint8_t(bufferShowFrameB[ibn]+128), penScopeB);
                }
                if (typeData == "CHART") {
                    if (gDataSizeA >= ib) { for (int j = 0; j < widthGRAF; j++)  scene->addLine(i * widthPOINT + j, 255, i * widthPOINT + j, 255 - uint8_t(bufferShowFrameA[ib]+128), penChartA); }
                    if (gDataSizeB >= ib) { for (int j = 0; j < widthGRAF; j++)  scene->addLine(i * widthPOINT + j, 255, i * widthPOINT + j, 255 - uint8_t(bufferShowFrameB[ib]+128), penChartB); }
                }
            }
        }
        ui->graphicsDataRx->setUpdatesEnabled(true); // возобновить обновление сцен
    }
}
// Подпрограммы работы с профилями настроек
void MainWindow::ListProfile()
{
    ui->comboProfile->clear();
    QDir path(programPath + "/Profiles/");
    QStringList profiles = path.entryList(QDir::Files);
    ui->comboProfile->addItems(profiles);
    ui->comboProfile->setCurrentText("default");
}
bool MainWindow::LoadProfile(QString nameFile)
{
    QFile file(programPath + "/Profiles/" + nameFile);
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream fileStream(&file);
            ui->comboSpeedPortA->setCurrentText(fileStream.readLine());
            ui->comboModePortA->setCurrentText(fileStream.readLine());
            ui->comboFlowPortA->setCurrentText(fileStream.readLine());
            ui->comboSpeedPortB->setCurrentText(fileStream.readLine());
            ui->comboModePortB->setCurrentText(fileStream.readLine());
            ui->comboFlowPortB->setCurrentText(fileStream.readLine());
            ui->comboRTO->setCurrentText(fileStream.readLine());
            ui->comboFormatRx->setCurrentText(fileStream.readLine());
            ui->comboTypeRx->setCurrentText(fileStream.readLine());
            ui->comboFrameRx->setCurrentText(fileStream.readLine());
            if (fileStream.readLine() == "SingleON") ui->pushSingleMode->setChecked(true);
            else ui->pushSingleMode->setChecked(false);
            if (fileStream.readLine() == "MarkerRxON" || (serialA.isOpen() && serialB.isOpen())) ui->pushMarkerRx->setChecked(true);
            else ui->pushMarkerRx->setChecked(false);
            if (fileStream.readLine() == "WordWrapON") { ui->pushWordWrap->setChecked(true); ui->textDataRx->setLineWrapMode(QTextEdit::WidgetWidth); }
            else { ui->pushWordWrap->setChecked(false); ui->textDataRx->setLineWrapMode(QTextEdit::NoWrap); }
            ui->comboFormatTx->setCurrentText(fileStream.readLine());
            ui->comboTypeTx->setCurrentText(fileStream.readLine());
            ui->comboEndTx->setCurrentText(fileStream.readLine());
            if (fileStream.readLine() == "LocalEchoON") ui->pushLocalEcho->setChecked(true);
            else ui->pushLocalEcho->setChecked(false);
            if (fileStream.readLine() == "MarkerTxON" || (serialA.isOpen() && serialB.isOpen())) ui->pushMarkerTx->setChecked(true);
            else ui->pushMarkerTx->setChecked(false);
            ui->comboSendPort->setCurrentText(fileStream.readLine());
            ui->lineDataTx->setText(fileStream.readLine());
            ui->comboPattern->setCurrentText(fileStream.readLine());
            ui->lineFile1->setText(fileStream.readLine());
            ui->lineFile2->setText(fileStream.readLine());
            ui->tableData->item(0,0)->setText(fileStream.readLine()); ui->tableData->item(0,1)->setText(fileStream.readLine()); ui->tableData->item(0,2)->setText(fileStream.readLine());
            ui->tableData->item(1,0)->setText(fileStream.readLine()); ui->tableData->item(1,1)->setText(fileStream.readLine()); ui->tableData->item(1,2)->setText(fileStream.readLine());
            ui->tableData->item(2,0)->setText(fileStream.readLine()); ui->tableData->item(2,1)->setText(fileStream.readLine()); ui->tableData->item(2,2)->setText(fileStream.readLine());
            ui->tableData->item(3,0)->setText(fileStream.readLine()); ui->tableData->item(3,1)->setText(fileStream.readLine()); ui->tableData->item(3,2)->setText(fileStream.readLine());
            ui->tableData->item(4,0)->setText(fileStream.readLine()); ui->tableData->item(4,1)->setText(fileStream.readLine()); ui->tableData->item(4,2)->setText(fileStream.readLine());
            ui->tableData->item(5,0)->setText(fileStream.readLine()); ui->tableData->item(5,1)->setText(fileStream.readLine()); ui->tableData->item(5,2)->setText(fileStream.readLine());
            ui->tableData->item(6,0)->setText(fileStream.readLine()); ui->tableData->item(6,1)->setText(fileStream.readLine()); ui->tableData->item(6,2)->setText(fileStream.readLine());
            ui->tableData->item(7,0)->setText(fileStream.readLine()); ui->tableData->item(7,1)->setText(fileStream.readLine()); ui->tableData->item(7,2)->setText(fileStream.readLine());
            ui->tableData->item(8,0)->setText(fileStream.readLine()); ui->tableData->item(8,1)->setText(fileStream.readLine()); ui->tableData->item(8,2)->setText(fileStream.readLine());
            ui->tableData->item(9,0)->setText(fileStream.readLine()); ui->tableData->item(9,1)->setText(fileStream.readLine()); ui->tableData->item(9,2)->setText(fileStream.readLine());
            ui->tableData->item(10,0)->setText(fileStream.readLine()); ui->tableData->item(10,1)->setText(fileStream.readLine()); ui->tableData->item(10,2)->setText(fileStream.readLine());
            ui->tableData->item(11,0)->setText(fileStream.readLine()); ui->tableData->item(11,1)->setText(fileStream.readLine()); ui->tableData->item(11,2)->setText(fileStream.readLine());
            ui->comboTmrSourceA->setCurrentText(fileStream.readLine());
            ui->comboTmrSourceB->setCurrentText(fileStream.readLine());
            ui->comboTmrTime->setCurrentText(fileStream.readLine());
            file.close();
            currentProfile = nameFile;
            currentProfileIndex = ui->comboProfile->currentIndex();
            if (ui->comboProfile->currentText() == "default") {
                ui->pushNewProfile->setEnabled(true);
                ui->pushRenProfile->setEnabled(false);
                ui->pushDelProfile->setEnabled(false);
                ui->pushSkipProfile->setEnabled(false);
            }
            else {
                ui->pushNewProfile->setEnabled(true);
                ui->pushRenProfile->setEnabled(true);
                ui->pushDelProfile->setEnabled(true);
                ui->pushSkipProfile->setEnabled(false);
            }
            ui->pushSaveProfile->setEnabled(false);
            return true;
        }
        else { QMessageBox::warning(this, programName, "Error load profile file \"" + nameFile + "\" !"); return false; }
    }
    else { QMessageBox::warning(this, programName, "Not found profile file \"" + nameFile + "\" !"); return false; }
}
bool MainWindow::SaveProfile(QString nameProfile, int mode)
{
    QFile file(programPath + "/Profiles/" + nameProfile);
    if (mode == 1 && file.exists()) {
        QMessageBox::warning(this, programName, "Profile \"" + nameProfile + "\" already exists !"); return false;
    }
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream fileStream(&file);
        fileStream << ui->comboSpeedPortA->currentText() + "\n";
        fileStream << ui->comboModePortA->currentText() + "\n";
        fileStream << ui->comboFlowPortA->currentText() + "\n";
        fileStream << ui->comboSpeedPortB->currentText() + "\n";
        fileStream << ui->comboModePortB->currentText() + "\n";
        fileStream << ui->comboFlowPortB->currentText() + "\n";
        fileStream << ui->comboRTO->currentText() + "\n";
        fileStream << ui->comboFormatRx->currentText() + "\n";
        fileStream << ui->comboTypeRx->currentText() + "\n";
        fileStream << ui->comboFrameRx->currentText() + "\n";
        if (ui->pushSingleMode->isChecked()) fileStream << "SingleON\n";
        else fileStream << "SingleOFF\n";
        if (ui->pushMarkerRx->isChecked()) fileStream << "MarkerRxON\n";
        else fileStream << "MarkerRxOFF\n";
        if (ui->pushWordWrap->isChecked()) fileStream << "WordWrapON\n";
        else fileStream << "WordWrapOFF\n";
        fileStream << ui->comboFormatTx->currentText() + "\n";
        fileStream << ui->comboTypeTx->currentText() + "\n";
        fileStream << ui->comboEndTx->currentText() + "\n";
        if (ui->pushLocalEcho->isChecked()) fileStream << "LocalEchoON\n";
        else fileStream << "LocalEchoOFF\n";
        if (ui->pushMarkerTx->isChecked()) fileStream << "MarkerTxON\n";
        else fileStream << "MarkerTxOFF\n";
        fileStream << ui->comboSendPort->currentText() + "\n";
        fileStream << ui->lineDataTx->text() + "\n";
        fileStream << ui->comboPattern->currentText() + "\n";
        fileStream << ui->lineFile1->text() + "\n";
        fileStream << ui->lineFile2->text() + "\n";
        fileStream << ui->tableData->item(0,0)->text() + "\n"; fileStream << ui->tableData->item(0,1)->text() + "\n"; fileStream << ui->tableData->item(0,2)->text() + "\n";
        fileStream << ui->tableData->item(1,0)->text() + "\n"; fileStream << ui->tableData->item(1,1)->text() + "\n"; fileStream << ui->tableData->item(1,2)->text() + "\n";
        fileStream << ui->tableData->item(2,0)->text() + "\n"; fileStream << ui->tableData->item(2,1)->text() + "\n"; fileStream << ui->tableData->item(2,2)->text() + "\n";
        fileStream << ui->tableData->item(3,0)->text() + "\n"; fileStream << ui->tableData->item(3,1)->text() + "\n"; fileStream << ui->tableData->item(3,2)->text() + "\n";
        fileStream << ui->tableData->item(4,0)->text() + "\n"; fileStream << ui->tableData->item(4,1)->text() + "\n"; fileStream << ui->tableData->item(4,2)->text() + "\n";
        fileStream << ui->tableData->item(5,0)->text() + "\n"; fileStream << ui->tableData->item(5,1)->text() + "\n"; fileStream << ui->tableData->item(5,2)->text() + "\n";
        fileStream << ui->tableData->item(6,0)->text() + "\n"; fileStream << ui->tableData->item(6,1)->text() + "\n"; fileStream << ui->tableData->item(6,2)->text() + "\n";
        fileStream << ui->tableData->item(7,0)->text() + "\n"; fileStream << ui->tableData->item(7,1)->text() + "\n"; fileStream << ui->tableData->item(7,2)->text() + "\n";
        fileStream << ui->tableData->item(8,0)->text() + "\n"; fileStream << ui->tableData->item(8,1)->text() + "\n"; fileStream << ui->tableData->item(8,2)->text() + "\n";
        fileStream << ui->tableData->item(9,0)->text() + "\n"; fileStream << ui->tableData->item(9,1)->text() + "\n"; fileStream << ui->tableData->item(9,2)->text() + "\n";
        fileStream << ui->tableData->item(10,0)->text() + "\n"; fileStream << ui->tableData->item(10,1)->text() + "\n"; fileStream << ui->tableData->item(10,2)->text() + "\n";
        fileStream << ui->tableData->item(11,0)->text() + "\n"; fileStream << ui->tableData->item(11,1)->text() + "\n"; fileStream << ui->tableData->item(11,2)->text() + "\n";
        fileStream << ui->comboTmrSourceA->currentText() + "\n";
        fileStream << ui->comboTmrSourceB->currentText() + "\n";
        fileStream << ui->comboTmrTime->currentText() + "\n";
        file.close();
        return true;
    }
    else { QMessageBox::warning(this, programName, "Error save profile file \"" + nameProfile + "\" !"); return false; }
}
bool MainWindow::RenameProfile(QString curNameProfile, QString newNameProfile)
{
    if (curNameProfile == newNameProfile) return false;
    QFile file(programPath + "/Profiles/" + newNameProfile);
    if (file.exists()) { QMessageBox::warning(this, programName, "Profile \"" + newNameProfile + "\" already exists !"); return false; }
    if (file.rename(programPath + "/Profiles/" + curNameProfile, programPath + "/Profiles/" + newNameProfile)) { return true; }
    else { QMessageBox::warning(this, programName, "Error rename profile file !"); return false; }
}
bool MainWindow::DeleteProfile(QString nameProfile)
{
    if (nameProfile == "default") { QMessageBox::information(this, programName, "Profile \"" + nameProfile + "\" cannot be deleted !"); return false; }
    QFile file(programPath + "/Profiles/" + nameProfile);
    if (QMessageBox::question(this, programName, "Profile \"" + nameProfile + "\" will be deleted !\r\nAre you sure ?") == QMessageBox::Yes) {
        if (file.remove()) { return true; }
        else { QMessageBox::warning(this, programName, "Error delete profile file \"" + nameProfile + "\" !"); return false; }
    }
    return false;
}
// Подпрограммы работы с паттернами
void MainWindow::ListPattern()
{
    ui->comboPattern->clear();
    QDir path(programPath + "/Patterns/");
    QStringList files = path.entryList(QDir::Files);
    ui->comboPattern->addItems(files);
}
// Подпрограммы прочие
char MainWindow::NibbleToHex(int inData, int numNibble)
{
    switch (numNibble)
    {
        case 0: inData >>= 0; break;
        case 1: inData >>= 4; break;
        case 2: inData >>= 8; break;
        case 3: inData >>= 12; break;
        case 4: inData >>= 16; break;
        case 5: inData >>= 20; break;
        case 6: inData >>= 24; break;
        case 7: inData >>= 28; break;
       default: inData = 0; break;
    }
    inData &= 0x0000000F; // Обнулить ненужные биты
    if (inData <= 0x09) inData += 0x30;
    if ((inData >= 0x0A) && (inData <= 0x0F)) inData += 0x37;
    return char(inData);
}
int  MainWindow::HexToNibble(char hex)
{
    if      ((hex >= '0') && (hex <= '9')) return hex - 0x30;
    else if ((hex >= 'A') && (hex <= 'F')) return hex - 0x37;
    else if ((hex >= 'a') && (hex <= 'f')) return hex - 0x57;
    else return 0;
}
void MainWindow::CalcSendData(QString srcData)
{
    QString nameFile = "";
    QFileInfo infoFile;
    int fileSize = 0;
    float fileSendTime = 0;
    int numBit = 10;
    int portSpeed = 1;

    // подготовка вычисления
    int portSpeedA;
    int portSpeedB;
    QString portMode;
    portSpeedA = ui->comboSpeedPortA->currentText().toInt();
    if (ui->comboSpeedPortB->currentText() == "Clone A") portSpeedB = portSpeedA;
    else portSpeedB = ui->comboSpeedPortB->currentText().toInt();
    if (portSpeedA <= portSpeedB) { portSpeed = portSpeedA; portMode = ui->comboModePortA->currentText(); }
    else { portSpeed = portSpeedB; portMode = ui->comboModePortB->currentText(); }
    if      (portMode == "8-N-1") numBit = 10;
    else if (portMode == "8-N-2") numBit = 11;
    else if (portMode == "8-E-1") numBit = 11;
    else if (portMode == "8-E-2") numBit = 12;
    else if (portMode == "8-O-1") numBit = 11;
    else if (portMode == "8-O-2") numBit = 12;

    // Pattern
    if (srcData == "Pattern" || srcData == "All") {
        nameFile = programPath + "/Patterns/" + ui->comboPattern->currentText();
        if (nameFile != "" && QFile::exists(nameFile)) {
            infoFile.setFile(nameFile);
            fileSize = infoFile.size();
            fileSendTime = (fileSize * numBit) / (float)portSpeed;
            ui->labelPatternInfo->setText(QString::number(fileSize) + " Byte    " + QString::number(fileSendTime) + " Sec");
        }
        else {
            ui->labelPatternInfo->setText("");
        }
    }
    // File 1
    if (srcData == "File1" || srcData == "All") {
        nameFile = ui->lineFile1->text();
        if (nameFile != "" && QFile::exists(nameFile)) {
            infoFile.setFile(nameFile);
            fileSize = infoFile.size();
            fileSendTime = (fileSize * numBit) / (float)portSpeed;
            ui->labelFileInfo1->setText(QString::number(fileSize) + " Byte    " + QString::number(fileSendTime) + " Sec");
        }
        else {
            ui->labelFileInfo1->setText("");
        }
    }
    // File 2
    if (srcData == "File2" || srcData == "All") {
        nameFile = ui->lineFile2->text();
        if (nameFile != "" && QFile::exists(nameFile)) {
            infoFile.setFile(nameFile);
            fileSize = infoFile.size();
            fileSendTime = (fileSize * numBit) / (float)portSpeed;
            ui->labelFileInfo2->setText(QString::number(fileSize) + " Byte    " + QString::number(fileSendTime) + " Sec");
        }
        else {
            ui->labelFileInfo2->setText("");
        }
    }
    // Table current row
    if (srcData == "Table" || srcData == "All") {
        int row = ui->tableData->currentIndex().row();
        if (row >= 0 && row <= 11) {
            if (ui->tableData->item(row,0)->text() == "FILE") {
                nameFile = ui->tableData->item(row,1)->text();
                if (nameFile != "" && QFile::exists(nameFile)) {
                    infoFile.setFile(nameFile);
                    fileSize = infoFile.size();
                    fileSendTime = (fileSize * numBit) / (float)portSpeed;
                    ui->labelTableInfo->setText(QString::number(fileSize) + " Byte    " + QString::number(fileSendTime) + " Sec");
                }
                else ui->labelTableInfo->setText("FILE NOT FOUND");
            }
            else {
                ui->labelTableInfo->setText("");
            }
        }
    }
}
void MainWindow::CorrectionTimings(QString target)
{
    int rto = ui->comboRTO->currentText().toInt();
    int tmr = ui->comboTmrTime->currentText().toInt();

    if (target == "TMR" && tmr > rto) return;

    if (target == "RTO" || target == "TMR") {
        if      (rto == 10  && (tmr < rto*2)) ui->comboTmrTime->setCurrentText("20");
        else if (rto == 20  && (tmr < rto*2)) ui->comboTmrTime->setCurrentText("50");
        else if (rto == 50  && (tmr < rto*2)) ui->comboTmrTime->setCurrentText("100");
        else if (rto == 100 && (tmr < rto*2)) ui->comboTmrTime->setCurrentText("200");
        else if (rto == 200 && (tmr < rto*2)) ui->comboTmrTime->setCurrentText("500");
        else if (rto == 500 && (tmr < rto*2)) ui->comboTmrTime->setCurrentText("1000");
    }
}

// ***********************************
// *** ОБРАБОТЧИКИ ЭЛЕМЕНТОВ ФОРМЫ ***
// ***********************************
// ===== MAIN WINDOW =====
// Перехватчики событий
void MainWindow::showEvent(QShowEvent *event)
{
    // если программа запущена - не обрабатывать это событие
    if (flagRunProg == true) return;
    else flagRunProg = true;

    event->ignore();

    // получить текущий путь программы
    programPath = QDir::currentPath();
    // проверить-загрузить конфигурацию программы
    CheckConfigApp();
    LoadConfigApp();
    // разрешить обработку событий от comboSelectPort A и B
    ui->comboSelectPortA->installEventFilter(this);
    ui->comboSelectPortB->installEventFilter(this);
    // начальная инициализация формы и данных
    this->setWindowTitle(programName);
    ListPort("ALL");
    ListProfile();
    ListPattern();
    // настройка меню кнопки REPORT
    QMenu *menuReport = new QMenu(this);
    reportView   = menuReport->addAction("View report files ...");
    menuReport->addSeparator();
    reportBinA   = menuReport->addAction("Create Port A Rx BINARY FILE");
    reportBinB   = menuReport->addAction("Create Port B Rx BINARY FILE");
    reportText   = menuReport->addAction("Create RD FRAME TEXT FILE");
    reportFrame  = menuReport->addAction("Create RD FRAME SCREENSHOT");
    reportBlock  = menuReport->addAction("Create RD BLOCK SCREENSHOT");
    reportWindow = menuReport->addAction("Create WINDOW SCREENSHOT");
    ui->pushReport->setMenu(menuReport);
    connect(menuReport, SIGNAL(triggered(QAction*)), SLOT(createReport(QAction*)));
    // настройка таблицы TRANSMIT TEMPLATES
    ui->tableData->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableData->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableData->setColumnWidth(0,45);
    ui->tableData->setColumnWidth(1,225);
    ui->tableData->setColumnWidth(2,45);
    for (int i = 0; i < 12; i++) {
        ui->tableData->item(i,0)->setBackground(QColor(224,224,224));
        ui->tableData->item(i,0)->setForeground(QColor(0,0,0));
        ui->tableData->item(i,0)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        ui->tableData->item(i,2)->setBackground(QColor(224,224,224));
        ui->tableData->item(i,2)->setForeground(QColor(0,0,0));
        ui->tableData->item(i,2)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    }
    // настройка обработки клика по заголовку строки таблицы
    QHeaderView *headRow = ui->tableData->verticalHeader();
    connect(headRow, SIGNAL(sectionClicked(int)), SLOT(ClickRowHeaderF1F12(int)));
    // загрузка профиля по умолчению
    ui->textDataRx->setVisible(true);
    ui->graphicsDataRx->setVisible(false);
    LoadProfile("default");
    ui->lineDataTx->setFocus();
    // проверка актуальной версии приложения
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyVersion(QNetworkReply*)));
    if (checkUpdates == true) { manager->get(QNetworkRequest(QUrl(linkUpdates))); }

    event->accept();
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();

    ClosePort("ALL",0);
    if (ui->pushSaveProfile->isEnabled()) {
        if (QMessageBox::question(this, programName, "The profile \"" + ui->comboProfile->currentText() + "\" was changed !\r\nSave profile ?") == QMessageBox::Yes) {
            SaveProfile(ui->comboProfile->currentText(), 0);
        }
    }

    event->accept();
}
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    int oldRow = -1;
    int key = event->key();
    if (timerTx->isActive() == true) oldRow = ui->tableData->currentRow();

    if      (key == Qt::Key_Up && ui->lineDataTx->hasFocus()) {
        ui->comboFormatTx->setCurrentText(lastTxLineF);
        ui->lineDataTx->setText(lastTxLineD);
        ui->comboEndTx->setCurrentText(lastTxLineE);
    }
    else if (key == Qt::Key_Down && ui->lineDataTx->hasFocus()) { ui->lineDataTx->clear(); }
    else if (key == Qt::Key_Escape && ui->comboProfile->isEditable()) {
        ui->comboProfile->setEditable(false);
        flagNewProfile = false;
        flagRenameProfile = false;
        LoadProfile(currentProfile);
        ui->frameACR->setEnabled(true);
        ui->groupPort->setEnabled(true);
        ui->groupRxData->setEnabled(true);
        ui->groupTxData->setEnabled(true);
        ui->groupTxTimer->setEnabled(true);
        ui->groupTxTemplates->setEnabled(true);
    }
    else if (ui->pushSendTx->isEnabled() == true) {
        if      (key == Qt::Key_F1)   { ui->tableData->selectRow(0);  SendData(ui->comboSendPort->currentText(), "TABLE", 0, 0);  ui->lineDataTx->setFocus(); }
        else if (key == Qt::Key_F2)   { ui->tableData->selectRow(1);  SendData(ui->comboSendPort->currentText(), "TABLE", 1, 0);  ui->lineDataTx->setFocus(); }
        else if (key == Qt::Key_F3)   { ui->tableData->selectRow(2);  SendData(ui->comboSendPort->currentText(), "TABLE", 2, 0);  ui->lineDataTx->setFocus(); }
        else if (key == Qt::Key_F4)   { ui->tableData->selectRow(3);  SendData(ui->comboSendPort->currentText(), "TABLE", 3, 0);  ui->lineDataTx->setFocus(); }
        else if (key == Qt::Key_F5)   { ui->tableData->selectRow(4);  SendData(ui->comboSendPort->currentText(), "TABLE", 4, 0);  ui->lineDataTx->setFocus(); }
        else if (key == Qt::Key_F6)   { ui->tableData->selectRow(5);  SendData(ui->comboSendPort->currentText(), "TABLE", 5, 0);  ui->lineDataTx->setFocus(); }
        else if (key == Qt::Key_F7)   { ui->tableData->selectRow(6);  SendData(ui->comboSendPort->currentText(), "TABLE", 6, 0);  ui->lineDataTx->setFocus(); }
        else if (key == Qt::Key_F8)   { ui->tableData->selectRow(7);  SendData(ui->comboSendPort->currentText(), "TABLE", 7, 0);  ui->lineDataTx->setFocus(); }
        else if (key == Qt::Key_F9)   { ui->tableData->selectRow(8);  SendData(ui->comboSendPort->currentText(), "TABLE", 8, 0);  ui->lineDataTx->setFocus(); }
        else if (key == Qt::Key_F10)  { ui->tableData->selectRow(9);  SendData(ui->comboSendPort->currentText(), "TABLE", 9, 0);  ui->lineDataTx->setFocus(); }
        else if (key == Qt::Key_F11)  { ui->tableData->selectRow(10); SendData(ui->comboSendPort->currentText(), "TABLE", 10, 0); ui->lineDataTx->setFocus(); }
        else if (key == Qt::Key_F12)  { ui->tableData->selectRow(11); SendData(ui->comboSendPort->currentText(), "TABLE", 11, 0); ui->lineDataTx->setFocus(); }
    }

    if (timerTx->isActive() == true) ui->tableData->selectRow(oldRow);
}
bool MainWindow::eventFilter(QObject *obj, QEvent *ev)
{
    if      (obj == ui->comboSelectPortA) {
        if(ev->type() == QEvent::MouseButtonPress) {
            QString t = ui->comboSelectPortA->currentText();
            ListPort("A");
            ui->comboSelectPortA->setCurrentText(t);
            return false;
        }
        else return false;
    }
    else if (obj == ui->comboSelectPortB) {
        if(ev->type() == QEvent::MouseButtonPress) {
            QString t = ui->comboSelectPortB->currentText();
            ListPort("B");
            ui->comboSelectPortB->setCurrentText(t);
            return false;
        }
        else return false;
    }
    else return MainWindow::eventFilter(obj, ev);
}
// Обработчики открытия диалоговых окон
void MainWindow::on_pushAbout_clicked()
{
    AboutDialog *aboutDlg = new AboutDialog();
    aboutDlg->show();
}
void MainWindow::on_pushConfig_clicked()
{
    ConfigDialog *configDlg = new ConfigDialog();
    configDlg->show();
}
// Обработчик слота создания и просмотра отчетов
void MainWindow::createReport(QAction *act)
{
    // Просмотр файлов отчетов
    if (act == reportView) {
        QString fileName = QFileDialog::getOpenFileName(this,"Open Report File",programPath + "/Reports","");
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
        return;
    }

    // Создание файлов отчетов
    QString fDir = programPath + "/Reports/" + "SerIO-" + QDateTime::currentDateTime().toString("yyMMdd-HHmmss");
    QString fFilter = "*.*";

    if      (act == reportBinA) { fDir += "-A.bin"; fFilter = "*.bin"; }
    else if (act == reportBinB) { fDir += "-B.bin"; fFilter = "*.bin"; }
    else if (act == reportText) { fDir += ".txt";   fFilter = "*.txt"; }
    else if (act == reportFrame || act == reportBlock || act == reportWindow) { fDir += ".png"; fFilter = "*.png"; }
    else return;

    QString fileName = QFileDialog::getSaveFileName(this,"Save Report File",fDir,fFilter);

    if (fileName != "") {
        QFile file(fileName);
        if      (act == reportBinA) {
            file.open(QIODevice::WriteOnly);
            file.write(bufferReportA);
            file.close();
        }
        else if (act == reportBinB) {
            file.open(QIODevice::WriteOnly);
            file.write(bufferReportB);
            file.close();
        }
        else if (act == reportText) {
            file.open(QIODevice::WriteOnly);
            file.write(ui->textDataRx->toPlainText().toUtf8());
            file.close();
        }
        else if (act == reportFrame || act == reportBlock || act == reportWindow) {
            QPixmap pixmaps;
            if (act == reportFrame) {
                if (ui->comboFormatRx->currentText() == "SCOPE" || ui->comboFormatRx->currentText() == "CHART") {
                    pixmaps = ui->graphicsDataRx->grab(ui->graphicsDataRx->rect());
                }
                else pixmaps = ui->textDataRx->grab(ui->textDataRx->rect());
            }
            else if (act == reportBlock) {
                pixmaps = ui->groupRxData->grab(ui->groupRxData->rect());
            }
            else if (act == reportWindow) {
                pixmaps = this->grab(this->rect());
            }
            pixmaps.save(fileName,"PNG");
        }
    }
}
// Обработчик слота проверки актуальной версии приложения
void MainWindow::replyVersion(QNetworkReply *reply)
{
    QString curAppVer;
    QString newAppVer;
    if (reply->error() == QNetworkReply::NoError)
    {
        curAppVer = programName + " " + programVersion;
        newAppVer = QString::fromUtf8(reply->readLine()).trimmed();
        if (curAppVer >= newAppVer) this->setWindowTitle("This is LATEST version - " + curAppVer + " " + programEdition);
        else this->setWindowTitle("Available NEW version - " + newAppVer);
        QTimer::singleShot(10000, this, SLOT(timerOutVersion()));
    }
    else {
        this->setWindowTitle("Failed check updates version - SerIO");
        QTimer::singleShot(10000, this, SLOT(timerOutVersion()));
    }
    reply->deleteLater(); // освободить память reply
}
void MainWindow::timerOutVersion()
{
    this->setWindowTitle(programName);
}
// ===== PROFILE =====
// Обработчики выбора и сохранения профилей
void MainWindow::on_comboProfile_activated(const QString &arg1)
{
    QString profile = arg1;

    if (ui->pushTmrRun->isChecked()) TimerTxMode("STOP");
    if (ui->pushSaveProfile->isEnabled()) {
        if (QMessageBox::question(this, programName, "The previous profile \"" + currentProfile + "\" was changed !\r\nSave changes ?") == QMessageBox::Yes) {
            SaveProfile(currentProfile, 0);
            ui->pushSaveProfile->setEnabled(false);
        }
    }
    // если активен флаг создания нового профиля
    if (flagNewProfile == true) {
        SaveProfile(profile, 1);
        flagNewProfile = false;
        ui->comboProfile->setEditable(false);
        ui->pushSaveProfile->setEnabled(false);
    }
    // если активен флаг переименования профиля
    else if (flagRenameProfile == true) {
        if (RenameProfile(currentProfile, profile)) {
            ui->comboProfile->removeItem(currentProfileIndex);
        }
        else {
            profile = currentProfile;
            ui->comboProfile->setCurrentText(currentProfile);
        }
        flagRenameProfile = false;
        ui->comboProfile->setEditable(false);
    }
    else {
        flagNewProfile = false;
        flagRenameProfile = false;
        ui->comboProfile->setEditable(false);
    }

    if (LoadProfile(profile) == false) { LoadProfile("default"); ui->comboProfile->setCurrentText("default"); }

    ui->frameACR->setEnabled(true);
    ui->groupPort->setEnabled(true);
    ui->groupRxData->setEnabled(true);
    ui->groupTxData->setEnabled(true);
    ui->groupTxTimer->setEnabled(true);
    ui->groupTxTemplates->setEnabled(true);
    //ui->pushSaveProfile->setEnabled(false);
    ui->lineTotalRxA->setText("");
    ui->lineTotalRxB->setText("");
    ui->lineTotalTxA->setText("");
    ui->lineTotalTxB->setText("");

    ui->lineDataTx->setFocus();
}
void MainWindow::on_pushSaveProfile_clicked()
{
    SaveProfile(ui->comboProfile->currentText(), 0);
    ui->pushSaveProfile->setEnabled(false);
    ui->comboProfile->setEditable(false);
    ui->lineDataTx->setFocus();
    ui->lineDataTx->deselect();
}
void MainWindow::on_pushNewProfile_clicked()
{
    ui->frameACR->setEnabled(false);
    ui->groupPort->setEnabled(false);
    ui->groupRxData->setEnabled(false);
    ui->groupTxData->setEnabled(false);
    ui->groupTxTimer->setEnabled(false);
    ui->groupTxTemplates->setEnabled(false);
    flagNewProfile = true;
    ui->comboProfile->setEditable(true);
    ui->comboProfile->setCurrentText("");
    ui->pushSaveProfile->setEnabled(false);
    ui->pushNewProfile->setEnabled(false);
    ui->pushRenProfile->setEnabled(false);
    ui->pushDelProfile->setEnabled(false);
    ui->pushSkipProfile->setEnabled(true);
    ui->comboProfile->setFocus();
}
void MainWindow::on_pushRenProfile_clicked()
{
    ui->frameACR->setEnabled(false);
    ui->groupPort->setEnabled(false);
    ui->groupRxData->setEnabled(false);
    ui->groupTxData->setEnabled(false);
    ui->groupTxTimer->setEnabled(false);
    ui->groupTxTemplates->setEnabled(false);
    flagRenameProfile = true;
    ui->comboProfile->setEditable(true);
    ui->pushSaveProfile->setEnabled(false);
    ui->pushNewProfile->setEnabled(false);
    ui->pushRenProfile->setEnabled(false);
    ui->pushDelProfile->setEnabled(false);
    ui->pushSkipProfile->setEnabled(true);
    ui->comboProfile->setFocus();
}
void MainWindow::on_pushDelProfile_clicked()
{
    if (DeleteProfile(ui->comboProfile->currentText())) {
        ui->comboProfile->removeItem(ui->comboProfile->currentIndex());
        LoadProfile("default"); ui->comboProfile->setCurrentText("default");
    }
}
void MainWindow::on_pushSkipProfile_clicked()
{
    flagNewProfile = false;
    flagRenameProfile = false;
    ui->comboProfile->setEditable(false);
    LoadProfile(currentProfile);
    ui->frameACR->setEnabled(true);
    ui->groupPort->setEnabled(true);
    ui->groupRxData->setEnabled(true);
    ui->groupTxData->setEnabled(true);
    ui->groupTxTimer->setEnabled(true);
    ui->groupTxTemplates->setEnabled(true);
}

// ===== SERIAL PORT =====
// Обработчики выбора последовательного порта
void MainWindow::on_pushPortA_clicked()
{
    if (ui->pushPortA->isChecked() == true) {
        if (OpenPort("A",ui->comboSelectPortA->currentText()) == false) {
            QMessageBox::warning(this, programName, "Error open Port A:  " + ui->comboSelectPortA->currentText());
            ui->pushPortA->setChecked(false);
        }
    }
    else {
        QString t = ui->comboSelectPortA->currentText();
        ClosePort("A",1);
        ui->comboSelectPortA->setCurrentText(t);
    }
    ui->lineDataTx->setFocus();
}
void MainWindow::on_pushPortB_clicked()
{
    if (ui->pushPortB->isChecked() == true) {
        if (OpenPort("B",ui->comboSelectPortB->currentText()) == false) {
            QMessageBox::warning(this, programName, "Error open Port B:  " + ui->comboSelectPortB->currentText());
            ui->pushPortB->setChecked(false);
        }
    }
    else {
        QString t = ui->comboSelectPortB->currentText();
        ClosePort("B",1);
        ui->comboSelectPortB->setCurrentText(t);
    }
    ui->lineDataTx->setFocus();
}
void MainWindow::on_comboSelectPortA_activated(const QString &arg1)
{
    if (arg1 == "CLOSE") ClosePort("A",0);
    else {
        if (serialA.isOpen()) serialA.close();
        if (OpenPort("A",arg1) == false) {
            QMessageBox::warning(this, programName, "Error open Port A:  " + arg1);
            ListPort("A");
            ui->comboSelectPortA->setCurrentText("CLOSE");
        }
    }
    if (serialA.isOpen() && serialB.isOpen()) ui->pushMarkerRx->setChecked(true);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_comboSelectPortB_activated(const QString &arg1)
{
    if (arg1 == "CLOSE") ClosePort("B",0);
    else {
        if (serialB.isOpen()) serialB.close();
        if (OpenPort("B", arg1) == false) {
            QMessageBox::warning(this, programName, "Error open Port B:  " + arg1);
            ListPort("B");
            ui->comboSelectPortB->setCurrentText("CLOSE");
        }
    }
    if (serialB.isOpen() && serialA.isOpen()) ui->pushMarkerRx->setChecked(true);
    ui->lineDataTx->setFocus();
}
// Обработчики изменения параметров последовательного порта
void MainWindow::on_comboSpeedPortA_currentTextChanged(const QString &arg1)
{
    QString tcp;
    if (arg1 == "") { }
    if (serialA.isOpen()) {
        tcp = ui->comboSelectPortA->currentText();
        serialA.close();
        OpenPort("A", tcp);
        ui->comboSelectPortA->setCurrentText(tcp);
    }
    if (serialB.isOpen() && ui->comboSpeedPortB->currentText() == "Clone A") {
        tcp = ui->comboSelectPortB->currentText();
        serialB.close();
        OpenPort("B", tcp);
        ui->comboSelectPortB->setCurrentText(tcp);
    }
    CalcSendData("All");
    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_comboSpeedPortB_currentTextChanged(const QString &arg1)
{
    QString tcp;
    if (arg1 == "") { }
    if (serialB.isOpen()) {
        tcp = ui->comboSelectPortB->currentText();
        serialB.close();
        OpenPort("B", tcp);
        ui->comboSelectPortB->setCurrentText(tcp);
    }
    CalcSendData("All");
    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_comboModePortA_currentTextChanged(const QString &arg1)
{
    QString tcp;
    if (arg1 == "") { }
    if (serialA.isOpen()) {
        tcp = ui->comboSelectPortA->currentText();
        serialA.close();
        OpenPort("A", tcp);
        ui->comboSelectPortA->setCurrentText(tcp);
    }
    if (serialB.isOpen() && ui->comboModePortB->currentText() == "Clone A") {
        tcp = ui->comboSelectPortB->currentText();
        serialB.close();
        OpenPort("B", tcp);
        ui->comboSelectPortB->setCurrentText(tcp);
    }
    CalcSendData("All");
    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_comboModePortB_currentTextChanged(const QString &arg1)
{
    QString tcp;
    if (arg1 == "") { }
    if (serialB.isOpen()) {
        tcp = ui->comboSelectPortB->currentText();
        serialB.close();
        OpenPort("B", tcp);
        ui->comboSelectPortB->setCurrentText(tcp);
    }
    CalcSendData("All");
    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_comboFlowPortA_currentTextChanged(const QString &arg1)
{
    QString tcp;
    if (arg1 == "") { }
    if (serialA.isOpen()) {
        tcp = ui->comboSelectPortA->currentText();
        serialA.close();
        OpenPort("A", tcp);
        ui->comboSelectPortA->setCurrentText(tcp);
    }
    if (serialB.isOpen() && ui->comboFlowPortB->currentText() == "Clone A") {
        tcp = ui->comboSelectPortB->currentText();
        serialB.close();
        OpenPort("B", tcp);
        ui->comboSelectPortB->setCurrentText(tcp);
    }
    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_comboFlowPortB_currentTextChanged(const QString &arg1)
{
    QString tcp;
    if (arg1 == "") { }
    if (serialB.isOpen()) {
        tcp = ui->comboSelectPortB->currentText();
        serialB.close();
        OpenPort("B", tcp);
        ui->comboSelectPortB->setCurrentText(tcp);
    }
    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_comboRTO_currentTextChanged(const QString &arg1)
{
    if (arg1 == "") { }
    timerRx->setInterval(ui->comboRTO->currentText().toInt());
    CorrectionTimings("RTO");
    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}

// ===== RECEIVE DATA =====
// Обработчик выбора параметров приема данных
void MainWindow::on_comboFormatRx_currentTextChanged(const QString &arg1)
{
    ui->labelFrameRx->setText("Frame:");
    if (arg1 == "CHAR" || arg1 == "HEX" || arg1 == "BIN") {
        ui->labelTypeRx->setEnabled(false); ui->comboTypeRx->setEnabled(false);
        ui->labelFrameRx->setEnabled(false); ui->comboFrameRx->setEnabled(false);
        ui->textDataRx->setVisible(true);
        ui->graphicsDataRx->setVisible(false);
    }
    else if (arg1 == "DEC") {
        ui->labelTypeRx->setEnabled(true); ui->comboTypeRx->setEnabled(true);
        ui->labelFrameRx->setEnabled(false); ui->comboFrameRx->setEnabled(false);
        ui->textDataRx->setVisible(true);
        ui->graphicsDataRx->setVisible(false);
    }
    else if (arg1 == "SCOPE" || arg1 == "CHART") {
        ui->labelTypeRx->setEnabled(true); ui->comboTypeRx->setEnabled(true);
        ui->labelFrameRx->setEnabled(true); ui->comboFrameRx->setEnabled(true);
        ui->textDataRx->setVisible(false);
        ui->graphicsDataRx->setVisible(true);

        //bufferRxDataA = bufferShowFrameA;
        ShowReadData("A", ui->comboFormatRx->currentText(), true);
        //bufferRxDataB = bufferShowFrameB;
        ShowReadData("B", ui->comboFormatRx->currentText(), true);
    }
    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_comboTypeRx_currentTextChanged(const QString &arg1)
{
    if (arg1 == "") { }
    ui->labelFrameRx->setText("Frame:");
    QString f = ui->comboFormatRx->currentText();
    if (f == "SCOPE" || f == "CHART") {
        //bufferRxDataA = bufferShowFrameA;
        ShowReadData("A", ui->comboFormatRx->currentText(), true);
        //bufferRxDataB = bufferShowFrameB;
        ShowReadData("B", ui->comboFormatRx->currentText(), true);
    }
    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_comboFrameRx_currentTextChanged(const QString &arg1)
{
    if (arg1 == "") { }
    QString f = ui->comboFormatRx->currentText();
    ui->labelFrameRx->setText("Frame:");
    if (f == "SCOPE" || f == "CHART") {
        //bufferRxDataA = bufferShowFrameA;
        ShowReadData("A", ui->comboFormatRx->currentText(), true);
        //bufferRxDataB = bufferShowFrameB;
        ShowReadData("B", ui->comboFormatRx->currentText(), true);
    }
    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_pushSingleMode_clicked()
{
    if (serialA.isOpen()) serialA.clear();
    if (serialB.isOpen()) serialB.clear();

    bufferRxDataA.clear(); bufferShowFrameA.clear(); bufferReportA.clear(); ui->labelTotalRxA->setText("Rx A:"); ui->lineTotalRxA->setText("");
    bufferRxDataB.clear(); bufferShowFrameB.clear(); bufferReportB.clear(); ui->labelTotalRxB->setText("Rx B:"); ui->lineTotalRxB->setText("");
    ui->textDataRx->clear();
    ShowReadData("A", "SCOPE", false);
    ShowReadData("B", "SCOPE", false);

    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_pushMarkerRx_clicked()
{
    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_pushWordWrap_clicked()
{
    if (ui->pushWordWrap->isChecked()) ui->textDataRx->setLineWrapMode(QTextEdit::WidgetWidth);
    else ui->textDataRx->setLineWrapMode(QTextEdit::NoWrap);
    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
// Обработчик кнопки очистки принятых данных
void MainWindow::on_pushClearRx_clicked()
{
    if (serialA.isOpen()) serialA.clear();
    if (serialB.isOpen()) serialB.clear();
    bufferRxDataA.clear(); bufferShowFrameA.clear(); bufferReportA.clear();
    bufferRxDataB.clear(); bufferShowFrameB.clear(); bufferReportB.clear();

    ui->textDataRx->clear();
    ShowReadData("A", "SCOPE", false);
    ShowReadData("B", "SCOPE", false);

    ui->labelTotalRxA->setText("Rx A:"); ui->lineTotalRxA->setText("");
    ui->labelTotalRxB->setText("Rx B:"); ui->lineTotalRxB->setText("");
    ui->lineTotalTxA->setText("");
    ui->lineTotalTxB->setText("");

    ui->lineDataTx->setFocus();
}
// Обработчик таймера приема данных
void MainWindow::timerReadPort()
{
    // flagRxPortX = 0  Нет данных
    //             = 1  Поступают данные
    //             = 2  Конец данных

    int RTO = ui->comboRTO->currentText().toInt();

    timerRx->stop();
    countRTOA += 10; countRTOB += 10;
    // проверка-прием данных
    if (serialA.bytesAvailable()) {
        countRTOA = 0;
        bufferRxDataA += serialA.readAll();
        if (flagRxPortA == 0) { flagRxPortA = 1; markerRxDataAS = QTime::currentTime().toString("hh:mm:ss.zzz"); }
    }
    if (flagRxPortA == 1 && countRTOA >= RTO) { flagRxPortA = 2; countRTOA = 0; markerRxDataAE = QTime::currentTime().toString("hh:mm:ss.zzz"); }

    if (serialB.bytesAvailable()) {
        countRTOB = 0;
        bufferRxDataB += serialB.readAll();
        if (flagRxPortB == 0) { flagRxPortB = 1; markerRxDataBS = QTime::currentTime().toString("hh:mm:ss.zzz"); }
    }
    if (flagRxPortB == 1 && countRTOB >= RTO) { flagRxPortB = 2; countRTOB = 0; markerRxDataBE = QTime::currentTime().toString("hh:mm:ss.zzz"); }

    // проверка-вывод данных
    if (flagRxPortA == 2) {
        flagRxPortA = 0;
        ShowReadData("A", ui->comboFormatRx->currentText(), false);
        bufferRxDataA = "";
    }
    if (flagRxPortB == 2) {
        flagRxPortB = 0;
        ShowReadData("B", ui->comboFormatRx->currentText(), false);
        bufferRxDataB = "";
    }
    timerRx->start(10);
}

// ===== TRANSMIT DATA =====
// Обработчики выбора параметров передачи данных
void MainWindow::on_comboFormatTx_currentTextChanged(const QString &arg1)
{
    if      (arg1 == "CHAR")  { ui->lineDataTx->setPlaceholderText(""); }
    else if (arg1 == "DEC")   { ui->lineDataTx->setPlaceholderText(" 0 255 -128 127"); }
    else if (arg1 == "HEX")   { ui->lineDataTx->setPlaceholderText(" 0 00 1F A5 7F 0001FA57F.."); }
    else if (arg1 == "BIN")   { ui->lineDataTx->setPlaceholderText(" 0 1010 10100101 000000000000101010100101.."); }
    else if (arg1 == "MIXED") { ui->lineDataTx->setPlaceholderText(" \"text\\r\\n\" 116 x65 b01111000 \"t\" \\r\\n \"\\\"text\\\"\" x0D0A"); }
    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_comboTypeTx_currentTextChanged(const QString &arg1)
{
    if (arg1 == "") { }
    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_comboEndTx_currentTextChanged(const QString &arg1)
{
    if (arg1 == "") { }
    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_pushLocalEcho_clicked()
{
    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_pushMarkerTx_clicked()
{
    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_comboSendPort_currentTextChanged(const QString &arg1)
{
    if (arg1 == "") { }
    if (ui->comboSendPort->currentText() == "A") { ui->lineTotalRxB->clear(); ui->lineTotalTxB->clear(); }
    if (ui->comboSendPort->currentText() == "B") { ui->lineTotalRxA->clear(); ui->lineTotalTxA->clear(); }

    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
// Обработчики отправки данных в порт
void MainWindow::on_lineDataTx_returnPressed()
{
    if (ui->pushSendTx->isEnabled() == true) {
        if (SendData(ui->comboSendPort->currentText(), "LINE", 0, 0) && timerTx->isActive() == false) ui->lineDataTx->clear();
    }
    ui->lineDataTx->setFocus();
}
void MainWindow::on_pushSendTx_clicked()
{
    if (SendData(ui->comboSendPort->currentText(), "LINE", 0, 0) && timerTx->isActive()==false) ui->lineDataTx->clear();
    ui->lineDataTx->setFocus();
}
void MainWindow::on_lineDataTx_customContextMenuRequested(const QPoint &pos)
{
    // создаем объект контекстного меню
    QMenu * lineMenu = new QMenu(this);
    lineMenu->setFont(fontContr);
    // создаём действия и элементы контекстного меню
    QAction * line_totableF1  = lineMenu->addAction("To Table F1");
    QAction * line_totableF2  = lineMenu->addAction("To Table F2");
    QAction * line_totableF3  = lineMenu->addAction("To Table F3");
    QAction * line_totableF4  = lineMenu->addAction("To Table F4");
    QAction * line_totableF5  = lineMenu->addAction("To Table F5");
    QAction * line_totableF6  = lineMenu->addAction("To Table F6");
    QAction * line_totableF7  = lineMenu->addAction("To Table F7");
    QAction * line_totableF8  = lineMenu->addAction("To Table F8");
    QAction * line_totableF9  = lineMenu->addAction("To Table F9");
    QAction * line_totableF10 = lineMenu->addAction("To Table F10");
    QAction * line_totableF11 = lineMenu->addAction("To Table F11");
    QAction * line_totableF12 = lineMenu->addAction("To Table F12");
    // вызов и обработка контекстного меню
    QAction * selectedItem = lineMenu->exec(ui->lineDataTx->mapToGlobal(pos));
    int row = 0;
    if      (selectedItem == line_totableF1) row = 1;
    else if (selectedItem == line_totableF2) row = 2;
    else if (selectedItem == line_totableF3) row = 3;
    else if (selectedItem == line_totableF4) row = 4;
    else if (selectedItem == line_totableF5) row = 5;
    else if (selectedItem == line_totableF6) row = 6;
    else if (selectedItem == line_totableF7) row = 7;
    else if (selectedItem == line_totableF8) row = 8;
    else if (selectedItem == line_totableF9) row = 9;
    else if (selectedItem == line_totableF10) row = 10;
    else if (selectedItem == line_totableF11) row = 11;
    else if (selectedItem == line_totableF12) row = 12;
    if (row >= 1 && row <=12) {
        row--;
        ui->tableData->item(row,0)->setText(ui->comboFormatTx->currentText());
        ui->tableData->item(row,1)->setText(ui->lineDataTx->text());
        ui->tableData->item(row,2)->setText(ui->comboEndTx->currentText());
    }
}

// ===== TRANSMIT TEMPLATES =====
// Обработчик выбора и передачи паттерна в порт
void MainWindow::on_comboPattern_currentTextChanged(const QString &arg1)
{
    if (arg1 == "") { }
    CalcSendData("Pattern");
    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_pushSendPattern_clicked()
{
    SendData(ui->comboSendPort->currentText(), "PATTERN", 0, 0);
    ui->lineDataTx->setFocus();
}
// Обработчики выбора и передачи файла в порт
void MainWindow::on_pushSelectFile1_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,"Select File 1","","");
    if (fileName != "") ui->lineFile1->setText(fileName);
    ui->lineFile1->setFocus();
}
void MainWindow::on_pushSelectFile2_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,"Select File 2","","");
    if (fileName != "") ui->lineFile2->setText(fileName);
    ui->lineFile2->setFocus();
}
void MainWindow::on_lineFile1_textChanged(const QString &arg1)
{
    if (arg1 == "") { }
    CalcSendData("File1");
    ui->pushSaveProfile->setEnabled(true);
}
void MainWindow::on_lineFile2_textChanged(const QString &arg1)
{
    if (arg1 == "") { }
    CalcSendData("File2");
    ui->pushSaveProfile->setEnabled(true);
}
void MainWindow::on_lineFile1_returnPressed()
{
    if (ui->pushSendTx->isEnabled() == true && ui->lineFile1->text() != "") SendData(ui->comboSendPort->currentText(), "FILE1", 0, 0);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_lineFile2_returnPressed()
{
    if (ui->pushSendTx->isEnabled() == true && ui->lineFile1->text() != "") SendData(ui->comboSendPort->currentText(), "FILE2", 0, 0);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_pushSendFile1_clicked()
{
    if (ui->lineFile1->text() != "") SendData(ui->comboSendPort->currentText(), "FILE1", 0, 0);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_pushSendFile2_clicked()
{
    if (ui->lineFile2->text() != "") SendData(ui->comboSendPort->currentText(), "FILE2", 0, 0);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_lineFile1_customContextMenuRequested(const QPoint &pos)
{
    // создаем объект контекстного меню
    QMenu * fileMenu = new QMenu(this);
    fileMenu->setFont(fontContr);
    // создаём действия и элементы контекстного меню
    QAction * file_totableF1  = fileMenu->addAction("To Table F1");
    QAction * file_totableF2  = fileMenu->addAction("To Table F2");
    QAction * file_totableF3  = fileMenu->addAction("To Table F3");
    QAction * file_totableF4  = fileMenu->addAction("To Table F4");
    QAction * file_totableF5  = fileMenu->addAction("To Table F5");
    QAction * file_totableF6  = fileMenu->addAction("To Table F6");
    QAction * file_totableF7  = fileMenu->addAction("To Table F7");
    QAction * file_totableF8  = fileMenu->addAction("To Table F8");
    QAction * file_totableF9  = fileMenu->addAction("To Table F9");
    QAction * file_totableF10 = fileMenu->addAction("To Table F10");
    QAction * file_totableF11 = fileMenu->addAction("To Table F11");
    QAction * file_totableF12 = fileMenu->addAction("To Table F12");
    // вызов и обработка контекстного меню
    QAction * selectedItem = fileMenu->exec(ui->lineFile1->mapToGlobal(pos));
    int row = 0;
    if      (selectedItem == file_totableF1) row = 1;
    else if (selectedItem == file_totableF2) row = 2;
    else if (selectedItem == file_totableF3) row = 3;
    else if (selectedItem == file_totableF4) row = 4;
    else if (selectedItem == file_totableF5) row = 5;
    else if (selectedItem == file_totableF6) row = 6;
    else if (selectedItem == file_totableF7) row = 7;
    else if (selectedItem == file_totableF8) row = 8;
    else if (selectedItem == file_totableF9) row = 9;
    else if (selectedItem == file_totableF10) row = 10;
    else if (selectedItem == file_totableF11) row = 11;
    else if (selectedItem == file_totableF12) row = 12;
    if (row >= 1 && row <=12) {
        row--;
        ui->tableData->item(row,0)->setText("FILE");
        ui->tableData->item(row,1)->setText(ui->lineFile1->text());
        ui->tableData->item(row,2)->setText("NONE");
    }
}
void MainWindow::on_lineFile2_customContextMenuRequested(const QPoint &pos)
{
    // создаем объект контекстного меню
    QMenu * fileMenu = new QMenu(this);
    fileMenu->setFont(fontContr);
    // создаём действия и элементы контекстного меню
    QAction * file_totableF1  = fileMenu->addAction("To Table F1");
    QAction * file_totableF2  = fileMenu->addAction("To Table F2");
    QAction * file_totableF3  = fileMenu->addAction("To Table F3");
    QAction * file_totableF4  = fileMenu->addAction("To Table F4");
    QAction * file_totableF5  = fileMenu->addAction("To Table F5");
    QAction * file_totableF6  = fileMenu->addAction("To Table F6");
    QAction * file_totableF7  = fileMenu->addAction("To Table F7");
    QAction * file_totableF8  = fileMenu->addAction("To Table F8");
    QAction * file_totableF9  = fileMenu->addAction("To Table F9");
    QAction * file_totableF10 = fileMenu->addAction("To Table F10");
    QAction * file_totableF11 = fileMenu->addAction("To Table F11");
    QAction * file_totableF12 = fileMenu->addAction("To Table F12");
    // вызов и обработка контекстного меню
    QAction * selectedItem = fileMenu->exec(ui->lineFile2->mapToGlobal(pos));
    int row = 0;
    if      (selectedItem == file_totableF1) row = 1;
    else if (selectedItem == file_totableF2) row = 2;
    else if (selectedItem == file_totableF3) row = 3;
    else if (selectedItem == file_totableF4) row = 4;
    else if (selectedItem == file_totableF5) row = 5;
    else if (selectedItem == file_totableF6) row = 6;
    else if (selectedItem == file_totableF7) row = 7;
    else if (selectedItem == file_totableF8) row = 8;
    else if (selectedItem == file_totableF9) row = 9;
    else if (selectedItem == file_totableF10) row = 10;
    else if (selectedItem == file_totableF11) row = 11;
    else if (selectedItem == file_totableF12) row = 12;
    if (row >= 1 && row <=12) {
        row--;
        ui->tableData->item(row,0)->setText("FILE");
        ui->tableData->item(row,1)->setText(ui->lineFile2->text());
        ui->tableData->item(row,2)->setText("NONE");
    }
}
// Обработчики отправки сомманды из таблицы в порт
void MainWindow::on_tableData_cellChanged(int row, int column)
{
    if ((row >= 0 && row <= 11) && (column >= 0 && column <= 1)) {  }
    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_tableData_itemSelectionChanged()
{
    CalcSendData("Table");
}
void MainWindow::ClickRowHeaderF1F12(int numRow)
{
    if ((ui->pushSendTx->isEnabled() == true) && (numRow >= 0 && numRow <= 11)) SendData(ui->comboSendPort->currentText(), "TABLE", numRow, 0);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_tableData_customContextMenuRequested(const QPoint &pos)
{
    int row = ui->tableData->currentRow();
    int col = ui->tableData->currentColumn();
    // меню выбора формата строки данных
    if (col == 0 && row >= 0) {
        // создаем объект контекстного меню
        QMenu * tableFormatMenu = new QMenu(this);
        tableFormatMenu->setFont(fontMonoT);
        // создаём действия и элементы контекстного меню
        QAction * format_char  = tableFormatMenu->addAction("CHAR");
        QAction * format_dec   = tableFormatMenu->addAction("DEC");
        QAction * format_hex   = tableFormatMenu->addAction("HEX");
        QAction * format_bin   = tableFormatMenu->addAction("BIN");
        QAction * format_mixed = tableFormatMenu->addAction("MIXED");
        QAction * format_file  = tableFormatMenu->addAction("FILE");
        // вызов и обработка контекстного меню
        QAction * selectedItem = tableFormatMenu->exec(ui->tableData->viewport()->mapToGlobal(pos));
        if      (selectedItem == format_char)  { ui->tableData->item(row,0)->setText("CHAR"); }
        else if (selectedItem == format_dec)   { ui->tableData->item(row,0)->setText("DEC"); }
        else if (selectedItem == format_hex)   { ui->tableData->item(row,0)->setText("HEX"); }
        else if (selectedItem == format_bin)   { ui->tableData->item(row,0)->setText("BIN"); }
        else if (selectedItem == format_mixed) { ui->tableData->item(row,0)->setText("MIXED"); }
        else if (selectedItem == format_file)  {
            QString fileName = QFileDialog::getOpenFileName(this,"Select File","","");
            if (fileName != "") {
                ui->tableData->item(row,0)->setText("FILE");
                ui->tableData->item(row,1)->setText(fileName);
                ui->tableData->item(row,2)->setText("NONE");
            }
        }
    }
    // меню строки данных
    else if (col == 1 && row >= 0) {
        // создаем объект контекстного меню
        QMenu * tableDataMenu = new QMenu(this);
        tableDataMenu->setFont(fontContr);
        // подключить системный буфер обмена
        //QClipboard * clipboard = QApplication::clipboard();
        // создаём действия и элементы контекстного меню
        QAction * data_cut     = tableDataMenu->addAction("Cut");
        QAction * data_copy    = tableDataMenu->addAction("Copy");
        QAction * data_paste   = tableDataMenu->addAction("Paste");
        tableDataMenu->addSeparator();
        QAction * data_toline  = tableDataMenu->addAction("To Line");
        QAction * data_tofile1 = tableDataMenu->addAction("To File 1");
        QAction * data_tofile2 = tableDataMenu->addAction("To File 2");
        tableDataMenu->addSeparator();
        QAction * data_resett = tableDataMenu->addAction("Reset Table");
        if (ui->tableData->item(row,0)->text() == "FILE") { data_toline->setEnabled(false); }
        else { data_tofile1->setEnabled(false); data_tofile2->setEnabled(false); }
        // вызов и обработка контекстного меню
        QAction* selectedItem = tableDataMenu->exec(ui->tableData->viewport()->mapToGlobal(pos));
        if (selectedItem == data_cut)  {
            ClipFormat = ui->tableData->item(row,0)->text();
            //clipboard->setText(ui->tableData->item(row,1)->text());
            ClipData = ui->tableData->item(row,1)->text();
            ClipEnd = ui->tableData->item(row,2)->text();
            ui->tableData->item(row,0)->setText("CHAR");
            ui->tableData->item(row,1)->setText("");
            ui->tableData->item(row,2)->setText("NONE");
        }
        else if (selectedItem == data_copy) {
            ClipFormat = ui->tableData->item(row,0)->text();
            //clipboard->setText(ui->tableData->item(row,1)->text());
            ClipData = ui->tableData->item(row,1)->text();
            ClipEnd = ui->tableData->item(row,2)->text();
        }
        else if (selectedItem == data_paste) {
            if (ClipFormat != "") ui->tableData->item(row,0)->setText(ClipFormat);
            //ui->tableData->item(row,1)->setText(clipboard->text());
            ui->tableData->item(row,1)->setText(ClipData);
            if (ClipEnd != "") ui->tableData->item(row,2)->setText(ClipEnd);
        }
        else if (selectedItem == data_toline) {
            ui->comboFormatTx->setCurrentText(ui->tableData->item(row,0)->text());
            ui->lineDataTx->setText(ui->tableData->item(row,1)->text());
            ui->comboEndTx->setCurrentText(ui->tableData->item(row,2)->text());
            ui->lineDataTx->setFocus();
        }
        else if (selectedItem == data_tofile1) {
            ui->lineFile1->setText(ui->tableData->item(row,1)->text());
            ui->pushSaveProfile->setEnabled(true);
            ui->lineDataTx->setFocus();
        }
        else if (selectedItem == data_tofile2) {
            ui->lineFile2->setText(ui->tableData->item(row,1)->text());
            ui->pushSaveProfile->setEnabled(true);
            ui->lineDataTx->setFocus();
        }
        else if (selectedItem == data_resett) {
            for (int r = 0; r < 12; r++) {
                ui->tableData->item(r,0)->setText("CHAR");
                ui->tableData->item(r,1)->setText("");
                ui->tableData->item(r,2)->setText("NONE");
            }
        }
    }
    // меню выбора терминатора строки данных
    else if (col == 2 && row >= 0) {
        // создаем объект контекстного меню
        QMenu * tableEndMenu = new QMenu(this);
        tableEndMenu->setFont(fontMonoT);
        // создаём действия и элементы контекстного меню
        QAction * end_none = tableEndMenu->addAction("NONE");
        QAction * end_cr   = tableEndMenu->addAction("CR");
        QAction * end_lf   = tableEndMenu->addAction("LF");
        QAction * end_crlf = tableEndMenu->addAction("CR+LF");
        QAction * end_sub  = tableEndMenu->addAction("SUB");
        QAction * end_esc  = tableEndMenu->addAction("ESC");
        QAction * end_tab  = tableEndMenu->addAction("TAB");
        QAction * end_nul  = tableEndMenu->addAction("NUL");
        //end_nul->setEnabled(false);   // активность элемента (пример)
        //endMenu->addSeparator();      // разделитель элементов (пример)
        // субменю (пример)
        //QMenu* sub_endMenu = new QMenu("&SubMenu", endMenu);
        //eosMenu->addMenu(sub_endMenu);
        //QAction * end_testSubMenu  = sub_endMenu->addAction("&Test SubMenu");
        // вызов и обработка контекстного меню
        QAction* selectedItem = tableEndMenu->exec(ui->tableData->viewport()->mapToGlobal(pos));
        if      (selectedItem == end_none) { ui->tableData->item(row,2)->setText("NONE"); }
        else if (selectedItem == end_cr)   { ui->tableData->item(row,2)->setText("CR"); }
        else if (selectedItem == end_lf)   { ui->tableData->item(row,2)->setText("LF"); }
        else if (selectedItem == end_crlf) { ui->tableData->item(row,2)->setText("CR+LF"); }
        else if (selectedItem == end_sub)  { ui->tableData->item(row,2)->setText("SUB"); }
        else if (selectedItem == end_esc)  { ui->tableData->item(row,2)->setText("ESC"); }
        else if (selectedItem == end_tab)  { ui->tableData->item(row,2)->setText("TAB"); }
        else if (selectedItem == end_nul)  { ui->tableData->item(row,2)->setText("NUL"); }
        //else if (selectedItem == end_testSubMenu)  { QApplication::beep(); } // обработка субменю (пример)
    }
}

// ===== TRANSMIT TIMER =====
// Обработчики выбора параметров и запуска таймера передачи данных
void MainWindow::on_comboTmrSourceA_currentTextChanged(const QString &arg1)
{
    if (arg1 == "") { }
    TimerInitFlagA = true; //
    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_comboTmrSourceB_currentTextChanged(const QString &arg1)
{
    if (arg1 == "") { }
    TimerInitFlagB = true; //
    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_comboTmrTime_currentTextChanged(const QString &arg1)
{
    if (arg1 == "") { }
    CorrectionTimings("TMR");
    timerTx->setInterval(ui->comboTmrTime->currentText().toInt());
    ui->pushSaveProfile->setEnabled(true);
    ui->lineDataTx->setFocus();
}
void MainWindow::on_pushTmrRun_clicked()
{
    if (ui->pushTmrRun->isChecked()) TimerTxMode("START");
    else TimerTxMode("STOP");

    if (ui->comboTmrSourceA->currentText() == "Line" || ui->comboTmrSourceB->currentText() == "Line") ui->lineDataTx->setFocus();
}
void MainWindow::on_pushTmrSend_clicked()
{
    bool flagSingleMode;

    QString sendPort = ui->comboSendPort->currentText();
    QString sourceA = ui->comboTmrSourceA->currentText();
    QString sourceB = ui->comboTmrSourceB->currentText();

    if (sendPort == "AB") {
        if (sourceA == sourceB || sourceB == "Clone A") {
            SendTimerSource("AB", sourceA);
        }
        else {
            if (SendTimerSource("A", sourceA) == true) {
                flagSingleMode = ui->pushSingleMode->isChecked(); ui->pushSingleMode->setChecked(false);
                SendTimerSource("B", sourceB);
                ui->pushSingleMode->setChecked(flagSingleMode);
            }
        }
    }
    else if (sendPort == "A") SendTimerSource("A", sourceA);
    else if (sendPort == "B") {
        if (sourceB == "Clone A") SendTimerSource("B", sourceA);
        else SendTimerSource("B", sourceB);
    }
}
// Обработчик таймера передачи данных
void MainWindow::timerWritePort()
{
    bool flagSingleMode;
    bool resultWrite = false;
    QString sendPort = ui->comboSendPort->currentText();
    QString sourceA = ui->comboTmrSourceA->currentText();
    QString sourceB = ui->comboTmrSourceB->currentText();

    timerTx->stop();

    if (sendPort == "AB") {
        if (sourceA == sourceB || sourceB == "Clone A") {
            resultWrite = SendTimerSource("AB", sourceA);
        }
        else {
            if ((resultWrite = SendTimerSource("A", sourceA)) == true) {
                flagSingleMode = ui->pushSingleMode->isChecked(); ui->pushSingleMode->setChecked(false);
                resultWrite = SendTimerSource("B", sourceB);
                ui->pushSingleMode->setChecked(flagSingleMode);
            }
        }
    }
    else if (sendPort == "A") resultWrite = SendTimerSource("A", sourceA);
    else if (sendPort == "B") {
        if (sourceB == "Clone A") resultWrite = SendTimerSource("B", sourceA);
        else resultWrite = SendTimerSource("B", sourceB);
    }


    if (resultWrite == true) {
        ui->labelTmrCount->setText(QString::number(++TimerSendCount));
        timerTx->start();
    }
    else TimerTxMode("STOP");
}

// =============================================================
