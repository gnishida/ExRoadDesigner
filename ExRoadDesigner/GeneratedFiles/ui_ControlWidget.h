/********************************************************************************
** Form generated from reading UI file 'ControlWidget.ui'
**
** Created: Tue Apr 22 18:09:22 2014
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONTROLWIDGET_H
#define UI_CONTROLWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDockWidget>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ControlWidget
{
public:
    QWidget *dockWidgetContents;
    QGroupBox *groupBox_8;
    QPushButton *pushButtonGenerateMultiEx;
    QCheckBox *checkBoxFadeOut;
    QPushButton *pushButtonGenerateMultiIntEx;
    QGroupBox *groupBox;
    QLabel *label;
    QLineEdit *lineEditNumAvenueIterations;
    QCheckBox *checkBoxLocalStreets;
    QCheckBox *checkBoxCropping;
    QCheckBox *checkBoxAnimation;
    QLabel *label_3;
    QLineEdit *lineEditNumStreetIterations;
    QCheckBox *checkBoxCleanAvenues;
    QCheckBox *checkBoxCleanStreets;
    QGroupBox *groupBox_5;
    QPushButton *pushButtonConnect;
    QPushButton *pushButtonMerge;
    QGroupBox *groupBox_2;
    QSlider *horizontalSliderInterpolationFactor;
    QLabel *labelInterpolationFactor;
    QPushButton *pushButtonGenerateInterpolation;
    QGroupBox *groupBox_3;
    QPushButton *pushButtonGenerateWarp;
    QPushButton *pushButtonGenerateUShape;
    QPushButton *pushButtonGenerateSmoothWarp;
    QPushButton *pushButtonGenerateVerySmoothWarp;
    QPushButton *pushButtonClear;

    void setupUi(QDockWidget *ControlWidget)
    {
        if (ControlWidget->objectName().isEmpty())
            ControlWidget->setObjectName(QString::fromUtf8("ControlWidget"));
        ControlWidget->resize(190, 977);
        ControlWidget->setMinimumSize(QSize(190, 240));
        ControlWidget->setStyleSheet(QString::fromUtf8("background-color: rgb(181, 181, 181);"));
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        groupBox_8 = new QGroupBox(dockWidgetContents);
        groupBox_8->setObjectName(QString::fromUtf8("groupBox_8"));
        groupBox_8->setGeometry(QRect(10, 320, 171, 121));
        pushButtonGenerateMultiEx = new QPushButton(groupBox_8);
        pushButtonGenerateMultiEx->setObjectName(QString::fromUtf8("pushButtonGenerateMultiEx"));
        pushButtonGenerateMultiEx->setGeometry(QRect(20, 40, 131, 31));
        checkBoxFadeOut = new QCheckBox(groupBox_8);
        checkBoxFadeOut->setObjectName(QString::fromUtf8("checkBoxFadeOut"));
        checkBoxFadeOut->setGeometry(QRect(10, 20, 121, 17));
        pushButtonGenerateMultiIntEx = new QPushButton(groupBox_8);
        pushButtonGenerateMultiIntEx->setObjectName(QString::fromUtf8("pushButtonGenerateMultiIntEx"));
        pushButtonGenerateMultiIntEx->setGeometry(QRect(20, 80, 131, 31));
        groupBox = new QGroupBox(dockWidgetContents);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(10, 10, 171, 191));
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 20, 91, 21));
        lineEditNumAvenueIterations = new QLineEdit(groupBox);
        lineEditNumAvenueIterations->setObjectName(QString::fromUtf8("lineEditNumAvenueIterations"));
        lineEditNumAvenueIterations->setGeometry(QRect(100, 20, 61, 20));
        lineEditNumAvenueIterations->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        checkBoxLocalStreets = new QCheckBox(groupBox);
        checkBoxLocalStreets->setObjectName(QString::fromUtf8("checkBoxLocalStreets"));
        checkBoxLocalStreets->setGeometry(QRect(10, 120, 131, 17));
        checkBoxCropping = new QCheckBox(groupBox);
        checkBoxCropping->setObjectName(QString::fromUtf8("checkBoxCropping"));
        checkBoxCropping->setGeometry(QRect(10, 140, 121, 17));
        checkBoxAnimation = new QCheckBox(groupBox);
        checkBoxAnimation->setObjectName(QString::fromUtf8("checkBoxAnimation"));
        checkBoxAnimation->setGeometry(QRect(10, 160, 111, 17));
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(10, 40, 81, 21));
        lineEditNumStreetIterations = new QLineEdit(groupBox);
        lineEditNumStreetIterations->setObjectName(QString::fromUtf8("lineEditNumStreetIterations"));
        lineEditNumStreetIterations->setGeometry(QRect(100, 40, 61, 20));
        lineEditNumStreetIterations->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        checkBoxCleanAvenues = new QCheckBox(groupBox);
        checkBoxCleanAvenues->setObjectName(QString::fromUtf8("checkBoxCleanAvenues"));
        checkBoxCleanAvenues->setGeometry(QRect(10, 80, 131, 17));
        checkBoxCleanStreets = new QCheckBox(groupBox);
        checkBoxCleanStreets->setObjectName(QString::fromUtf8("checkBoxCleanStreets"));
        checkBoxCleanStreets->setGeometry(QRect(10, 100, 131, 17));
        groupBox_5 = new QGroupBox(dockWidgetContents);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        groupBox_5->setGeometry(QRect(10, 680, 171, 101));
        pushButtonConnect = new QPushButton(groupBox_5);
        pushButtonConnect->setObjectName(QString::fromUtf8("pushButtonConnect"));
        pushButtonConnect->setGeometry(QRect(20, 60, 131, 31));
        pushButtonMerge = new QPushButton(groupBox_5);
        pushButtonMerge->setObjectName(QString::fromUtf8("pushButtonMerge"));
        pushButtonMerge->setGeometry(QRect(20, 20, 131, 31));
        groupBox_2 = new QGroupBox(dockWidgetContents);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setGeometry(QRect(10, 210, 171, 101));
        horizontalSliderInterpolationFactor = new QSlider(groupBox_2);
        horizontalSliderInterpolationFactor->setObjectName(QString::fromUtf8("horizontalSliderInterpolationFactor"));
        horizontalSliderInterpolationFactor->setGeometry(QRect(10, 30, 111, 20));
        horizontalSliderInterpolationFactor->setMaximum(100);
        horizontalSliderInterpolationFactor->setOrientation(Qt::Horizontal);
        labelInterpolationFactor = new QLabel(groupBox_2);
        labelInterpolationFactor->setObjectName(QString::fromUtf8("labelInterpolationFactor"));
        labelInterpolationFactor->setGeometry(QRect(130, 30, 31, 20));
        labelInterpolationFactor->setStyleSheet(QString::fromUtf8(""));
        labelInterpolationFactor->setFrameShape(QFrame::NoFrame);
        labelInterpolationFactor->setFrameShadow(QFrame::Plain);
        labelInterpolationFactor->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        pushButtonGenerateInterpolation = new QPushButton(groupBox_2);
        pushButtonGenerateInterpolation->setObjectName(QString::fromUtf8("pushButtonGenerateInterpolation"));
        pushButtonGenerateInterpolation->setGeometry(QRect(20, 60, 131, 31));
        groupBox_3 = new QGroupBox(dockWidgetContents);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setGeometry(QRect(10, 450, 171, 181));
        pushButtonGenerateWarp = new QPushButton(groupBox_3);
        pushButtonGenerateWarp->setObjectName(QString::fromUtf8("pushButtonGenerateWarp"));
        pushButtonGenerateWarp->setGeometry(QRect(20, 60, 131, 31));
        pushButtonGenerateUShape = new QPushButton(groupBox_3);
        pushButtonGenerateUShape->setObjectName(QString::fromUtf8("pushButtonGenerateUShape"));
        pushButtonGenerateUShape->setGeometry(QRect(20, 20, 131, 31));
        pushButtonGenerateSmoothWarp = new QPushButton(groupBox_3);
        pushButtonGenerateSmoothWarp->setObjectName(QString::fromUtf8("pushButtonGenerateSmoothWarp"));
        pushButtonGenerateSmoothWarp->setGeometry(QRect(20, 100, 131, 31));
        pushButtonGenerateVerySmoothWarp = new QPushButton(groupBox_3);
        pushButtonGenerateVerySmoothWarp->setObjectName(QString::fromUtf8("pushButtonGenerateVerySmoothWarp"));
        pushButtonGenerateVerySmoothWarp->setGeometry(QRect(20, 140, 131, 31));
        pushButtonClear = new QPushButton(dockWidgetContents);
        pushButtonClear->setObjectName(QString::fromUtf8("pushButtonClear"));
        pushButtonClear->setGeometry(QRect(30, 640, 131, 31));
        ControlWidget->setWidget(dockWidgetContents);

        retranslateUi(ControlWidget);

        QMetaObject::connectSlotsByName(ControlWidget);
    } // setupUi

    void retranslateUi(QDockWidget *ControlWidget)
    {
        groupBox_8->setTitle(QApplication::translate("ControlWidget", "Multiple Examples", 0, QApplication::UnicodeUTF8));
        pushButtonGenerateMultiEx->setText(QApplication::translate("ControlWidget", "Multi Ex", 0, QApplication::UnicodeUTF8));
        checkBoxFadeOut->setText(QApplication::translate("ControlWidget", "Fade out", 0, QApplication::UnicodeUTF8));
        pushButtonGenerateMultiIntEx->setText(QApplication::translate("ControlWidget", "Multi Ex Interpolation", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("ControlWidget", "Options", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("ControlWidget", "Avenue Iteration:", 0, QApplication::UnicodeUTF8));
        checkBoxLocalStreets->setText(QApplication::translate("ControlWidget", "Generate Local Streets", 0, QApplication::UnicodeUTF8));
        checkBoxCropping->setText(QApplication::translate("ControlWidget", "Cropping", 0, QApplication::UnicodeUTF8));
        checkBoxAnimation->setText(QApplication::translate("ControlWidget", "Animation", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("ControlWidget", "Street Iteration:", 0, QApplication::UnicodeUTF8));
        checkBoxCleanAvenues->setText(QApplication::translate("ControlWidget", "Clean Avenues", 0, QApplication::UnicodeUTF8));
        checkBoxCleanStreets->setText(QApplication::translate("ControlWidget", "Clean Local Streets", 0, QApplication::UnicodeUTF8));
        groupBox_5->setTitle(QApplication::translate("ControlWidget", "Finalize", 0, QApplication::UnicodeUTF8));
        pushButtonConnect->setText(QApplication::translate("ControlWidget", "Connect", 0, QApplication::UnicodeUTF8));
        pushButtonMerge->setText(QApplication::translate("ControlWidget", "Merge", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("ControlWidget", "Interpolation", 0, QApplication::UnicodeUTF8));
        labelInterpolationFactor->setText(QApplication::translate("ControlWidget", "0", 0, QApplication::UnicodeUTF8));
        pushButtonGenerateInterpolation->setText(QApplication::translate("ControlWidget", "Interpolation", 0, QApplication::UnicodeUTF8));
        groupBox_3->setTitle(QApplication::translate("ControlWidget", "Warp", 0, QApplication::UnicodeUTF8));
        pushButtonGenerateWarp->setText(QApplication::translate("ControlWidget", "Warp", 0, QApplication::UnicodeUTF8));
        pushButtonGenerateUShape->setText(QApplication::translate("ControlWidget", "Weird Shape Fitting", 0, QApplication::UnicodeUTF8));
        pushButtonGenerateSmoothWarp->setText(QApplication::translate("ControlWidget", "Smooth Warp", 0, QApplication::UnicodeUTF8));
        pushButtonGenerateVerySmoothWarp->setText(QApplication::translate("ControlWidget", "Very Smooth Warp", 0, QApplication::UnicodeUTF8));
        pushButtonClear->setText(QApplication::translate("ControlWidget", "Clear", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(ControlWidget);
    } // retranslateUi

};

namespace Ui {
    class ControlWidget: public Ui_ControlWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONTROLWIDGET_H
