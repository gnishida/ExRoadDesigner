/********************************************************************************
** Form generated from reading UI file 'RotationVideoSettingDialog.ui'
**
** Created: Wed Jan 21 14:53:56 2015
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ROTATIONVIDEOSETTINGDIALOG_H
#define UI_ROTATIONVIDEOSETTINGDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_RotationVideoSettingDialog
{
public:
    QWidget *layoutWidget;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QCheckBox *checkBoxHDImage;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLineEdit *lineEditStart;
    QLineEdit *lineEditEnd;
    QLineEdit *lineEditStep;

    void setupUi(QDialog *RotationVideoSettingDialog)
    {
        if (RotationVideoSettingDialog->objectName().isEmpty())
            RotationVideoSettingDialog->setObjectName(QString::fromUtf8("RotationVideoSettingDialog"));
        RotationVideoSettingDialog->resize(244, 179);
        layoutWidget = new QWidget(RotationVideoSettingDialog);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(30, 130, 201, 33));
        hboxLayout = new QHBoxLayout(layoutWidget);
#ifndef Q_OS_MAC
        hboxLayout->setSpacing(6);
#endif
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        spacerItem = new QSpacerItem(131, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        okButton = new QPushButton(layoutWidget);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        hboxLayout->addWidget(okButton);

        cancelButton = new QPushButton(layoutWidget);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        hboxLayout->addWidget(cancelButton);

        checkBoxHDImage = new QCheckBox(RotationVideoSettingDialog);
        checkBoxHDImage->setObjectName(QString::fromUtf8("checkBoxHDImage"));
        checkBoxHDImage->setGeometry(QRect(20, 20, 70, 17));
        label = new QLabel(RotationVideoSettingDialog);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 50, 46, 13));
        label_2 = new QLabel(RotationVideoSettingDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 70, 46, 13));
        label_3 = new QLabel(RotationVideoSettingDialog);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(20, 90, 46, 13));
        lineEditStart = new QLineEdit(RotationVideoSettingDialog);
        lineEditStart->setObjectName(QString::fromUtf8("lineEditStart"));
        lineEditStart->setGeometry(QRect(80, 50, 71, 20));
        lineEditEnd = new QLineEdit(RotationVideoSettingDialog);
        lineEditEnd->setObjectName(QString::fromUtf8("lineEditEnd"));
        lineEditEnd->setGeometry(QRect(80, 70, 71, 20));
        lineEditStep = new QLineEdit(RotationVideoSettingDialog);
        lineEditStep->setObjectName(QString::fromUtf8("lineEditStep"));
        lineEditStep->setGeometry(QRect(80, 90, 71, 20));

        retranslateUi(RotationVideoSettingDialog);

        QMetaObject::connectSlotsByName(RotationVideoSettingDialog);
    } // setupUi

    void retranslateUi(QDialog *RotationVideoSettingDialog)
    {
        RotationVideoSettingDialog->setWindowTitle(QApplication::translate("RotationVideoSettingDialog", "Dialog", 0, QApplication::UnicodeUTF8));
        okButton->setText(QApplication::translate("RotationVideoSettingDialog", "OK", 0, QApplication::UnicodeUTF8));
        cancelButton->setText(QApplication::translate("RotationVideoSettingDialog", "Cancel", 0, QApplication::UnicodeUTF8));
        checkBoxHDImage->setText(QApplication::translate("RotationVideoSettingDialog", "HD Image", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("RotationVideoSettingDialog", "Start:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("RotationVideoSettingDialog", "End:", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("RotationVideoSettingDialog", "Step:", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class RotationVideoSettingDialog: public Ui_RotationVideoSettingDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ROTATIONVIDEOSETTINGDIALOG_H
