/********************************************************************************
** Form generated from reading UI file 'ControlWidget.ui'
**
** Created: Wed Feb 4 11:17:04 2015
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
#include <QtGui/QTabWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ControlWidget
{
public:
    QWidget *dockWidgetContents;
    QTabWidget *TabWidget;
    QWidget *RoadGenerationTab;
    QGroupBox *groupBox;
    QLabel *label;
    QLineEdit *lineEditNumAvenueIterations;
    QCheckBox *checkBoxLocalStreets;
    QCheckBox *checkBoxCropping;
    QLabel *label_3;
    QLineEdit *lineEditNumStreetIterations;
    QCheckBox *checkBoxCleanAvenues;
    QCheckBox *checkBoxCleanStreets;
    QLabel *label_7;
    QLineEdit *lineEditNumExamples;
    QCheckBox *checkBoxUseLayer;
    QLabel *label_11;
    QLineEdit *lineEditRandomSeed;
    QGroupBox *groupBox_8;
    QLabel *label_2;
    QLineEdit *lineEditInterpolateSigma2;
    QLineEdit *lineEditInterpolateSigma1;
    QLabel *label_6;
    QPushButton *pushButtonGeneratePM;
    QPushButton *pushButtonClear;
    QLabel *label_8;
    QLineEdit *lineEditPatchDistance1;
    QLineEdit *lineEditPatchDistance2;
    QLabel *label_9;
    QLabel *label_5;
    QLineEdit *lineEditRotationAngle;
    QLineEdit *lineEditHoughScale;
    QLabel *label_10;
    QLabel *label_12;
    QLineEdit *lineEditRoadAngleTolerance;
    QPushButton *pushButtonGenerateEx;
    QPushButton *pushButtonGenerateWarp;
    QPushButton *pushButtonGenerateAliaga;
    QCheckBox *checkBoxSaveRoadImages;
    QCheckBox *checkBoxSavePatchImages;
    QLabel *label_14;
    QLineEdit *lineEditSlopeTolerance;
    QLabel *label_16;
    QLineEdit *lineEditAcrossRiverTolerance;
    QWidget *CleanUpTab;
    QGroupBox *groupBox_5;
    QPushButton *pushButtonMerge;
    QPushButton *pushButtonConnect2;
    QPushButton *pushButtonTrim;
    QPushButton *pushButtonRemoveDanglingEdges;
    QPushButton *pushButtonClearBoundaryFlag;
    QGroupBox *groupBox_3;
    QLabel *label_13;
    QLineEdit *lineEditMinBlockSize;
    QPushButton *pushButtonRemoveSmallBlocks;
    QPushButton *pushButtonRemoveCloseEdges;
    QWidget *ContentEditionTab;
    QGroupBox *groupBox_6;
    QLabel *terrainPaint_sizeLabel;
    QLabel *terrainPaint_changeLabel;
    QSlider *terrainPaint_sizeSlider;
    QSlider *terrainPaint_changeSlider;
    QCheckBox *terrain_2DShader;
    QLabel *render_2DroadsStrokeLabel;
    QSlider *render_2DroadsStrokeSlider;
    QSlider *render_2DroadsExtraWidthSlider;
    QLabel *render_2DroadsExtraWidthLabel;
    QLabel *render_2DparksLabel;
    QSlider *render_2DparksSlider;
    QCheckBox *render_3DtreesCheckBox;
    QLabel *render_2DparcelLineLabel;
    QSlider *render_2DparcelLineSlider;
    QCheckBox *content_checkbox;
    QGroupBox *groupBox_2;
    QPushButton *content_0;
    QPushButton *content_1;
    QPushButton *content_8;
    QPushButton *content_7;
    QPushButton *content_9;
    QPushButton *content_11;
    QPushButton *content_10;
    QLineEdit *lineEditMaxBlockSizeForPark;
    QLabel *label_15;
    QLineEdit *terrainPaint_changeEdit;
    QLabel *terrainPaint_changeLabel_2;

    void setupUi(QDockWidget *ControlWidget)
    {
        if (ControlWidget->objectName().isEmpty())
            ControlWidget->setObjectName(QString::fromUtf8("ControlWidget"));
        ControlWidget->resize(198, 621);
        ControlWidget->setMinimumSize(QSize(190, 240));
        ControlWidget->setStyleSheet(QString::fromUtf8("background-color: rgb(181, 181, 181);"));
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        TabWidget = new QTabWidget(dockWidgetContents);
        TabWidget->setObjectName(QString::fromUtf8("TabWidget"));
        TabWidget->setGeometry(QRect(1, 0, 196, 581));
        RoadGenerationTab = new QWidget();
        RoadGenerationTab->setObjectName(QString::fromUtf8("RoadGenerationTab"));
        groupBox = new QGroupBox(RoadGenerationTab);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(10, 10, 171, 201));
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 13, 91, 21));
        lineEditNumAvenueIterations = new QLineEdit(groupBox);
        lineEditNumAvenueIterations->setObjectName(QString::fromUtf8("lineEditNumAvenueIterations"));
        lineEditNumAvenueIterations->setGeometry(QRect(100, 13, 61, 20));
        lineEditNumAvenueIterations->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        checkBoxLocalStreets = new QCheckBox(groupBox);
        checkBoxLocalStreets->setObjectName(QString::fromUtf8("checkBoxLocalStreets"));
        checkBoxLocalStreets->setGeometry(QRect(10, 137, 131, 17));
        checkBoxCropping = new QCheckBox(groupBox);
        checkBoxCropping->setObjectName(QString::fromUtf8("checkBoxCropping"));
        checkBoxCropping->setGeometry(QRect(10, 156, 121, 17));
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(10, 33, 81, 21));
        lineEditNumStreetIterations = new QLineEdit(groupBox);
        lineEditNumStreetIterations->setObjectName(QString::fromUtf8("lineEditNumStreetIterations"));
        lineEditNumStreetIterations->setGeometry(QRect(100, 33, 61, 20));
        lineEditNumStreetIterations->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        checkBoxCleanAvenues = new QCheckBox(groupBox);
        checkBoxCleanAvenues->setObjectName(QString::fromUtf8("checkBoxCleanAvenues"));
        checkBoxCleanAvenues->setGeometry(QRect(10, 100, 131, 17));
        checkBoxCleanStreets = new QCheckBox(groupBox);
        checkBoxCleanStreets->setObjectName(QString::fromUtf8("checkBoxCleanStreets"));
        checkBoxCleanStreets->setGeometry(QRect(10, 118, 131, 17));
        label_7 = new QLabel(groupBox);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(10, 53, 81, 21));
        lineEditNumExamples = new QLineEdit(groupBox);
        lineEditNumExamples->setObjectName(QString::fromUtf8("lineEditNumExamples"));
        lineEditNumExamples->setGeometry(QRect(100, 53, 61, 20));
        lineEditNumExamples->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        checkBoxUseLayer = new QCheckBox(groupBox);
        checkBoxUseLayer->setObjectName(QString::fromUtf8("checkBoxUseLayer"));
        checkBoxUseLayer->setGeometry(QRect(10, 176, 121, 17));
        label_11 = new QLabel(groupBox);
        label_11->setObjectName(QString::fromUtf8("label_11"));
        label_11->setGeometry(QRect(10, 73, 81, 21));
        lineEditRandomSeed = new QLineEdit(groupBox);
        lineEditRandomSeed->setObjectName(QString::fromUtf8("lineEditRandomSeed"));
        lineEditRandomSeed->setGeometry(QRect(100, 73, 61, 20));
        lineEditRandomSeed->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        groupBox_8 = new QGroupBox(RoadGenerationTab);
        groupBox_8->setObjectName(QString::fromUtf8("groupBox_8"));
        groupBox_8->setGeometry(QRect(10, 230, 171, 311));
        label_2 = new QLabel(groupBox_8);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(10, 92, 61, 21));
        lineEditInterpolateSigma2 = new QLineEdit(groupBox_8);
        lineEditInterpolateSigma2->setObjectName(QString::fromUtf8("lineEditInterpolateSigma2"));
        lineEditInterpolateSigma2->setGeometry(QRect(110, 92, 51, 20));
        lineEditInterpolateSigma1 = new QLineEdit(groupBox_8);
        lineEditInterpolateSigma1->setObjectName(QString::fromUtf8("lineEditInterpolateSigma1"));
        lineEditInterpolateSigma1->setGeometry(QRect(110, 72, 51, 20));
        label_6 = new QLabel(groupBox_8);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(10, 72, 61, 21));
        pushButtonGeneratePM = new QPushButton(groupBox_8);
        pushButtonGeneratePM->setObjectName(QString::fromUtf8("pushButtonGeneratePM"));
        pushButtonGeneratePM->setGeometry(QRect(10, 277, 51, 28));
        pushButtonClear = new QPushButton(groupBox_8);
        pushButtonClear->setObjectName(QString::fromUtf8("pushButtonClear"));
        pushButtonClear->setGeometry(QRect(110, 277, 51, 28));
        label_8 = new QLabel(groupBox_8);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(10, 32, 101, 21));
        lineEditPatchDistance1 = new QLineEdit(groupBox_8);
        lineEditPatchDistance1->setObjectName(QString::fromUtf8("lineEditPatchDistance1"));
        lineEditPatchDistance1->setGeometry(QRect(110, 32, 51, 20));
        lineEditPatchDistance2 = new QLineEdit(groupBox_8);
        lineEditPatchDistance2->setObjectName(QString::fromUtf8("lineEditPatchDistance2"));
        lineEditPatchDistance2->setGeometry(QRect(110, 52, 51, 20));
        label_9 = new QLabel(groupBox_8);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(10, 52, 91, 21));
        label_5 = new QLabel(groupBox_8);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(10, 113, 61, 21));
        lineEditRotationAngle = new QLineEdit(groupBox_8);
        lineEditRotationAngle->setObjectName(QString::fromUtf8("lineEditRotationAngle"));
        lineEditRotationAngle->setGeometry(QRect(110, 113, 51, 20));
        lineEditHoughScale = new QLineEdit(groupBox_8);
        lineEditHoughScale->setObjectName(QString::fromUtf8("lineEditHoughScale"));
        lineEditHoughScale->setGeometry(QRect(110, 12, 51, 20));
        label_10 = new QLabel(groupBox_8);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setGeometry(QRect(10, 12, 91, 21));
        label_12 = new QLabel(groupBox_8);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        label_12->setGeometry(QRect(10, 131, 81, 21));
        lineEditRoadAngleTolerance = new QLineEdit(groupBox_8);
        lineEditRoadAngleTolerance->setObjectName(QString::fromUtf8("lineEditRoadAngleTolerance"));
        lineEditRoadAngleTolerance->setGeometry(QRect(110, 133, 51, 20));
        pushButtonGenerateEx = new QPushButton(groupBox_8);
        pushButtonGenerateEx->setObjectName(QString::fromUtf8("pushButtonGenerateEx"));
        pushButtonGenerateEx->setGeometry(QRect(10, 247, 71, 28));
        pushButtonGenerateWarp = new QPushButton(groupBox_8);
        pushButtonGenerateWarp->setObjectName(QString::fromUtf8("pushButtonGenerateWarp"));
        pushButtonGenerateWarp->setGeometry(QRect(90, 247, 71, 28));
        pushButtonGenerateAliaga = new QPushButton(groupBox_8);
        pushButtonGenerateAliaga->setObjectName(QString::fromUtf8("pushButtonGenerateAliaga"));
        pushButtonGenerateAliaga->setGeometry(QRect(60, 277, 51, 28));
        checkBoxSaveRoadImages = new QCheckBox(groupBox_8);
        checkBoxSaveRoadImages->setObjectName(QString::fromUtf8("checkBoxSaveRoadImages"));
        checkBoxSaveRoadImages->setGeometry(QRect(20, 200, 111, 17));
        checkBoxSavePatchImages = new QCheckBox(groupBox_8);
        checkBoxSavePatchImages->setObjectName(QString::fromUtf8("checkBoxSavePatchImages"));
        checkBoxSavePatchImages->setGeometry(QRect(20, 218, 131, 17));
        label_14 = new QLabel(groupBox_8);
        label_14->setObjectName(QString::fromUtf8("label_14"));
        label_14->setGeometry(QRect(10, 151, 81, 21));
        lineEditSlopeTolerance = new QLineEdit(groupBox_8);
        lineEditSlopeTolerance->setObjectName(QString::fromUtf8("lineEditSlopeTolerance"));
        lineEditSlopeTolerance->setGeometry(QRect(110, 153, 51, 20));
        label_16 = new QLabel(groupBox_8);
        label_16->setObjectName(QString::fromUtf8("label_16"));
        label_16->setGeometry(QRect(10, 170, 81, 21));
        lineEditAcrossRiverTolerance = new QLineEdit(groupBox_8);
        lineEditAcrossRiverTolerance->setObjectName(QString::fromUtf8("lineEditAcrossRiverTolerance"));
        lineEditAcrossRiverTolerance->setGeometry(QRect(110, 173, 51, 20));
        TabWidget->addTab(RoadGenerationTab, QString());
        CleanUpTab = new QWidget();
        CleanUpTab->setObjectName(QString::fromUtf8("CleanUpTab"));
        groupBox_5 = new QGroupBox(CleanUpTab);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        groupBox_5->setGeometry(QRect(10, 210, 171, 51));
        pushButtonMerge = new QPushButton(groupBox_5);
        pushButtonMerge->setObjectName(QString::fromUtf8("pushButtonMerge"));
        pushButtonMerge->setGeometry(QRect(10, 20, 51, 28));
        pushButtonConnect2 = new QPushButton(groupBox_5);
        pushButtonConnect2->setObjectName(QString::fromUtf8("pushButtonConnect2"));
        pushButtonConnect2->setGeometry(QRect(110, 20, 51, 28));
        pushButtonTrim = new QPushButton(groupBox_5);
        pushButtonTrim->setObjectName(QString::fromUtf8("pushButtonTrim"));
        pushButtonTrim->setGeometry(QRect(60, 20, 51, 28));
        pushButtonRemoveDanglingEdges = new QPushButton(CleanUpTab);
        pushButtonRemoveDanglingEdges->setObjectName(QString::fromUtf8("pushButtonRemoveDanglingEdges"));
        pushButtonRemoveDanglingEdges->setGeometry(QRect(20, 50, 141, 31));
        pushButtonClearBoundaryFlag = new QPushButton(CleanUpTab);
        pushButtonClearBoundaryFlag->setObjectName(QString::fromUtf8("pushButtonClearBoundaryFlag"));
        pushButtonClearBoundaryFlag->setGeometry(QRect(20, 20, 141, 31));
        groupBox_3 = new QGroupBox(CleanUpTab);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setGeometry(QRect(10, 300, 171, 81));
        label_13 = new QLabel(groupBox_3);
        label_13->setObjectName(QString::fromUtf8("label_13"));
        label_13->setGeometry(QRect(40, 20, 31, 21));
        lineEditMinBlockSize = new QLineEdit(groupBox_3);
        lineEditMinBlockSize->setObjectName(QString::fromUtf8("lineEditMinBlockSize"));
        lineEditMinBlockSize->setGeometry(QRect(110, 20, 41, 20));
        lineEditMinBlockSize->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        pushButtonRemoveSmallBlocks = new QPushButton(groupBox_3);
        pushButtonRemoveSmallBlocks->setObjectName(QString::fromUtf8("pushButtonRemoveSmallBlocks"));
        pushButtonRemoveSmallBlocks->setGeometry(QRect(30, 40, 121, 31));
        pushButtonRemoveCloseEdges = new QPushButton(CleanUpTab);
        pushButtonRemoveCloseEdges->setObjectName(QString::fromUtf8("pushButtonRemoveCloseEdges"));
        pushButtonRemoveCloseEdges->setGeometry(QRect(20, 120, 141, 31));
        TabWidget->addTab(CleanUpTab, QString());
        ContentEditionTab = new QWidget();
        ContentEditionTab->setObjectName(QString::fromUtf8("ContentEditionTab"));
        groupBox_6 = new QGroupBox(ContentEditionTab);
        groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
        groupBox_6->setGeometry(QRect(10, 20, 171, 251));
        terrainPaint_sizeLabel = new QLabel(groupBox_6);
        terrainPaint_sizeLabel->setObjectName(QString::fromUtf8("terrainPaint_sizeLabel"));
        terrainPaint_sizeLabel->setGeometry(QRect(10, 20, 61, 16));
        terrainPaint_changeLabel = new QLabel(groupBox_6);
        terrainPaint_changeLabel->setObjectName(QString::fromUtf8("terrainPaint_changeLabel"));
        terrainPaint_changeLabel->setGeometry(QRect(10, 40, 41, 16));
        terrainPaint_sizeSlider = new QSlider(groupBox_6);
        terrainPaint_sizeSlider->setObjectName(QString::fromUtf8("terrainPaint_sizeSlider"));
        terrainPaint_sizeSlider->setGeometry(QRect(80, 20, 81, 15));
        terrainPaint_sizeSlider->setMaximumSize(QSize(16777215, 17));
        terrainPaint_sizeSlider->setMinimum(1);
        terrainPaint_sizeSlider->setMaximum(1000);
        terrainPaint_sizeSlider->setValue(5);
        terrainPaint_sizeSlider->setOrientation(Qt::Horizontal);
        terrainPaint_changeSlider = new QSlider(groupBox_6);
        terrainPaint_changeSlider->setObjectName(QString::fromUtf8("terrainPaint_changeSlider"));
        terrainPaint_changeSlider->setGeometry(QRect(100, 40, 61, 17));
        terrainPaint_changeSlider->setMaximumSize(QSize(16777215, 17));
        terrainPaint_changeSlider->setMinimum(1);
        terrainPaint_changeSlider->setValue(10);
        terrainPaint_changeSlider->setOrientation(Qt::Horizontal);
        terrain_2DShader = new QCheckBox(groupBox_6);
        terrain_2DShader->setObjectName(QString::fromUtf8("terrain_2DShader"));
        terrain_2DShader->setGeometry(QRect(10, 60, 111, 20));
        terrain_2DShader->setChecked(true);
        render_2DroadsStrokeLabel = new QLabel(groupBox_6);
        render_2DroadsStrokeLabel->setObjectName(QString::fromUtf8("render_2DroadsStrokeLabel"));
        render_2DroadsStrokeLabel->setGeometry(QRect(10, 100, 71, 16));
        render_2DroadsStrokeSlider = new QSlider(groupBox_6);
        render_2DroadsStrokeSlider->setObjectName(QString::fromUtf8("render_2DroadsStrokeSlider"));
        render_2DroadsStrokeSlider->setGeometry(QRect(80, 100, 81, 17));
        render_2DroadsStrokeSlider->setMaximumSize(QSize(16777215, 17));
        render_2DroadsStrokeSlider->setMaximum(100);
        render_2DroadsStrokeSlider->setValue(15);
        render_2DroadsStrokeSlider->setOrientation(Qt::Horizontal);
        render_2DroadsExtraWidthSlider = new QSlider(groupBox_6);
        render_2DroadsExtraWidthSlider->setObjectName(QString::fromUtf8("render_2DroadsExtraWidthSlider"));
        render_2DroadsExtraWidthSlider->setGeometry(QRect(90, 80, 71, 17));
        render_2DroadsExtraWidthSlider->setMaximumSize(QSize(16777215, 17));
        render_2DroadsExtraWidthSlider->setMaximum(100);
        render_2DroadsExtraWidthSlider->setOrientation(Qt::Horizontal);
        render_2DroadsExtraWidthLabel = new QLabel(groupBox_6);
        render_2DroadsExtraWidthLabel->setObjectName(QString::fromUtf8("render_2DroadsExtraWidthLabel"));
        render_2DroadsExtraWidthLabel->setGeometry(QRect(10, 80, 71, 16));
        render_2DparksLabel = new QLabel(groupBox_6);
        render_2DparksLabel->setObjectName(QString::fromUtf8("render_2DparksLabel"));
        render_2DparksLabel->setGeometry(QRect(10, 130, 61, 16));
        render_2DparksSlider = new QSlider(groupBox_6);
        render_2DparksSlider->setObjectName(QString::fromUtf8("render_2DparksSlider"));
        render_2DparksSlider->setGeometry(QRect(80, 130, 81, 17));
        render_2DparksSlider->setMaximumSize(QSize(16777215, 17));
        render_2DparksSlider->setValue(10);
        render_2DparksSlider->setOrientation(Qt::Horizontal);
        render_3DtreesCheckBox = new QCheckBox(groupBox_6);
        render_3DtreesCheckBox->setObjectName(QString::fromUtf8("render_3DtreesCheckBox"));
        render_3DtreesCheckBox->setGeometry(QRect(90, 60, 71, 20));
        render_3DtreesCheckBox->setChecked(true);
        render_2DparcelLineLabel = new QLabel(groupBox_6);
        render_2DparcelLineLabel->setObjectName(QString::fromUtf8("render_2DparcelLineLabel"));
        render_2DparcelLineLabel->setGeometry(QRect(10, 170, 81, 16));
        render_2DparcelLineSlider = new QSlider(groupBox_6);
        render_2DparcelLineSlider->setObjectName(QString::fromUtf8("render_2DparcelLineSlider"));
        render_2DparcelLineSlider->setGeometry(QRect(100, 170, 61, 17));
        render_2DparcelLineSlider->setMaximumSize(QSize(16777215, 17));
        render_2DparcelLineSlider->setMaximum(100);
        render_2DparcelLineSlider->setValue(20);
        render_2DparcelLineSlider->setOrientation(Qt::Horizontal);
        content_checkbox = new QCheckBox(groupBox_6);
        content_checkbox->setObjectName(QString::fromUtf8("content_checkbox"));
        content_checkbox->setGeometry(QRect(10, 190, 121, 17));
        groupBox_2 = new QGroupBox(groupBox_6);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setGeometry(QRect(10, 210, 151, 31));
        content_0 = new QPushButton(groupBox_2);
        content_0->setObjectName(QString::fromUtf8("content_0"));
        content_0->setGeometry(QRect(10, 10, 19, 19));
        content_0->setMaximumSize(QSize(20, 20));
        content_0->setCheckable(true);
        content_0->setAutoExclusive(true);
        content_1 = new QPushButton(groupBox_2);
        content_1->setObjectName(QString::fromUtf8("content_1"));
        content_1->setGeometry(QRect(30, 10, 19, 19));
        content_1->setMaximumSize(QSize(20, 20));
        content_1->setCheckable(true);
        content_1->setAutoExclusive(true);
        content_8 = new QPushButton(groupBox_2);
        content_8->setObjectName(QString::fromUtf8("content_8"));
        content_8->setGeometry(QRect(70, 10, 19, 19));
        content_8->setMaximumSize(QSize(20, 20));
        content_8->setCheckable(true);
        content_8->setAutoExclusive(true);
        content_7 = new QPushButton(groupBox_2);
        content_7->setObjectName(QString::fromUtf8("content_7"));
        content_7->setGeometry(QRect(50, 10, 19, 19));
        content_7->setMaximumSize(QSize(20, 20));
        content_7->setCheckable(true);
        content_7->setAutoExclusive(true);
        content_9 = new QPushButton(groupBox_2);
        content_9->setObjectName(QString::fromUtf8("content_9"));
        content_9->setGeometry(QRect(90, 10, 19, 19));
        content_9->setMaximumSize(QSize(20, 20));
        content_9->setCheckable(true);
        content_9->setAutoExclusive(true);
        content_11 = new QPushButton(groupBox_2);
        content_11->setObjectName(QString::fromUtf8("content_11"));
        content_11->setGeometry(QRect(130, 10, 19, 19));
        content_11->setMaximumSize(QSize(20, 20));
        content_11->setCheckable(true);
        content_11->setAutoExclusive(true);
        content_10 = new QPushButton(groupBox_2);
        content_10->setObjectName(QString::fromUtf8("content_10"));
        content_10->setGeometry(QRect(110, 10, 19, 19));
        content_10->setMaximumSize(QSize(20, 20));
        content_10->setCheckable(true);
        content_10->setChecked(true);
        content_10->setAutoExclusive(true);
        lineEditMaxBlockSizeForPark = new QLineEdit(groupBox_6);
        lineEditMaxBlockSizeForPark->setObjectName(QString::fromUtf8("lineEditMaxBlockSizeForPark"));
        lineEditMaxBlockSizeForPark->setGeometry(QRect(82, 150, 81, 20));
        label_15 = new QLabel(groupBox_6);
        label_15->setObjectName(QString::fromUtf8("label_15"));
        label_15->setGeometry(QRect(50, 150, 31, 21));
        terrainPaint_changeEdit = new QLineEdit(groupBox_6);
        terrainPaint_changeEdit->setObjectName(QString::fromUtf8("terrainPaint_changeEdit"));
        terrainPaint_changeEdit->setGeometry(QRect(55, 38, 31, 20));
        terrainPaint_changeLabel_2 = new QLabel(groupBox_6);
        terrainPaint_changeLabel_2->setObjectName(QString::fromUtf8("terrainPaint_changeLabel_2"));
        terrainPaint_changeLabel_2->setGeometry(QRect(88, 40, 16, 16));
        TabWidget->addTab(ContentEditionTab, QString());
        ControlWidget->setWidget(dockWidgetContents);
        QWidget::setTabOrder(pushButtonGeneratePM, pushButtonClear);
        QWidget::setTabOrder(pushButtonClear, pushButtonMerge);
        QWidget::setTabOrder(pushButtonMerge, pushButtonConnect2);
        QWidget::setTabOrder(pushButtonConnect2, terrainPaint_sizeSlider);
        QWidget::setTabOrder(terrainPaint_sizeSlider, terrainPaint_changeSlider);
        QWidget::setTabOrder(terrainPaint_changeSlider, terrain_2DShader);
        QWidget::setTabOrder(terrain_2DShader, render_3DtreesCheckBox);
        QWidget::setTabOrder(render_3DtreesCheckBox, render_2DroadsExtraWidthSlider);
        QWidget::setTabOrder(render_2DroadsExtraWidthSlider, render_2DroadsStrokeSlider);
        QWidget::setTabOrder(render_2DroadsStrokeSlider, render_2DparksSlider);
        QWidget::setTabOrder(render_2DparksSlider, render_2DparcelLineSlider);
        QWidget::setTabOrder(render_2DparcelLineSlider, lineEditNumAvenueIterations);
        QWidget::setTabOrder(lineEditNumAvenueIterations, lineEditNumStreetIterations);
        QWidget::setTabOrder(lineEditNumStreetIterations, lineEditNumExamples);
        QWidget::setTabOrder(lineEditNumExamples, checkBoxCleanAvenues);
        QWidget::setTabOrder(checkBoxCleanAvenues, checkBoxCleanStreets);
        QWidget::setTabOrder(checkBoxCleanStreets, checkBoxLocalStreets);
        QWidget::setTabOrder(checkBoxLocalStreets, checkBoxCropping);
        QWidget::setTabOrder(checkBoxCropping, checkBoxUseLayer);
        QWidget::setTabOrder(checkBoxUseLayer, lineEditHoughScale);
        QWidget::setTabOrder(lineEditHoughScale, lineEditPatchDistance1);
        QWidget::setTabOrder(lineEditPatchDistance1, lineEditPatchDistance2);
        QWidget::setTabOrder(lineEditPatchDistance2, lineEditInterpolateSigma1);
        QWidget::setTabOrder(lineEditInterpolateSigma1, lineEditInterpolateSigma2);
        QWidget::setTabOrder(lineEditInterpolateSigma2, lineEditRotationAngle);

        retranslateUi(ControlWidget);

        TabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(ControlWidget);
    } // setupUi

    void retranslateUi(QDockWidget *ControlWidget)
    {
        groupBox->setTitle(QApplication::translate("ControlWidget", "Options", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("ControlWidget", "Avenue Iteration:", 0, QApplication::UnicodeUTF8));
        checkBoxLocalStreets->setText(QApplication::translate("ControlWidget", "Generate Local Streets", 0, QApplication::UnicodeUTF8));
        checkBoxCropping->setText(QApplication::translate("ControlWidget", "Cropping", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("ControlWidget", "Street Iteration:", 0, QApplication::UnicodeUTF8));
        checkBoxCleanAvenues->setText(QApplication::translate("ControlWidget", "Clean Avenues", 0, QApplication::UnicodeUTF8));
        checkBoxCleanStreets->setText(QApplication::translate("ControlWidget", "Clean Local Streets", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("ControlWidget", "# Examples:", 0, QApplication::UnicodeUTF8));
        checkBoxUseLayer->setText(QApplication::translate("ControlWidget", "Use Layer", 0, QApplication::UnicodeUTF8));
        label_11->setText(QApplication::translate("ControlWidget", "Random seed:", 0, QApplication::UnicodeUTF8));
        groupBox_8->setTitle(QApplication::translate("ControlWidget", "Road Generation", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("ControlWidget", "sigma2 ^2", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("ControlWidget", "sigma1 ^2", 0, QApplication::UnicodeUTF8));
        pushButtonGeneratePM->setText(QApplication::translate("ControlWidget", "PM", 0, QApplication::UnicodeUTF8));
        pushButtonClear->setText(QApplication::translate("ControlWidget", "Clear", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("ControlWidget", "Avenue patch dist", 0, QApplication::UnicodeUTF8));
        label_9->setText(QApplication::translate("ControlWidget", "Street patch dist", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("ControlWidget", "rotation", 0, QApplication::UnicodeUTF8));
        label_10->setText(QApplication::translate("ControlWidget", "Circle Hough scale", 0, QApplication::UnicodeUTF8));
        label_12->setText(QApplication::translate("ControlWidget", "angle tolerance", 0, QApplication::UnicodeUTF8));
        pushButtonGenerateEx->setText(QApplication::translate("ControlWidget", "Example", 0, QApplication::UnicodeUTF8));
        pushButtonGenerateWarp->setText(QApplication::translate("ControlWidget", "Warp", 0, QApplication::UnicodeUTF8));
        pushButtonGenerateAliaga->setText(QApplication::translate("ControlWidget", "Aliaga", 0, QApplication::UnicodeUTF8));
        checkBoxSaveRoadImages->setText(QApplication::translate("ControlWidget", "save road images", 0, QApplication::UnicodeUTF8));
        checkBoxSavePatchImages->setText(QApplication::translate("ControlWidget", "save patch images", 0, QApplication::UnicodeUTF8));
        label_14->setText(QApplication::translate("ControlWidget", "slope tolerance", 0, QApplication::UnicodeUTF8));
        label_16->setText(QApplication::translate("ControlWidget", "across river", 0, QApplication::UnicodeUTF8));
        TabWidget->setTabText(TabWidget->indexOf(RoadGenerationTab), QApplication::translate("ControlWidget", "Road", 0, QApplication::UnicodeUTF8));
        groupBox_5->setTitle(QApplication::translate("ControlWidget", "Finalize", 0, QApplication::UnicodeUTF8));
        pushButtonMerge->setText(QApplication::translate("ControlWidget", "Merge", 0, QApplication::UnicodeUTF8));
        pushButtonMerge->setShortcut(QApplication::translate("ControlWidget", "Ctrl+M", 0, QApplication::UnicodeUTF8));
        pushButtonConnect2->setText(QApplication::translate("ControlWidget", "Connect", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        pushButtonTrim->setToolTip(QApplication::translate("ControlWidget", "Trim the roads with the terrain area", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        pushButtonTrim->setText(QApplication::translate("ControlWidget", "Trim", 0, QApplication::UnicodeUTF8));
        pushButtonTrim->setShortcut(QApplication::translate("ControlWidget", "Ctrl+M", 0, QApplication::UnicodeUTF8));
        pushButtonRemoveDanglingEdges->setText(QApplication::translate("ControlWidget", "Remove Dangling Edges", 0, QApplication::UnicodeUTF8));
        pushButtonClearBoundaryFlag->setText(QApplication::translate("ControlWidget", "Clear Boudary Flag", 0, QApplication::UnicodeUTF8));
        groupBox_3->setTitle(QApplication::translate("ControlWidget", "Remove Small Blocks", 0, QApplication::UnicodeUTF8));
        label_13->setText(QApplication::translate("ControlWidget", "Size:", 0, QApplication::UnicodeUTF8));
        pushButtonRemoveSmallBlocks->setText(QApplication::translate("ControlWidget", "Remove Small Blocks", 0, QApplication::UnicodeUTF8));
        pushButtonRemoveCloseEdges->setText(QApplication::translate("ControlWidget", "Remove Close Edges", 0, QApplication::UnicodeUTF8));
        TabWidget->setTabText(TabWidget->indexOf(CleanUpTab), QApplication::translate("ControlWidget", "Clean up", 0, QApplication::UnicodeUTF8));
        groupBox_6->setTitle(QApplication::translate("ControlWidget", "Terrain Controls/Render", 0, QApplication::UnicodeUTF8));
        terrainPaint_sizeLabel->setText(QApplication::translate("ControlWidget", "Size: 5%", 0, QApplication::UnicodeUTF8));
        terrainPaint_changeLabel->setText(QApplication::translate("ControlWidget", "Change", 0, QApplication::UnicodeUTF8));
        terrain_2DShader->setText(QApplication::translate("ControlWidget", "2D Shader", 0, QApplication::UnicodeUTF8));
        render_2DroadsStrokeLabel->setText(QApplication::translate("ControlWidget", "Stroke: 1.5", 0, QApplication::UnicodeUTF8));
        render_2DroadsExtraWidthLabel->setText(QApplication::translate("ControlWidget", "R Width: 0.0", 0, QApplication::UnicodeUTF8));
        render_2DparksLabel->setText(QApplication::translate("ControlWidget", "Park 10 %", 0, QApplication::UnicodeUTF8));
        render_3DtreesCheckBox->setText(QApplication::translate("ControlWidget", "3D Trees", 0, QApplication::UnicodeUTF8));
        render_2DparcelLineLabel->setText(QApplication::translate("ControlWidget", "Par. Line: 2.0", 0, QApplication::UnicodeUTF8));
        content_checkbox->setText(QApplication::translate("ControlWidget", "Content Design", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QString());
        content_0->setText(QApplication::translate("ControlWidget", "S", 0, QApplication::UnicodeUTF8));
        content_1->setText(QApplication::translate("ControlWidget", "R", 0, QApplication::UnicodeUTF8));
        content_8->setText(QApplication::translate("ControlWidget", "G", 0, QApplication::UnicodeUTF8));
        content_7->setText(QApplication::translate("ControlWidget", "M", 0, QApplication::UnicodeUTF8));
        content_9->setText(QApplication::translate("ControlWidget", "C", 0, QApplication::UnicodeUTF8));
        content_11->setText(QApplication::translate("ControlWidget", "M", 0, QApplication::UnicodeUTF8));
        content_10->setText(QApplication::translate("ControlWidget", "F", 0, QApplication::UnicodeUTF8));
        label_15->setText(QApplication::translate("ControlWidget", "Size:", 0, QApplication::UnicodeUTF8));
        terrainPaint_changeLabel_2->setText(QApplication::translate("ControlWidget", "m", 0, QApplication::UnicodeUTF8));
        TabWidget->setTabText(TabWidget->indexOf(ContentEditionTab), QApplication::translate("ControlWidget", "Content", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(ControlWidget);
    } // retranslateUi

};

namespace Ui {
    class ControlWidget: public Ui_ControlWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONTROLWIDGET_H
