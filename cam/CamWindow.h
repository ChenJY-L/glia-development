//
// Created by jack-chen on 2023/3/4.
//

#ifndef GLIA_ART_CAMWINDOW_H
#define GLIA_ART_CAMWINDOW_H

#include <QWidget>

#include <opencv2/imgproc/types_c.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "MvCamera/MvCamera.h"

#define CamOpened 0         // 相机开启
#define CamClosed 1         // 相机关闭
#define CamInited 2         // 相机初始化完成
#define CamStarted 3        // 相机开始采集
#define CamStopped 4        // 相机采集暂停
#define CamStepInited 5     // 相机软触发初始化完成

QT_BEGIN_NAMESPACE
namespace Ui { class CamWindow; }
QT_END_NAMESPACE

class CamWindow : public QWidget {
    Q_OBJECT

public:
    explicit CamWindow(QWidget* parent = nullptr);

    ~CamWindow() override;

    void static __stdcall ImageCallBack(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser);

    void ImageCallBackInner(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInf);

    void grabStart() { on_bnStart_clicked(); }

    void grabStop() { on_bnStop_clicked(); }

    void grabOneImg() { on_bnTriggerExec_clicked(); }
private slots:
    void on_cbRecordTrigger_clicked();

    void on_bnEnum_clicked();

    void on_bnOpen_clicked();

    void on_bnClose_clicked();

    void on_bnStart_clicked();

    void on_bnStop_clicked();

    void on_bnContinuesMode_clicked();

    void on_bnTriggerMode_clicked();

    void on_cbSoftTrigger_clicked();

    void on_bnTriggerExec_clicked();

    void on_bnGetParam_clicked();

    void on_bnSetParam_clicked();

    void timer_UpdateFrame();
private:
    static void ShowErrorMsg(QString csMessage, int nErrorNum);

    void getFileName();

public:
    std::string             sFileName;  // 存储文件名字
    int                     bStatus;    // 相机状态
    double                  nFps;

private:
    Ui::CamWindow* ui;
    /* cam parameters */
    void* m_hWnd;
    QTimer* timer;
    MV_CC_DEVICE_INFO_LIST  m_stDevList{};
    MV_SAVE_IMAGE_PARAM_EX  stSaveParam{};
    CMvCamera* m_pcMyCamera;
    bool                    bGrabbing;   // 是否开始抓图
    bool                    bRecording;  // 是否存储数据
    double                  nTimeCount;
    cv::Mat                 mFrame;
    cv::VideoWriter         hVideoWriter;
    /* record parameters */
    unsigned char* pDataForSaveImage = nullptr;
    int                     nCodec{};
    unsigned int            nHeight{};
    unsigned int            nWidth{};
    double                  nCount = 0;
};


#endif //GLIA_ART_CAMWINDOW_H
