/********************************************************************************
** Form generated from reading UI file 'StructureDetectionSettingDialog.ui'
**
** Created: Tue Jan 13 14:53:38 2015
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_STRUCTUREDETECTIONSETTINGDIALOG_H
#define UI_STRUCTUREDETECTIONSETTINGDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_StructureDetectionSettingDialog
{
public:
    QWidget *layoutWidget;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QLabel *label;
    QLineEdit *lineEditScale;
    QLabel *label_2;
    QLineEdit *lineEditDistance;

    void setupUi(QDialog *StructureDetectionSettingDialog)
    {
        if (StructureDetectionSettingDialog->objectName().isEmpty())
            StructureDetectionSettingDialog->setObjectName(QString::fromUtf8("StructureDetectionSettingDialog"));
        StructureDetectionSettingDialog->resize(268, 129);
        layoutWidget = new QWidget(StructureDetectionSettingDialog);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(80, 80, 171, 33));
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

        label = new QLabel(StructureDetectionSettingDialog);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 20, 171, 16));
        lineEditScale = new QLineEdit(StructureDetectionSettingDialog);
        lineEditScale->setObjectName(QString::fromUtf8("lineEditScale"));
        lineEditScale->setGeometry(QRect(190, 20, 61, 20));
        label_2 = new QLabel(StructureDetectionSettingDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(20, 40, 171, 16));
        lineEditDistance = new QLineEdit(StructureDetectionSettingDialog);
        lineEditDistance->setObjectName(QString::fromUtf8("lineEditDistance"));
        lineEditDistance->setGeometry(QRect(190, 40, 61, 20));

        retranslateUi(StructureDetectionSettingDialog);

        QMetaObject::connectSlotsByName(StructureDetectionSettingDialog);
    } // setupUi

    void retranslateUi(QDialog *StructureDetectionSettingDialog)
    {
        StructureDetectionSettingDialog->setWindowTitle(QApplication::translate("StructureDetectionSettingDialog", "Dialog", 0, QApplication::UnicodeUTF8));
        okButton->setText(QApplication::translate("StructureDetectionSettingDialog", "OK", 0, QApplication::UnicodeUTF8));
        cancelButton->setText(QApplication::translate("StructureDetectionSettingDialog", "Cancel", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("StructureDetectionSettingDialog", "Scale for Circle Hough Transform:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("StructureDetectionSettingDialog", "Distance for grouping vertices:", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class StructureDetectionSettingDialog: public Ui_StructureDetectionSettingDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STRUCTUREDETECTIONSETTINGDIALOG_H
