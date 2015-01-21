/********************************************************************************
** Form generated from reading UI file 'BlendingVideoSettingDialog.ui'
**
** Created: Wed Jan 21 16:53:23 2015
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BLENDINGVIDEOSETTINGDIALOG_H
#define UI_BLENDINGVIDEOSETTINGDIALOG_H

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

class Ui_BlendingVideoSettingDialog
{
public:
    QCheckBox *checkBoxHDImage;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLineEdit *lineEditStart;
    QLineEdit *lineEditEnd;
    QLineEdit *lineEditStep;

    void setupUi(QDialog *BlendingVideoSettingDialog)
    {
        if (BlendingVideoSettingDialog->objectName().isEmpty())
            BlendingVideoSettingDialog->setObjectName(QString::fromUtf8("BlendingVideoSettingDialog"));
        BlendingVideoSettingDialog->resize(241, 175);
        checkBoxHDImage = new QCheckBox(BlendingVideoSettingDialog);
        checkBoxHDImage->setObjectName(QString::fromUtf8("checkBoxHDImage"));
        checkBoxHDImage->setGeometry(QRect(40, 20, 70, 17));
        horizontalLayoutWidget = new QWidget(BlendingVideoSettingDialog);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(40, 120, 181, 41));
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

        label = new QLabel(BlendingVideoSettingDialog);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(40, 50, 46, 13));
        label_2 = new QLabel(BlendingVideoSettingDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(40, 70, 46, 13));
        label_3 = new QLabel(BlendingVideoSettingDialog);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(40, 90, 46, 13));
        lineEditStart = new QLineEdit(BlendingVideoSettingDialog);
        lineEditStart->setObjectName(QString::fromUtf8("lineEditStart"));
        lineEditStart->setGeometry(QRect(110, 50, 71, 20));
        lineEditEnd = new QLineEdit(BlendingVideoSettingDialog);
        lineEditEnd->setObjectName(QString::fromUtf8("lineEditEnd"));
        lineEditEnd->setGeometry(QRect(110, 70, 71, 20));
        lineEditStep = new QLineEdit(BlendingVideoSettingDialog);
        lineEditStep->setObjectName(QString::fromUtf8("lineEditStep"));
        lineEditStep->setGeometry(QRect(110, 90, 71, 20));

        retranslateUi(BlendingVideoSettingDialog);

        QMetaObject::connectSlotsByName(BlendingVideoSettingDialog);
    } // setupUi

    void retranslateUi(QDialog *BlendingVideoSettingDialog)
    {
        BlendingVideoSettingDialog->setWindowTitle(QApplication::translate("BlendingVideoSettingDialog", "Dialog", 0, QApplication::UnicodeUTF8));
        checkBoxHDImage->setText(QApplication::translate("BlendingVideoSettingDialog", "HD Image", 0, QApplication::UnicodeUTF8));
        okButton->setText(QApplication::translate("BlendingVideoSettingDialog", "OK", 0, QApplication::UnicodeUTF8));
        cancelButton->setText(QApplication::translate("BlendingVideoSettingDialog", "Cancel", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("BlendingVideoSettingDialog", "Start:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("BlendingVideoSettingDialog", "End:", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("BlendingVideoSettingDialog", "Step:", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class BlendingVideoSettingDialog: public Ui_BlendingVideoSettingDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BLENDINGVIDEOSETTINGDIALOG_H
