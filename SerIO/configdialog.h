#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog();

    void LoadConfig();
    void SaveConfig();
    void ApplyFonts();

private slots:
    void on_fontCombo1_activated(const QString &arg1);
    void on_comboFontSize1_activated(const QString &arg1);
    void on_pushFontBold1_clicked();
    void on_pushFontItalic1_clicked();

    void on_fontCombo2_activated(const QString &arg1);
    void on_comboFontSize2_activated(const QString &arg1);
    void on_pushFontBold2_clicked();
    void on_pushFontItalic2_clicked();

    void on_fontCombo3_activated(const QString &arg1);
    void on_comboFontSize3_activated(const QString &arg1);
    void on_pushFontBold3_clicked();
    void on_pushFontItalic3_clicked();

    void on_fontCombo4_activated(const QString &arg1);
    void on_comboFontSize4_activated(const QString &arg1);
    void on_pushFontBold4_clicked();
    void on_pushFontItalic4_clicked();

    void on_fontCombo5_activated(const QString &arg1);
    void on_comboFontSize5_activated(const QString &arg1);
    void on_pushFontBold5_clicked();
    void on_pushFontItalic5_clicked();

    void on_pushApply_clicked();

private:
    Ui::ConfigDialog *ui;
};

#endif // CONFIGDIALOG_H
