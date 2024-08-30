// ******************************************************
// ***  Application: "SerIO"                          ***
// ***      License: GNU General Public License v3    ***
// ***        Autor: Kozhevnikov Y.V.  ©2020  RUSSIA  ***
// ***       e-Mail: tvm-system@yandex.ru             ***
// ***          Web: tvm-system.ru                    ***
// ******************************************************

#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <QDir>
#include <QFile>
#include <QFont>
#include <QDesktopServices>

extern QString programName;
extern QString programVersion;
extern QString programEdition;
extern QString programQtComp;
extern QString programPath;


// Процедуры конструктора-деструктора диалогового окна
AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent), ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true); // удалять виджет при закрытии
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint); // убирать кнопку с вопросом

    ui->labelProgName->setText(programName);
    ui->labelProgVersion->setText("Version  " + programVersion + " " + programEdition + " " + programQtComp);

    QString ff = "";
    QFile file(programPath + "/serio.conf");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        for (int i = 0; i < 9; i++) ff = file.readLine();
        file.close();
    }

    QFont fontName; fontName.setFamily(ff); fontName.setBold(true); fontName.setPointSize(18);
    QFont fontVers; fontVers.setFamily(ff); fontVers.setBold(true); fontVers.setPointSize(11);
    QFont fontText; fontText.setFamily(ff); fontText.setBold(false); fontText.setPointSize(10);
    ui->labelProgName->setFont(fontName);
    ui->labelProgVersion->setFont(fontVers);
    ui->textBrowserAbout->setFont(fontText);

    //ui->textBrowserAbout->clear();
    //ui->textBrowserAbout->setSource(QUrl::fromLocalFile(QFileInfo(programPath + "/serio.html").absoluteFilePath()));
    //ui->textBrowserAbout->setSource(QUrl(programPath + "/serio.html"));
}
AboutDialog::~AboutDialog()
{
    delete ui;
}


