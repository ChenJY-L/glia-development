//
// Created by jack-chen on 2023/3/4.
//

// You may need to build the project (run Qt uic code generator) to get "ui_camwindow.h" resolved

#include <QMessageBox>
#include "CamWindow.h"
#include "ui_camwindow.h"
#include "QTimer"


CamWindow::CamWindow(QWidget* parent) :
    QWidget(parent), ui(new Ui::CamWindow) {
    ui->setupUi(this);
    /* Timer Init */
    timer = new QTimer(this);
    timer->setTimerType(Qt::PreciseTimer);
    connect(timer, SIGNAL(timeout()), this, SLOT(timer_UpdateFrame()));
    timer->stop();
    nTimeCount = 0;
    /* Make handle for display image */
    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    m_pcMyCamera = nullptr;
    bRecording = false;
    bGrabbing = false;
    m_hWnd = (void*)ui->widgetDisplay->winId();
    bStatus = CamClosed;
}

CamWindow::~CamWindow() {
    delete ui;
}

// ch:��ʾ������Ϣ | en:Show error message
void CamWindow::ShowErrorMsg(QString csMessage, int nErrorNum)
{
    QString errorMsg = std::move(csMessage);
    if (nErrorNum != 0)
    {
        QString TempMsg;
        QString::asprintf(": Error = %x: ", nErrorNum);
        errorMsg += TempMsg;
    }

    switch (nErrorNum)
    {
    case MV_E_HANDLE:           errorMsg += "Error or invalid handle ";                                         break;
    case MV_E_SUPPORT:          errorMsg += "Not supported function ";                                          break;
    case MV_E_BUFOVER:          errorMsg += "Cache is full ";                                                   break;
    case MV_E_CALLORDER:        errorMsg += "Function calling order error ";                                    break;
    case MV_E_PARAMETER:        errorMsg += "Incorrect parameter ";                                             break;
    case MV_E_RESOURCE:         errorMsg += "Applying resource failed ";                                        break;
    case MV_E_NODATA:           errorMsg += "No data ";                                                         break;
    case MV_E_PRECONDITION:     errorMsg += "Precondition error, or running environment changed ";              break;
    case MV_E_VERSION:          errorMsg += "Version mismatches ";                                              break;
    case MV_E_NOENOUGH_BUF:     errorMsg += "Insufficient memory ";                                             break;
    case MV_E_ABNORMAL_IMAGE:   errorMsg += "Abnormal image, maybe incomplete image because of lost packet ";   break;
    case MV_E_UNKNOW:           errorMsg += "Unknown error ";                                                   break;
    case MV_E_GC_GENERIC:       errorMsg += "General error ";                                                   break;
    case MV_E_GC_ACCESS:        errorMsg += "Node accessing condition error ";                                  break;
    case MV_E_ACCESS_DENIED:	errorMsg += "No permission ";                                                   break;
    case MV_E_BUSY:             errorMsg += "Device is busy, or network disconnected ";                         break;
    case MV_E_NETER:            errorMsg += "Network error ";                                                   break;
    }

    QMessageBox::information(nullptr, "PROMPT", errorMsg);
}

