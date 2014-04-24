/********************************************************************************
** Form generated from reading UI file 'TerrainSizeInputDialog.ui'
**
** Created: Thu Apr 24 09:55:32 2014
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TERRAINSIZEINPUTDIALOG_H
#define UI_TERRAINSIZEINPUTDIALOG_H

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

class Ui_TerrainSizeInputDialog
{
public:
    QWidget *layoutWidget;
    QHBoxLayout *hboxLayout;
    QSpacerItem *spacerItem;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QLabel *label;
    QLineEdit *lineEditWidth;
    QLabel *label_2;
    QLabel *label_3;
    QLineEdit *lineEditDepth;
    QLabel *label_4;
    QLabel *label_5;
    QLineEdit *lineEditCellLength;
    QLabel *label_6;

    void setupUi(QDialog *TerrainSizeInputDialog)
    {
        if (TerrainSizeInputDialog->objectName().isEmpty())
            TerrainSizeInputDialog->setObjectName(QString::fromUtf8("TerrainSizeInputDialog"));
        TerrainSizeInputDialog->resize(220, 151);
        layoutWidget = new QWidget(TerrainSizeInputDialog);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(10, 110, 201, 33));
        hboxLayout = new QHBoxLayout(layoutWidget);
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
        hboxLayout->setContentsMargins(0, 0, 0, 0);
        spacerItem = new QSpacerItem(31, 31, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hboxLayout->addItem(spacerItem);

        okButton = new QPushButton(layoutWidget);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        hboxLayout->addWidget(okButton);

        cancelButton = new QPushButton(layoutWidget);
        cancelButton->setObjectName(QString::fromUtf8("cancelButton"));

        hboxLayout->addWidget(cancelButton);

        label = new QLabel(TerrainSizeInputDialog);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(40, 16, 41, 16));
        lineEditWidth = new QLineEdit(TerrainSizeInputDialog);
        lineEditWidth->setObjectName(QString::fromUtf8("lineEditWidth"));
        lineEditWidth->setGeometry(QRect(100, 16, 51, 20));
        label_2 = new QLabel(TerrainSizeInputDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(160, 16, 21, 16));
        label_3 = new QLabel(TerrainSizeInputDialog);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(40, 46, 41, 16));
        lineEditDepth = new QLineEdit(TerrainSizeInputDialog);
        lineEditDepth->setObjectName(QString::fromUtf8("lineEditDepth"));
        lineEditDepth->setGeometry(QRect(100, 46, 51, 20));
        label_4 = new QLabel(TerrainSizeInputDialog);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(160, 46, 21, 16));
        label_5 = new QLabel(TerrainSizeInputDialog);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(40, 76, 61, 16));
        lineEditCellLength = new QLineEdit(TerrainSizeInputDialog);
        lineEditCellLength->setObjectName(QString::fromUtf8("lineEditCellLength"));
        lineEditCellLength->setGeometry(QRect(100, 76, 51, 20));
        label_6 = new QLabel(TerrainSizeInputDialog);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(160, 76, 21, 16));

        retranslateUi(TerrainSizeInputDialog);

        QMetaObject::connectSlotsByName(TerrainSizeInputDialog);
    } // setupUi

    void retranslateUi(QDialog *TerrainSizeInputDialog)
    {
        TerrainSizeInputDialog->setWindowTitle(QApplication::translate("TerrainSizeInputDialog", "Dialog", 0, QApplication::UnicodeUTF8));
        okButton->setText(QApplication::translate("TerrainSizeInputDialog", "OK", 0, QApplication::UnicodeUTF8));
        cancelButton->setText(QApplication::translate("TerrainSizeInputDialog", "Cancel", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("TerrainSizeInputDialog", "Width:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("TerrainSizeInputDialog", "[m]", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("TerrainSizeInputDialog", "Depth:", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("TerrainSizeInputDialog", "[m]", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("TerrainSizeInputDialog", "Cell Length:", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("TerrainSizeInputDialog", "[m]", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TerrainSizeInputDialog: public Ui_TerrainSizeInputDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TERRAINSIZEINPUTDIALOG_H
