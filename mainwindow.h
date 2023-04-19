/*
 * 毕设代码：基于广义锁相检测技术的光学干涉仪解调软件设计
 * 开发平台：Qt， Opencv， MVS， Root
 * 软件版本：4.0
 * 程序猿：陈嘉宇
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <iostream>
#include <vector>
#include <fstream>

#include <opencv2/imgproc/types_c.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "ART/artwindow.h"
#include "cam/CamWindow.h"
#include "canvas/canvas.h"
#include "algorithm/algorithm.h"


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    void Mat2Vec(cv::Mat src);

    void showLogMessage(const std::string& m);

private slots:
    void on_bnShow2D_clicked();

    void on_bnStartGrab_clicked();

    void timer_UpdateFrame();

    void on_cbDoubleCam_clicked();

    void on_bnStartStepGrab_clicked();

private:
    Ui::MainWindow* ui;
    /* cam parameters */
    CamWindow* hCamWidget;
    CamWindow* hCam1;
    QTimer* timer;
    // double                  nTimeCount;
    /* Root parameters */
    TApplication* hRootApp;
    QMainCanvas* hCanvas;
    std::vector<double>     vDx;
    std::vector<double>     vDy;
    /* 2D Parameters */
    bool                    bStepModel = false;
    std::vector<double>     vPhaseX;
    std::vector<double>     vPhaseY;
    std::vector<double>     vPhaseZ;
    /* Modulation Parameters */
    ARTWindow* hArtWidget;
    double                  nAmplitude;
    double                  nFrequency;
    std::vector<double>     vModulation;
    std::vector<double>     vVoltData;
};

#endif // MAINWINDOW_H
