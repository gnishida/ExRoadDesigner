/********************************************************************************
** Form generated from reading UI file 'PropertyWidget.ui'
**
** Created: Sun Jan 18 16:28:41 2015
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROPERTYWIDGET_H
#define UI_PROPERTYWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDockWidget>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PropertyWidget
{
public:
    QWidget *dockWidgetContents;
    QGroupBox *groupBox;
    QLabel *label;
    QLineEdit *lineEditEdgeType;
    QLabel *label_2;
    QLineEdit *lineEditEdgeLanes;
    QLabel *label_3;
    QLineEdit *lineEditEdgeOneWay;
    QLineEdit *lineEditEdgeLink;
    QLabel *label_9;
    QLabel *label_10;
    QLineEdit *lineEditEdgeRoundabout;
    QLabel *label_17;
    QLineEdit *lineEditEdgeGroupId;
    QLabel *label_18;
    QLineEdit *lineEditEdgeGenerationType;
    QLabel *label_4;
    QLineEdit *lineEditEdgeSource;
    QLineEdit *lineEditEdgeTarget;
    QLabel *label_5;
    QGroupBox *groupBox_5;
    QLabel *label_6;
    QLineEdit *lineEditVertexDesc;
    QLabel *label_7;
    QLineEdit *lineEditVertexPos;
    QLabel *label_8;
    QTextEdit *textEditVertexNeighbors;
    QLabel *label_11;
    QLineEdit *lineEditVertexGenerationType;
    QLabel *label_12;
    QLabel *label_13;
    QLineEdit *lineEditVertexParent;
    QTextEdit *textEditVertexUncles;
    QLabel *label_14;
    QLineEdit *lineEditVertexOnBoundary;
    QLabel *label_15;
    QLineEdit *lineEditVertexGroupId;
    QLabel *label_16;
    QLineEdit *lineEditVertexExampleDesc;
    QLabel *label_19;
    QLineEdit *lineEditVertexDeadend;
    QLineEdit *lineEditVertexSearch;
    QPushButton *pushButtonVertexSearch;
    QLabel *label_20;
    QLineEdit *lineEditVertexExampleStreetDesc;
    QLineEdit *lineEditVertexRotationAngle;
    QLabel *label_21;

    void setupUi(QDockWidget *PropertyWidget)
    {
        if (PropertyWidget->objectName().isEmpty())
            PropertyWidget->setObjectName(QString::fromUtf8("PropertyWidget"));
        PropertyWidget->resize(192, 831);
        PropertyWidget->setMinimumSize(QSize(192, 240));
        PropertyWidget->setStyleSheet(QString::fromUtf8("background-color: rgb(181, 181, 181);"));
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        groupBox = new QGroupBox(dockWidgetContents);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(10, 450, 171, 211));
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 60, 31, 16));
        lineEditEdgeType = new QLineEdit(groupBox);
        lineEditEdgeType->setObjectName(QString::fromUtf8("lineEditEdgeType"));
        lineEditEdgeType->setGeometry(QRect(80, 60, 81, 20));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(10, 80, 51, 16));
        lineEditEdgeLanes = new QLineEdit(groupBox);
        lineEditEdgeLanes->setObjectName(QString::fromUtf8("lineEditEdgeLanes"));
        lineEditEdgeLanes->setGeometry(QRect(80, 80, 81, 20));
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(10, 100, 51, 16));
        lineEditEdgeOneWay = new QLineEdit(groupBox);
        lineEditEdgeOneWay->setObjectName(QString::fromUtf8("lineEditEdgeOneWay"));
        lineEditEdgeOneWay->setGeometry(QRect(80, 100, 81, 20));
        lineEditEdgeLink = new QLineEdit(groupBox);
        lineEditEdgeLink->setObjectName(QString::fromUtf8("lineEditEdgeLink"));
        lineEditEdgeLink->setGeometry(QRect(80, 120, 81, 20));
        label_9 = new QLabel(groupBox);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(10, 120, 51, 16));
        label_10 = new QLabel(groupBox);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setGeometry(QRect(10, 140, 61, 16));
        lineEditEdgeRoundabout = new QLineEdit(groupBox);
        lineEditEdgeRoundabout->setObjectName(QString::fromUtf8("lineEditEdgeRoundabout"));
        lineEditEdgeRoundabout->setGeometry(QRect(80, 140, 81, 20));
        label_17 = new QLabel(groupBox);
        label_17->setObjectName(QString::fromUtf8("label_17"));
        label_17->setGeometry(QRect(10, 160, 51, 16));
        lineEditEdgeGroupId = new QLineEdit(groupBox);
        lineEditEdgeGroupId->setObjectName(QString::fromUtf8("lineEditEdgeGroupId"));
        lineEditEdgeGroupId->setGeometry(QRect(80, 160, 81, 20));
        label_18 = new QLabel(groupBox);
        label_18->setObjectName(QString::fromUtf8("label_18"));
        label_18->setGeometry(QRect(10, 180, 51, 16));
        lineEditEdgeGenerationType = new QLineEdit(groupBox);
        lineEditEdgeGenerationType->setObjectName(QString::fromUtf8("lineEditEdgeGenerationType"));
        lineEditEdgeGenerationType->setGeometry(QRect(80, 180, 81, 20));
        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(10, 20, 31, 16));
        lineEditEdgeSource = new QLineEdit(groupBox);
        lineEditEdgeSource->setObjectName(QString::fromUtf8("lineEditEdgeSource"));
        lineEditEdgeSource->setGeometry(QRect(80, 20, 81, 20));
        lineEditEdgeTarget = new QLineEdit(groupBox);
        lineEditEdgeTarget->setObjectName(QString::fromUtf8("lineEditEdgeTarget"));
        lineEditEdgeTarget->setGeometry(QRect(80, 40, 81, 20));
        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(10, 40, 31, 16));
        groupBox_5 = new QGroupBox(dockWidgetContents);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        groupBox_5->setGeometry(QRect(10, 10, 171, 431));
        label_6 = new QLabel(groupBox_5);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(10, 60, 31, 16));
        lineEditVertexDesc = new QLineEdit(groupBox_5);
        lineEditVertexDesc->setObjectName(QString::fromUtf8("lineEditVertexDesc"));
        lineEditVertexDesc->setGeometry(QRect(80, 60, 81, 20));
        label_7 = new QLabel(groupBox_5);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(10, 80, 51, 16));
        lineEditVertexPos = new QLineEdit(groupBox_5);
        lineEditVertexPos->setObjectName(QString::fromUtf8("lineEditVertexPos"));
        lineEditVertexPos->setGeometry(QRect(80, 80, 81, 20));
        label_8 = new QLabel(groupBox_5);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(10, 230, 51, 16));
        textEditVertexNeighbors = new QTextEdit(groupBox_5);
        textEditVertexNeighbors->setObjectName(QString::fromUtf8("textEditVertexNeighbors"));
        textEditVertexNeighbors->setGeometry(QRect(80, 230, 81, 61));
        label_11 = new QLabel(groupBox_5);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setGeometry(QRect(10, 320, 51, 16));
        lineEditVertexGenerationType = new QLineEdit(groupBox_5);
        lineEditVertexGenerationType->setObjectName(QString::fromUtf8("lineEditVertexGenerationType"));
        lineEditVertexGenerationType->setGeometry(QRect(80, 320, 81, 20));
        label_12 = new QLabel(groupBox_5);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        label_12->setGeometry(QRect(10, 100, 51, 16));
        label_13 = new QLabel(groupBox_5);
        label_13->setObjectName(QString::fromUtf8("label_13"));
        label_13->setGeometry(QRect(10, 160, 51, 16));
        lineEditVertexParent = new QLineEdit(groupBox_5);
        lineEditVertexParent->setObjectName(QString::fromUtf8("lineEditVertexParent"));
        lineEditVertexParent->setGeometry(QRect(80, 100, 81, 20));
        textEditVertexUncles = new QTextEdit(groupBox_5);
        textEditVertexUncles->setObjectName(QString::fromUtf8("textEditVertexUncles"));
        textEditVertexUncles->setGeometry(QRect(80, 160, 81, 61));
        label_14 = new QLabel(groupBox_5);
        label_14->setObjectName(QString::fromUtf8("label_14"));
        label_14->setGeometry(QRect(10, 120, 71, 16));
        lineEditVertexOnBoundary = new QLineEdit(groupBox_5);
        lineEditVertexOnBoundary->setObjectName(QString::fromUtf8("lineEditVertexOnBoundary"));
        lineEditVertexOnBoundary->setGeometry(QRect(80, 120, 81, 20));
        label_15 = new QLabel(groupBox_5);
        label_15->setObjectName(QString::fromUtf8("label_15"));
        label_15->setGeometry(QRect(10, 300, 51, 16));
        lineEditVertexGroupId = new QLineEdit(groupBox_5);
        lineEditVertexGroupId->setObjectName(QString::fromUtf8("lineEditVertexGroupId"));
        lineEditVertexGroupId->setGeometry(QRect(80, 300, 81, 20));
        label_16 = new QLabel(groupBox_5);
        label_16->setObjectName(QString::fromUtf8("label_16"));
        label_16->setGeometry(QRect(10, 340, 61, 16));
        lineEditVertexExampleDesc = new QLineEdit(groupBox_5);
        lineEditVertexExampleDesc->setObjectName(QString::fromUtf8("lineEditVertexExampleDesc"));
        lineEditVertexExampleDesc->setGeometry(QRect(80, 340, 81, 20));
        label_19 = new QLabel(groupBox_5);
        label_19->setObjectName(QString::fromUtf8("label_19"));
        label_19->setGeometry(QRect(10, 380, 61, 16));
        lineEditVertexDeadend = new QLineEdit(groupBox_5);
        lineEditVertexDeadend->setObjectName(QString::fromUtf8("lineEditVertexDeadend"));
        lineEditVertexDeadend->setGeometry(QRect(80, 380, 81, 20));
        lineEditVertexSearch = new QLineEdit(groupBox_5);
        lineEditVertexSearch->setObjectName(QString::fromUtf8("lineEditVertexSearch"));
        lineEditVertexSearch->setGeometry(QRect(10, 20, 81, 20));
        pushButtonVertexSearch = new QPushButton(groupBox_5);
        pushButtonVertexSearch->setObjectName(QString::fromUtf8("pushButtonVertexSearch"));
        pushButtonVertexSearch->setGeometry(QRect(100, 20, 61, 23));
        label_20 = new QLabel(groupBox_5);
        label_20->setObjectName(QString::fromUtf8("label_20"));
        label_20->setGeometry(QRect(10, 360, 61, 16));
        lineEditVertexExampleStreetDesc = new QLineEdit(groupBox_5);
        lineEditVertexExampleStreetDesc->setObjectName(QString::fromUtf8("lineEditVertexExampleStreetDesc"));
        lineEditVertexExampleStreetDesc->setGeometry(QRect(80, 360, 81, 20));
        lineEditVertexRotationAngle = new QLineEdit(groupBox_5);
        lineEditVertexRotationAngle->setObjectName(QString::fromUtf8("lineEditVertexRotationAngle"));
        lineEditVertexRotationAngle->setGeometry(QRect(80, 400, 81, 20));
        label_21 = new QLabel(groupBox_5);
        label_21->setObjectName(QString::fromUtf8("label_21"));
        label_21->setGeometry(QRect(10, 400, 61, 16));
        PropertyWidget->setWidget(dockWidgetContents);

        retranslateUi(PropertyWidget);

        QMetaObject::connectSlotsByName(PropertyWidget);
    } // setupUi

    void retranslateUi(QDockWidget *PropertyWidget)
    {
        groupBox->setTitle(QApplication::translate("PropertyWidget", "Edge", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("PropertyWidget", "Type", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("PropertyWidget", "Lanes", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("PropertyWidget", "One Way", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("PropertyWidget", "Link", 0, QApplication::UnicodeUTF8));
        label_10->setText(QApplication::translate("PropertyWidget", "Roundabout", 0, QApplication::UnicodeUTF8));
        label_17->setText(QApplication::translate("PropertyWidget", "Group", 0, QApplication::UnicodeUTF8));
        label_18->setText(QApplication::translate("PropertyWidget", "Example?", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("PropertyWidget", "Source", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("PropertyWidget", "Target", 0, QApplication::UnicodeUTF8));
        groupBox_5->setTitle(QApplication::translate("PropertyWidget", "Vertex", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("PropertyWidget", "ID", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("PropertyWidget", "Location", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("PropertyWidget", "Neighbors", 0, QApplication::UnicodeUTF8));
        label_11->setText(QApplication::translate("PropertyWidget", "Example?", 0, QApplication::UnicodeUTF8));
        label_12->setText(QApplication::translate("PropertyWidget", "Parent", 0, QApplication::UnicodeUTF8));
        label_13->setText(QApplication::translate("PropertyWidget", "Uncles", 0, QApplication::UnicodeUTF8));
        label_14->setText(QApplication::translate("PropertyWidget", "On boundary?", 0, QApplication::UnicodeUTF8));
        label_15->setText(QApplication::translate("PropertyWidget", "Group", 0, QApplication::UnicodeUTF8));
        label_16->setText(QApplication::translate("PropertyWidget", "Example Id", 0, QApplication::UnicodeUTF8));
        label_19->setText(QApplication::translate("PropertyWidget", "Deadend", 0, QApplication::UnicodeUTF8));
        pushButtonVertexSearch->setText(QApplication::translate("PropertyWidget", "Search", 0, QApplication::UnicodeUTF8));
        label_20->setText(QApplication::translate("PropertyWidget", "Local Ex Id", 0, QApplication::UnicodeUTF8));
        label_21->setText(QApplication::translate("PropertyWidget", "Rotation", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(PropertyWidget);
    } // retranslateUi

};

namespace Ui {
    class PropertyWidget: public Ui_PropertyWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROPERTYWIDGET_H
