/*
 * ��Ŀ���ƣ����ڰ���̩�ɼ�����qtģ��
 * ���幦��
 * 1. ���������Ƶ�ʲ�����Ӧ�������ź�
 * 2. ��װ����̩���ú�����Ϊ��������������Ŀ�����
 * 3. ����״̬��ʾ����
 * 4. ������־�������
 * ����Գ���¼���
 */

#ifndef QTART_ARTWINDOW_H
#define QTART_ARTWINDOW_H

#define _USE_MATH_DEFINES
#include <iostream>
#include <cstdio>
#include <cmath>

#include <QWidget>
#include <QTimer>

 // #include "lib/usb3106A.h"
#include "USB3106A.h"

#define ARTClosed 0     // �ɼ����ȴ�ָ��
#define ARTWorking 1    // �ɼ���������
#define ARTFinish 2     // �ɼ����������
#define ARTInited 3     // �ɼ�����ʼ��
#define ARTStepInited 4 // �ɼ������������ʼ��


QT_BEGIN_NAMESPACE
namespace Ui { class ARTWindow; }
QT_END_NAMESPACE

class ARTWindow : public QWidget {
    Q_OBJECT

public:
    explicit ARTWindow(QWidget* parent = nullptr);

    ~ARTWindow() override;

    void getModulationData(std::vector<double>& modulation) const;

    void getCurrentAnalog(std::vector<double>& Analog, int channel = 1) const;

    void TaskInit();

    void writeAnalog(double AngleData);

    void readAnalog();

    void startAO() { on_bnStartAO_clicked(); }

    void stopAO() { on_bnStopAO_clicked(); }

    void stopStepAO();

private slots:
    void on_bnSetParam_clicked();

    void on_bnGetParam_clicked();

    void on_bnOpenDevice_clicked();

    void on_bnCloseDevice_clicked();

    void on_bnStartAO_clicked();

    void on_bnStopAO_clicked();

    void on_cbOneSample_clicked();

    void getStatus();

private:
    void AO_CreateWave(double* Angle, int n = 1);

    static void showErrorMessage(const std::string& message);

    void showLogMessage(const std::string& message);

    void WriteAnalog();

    double calVolt(double amplitude);

public:
    double nVoltage;
    double nAmplitude;
    double nFrequency;
    double nBiasVoltage;
    float fSampleRate = 0;
    int nSampsPerChan = 0;
    USB3106A_AO_PARAM AOParam{};
    USB3106A_AO_STATUS AOStatus{};
    int nWaveCount = 2; // ���n�β���
    bool bTaskFlag; // �������ɱ�־
    int bStatus; // �ɼ���״̬

    std::vector<double> fAngleArray;
    std::vector<double> fAnalogArray1;
    std::vector<double> fAnalogArray2;
    std::vector<double> vTime;

private:
    Ui::ARTWindow* ui;
    QTimer* timer;
    HANDLE hArtDevice;
    unsigned int nTotalSamps = 0;
    U32 nWriteSampsPerChan = 0, nSampsPerChanWritten = 0;
    U32 nAvailSampsPerChan = 0;
    double fTimeout = 10.0;
    int nWriteCnt = 0, nChannel = 0;
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point now;
    double time = 0;
    // double *fAngle = nullptr;
};

#endif //QTART_ARTWINDOW_H

