/********************************************************************************
** Form generated from reading UI file 'VideoCaptureSettingDialog.ui'
**
** Created: Fri Jan 23 16:30:31 2015
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VIDEOCAPTURESETTINGDIALOG_H
#define UI_VIDEOCAPTURESETTINGDIALOG_H

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

class Ui_VideoCaptureSettingDialog
{
public:
    QLabel *label;
    QLabel *label_2;
    QLineEdit *lineEditCameraStep;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *cancelButton;
    QPushButton *okButton;
    QCheckBox *checkBoxHDImage;

    void setupUi(QDialog *VideoCaptureSettingDialog)
    {
        if (VideoCaptureSettingDialog->objectName().isEmpty())
            VideoCaptureSettingDialog->setObjectName(QString::fromUtf8("VideoCaptureSettingDialog"));
        VideoCaptureSettingDialog->resize(245, 130);
        label = new QLabel(VideoCaptureSettingDialog);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(30, 50, 71, 21));
        label_2 = new QLabel(VideoCaptureSettingDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(170, 50, 31, 21));
        lineEditCameraStep = new QLineEdit(VideoCaptureSettingDialog);
        lineEditCameraStep->setObjectName(QString::fromUtf8("lineEditCameraStep"));
        lineEditCameraStep->setGeometry(QRect(110, 50, 51, 20));
        horizontalLayoutWidget = new QWidget(VideoCaptureSettingDialog);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(50, 80, 181, 41));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        cancelButton = new QPushButton(horizontalLayoutWidget);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        horizontalLayout->addWidget(cancelButton);

        okButton = new QPushButton(horizontalLayoutWidget);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        horizontalLayout->addWidget(okButton);

        checkBoxHDImage = new QCheckBox(VideoCaptureSettingDialog);
        checkBoxHDImage->setObjectName(QString::fromUtf8("checkBoxHDImage"));
        checkBoxHDImage->setGeometry(QRect(30, 20, 81, 17));

        retranslateUi(VideoCaptureSettingDialog);

        QMetaObject::connectSlotsByName(VideoCaptureSettingDialog);
    } // setupUi

    void retranslateUi(QDialog *VideoCaptureSettingDialog)
    {
        VideoCaptureSettingDialog->setWindowTitle(QApplication::translate("VideoCaptureSettingDialog", "Dialog", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("VideoCaptureSettingDialog", "Camera step: ", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("VideoCaptureSettingDialog", "[m]", 0, QApplication::UnicodeUTF8));
        cancelButton->setText(QApplication::translate("VideoCaptureSettingDialog", "Cancel", 0, QApplication::UnicodeUTF8));
        okButton->setText(QApplication::translate("VideoCaptureSettingDialog", "OK", 0, QApplication::UnicodeUTF8));
        checkBoxHDImage->setText(QApplication::translate("VideoCaptureSettingDialog", "HD Image", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class VideoCaptureSettingDialog: public Ui_VideoCaptureSettingDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VIDEOCAPTURESETTINGDIALOG_H