void __stdcall CamWindow::ImageCallBack(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{
    if (pUser)
    {
        CamWindow* pMainWindow = (CamWindow*)pUser;
        pMainWindow->ImageCallBackInner(pData, pFrameInfo);
    }
}

void CamWindow::ImageCallBackInner(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo) {
    MV_DISPLAY_FRAME_INFO stDisplayInfo;
    memset(&stDisplayInfo, 0, sizeof(MV_DISPLAY_FRAME_INFO));

    stDisplayInfo.hWnd = m_hWnd;
    stDisplayInfo.pData = pData;
    stDisplayInfo.nDataLen = pFrameInfo->nFrameLen;
    stDisplayInfo.nWidth = pFrameInfo->nWidth;
    stDisplayInfo.nHeight = pFrameInfo->nHeight;
    stDisplayInfo.enPixelType = pFrameInfo->enPixelType;
    m_pcMyCamera->DisplayOneFrame(&stDisplayInfo);

    /* ��pdata����Ϊbmp�ļ�����ʹ��videoWriter�洢Ϊ��Ƶ */
    if (bRecording) {
        pDataForSaveImage = (unsigned char*)malloc(pFrameInfo->nWidth * pFrameInfo->nHeight * 4 + 2048);
        if (nullptr == pDataForSaveImage) {
            ShowErrorMsg("No Image Data ", 0);
        }
        // ����ͼ����
        // fill in the parameters of save image
        memset(&stSaveParam, 0, sizeof(MV_SAVE_IMAGE_PARAM_EX));
        // ���ϵ��������ǣ����ͼƬ��ʽ���������ݵ����ظ�ʽ���ṩ�������������С��ͼ���
        // ͼ��ߣ��������ݻ��棬���ͼƬ���棬JPG��������
        // Top to bottom are��
        stSaveParam.enImageType = MV_Image_Bmp;
        stSaveParam.enPixelType = pFrameInfo->enPixelType;
        stSaveParam.nBufferSize = pFrameInfo->nWidth * pFrameInfo->nHeight * 4 + 2048;
        stSaveParam.nWidth = pFrameInfo->nWidth;
        stSaveParam.nHeight = pFrameInfo->nHeight;
        stSaveParam.pData = pData;
        stSaveParam.nDataLen = pFrameInfo->nFrameLen;
        stSaveParam.pImageBuffer = pDataForSaveImage;
        stSaveParam.nJpgQuality = 99;

        int nRet = m_pcMyCamera->SaveImage(&stSaveParam);
        if (MV_OK != nRet) {
            ShowErrorMsg("failed in MV_CC_SaveImage,nRet[%x]\n", nRet);
        }
        std::string imgName;
        imgName = sFileName + "/" + std::to_string(nCount) + ".bmp";
        FILE* fp = fopen(imgName.c_str(), "wb");
        if (NULL == fp) {
            printf("fopen failed\n");
        }
        fwrite(pDataForSaveImage, 1, stSaveParam.nImageLen, fp);
        fclose(fp);
        nCount = nCount + 1 / nFps;
        //printf("save image succeed\n");
        /* ��ͼƬ��������Ƶ */
//        mFrame = cv::imread("i.bmp");
//        hVideoWriter << mFrame;
        /*
        vDy.push_back(mFrame.at<uchar>(320, 480));
        double tmp;
        tmp = nAmplitude * sin(2 * M_PI * nFrequency * nTimeCount);
        vModulation.push_back(tmp);
         */
    }
}

void CamWindow::on_bnEnum_clicked()
{
    ui->ComboDevices->clear();

    // ch:ö�������������豸 | en:Enumerate all devices within subnet
    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    int nRet = CMvCamera::EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &m_stDevList);
    if (MV_OK != nRet)
    {
        return;
    }

    // ch:��ֵ���뵽��Ϣ�б���в���ʾ���� | en:Add value to the information list box and display
    for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
    {
        QString strMsg;
        MV_CC_DEVICE_INFO* pDeviceInfo = m_stDevList.pDeviceInfo[i];
        if (NULL == pDeviceInfo)
        {
            continue;
        }

        if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
        {
            int nIp1 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
            int nIp2 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
            int nIp3 = ((m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
            int nIp4 = (m_stDevList.pDeviceInfo[i]->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

            if (strcmp("", (char*)pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName) != 0)
            {
                strMsg.sprintf("[%d]GigE:   %s  (%d.%d.%d.%d)", i, pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName, nIp1, nIp2, nIp3, nIp4);
            }
            else
            {
                strMsg.sprintf("[%d]GigE:   %s %s (%s)  (%d.%d.%d.%d)", i, pDeviceInfo->SpecialInfo.stGigEInfo.chManufacturerName,
                    pDeviceInfo->SpecialInfo.stGigEInfo.chModelName, pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber, nIp1, nIp2, nIp3, nIp4);
            }
        }
        else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE)
        {
            if (strcmp("", (char*)pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName) != 0)
            {
                strMsg.sprintf("[%d]UsbV3:  %s", i, pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
            }
            else
            {
                strMsg.sprintf("[%d]UsbV3:  %s %s (%s)", i, pDeviceInfo->SpecialInfo.stUsb3VInfo.chManufacturerName,
                    pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName, pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
            }
        }
        else
        {
            ShowErrorMsg("Unknown device enumerated", 0);
        }
        ui->ComboDevices->addItem(strMsg);
    }

    if (0 == m_stDevList.nDeviceNum)
    {
        ShowErrorMsg("No device", 0);
        return;
    }
    ui->ComboDevices->setCurrentIndex(0);
}

void CamWindow::on_bnOpen_clicked()
{
    int nIndex = ui->ComboDevices->currentIndex();
    if ((nIndex < 0) || (nIndex >= MV_MAX_DEVICE_NUM))
    {
        ShowErrorMsg("Please select device", 0);
        return;
    }

    // ch:���豸��Ϣ�����豸ʵ�� | en:Device instance created by device information
    if (NULL == m_stDevList.pDeviceInfo[nIndex])
    {
        ShowErrorMsg("Device does not exist", 0);
        return;
    }

    if (m_pcMyCamera == NULL)
    {
        m_pcMyCamera = new CMvCamera;
        if (NULL == m_pcMyCamera)
        {
            return;
        }
    }

    int nRet = m_pcMyCamera->Open(m_stDevList.pDeviceInfo[nIndex]);
    if (MV_OK != nRet)
    {
        delete m_pcMyCamera;
        m_pcMyCamera = NULL;
        ShowErrorMsg("Open Fail", nRet);
        return;
    }

    // ch:̽��������Ѱ���С(ֻ��GigE�����Ч) | en:Detection network optimal package size(It only works for the GigE camera)
    if (m_stDevList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
    {
        unsigned int nPacketSize = 0;
        nRet = m_pcMyCamera->GetOptimalPacketSize(&nPacketSize);
        if (nRet == MV_OK)
        {
            nRet = m_pcMyCamera->SetIntValue("GevSCPSPacketSize", nPacketSize);
            if (nRet != MV_OK)
            {
                ShowErrorMsg("Warning: Set Packet Size fail!", nRet);
            }
        }
        else
        {
            ShowErrorMsg("Warning: Get Packet Size fail!", nRet);
        }
    }

    m_pcMyCamera->SetEnumValue("AcquisitionMode", MV_ACQ_MODE_CONTINUOUS);
    m_pcMyCamera->SetEnumValue("TriggerMode", MV_TRIGGER_MODE_OFF);

    on_bnGetParam_clicked(); // ch:��ȡ���� | en:Get Parameter

    ui->bnOpen->setEnabled(false);
    ui->bnClose->setEnabled(true);
    ui->bnStart->setEnabled(true);
    ui->bnStop->setEnabled(false);
    ui->bnContinuesMode->setEnabled(true);
    ui->bnContinuesMode->setChecked(true);
    ui->bnTriggerMode->setEnabled(true);
    ui->cbSoftTrigger->setEnabled(false);
    ui->bnTriggerExec->setEnabled(false);

    ui->tbExposure->setEnabled(true);
    ui->tbGain->setEnabled(true);
    ui->tbFrameRate->setEnabled(true);
    ui->bnSetParam->setEnabled(true);
    ui->bnGetParam->setEnabled(true);
    ui->cbRecordTrigger->setEnabled(true);

    bStatus = CamOpened;
}

void CamWindow::on_bnClose_clicked()
{
    if (m_pcMyCamera)
    {
        m_pcMyCamera->Close();
        delete m_pcMyCamera;
        m_pcMyCamera = NULL;
    }
    bGrabbing = false;

    ui->bnOpen->setEnabled(true);
    ui->bnClose->setEnabled(false);
    ui->bnStart->setEnabled(false);
    ui->bnStop->setEnabled(false);
    ui->bnContinuesMode->setEnabled(false);
    ui->bnTriggerMode->setEnabled(false);
    ui->cbSoftTrigger->setEnabled(false);
    ui->bnTriggerExec->setEnabled(false);

    ui->tbExposure->setEnabled(false);
    ui->tbGain->setEnabled(false);
    ui->tbFrameRate->setEnabled(false);
    ui->bnSetParam->setEnabled(false);
    ui->bnGetParam->setEnabled(false);
    ui->cbRecordTrigger->setEnabled(false);

    bStatus = CamClosed;
}

void CamWindow::on_bnContinuesMode_clicked()
{
    if (true == ui->bnContinuesMode->isChecked())
    {
        m_pcMyCamera->SetEnumValue("TriggerMode", MV_TRIGGER_MODE_OFF);
        ui->cbSoftTrigger->setEnabled(false);
        ui->bnTriggerExec->setEnabled(false);
    }
}

void CamWindow::on_bnTriggerMode_clicked()
{
    if (true == ui->bnTriggerMode->isChecked())
    {
        m_pcMyCamera->SetEnumValue("TriggerMode", MV_TRIGGER_MODE_ON);
        if (true == ui->cbSoftTrigger->isChecked())
        {
            m_pcMyCamera->SetEnumValue("TriggerSource", MV_TRIGGER_SOURCE_SOFTWARE);
            if (bGrabbing)
            {
                ui->bnTriggerExec->setEnabled(true);
            }
        }
        else
        {
            m_pcMyCamera->SetEnumValue("TriggerSource", MV_TRIGGER_SOURCE_LINE0);
        }
        ui->cbSoftTrigger->setEnabled(true);
    }
}

void CamWindow::on_bnStart_clicked()
{
    m_pcMyCamera->RegisterImageCallBack(ImageCallBack, this);

    int nRet = m_pcMyCamera->StartGrabbing();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Start grabbing fail", nRet);
        return;
    }
    bGrabbing = true;

    ui->bnStart->setEnabled(false);
    ui->bnStop->setEnabled(true);
    ui->cbRecordTrigger->setEnabled(false);
    ui->bnClose->setEnabled(false);
    if (ui->bnTriggerMode->isChecked() && ui->cbSoftTrigger->isChecked())
    {
        ui->bnTriggerExec->setEnabled(true);
    }

    timer->start(10);
    if (bRecording)
    {
        ui->lRate->setNum(0);
        //timer->start(1000);
        getFileName();
        std::string command;
        command = "mkdir -p " + sFileName;
        system(command.c_str());
        //        hVideoWriter.open(sFileName, nCodec, nFps,
        //                          cv::Size(int(nWidth), int(nHeight)), true);
    }
    bStatus = CamStarted;
}

void CamWindow::on_bnStop_clicked()
{
    int nRet = m_pcMyCamera->StopGrabbing();
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Stop grabbing fail", nRet);
        return;
    }
    bGrabbing = false;

    ui->bnStart->setEnabled(true);
    ui->bnStop->setEnabled(false);
    ui->bnTriggerExec->setEnabled(false);
    ui->cbRecordTrigger->setEnabled(true);
    ui->bnClose->setEnabled(true);

    timer->stop();
    nTimeCount = 0;
    nCount = 0;
    ui->lRate->setNum(nTimeCount);
    if (bRecording)
    {
        //        timer->stop();
        //        nTimeCount = 0;
        //        hVideoWriter.release();
                // ui->bnShow2D->setEnabled(true);
                // vDy.clear();
    }
    if (bRecording)
        bStatus = CamInited;
    else
        bStatus = CamOpened;
}

void CamWindow::on_cbSoftTrigger_clicked()
{
    if (ui->cbSoftTrigger->isChecked())
    {
        m_pcMyCamera->SetEnumValue("TriggerSource", MV_TRIGGER_SOURCE_SOFTWARE);
        if (bGrabbing)
        {
            ui->bnTriggerExec->setEnabled(true);
        }
    }
    else
    {
        m_pcMyCamera->SetEnumValue("TriggerSource", MV_TRIGGER_SOURCE_LINE0);
        ui->bnTriggerExec->setEnabled(false);
    }
    bStatus = CamStepInited;
}

void CamWindow::on_bnTriggerExec_clicked()
{
    int nRet = m_pcMyCamera->CommandExecute("TriggerSoftware");
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Trigger Software fail", nRet);
    }
    // timer_UpdateFrame();
}

void CamWindow::on_bnGetParam_clicked()
{
    MVCC_FLOATVALUE stFloatValue;
    memset(&stFloatValue, 0, sizeof(MVCC_FLOATVALUE));

    int nRet = m_pcMyCamera->GetFloatValue("ExposureTime", &stFloatValue);
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Get Exposure Time Fail", nRet);
    }
    else
    {
        ui->tbExposure->setText(QString("%1").arg(stFloatValue.fCurValue));
    }

    nRet = m_pcMyCamera->GetFloatValue("Gain", &stFloatValue);
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Get Gain Fail", nRet);
    }
    else
    {
        ui->tbGain->setText(QString("%1").arg(stFloatValue.fCurValue));
    }

    nRet = m_pcMyCamera->GetFloatValue("ResultingFrameRate", &stFloatValue);
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Get Frame Rate Fail", nRet);
    }
    else
    {
        ui->tbFrameRate->setText(QString("%1").arg(stFloatValue.fCurValue));
        nFps = stFloatValue.fCurValue;
    }
}

