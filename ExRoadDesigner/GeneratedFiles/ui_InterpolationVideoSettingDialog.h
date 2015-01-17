/********************************************************************************
** Form generated from reading UI file 'InterpolationVideoSettingDialog.ui'
**
** Created: Sat Jan 17 14:56:59 2015
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INTERPOLATIONVIDEOSETTINGDIALOG_H
#define UI_INTERPOLATIONVIDEOSETTINGDIALOG_H

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

class Ui_InterpolationVideoSettingDialog
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

    void setupUi(QDialog *InterpolationVideoSettingDialog)
    {
        if (InterpolationVideoSettingDialog->objectName().isEmpty())
            InterpolationVideoSettingDialog->setObjectName(QString::fromUtf8("InterpolationVideoSettingDialog"));
        InterpolationVideoSettingDialog->resize(232, 174);
        layoutWidget = new QWidget(InterpolationVideoSettingDialog);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(30, 120, 181, 41));
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

        checkBoxHDImage = new QCheckBox(InterpolationVideoSettingDialog);
        checkBoxHDImage->setObjectName(QString::fromUtf8("checkBoxHDImage"));
        checkBoxHDImage->setGeometry(QRect(20, 20, 81, 21));
        label = new QLabel(InterpolationVideoSettingDialog);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 50, 46, 13));
        label_2 = new QLabel(InterpolationVideoSettingDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 70, 46, 13));
        label_3 = new QLabel(InterpolationVideoSettingDialog);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(20, 90, 46, 13));
        lineEditStart = new QLineEdit(InterpolationVideoSettingDialog);
        lineEditStart->setObjectName(QString::fromUtf8("lineEditStart"));
        lineEditStart->setGeometry(QRect(90, 50, 71, 20));
        lineEditEnd = new QLineEdit(InterpolationVideoSettingDialog);
        lineEditEnd->setObjectName(QString::fromUtf8("lineEditEnd"));
        lineEditEnd->setGeometry(QRect(90, 70, 71, 20));
        lineEditStep = new QLineEdit(InterpolationVideoSettingDialog);
        lineEditStep->setObjectName(QString::fromUtf8("lineEditStep"));
        lineEditStep->setGeometry(QRect(90, 90, 71, 20));

        retranslateUi(InterpolationVideoSettingDialog);

        QMetaObject::connectSlotsByName(InterpolationVideoSettingDialog);
    } // setupUi

    void retranslateUi(QDialog *InterpolationVideoSettingDialog)
    {
        InterpolationVideoSettingDialog->setWindowTitle(QApplication::translate("InterpolationVideoSettingDialog", "Dialog", 0, QApplication::UnicodeUTF8));
        okButton->setText(QApplication::translate("InterpolationVideoSettingDialog", "OK", 0, QApplication::UnicodeUTF8));
        cancelButton->setText(QApplication::translate("InterpolationVideoSettingDialog", "Cancel", 0, QApplication::UnicodeUTF8));
        checkBoxHDImage->setText(QApplication::translate("InterpolationVideoSettingDialog", "HD Image", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("InterpolationVideoSettingDialog", "Start:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("InterpolationVideoSettingDialog", "End:", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("InterpolationVideoSettingDialog", "Step:", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class InterpolationVideoSettingDialog: public Ui_InterpolationVideoSettingDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INTERPOLATIONVIDEOSETTINGDIALOG_H
