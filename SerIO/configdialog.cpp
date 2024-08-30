// ******************************************************
// ***  Application: "SerIO"                          ***
// ***      License: GNU General Public License v3    ***
// ***        Autor: Kozhevnikov Y.V.  ©2020  RUSSIA  ***
// ***       e-Mail: tvm-system@yandex.ru             ***
// ***          Web: tvm-system.ru                    ***
// ******************************************************

#include "configdialog.h"
#include "ui_configdialog.h"
#include <QFont>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QProcess>

extern QString programName;
extern QString programVersion;
extern QString programEdition;
extern QString programPath;

QFont fntGroup;
QFont fntLabel;
QFont fntContr;
QFont fntMonoD;
QFont fntMonoT;


// Процедуры конструктора-деструктора диалогового окна
ConfigDialog::ConfigDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ConfigDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true); // удалять виджет при закрытии
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint); // убирать кнопку с вопросом

    // получить текущую конфигурацию
    LoadConfig();
    ApplyFonts();
}
ConfigDialog::~ConfigDialog()
{
    delete ui;
}


// ********************
// *** ПОДПРОГРАММЫ ***
// ********************
void ConfigDialog::LoadConfig()
{
    // ***** загрузка конфигурации
    if (QFile::exists(programPath + "/serio.conf") == true) {
        QFile file(programPath + "/serio.conf");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream fileStream(&file);
            // шрифты программы
            fntGroup.setFamily(fileStream.readLine());
                fntGroup.setPointSize(fileStream.readLine().toInt());
                fntGroup.setBold(fileStream.readLine().toInt());
                fntGroup.setItalic(fileStream.readLine().toInt());
                ui->fontCombo1->setCurrentFont(fntGroup.family());
                ui->comboFontSize1->setCurrentText(QString::number(fntGroup.pointSize()));
                ui->pushFontBold1->setChecked(fntGroup.bold());
                ui->pushFontItalic1->setChecked(fntGroup.italic());
            fntLabel.setFamily(fileStream.readLine()); ui->fontCombo2->setCurrentFont(fntLabel.family());
                fntLabel.setPointSize(fileStream.readLine().toInt());
                fntLabel.setBold(fileStream.readLine().toInt());
                fntLabel.setItalic(fileStream.readLine().toInt());
                ui->fontCombo2->setCurrentFont(fntLabel.family());
                ui->comboFontSize2->setCurrentText(QString::number(fntLabel.pointSize()));
                ui->pushFontBold2->setChecked(fntLabel.bold());
                ui->pushFontItalic2->setChecked(fntLabel.italic());
            fntContr.setFamily(fileStream.readLine()); ui->fontCombo3->setCurrentFont(fntContr.family());
                fntContr.setPointSize(fileStream.readLine().toInt());
                fntContr.setBold(fileStream.readLine().toInt());
                fntContr.setItalic(fileStream.readLine().toInt());
                ui->fontCombo3->setCurrentFont(fntContr.family());
                ui->comboFontSize3->setCurrentText(QString::number(fntContr.pointSize()));
                ui->pushFontBold3->setChecked(fntContr.bold());
                ui->pushFontItalic3->setChecked(fntContr.italic());
            fntMonoD.setFamily(fileStream.readLine()); ui->fontCombo4->setCurrentFont(fntMonoD.family());
                fntMonoD.setPointSize(fileStream.readLine().toInt());
                fntMonoD.setBold(fileStream.readLine().toInt());
                fntMonoD.setItalic(fileStream.readLine().toInt());
                ui->fontCombo4->setCurrentFont(fntMonoD.family());
                ui->comboFontSize4->setCurrentText(QString::number(fntMonoD.pointSize()));
                ui->pushFontBold4->setChecked(fntMonoD.bold());
                ui->pushFontItalic4->setChecked(fntMonoD.italic());
            fntMonoT.setFamily(fileStream.readLine()); ui->fontCombo5->setCurrentFont(fntMonoT.family());
                fntMonoT.setPointSize(fileStream.readLine().toInt());
                fntMonoT.setBold(fileStream.readLine().toInt());
                fntMonoT.setItalic(fileStream.readLine().toInt());
                ui->fontCombo5->setCurrentFont(fntMonoT.family());
                ui->comboFontSize5->setCurrentText(QString::number(fntMonoT.pointSize()));
                ui->pushFontBold5->setChecked(fntMonoT.bold());
                ui->pushFontItalic5->setChecked(fntMonoT.italic());
            // проверка обновлений
            if (fileStream.readLine() == "CheckUpdatesON") ui->checkUpdates->setCheckState(Qt::Checked);
            else ui->checkUpdates->setCheckState(Qt::Unchecked);
            ui->lineLinkUpdates->setText(fileStream.readLine());
            // список скоростей портов
            ui->plainPortSpeed->clear();
            while (!fileStream.atEnd()) {
                QString tmp = fileStream.readLine();
                if (tmp != "") ui->plainPortSpeed->appendPlainText(tmp);
            }
            file.close();
        }
    }
    else {
        QMessageBox::warning(this, programName + " Config", "Error load \"serio.conf\" file !");
    }
}
void ConfigDialog::SaveConfig()
{
    QFile file(programPath + "/serio.conf");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream fileStream(&file);
        // шрифты
        fileStream << fntGroup.family() + "\n";
            fileStream << QString::number(fntGroup.pointSize()) + "\n";
            fileStream << QString::number(fntGroup.bold()) + "\n";
            fileStream << QString::number(fntGroup.italic()) + "\n";
        fileStream << fntLabel.family() + "\n";
            fileStream << QString::number(fntLabel.pointSize()) + "\n";
            fileStream << QString::number(fntLabel.bold()) + "\n";
            fileStream << QString::number(fntLabel.italic()) + "\n";
        fileStream << fntContr.family() + "\n";
            fileStream << QString::number(fntContr.pointSize()) + "\n";
            fileStream << QString::number(fntContr.bold()) + "\n";
            fileStream << QString::number(fntContr.italic()) + "\n";
        fileStream << fntMonoD.family() + "\n";
            fileStream << QString::number(fntMonoD.pointSize()) + "\n";
            fileStream << QString::number(fntMonoD.bold()) + "\n";
            fileStream << QString::number(fntMonoD.italic()) + "\n";
        fileStream << fntMonoT.family() + "\n";
            fileStream << QString::number(fntMonoT.pointSize()) + "\n";
            fileStream << QString::number(fntMonoT.bold()) + "\n";
            fileStream << QString::number(fntMonoT.italic()) + "\n";
        // разрешение и http адрес проверки обновлений
        if (ui->checkUpdates->checkState() == Qt::Checked) fileStream << "CheckUpdatesON\n";
        else fileStream << "CheckUpdatesOFF\n";
        fileStream << ui->lineLinkUpdates->text() + "\n";
        // скорости порта
        fileStream << ui->plainPortSpeed->toPlainText() + "\n";

        file.close();
    }
    else {
        QMessageBox::warning(this, programName + " Config", "Error save \"serio.conf\" file !");
    }
}
void ConfigDialog::ApplyFonts()
{
    // DIALOG WINDOW
    ConfigDialog::setFont(fntContr);
    ui->checkUpdates->setFont(fntContr);
    ui->lineLinkUpdates->setFont(fntContr);
    ui->pushApply->setFont(fntContr);
    // SERIAL PORT
    ui->groupCnfPort->setFont(fntGroup);
    ui->labelPortSpeed->setFont(fntLabel); ui->plainPortSpeed->setFont(fntContr);
    // FONTS
    ui->groupCnfFonts->setFont(fntGroup);
    ui->labelFont1->setFont(fntLabel); ui->fontCombo1->setFont(fntContr); ui->comboFontSize1->setFont(fntContr); ui->pushFontBold1->setFont(fntContr); ui->pushFontItalic1->setFont(fntContr);
    ui->labelFont2->setFont(fntLabel); ui->fontCombo2->setFont(fntContr); ui->comboFontSize2->setFont(fntContr); ui->pushFontBold2->setFont(fntContr); ui->pushFontItalic2->setFont(fntContr);
    ui->labelFont3->setFont(fntLabel); ui->fontCombo3->setFont(fntContr); ui->comboFontSize3->setFont(fntContr); ui->pushFontBold3->setFont(fntContr); ui->pushFontItalic3->setFont(fntContr);
    ui->labelFont4->setFont(fntLabel); ui->fontCombo4->setFont(fntMonoD); ui->comboFontSize4->setFont(fntContr); ui->pushFontBold4->setFont(fntContr); ui->pushFontItalic4->setFont(fntContr);
    ui->labelFont5->setFont(fntLabel); ui->fontCombo5->setFont(fntMonoT); ui->comboFontSize5->setFont(fntContr); ui->pushFontBold5->setFont(fntContr); ui->pushFontItalic5->setFont(fntContr);
}


