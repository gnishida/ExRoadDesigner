/********************************************************************************
** Form generated from reading UI file 'TerrainSizeInputDialog.ui'
**
** Created: Wed Jun 17 12:11:58 2015
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
    QLineEdit *lineEditSide;
    QLabel *label_2;
    QLabel *label_5;
    QLineEdit *lineEditCellResolution;

    void setupUi(QDialog *TerrainSizeInputDialog)
    {
        if (TerrainSizeInputDialog->objectName().isEmpty())
            TerrainSizeInputDialog->setObjectName(QString::fromUtf8("TerrainSizeInputDialog"));
        TerrainSizeInputDialog->resize(220, 126);
        layoutWidget = new QWidget(TerrainSizeInputDialog);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(10, 80, 201, 33));
        hboxLayout = new QHBoxLayout(layoutWidget);
#ifndef Q_OS_MAC
        hboxLayout->setSpacing(6);
#endif
        hboxLayout->setContentsMargins(0, 0, 0, 0);
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
        label->setGeometry(QRect(30, 20, 41, 16));
        lineEditSide = new QLineEdit(TerrainSizeInputDialog);
        lineEditSide->setObjectName(QString::fromUtf8("lineEditSide"));
        lineEditSide->setGeometry(QRect(110, 16, 51, 20));
        label_2 = new QLabel(TerrainSizeInputDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(180, 16, 21, 16));
        label_5 = new QLabel(TerrainSizeInputDialog);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(30, 54, 71, 16));
        lineEditCellResolution = new QLineEdit(TerrainSizeInputDialog);
        lineEditCellResolution->setObjectName(QString::fromUtf8("lineEditCellResolution"));
        lineEditCellResolution->setGeometry(QRect(110, 50, 51, 20));

        retranslateUi(TerrainSizeInputDialog);

        QMetaObject::connectSlotsByName(TerrainSizeInputDialog);
    } // setupUi

    void retranslateUi(QDialog *TerrainSizeInputDialog)
    {
        TerrainSizeInputDialog->setWindowTitle(QApplication::translate("TerrainSizeInputDialog", "Dialog", 0, QApplication::UnicodeUTF8));
        okButton->setText(QApplication::translate("TerrainSizeInputDialog", "OK", 0, QApplication::UnicodeUTF8));
        cancelButton->setText(QApplication::translate("TerrainSizeInputDialog", "Cancel", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("TerrainSizeInputDialog", "Side:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("TerrainSizeInputDialog", "[m]", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("TerrainSizeInputDialog", "Cell Resolution:", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TerrainSizeInputDialog: public Ui_TerrainSizeInputDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TERRAINSIZEINPUTDIALOG_H
