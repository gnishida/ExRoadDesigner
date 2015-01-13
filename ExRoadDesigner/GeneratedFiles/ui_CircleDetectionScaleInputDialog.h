/********************************************************************************
** Form generated from reading UI file 'CircleDetectionScaleInputDialog.ui'
**
** Created: Tue Jan 13 14:53:38 2015
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CIRCLEDETECTIONSCALEINPUTDIALOG_H
#define UI_CIRCLEDETECTIONSCALEINPUTDIALOG_H

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

class Ui_CircleDetectionScaleInputDialog
{
public:
    QLabel *label;
    QLineEdit *lineEditScale;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *okButton;
    QPushButton *cancelButton;

    void setupUi(QDialog *CircleDetectionScaleInputDialog)
    {
        if (CircleDetectionScaleInputDialog->objectName().isEmpty())
            CircleDetectionScaleInputDialog->setObjectName(QString::fromUtf8("CircleDetectionScaleInputDialog"));
        CircleDetectionScaleInputDialog->resize(207, 103);
        label = new QLabel(CircleDetectionScaleInputDialog);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(30, 20, 46, 13));
        lineEditScale = new QLineEdit(CircleDetectionScaleInputDialog);
        lineEditScale->setObjectName(QString::fromUtf8("lineEditScale"));
        lineEditScale->setGeometry(QRect(80, 20, 71, 20));
        horizontalLayoutWidget = new QWidget(CircleDetectionScaleInputDialog);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(10, 50, 181, 51));
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


        retranslateUi(CircleDetectionScaleInputDialog);

        QMetaObject::connectSlotsByName(CircleDetectionScaleInputDialog);
    } // setupUi

    void retranslateUi(QDialog *CircleDetectionScaleInputDialog)
    {
        CircleDetectionScaleInputDialog->setWindowTitle(QApplication::translate("CircleDetectionScaleInputDialog", "Dialog", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("CircleDetectionScaleInputDialog", "Scale:", 0, QApplication::UnicodeUTF8));
        okButton->setText(QApplication::translate("CircleDetectionScaleInputDialog", "OK", 0, QApplication::UnicodeUTF8));
        cancelButton->setText(QApplication::translate("CircleDetectionScaleInputDialog", "Cancel", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class CircleDetectionScaleInputDialog: public Ui_CircleDetectionScaleInputDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CIRCLEDETECTIONSCALEINPUTDIALOG_H
