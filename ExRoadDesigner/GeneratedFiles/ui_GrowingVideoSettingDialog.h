/********************************************************************************
** Form generated from reading UI file 'GrowingVideoSettingDialog.ui'
**
** Created: Thu Jan 15 11:26:50 2015
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GROWINGVIDEOSETTINGDIALOG_H
#define UI_GROWINGVIDEOSETTINGDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_GrowingVideoSettingDialog
{
public:
    QCheckBox *checkBoxHDImage;
    QLabel *label;
    QLineEdit *lineEditFrequency;
    QLabel *label_2;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QLabel *label_3;
    QLabel *label_4;
    QLineEdit *lineEditNumAvenueIterations;
    QLineEdit *lineEditNumStreetIterations;
    QComboBox *comboBoxGenerationMethod;

    void setupUi(QDialog *GrowingVideoSettingDialog)
    {
        if (GrowingVideoSettingDialog->objectName().isEmpty())
            GrowingVideoSettingDialog->setObjectName(QString::fromUtf8("GrowingVideoSettingDialog"));
        GrowingVideoSettingDialog->resize(283, 220);
        checkBoxHDImage = new QCheckBox(GrowingVideoSettingDialog);
        checkBoxHDImage->setObjectName(QString::fromUtf8("checkBoxHDImage"));
        checkBoxHDImage->setGeometry(QRect(30, 60, 70, 17));
        label = new QLabel(GrowingVideoSettingDialog);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(30, 90, 131, 16));
        lineEditFrequency = new QLineEdit(GrowingVideoSettingDialog);
        lineEditFrequency->setObjectName(QString::fromUtf8("lineEditFrequency"));
        lineEditFrequency->setGeometry(QRect(163, 90, 41, 20));
        label_2 = new QLabel(GrowingVideoSettingDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(210, 90, 41, 16));
        horizontalLayoutWidget = new QWidget(GrowingVideoSettingDialog);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(80, 170, 191, 41));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        okButton = new QPushButton(horizontalLayoutWidget);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        horizontalLayout->addWidget(okButton);

        cancelButton = new QPushButton(horizontalLayoutWidget);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        horizontalLayout->addWidget(cancelButton);

        label_3 = new QLabel(GrowingVideoSettingDialog);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(30, 120, 101, 16));
        label_4 = new QLabel(GrowingVideoSettingDialog);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(30, 140, 101, 16));
        lineEditNumAvenueIterations = new QLineEdit(GrowingVideoSettingDialog);
        lineEditNumAvenueIterations->setObjectName(QString::fromUtf8("lineEditNumAvenueIterations"));
        lineEditNumAvenueIterations->setGeometry(QRect(130, 120, 61, 20));
        lineEditNumStreetIterations = new QLineEdit(GrowingVideoSettingDialog);
        lineEditNumStreetIterations->setObjectName(QString::fromUtf8("lineEditNumStreetIterations"));
        lineEditNumStreetIterations->setGeometry(QRect(130, 140, 61, 20));
        comboBoxGenerationMethod = new QComboBox(GrowingVideoSettingDialog);
        comboBoxGenerationMethod->setObjectName(QString::fromUtf8("comboBoxGenerationMethod"));
        comboBoxGenerationMethod->setGeometry(QRect(28, 20, 161, 22));

        retranslateUi(GrowingVideoSettingDialog);

        QMetaObject::connectSlotsByName(GrowingVideoSettingDialog);
    } // setupUi

    void retranslateUi(QDialog *GrowingVideoSettingDialog)
    {
        GrowingVideoSettingDialog->setWindowTitle(QApplication::translate("GrowingVideoSettingDialog", "Dialog", 0, QApplication::UnicodeUTF8));
        checkBoxHDImage->setText(QApplication::translate("GrowingVideoSettingDialog", "HD Image", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("GrowingVideoSettingDialog", "Capture Frequency: Every ", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("GrowingVideoSettingDialog", "frames", 0, QApplication::UnicodeUTF8));
        okButton->setText(QApplication::translate("GrowingVideoSettingDialog", "OK", 0, QApplication::UnicodeUTF8));
        cancelButton->setText(QApplication::translate("GrowingVideoSettingDialog", "Cancel", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("GrowingVideoSettingDialog", "Avenue Iteration:", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("GrowingVideoSettingDialog", "Street Iteration:", 0, QApplication::UnicodeUTF8));
        comboBoxGenerationMethod->clear();
        comboBoxGenerationMethod->insertItems(0, QStringList()
         << QApplication::translate("GrowingVideoSettingDialog", "Multi Examples", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("GrowingVideoSettingDialog", "Warp", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("GrowingVideoSettingDialog", "Procedural Modeling", 0, QApplication::UnicodeUTF8)
        );
    } // retranslateUi

};

namespace Ui {
    class GrowingVideoSettingDialog: public Ui_GrowingVideoSettingDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GROWINGVIDEOSETTINGDIALOG_H