void CamWindow::on_bnSetParam_clicked()
{
    m_pcMyCamera->SetEnumValue("ExposureAuto", 0);
    int nRet = m_pcMyCamera->SetFloatValue("ExposureTime", ui->tbExposure->text().toFloat());
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Set Exposure Time Fail", nRet);
    }

    m_pcMyCamera->SetEnumValue("GainAuto", 0);
    nRet = m_pcMyCamera->SetFloatValue("Gain", ui->tbGain->text().toFloat());
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Set Gain Fail", nRet);
    }

    nFps = ui->tbFrameRate->text().toFloat();
    nRet = m_pcMyCamera->SetFloatValue("AcquisitionFrameRate", ui->tbFrameRate->text().toFloat());
    if (MV_OK != nRet)
    {
        ShowErrorMsg("Set Frame Rate Fail", nRet);
    }
}

void CamWindow::on_cbRecordTrigger_clicked()
{
    bRecording = ui->cbRecordTrigger->isChecked();

    if (bRecording)
    {
        ui->lRate->setNum(0);   // ʱ������
        /* ��ȡ��ǰͼ����Ϣ */
        nCodec = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');

        // ������ѹ��
        //nCodec = 0;
        //nFps   = 0;
        MVCC_INTVALUE_EX stIntValue;
        int nRet = m_pcMyCamera->GetIntValue("Width", &stIntValue);
        if (nRet != MV_OK)
        {
            ShowErrorMsg("Get nWidth Failed\n", nRet);
        }
        else
        {
            nWidth = stIntValue.nCurValue;
        }

        nRet = m_pcMyCamera->GetIntValue("Height", &stIntValue);
        if (nRet != MV_OK)
        {
            ShowErrorMsg("Get nHeight Failed\n", nRet);
        }
        else
        {
            nHeight = stIntValue.nCurValue;
        }
        on_bnGetParam_clicked();

        if (!ui->cbSoftTrigger->isChecked())
            bStatus = CamInited;
        else
            bStatus = CamStepInited;
    }

    if (!bRecording)
    {
        timer->stop();
        // hVideoWriter.release();
    }
}

void CamWindow::getFileName()
{
    time_t t;
    time(&t);
    char name[256];
    strftime(name, sizeof(name), "%H:%M:%S", localtime(&t));
    // QString camName = ui->ComboDevices->currentText();
    // sFileName = "test.avi";
    sFileName = std::string(name);

    //    sFileName = camName.toStdString() + std::string(name) + ".avi";
}

void CamWindow::timer_UpdateFrame() {
    nTimeCount = nTimeCount + 0.01;
    ui->lRate->setNum(nTimeCount);

    /* ����fps */
    MVCC_FLOATVALUE stFloatValue;
    memset(&stFloatValue, 0, sizeof(MVCC_FLOATVALUE));

    int nRet = m_pcMyCamera->GetFloatValue("ResultingFrameRate", &stFloatValue);
    if (MV_OK != nRet) {
        ShowErrorMsg("Get Frame Rate Fail", nRet);
    }
    else {
        ui->tbFrameRate->setText(QString("%1").arg(stFloatValue.fCurValue));
        if (abs(nFps - stFloatValue.fCurValue) > 1)
            ShowErrorMsg("Fps not stable!\n", 0);
    }
}