// *************************************
// *** ОБРАБОТЧИКИ ЭЛЕМЕНТОВ ДИАЛОГА ***
// *************************************
// Настройка шрифта блоков (заголовки)
void ConfigDialog::on_fontCombo1_activated(const QString &arg1)
{
    fntGroup.setFamily(arg1);
    ApplyFonts();
}
void ConfigDialog::on_comboFontSize1_activated(const QString &arg1)
{
    fntGroup.setPointSize(arg1.toInt());
    ApplyFonts();
}
void ConfigDialog::on_pushFontBold1_clicked()
{
    fntGroup.setBold(ui->pushFontBold1->isChecked());
    ApplyFonts();
}
void ConfigDialog::on_pushFontItalic1_clicked()
{
    fntGroup.setItalic(ui->pushFontItalic1->isChecked());
    ApplyFonts();
}
// Настройка шрифта меток
void ConfigDialog::on_fontCombo2_activated(const QString &arg1)
{
    fntLabel.setFamily(arg1);
    ApplyFonts();
}
void ConfigDialog::on_comboFontSize2_activated(const QString &arg1)
{
    fntLabel.setPointSize(arg1.toInt());
    ApplyFonts();
}
void ConfigDialog::on_pushFontBold2_clicked()
{
    fntLabel.setBold(ui->pushFontBold2->isChecked());
    ApplyFonts();
}
void ConfigDialog::on_pushFontItalic2_clicked()
{
    fntLabel.setItalic(ui->pushFontItalic2->isChecked());
    ApplyFonts();
}
// Настройка шрифта контролов
void ConfigDialog::on_fontCombo3_activated(const QString &arg1)
{
    fntContr.setFamily(arg1);
    ApplyFonts();
}
void ConfigDialog::on_comboFontSize3_activated(const QString &arg1)
{
    fntContr.setPointSize(arg1.toInt());
    ApplyFonts();
}
void ConfigDialog::on_pushFontBold3_clicked()
{
    fntContr.setBold(ui->pushFontBold3->isChecked());
    ApplyFonts();
}
void ConfigDialog::on_pushFontItalic3_clicked()
{
    fntContr.setItalic(ui->pushFontItalic3->isChecked());
    ApplyFonts();
}
// Настройка шрифта оперативных данных
void ConfigDialog::on_fontCombo4_activated(const QString &arg1)
{
    fntMonoD.setFamily(arg1);
    ApplyFonts();
}
void ConfigDialog::on_comboFontSize4_activated(const QString &arg1)
{
    fntMonoD.setPointSize(arg1.toInt());
    ApplyFonts();
}
void ConfigDialog::on_pushFontBold4_clicked()
{
    fntMonoD.setBold(ui->pushFontBold4->isChecked());
    ApplyFonts();
}
void ConfigDialog::on_pushFontItalic4_clicked()
{
    fntMonoD.setItalic(ui->pushFontItalic4->isChecked());
    ApplyFonts();
}
// Настройка шрифта данных шаблонов
void ConfigDialog::on_fontCombo5_activated(const QString &arg1)
{
    fntMonoT.setFamily(arg1);
    ApplyFonts();
}
void ConfigDialog::on_comboFontSize5_activated(const QString &arg1)
{
    fntMonoT.setPointSize(arg1.toInt());
    ApplyFonts();
}
void ConfigDialog::on_pushFontBold5_clicked()
{
    fntMonoT.setBold(ui->pushFontBold5->isChecked());
    ApplyFonts();
}
void ConfigDialog::on_pushFontItalic5_clicked()
{
    fntMonoT.setItalic(ui->pushFontItalic5->isChecked());
    ApplyFonts();
}
// Применить настройки
void ConfigDialog::on_pushApply_clicked()
{
    SaveConfig();
    QProcess::startDetached(QApplication::applicationFilePath(), QStringList(), QApplication::applicationDirPath());
    QApplication::quit();
}

