#define _USE_MATH_DEFINES
#pragma execution_character_set("utf-8")

#include <math.h>
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>
#include <cerrno>
#include <chrono>
#include <fstream>

#include "artwindow.h"
#include "ui_artwindow.h"

namespace ch = std::chrono;

double* fAngle;

ARTWindow::ARTWindow(QWidget* parent)
	: QWidget(parent)
	, ui(new Ui::ARTWindow)
{
	ui->setupUi(this);

	hArtDevice = nullptr;
	bTaskFlag = false;
	bStatus = ARTClosed;

	nAmplitude = 2.414;
	nVoltage = calVolt(nAmplitude);
	nFrequency = 0.1;
	nBiasVoltage = 5;

	memset(&AOParam, 0, sizeof(USB3106A_AO_PARAM));
	/* 使能通道参数 */
	AOParam.CHParam[0].bChannelEn = 1;
	AOParam.CHParam[0].nSampleRange = USB3106A_AO_SAMPRANGE_N10_P10V;
	AOParam.CHParam[0].nReserved0 = 0;
	AOParam.CHParam[0].nReserved1 = 0;
	AOParam.CHParam[0].nReserved2 = 0;

	// 时钟参数
	// f = fSampleRate / nSamplesPerChan
	AOParam.nSampleMode = USB3106A_AO_SAMPMODE_FINITE;
	AOParam.fSampleRate = 100000;
	AOParam.nSampsPerChan = 1000 * 1000;
	AOParam.bRegenModeEn = 0;
	AOParam.nClockSource = USB3106A_AO_CLKSRC_LOCAL;
	AOParam.bClockOutput = 0;
	// 触发参数
	AOParam.bDTriggerEn = 1;
	AOParam.nDTriggerDir = USB3106A_AO_TRIGDIR_CHANGE;
	AOParam.nTriggerSens = 5;
	AOParam.nDelaySamps = 0;

	// 保留
	AOParam.nReserved0 = 0;
	AOParam.nReserved1 = 0;
	AOParam.nReserved2 = 0;

	timer = new QTimer();
	timer->setTimerType(Qt::PreciseTimer);
	connect(timer, SIGNAL(timeout()), this, SLOT(getStatus()));
	on_bnGetParam_clicked();

	connect(ui->cbOneSample, SIGNAL(clicked(bool)), this, SLOT(on_cbOneSample_clicked()));

	ui->tbFrequency->setEnabled(false);
	//ui->cbOneSample->setEnabled(false);
	ui->bnOpenDevice->setEnabled(true);
	ui->bnCloseDevice->setEnabled(false);
	ui->bnStartAO->setEnabled(false);
	ui->bnStopAO->setEnabled(false);
}

ARTWindow::~ARTWindow()
{
	delete ui;
}

/* 获取采集卡调制输出数据 */
void ARTWindow::getModulationData(std::vector<double>& modulation) const {
	modulation = fAngleArray;
	/*    for (auto value : fAngleArray) {
			std::cout << value << std::endl;
		}

		for (auto value : modulation) {
			std::cout << value << std::endl;
		}*/
}

/* ART读回数据接口 */
void ARTWindow::getCurrentAnalog(std::vector<double>& Analog, int channel) const {
	switch (channel) {
	case 0:
		Analog = fAnalogArray1;
		break;
	case 1:
		Analog = fAnalogArray2;
		break;
	default:
		break;
	}
}

/* 外部控制停止 */
void ARTWindow::stopStepAO() {
	if (USB3106A_AO_StopTask(hArtDevice) < 0) {
		showErrorMessage("AO_StopTask Error");
		return;
	}
	showLogMessage("_AO_StopTask OK");
}

/* 配置参数信息 */
void ARTWindow::on_bnSetParam_clicked()
{
	nAmplitude = ui->tbAmplitude->text().toDouble();
	nWaveCount = ui->tbTerm->text().toInt();
	nFrequency = ui->tbFrequency->text().toDouble();
	nBiasVoltage = ui->tbBiasVoltage->text().toDouble();
	AOParam.nSampsPerChan = ui->tbSamplePerChan->text().toInt();
	AOParam.fSampleRate = ui->tbSampleRate->text().toFloat();

	// nSampsPerChan = ui->tbSamplePerChan->text().toInt();
	// fSampleRate = ui->tbSampleRate->text().toFloat();
	// 检验参数配置
	if (USB3106A_AO_VerifyParam(hArtDevice, &AOParam) < 0) {
		showErrorMessage("param data error");
		return;
	}
	showLogMessage("_AO_VerifyParam OK");
	nWriteSampsPerChan = AOParam.nSampsPerChan;

	nVoltage = calVolt(nAmplitude);
	if (bStatus == ARTWorking) {
		on_bnStopAO_clicked();
		on_bnCloseDevice_clicked();
	}
	else if (bStatus == ARTInited || bStatus == ARTStepInited) {
		// on_bnStopAO_clicked();   // 存在稳定性bug
	}
}

/* 获取配置参数信息 */
void ARTWindow::on_bnGetParam_clicked()
{
	ui->tbAmplitude->setText(QString("%1").arg(nAmplitude));
	ui->tbFrequency->setText(QString("%1").arg(nFrequency));
	ui->tbBiasVoltage->setText(QString("%1").arg(nBiasVoltage));
	ui->tbTerm->setText(QString("%1").arg(nWaveCount));
	ui->tbSamplePerChan->setText(QString("%1").arg(AOParam.nSampsPerChan));
	ui->tbSampleRate->setText(QString("%1").arg(AOParam.fSampleRate));
}

/* 获取采集卡基本参数，并打开采集卡设备 */
void ARTWindow::on_bnOpenDevice_clicked()
{
	/* make handle for Art */
	hArtDevice = USB3106A_DEV_Create(0, false);
	if (hArtDevice == INVALID_HANDLE_VALUE) {
		showErrorMessage("Create Device Error!");
		return;
	}
	showLogMessage("_CreateDevice OK");


	// 通道参数
	/*for (nChannel = 0; nChannel < USB3106A_AO_MAX_CHANNELS; nChannel++) {
		AOParam.CHParam[nChannel].bChannelEn = 1;
		AOParam.CHParam[nChannel].nSampleRange = USB3106A_AO_SAMPRANGE_N10_P10V;
		AOParam.CHParam[nChannel].nReserved0 = 0;
		AOParam.CHParam[nChannel].nReserved1 = 0;
		AOParam.CHParam[nChannel].nReserved2 = 0;
	}*/

	on_bnGetParam_clicked();    // 更新参数
	if (USB3106A_AO_VerifyParam(hArtDevice, &AOParam) < 0) {
		showErrorMessage("param data error");
		return;
	}
	showLogMessage("_AO_VerifyParam OK");

	nWriteSampsPerChan = AOParam.nSampsPerChan;
	nTotalSamps = AOParam.nSampsPerChan * USB3106A_AO_MAX_CHANNELS;

	//writeAnalog();

	ui->cbOneSample->setEnabled(true);
	ui->bnOpenDevice->setEnabled(false);
	ui->bnCloseDevice->setEnabled(true);
	ui->bnStartAO->setEnabled(true);
	if (ui->cbOneSample->isChecked())
		bStatus = ARTStepInited;
	else
		bStatus = ARTInited;
	//bTaskFlag = true;
}

/* 关闭设备，并删除句柄 */
void ARTWindow::on_bnCloseDevice_clicked()
{
	// 释放设备对象
	USB3106A_DEV_Release(hArtDevice);
	showLogMessage("_ReleaseDevice OK");

	bTaskFlag = false;
	bStatus = ARTClosed;
	ui->cbOneSample->setEnabled(false);
	ui->bnOpenDevice->setEnabled(true);
	ui->bnCloseDevice->setEnabled(false);
	ui->bnStartAO->setEnabled(false);
	ui->bnStopAO->setEnabled(false);
}

/* 开始AO输出 */
void ARTWindow::on_bnStartAO_clicked()
{
	if (!bTaskFlag) {
		WriteAnalog();
	}
	std::vector<double>().swap(vTime);
	std::vector<double>().swap(fAnalogArray1);
	std::vector<double>().swap(fAnalogArray2);

	if (USB3106A_AO_StartTask(hArtDevice) < 0) {
		showErrorMessage("AO_StartTask Error");
		on_bnCloseDevice_clicked();
		return;
	}
	showLogMessage("_AO_StartTask OK");
	if (!ui->cbOneSample->isChecked()) {
		if (USB3106A_AO_SendSoftTrig(hArtDevice) < 0) {
			showErrorMessage("AO_SendSoftTrig Error");
			on_bnStopAO_clicked();
			return;
		}
		showLogMessage("_AO_SendSoftTrig OK");

		start = std::chrono::system_clock::now();
		timer->start(10);
	}

	ui->bnStartAO->setEnabled(false);
	ui->bnStopAO->setEnabled(true);
	ui->bnCloseDevice->setEnabled(false);

	bStatus = ARTWorking;
}

/* 停止AO输出 */
void ARTWindow::on_bnStopAO_clicked()
{
	if (USB3106A_AO_StopTask(hArtDevice) < 0) {
		showErrorMessage("AO_StopTask Error");
		return;
	}
	showLogMessage("_AO_StopTask OK");

	if (USB3106A_AO_ReleaseTask(hArtDevice) < 0) {
		QMessageBox::information(nullptr, "Error", "AO_ReleaseTask Error");
		return;
	}
	showLogMessage("_AO_ReleaseTask OK");

	delete[] fAngle;
	timer->stop();
	// startTime = 0;
	time = 0;
	ui->bnStartAO->setEnabled(true);
	ui->bnStopAO->setEnabled(false);
	ui->bnCloseDevice->setEnabled(true);
	bTaskFlag = false;
}

/* 产生采集卡输出波形数据
 * Angle: 数组存储采样点电压
 * n: 输出n个波形数据
 * */
void ARTWindow::AO_CreateWave(double* Angle, int n) {
	unsigned int nCyclePoints0 = AOParam.nSampsPerChan / n;
	unsigned int nCyclePoints1 = AOParam.nSampsPerChan;
	for (int i = 0; i < AOParam.nSampsPerChan; i++) {
		if (AOParam.CHParam[0].bChannelEn) {
			// fAngleArray.push_back(sin(2* M_PI * (i % nCyclePoints0)/nCyclePoints0) * nAmplitude);
			Angle[i] = (double)(sin(2 * M_PI * (i % nCyclePoints0) / nCyclePoints0) * nVoltage) + nBiasVoltage;
			fAngleArray.push_back(Angle[i]);
		}

		if (AOParam.CHParam[1].bChannelEn) {
			// fAngleArray.push_back(sin(2* M_PI * (i % nCyclePoints1)/nCyclePoints1) * nAmplitude);
			Angle[i] = (double)(sin(2 * M_PI * (i % nCyclePoints1) / nCyclePoints1) * nVoltage) + nBiasVoltage;
			fAngleArray.push_back(Angle[i]);
		}
	}
}

/*
 * 设置采集卡输出模式
 * */
void ARTWindow::on_cbOneSample_clicked() {

	if (ui->cbOneSample->isChecked()) {
		// checkBox勾选
		if (bStatus == ARTWorking) {
			on_bnStopAO_clicked();
			on_bnCloseDevice_clicked();
		}
		else if (bStatus == ARTInited) {
			on_bnCloseDevice_clicked();
		}

		// 设置为单点输出模式
		AOParam.nSampleMode = USB3106A_AO_SAMPMODE_ONE_DEMAND;
		AOParam.nSampsPerChan = 1;
		AOParam.fSampleRate = 50000.0;
		AOParam.bRegenModeEn = 1;
		AOParam.nClockSource = USB3106A_AO_CLKSRC_LOCAL;
		AOParam.bClockOutput = 0;

		// 触发参数
		AOParam.bDTriggerEn = 1;
		AOParam.nDTriggerDir = USB3106A_AO_TRIGDIR_FALLING;
		AOParam.nTriggerSens = 5;
		AOParam.nDelaySamps = 0;

		// 保留
		AOParam.nReserved0 = 0;
		AOParam.nReserved1 = 0;
		AOParam.nReserved2 = 0;

		ui->bnStartAO->setEnabled(false);
		ui->tbSampleRate->setEnabled(false);
		ui->tbSamplePerChan->setEnabled(false);
		ui->tbFrequency->setEnabled(true);
	}
	else {
		// CheckBox取消
		if (bStatus == ARTWorking) {
			on_bnStopAO_clicked();
			on_bnCloseDevice_clicked();
		}
		else if (bStatus == ARTInited) {
			on_bnCloseDevice_clicked();
		}
		// 设置为默认模式
		AOParam.nSampleMode = USB3106A_AO_SAMPMODE_FINITE;
		AOParam.fSampleRate = 100000;
		AOParam.nSampsPerChan = 1000 * 1000;
		AOParam.bRegenModeEn = 0;
		AOParam.nClockSource = USB3106A_AO_CLKSRC_LOCAL;
		AOParam.bClockOutput = 0;
		// 触发参数
		AOParam.bDTriggerEn = 1;
		AOParam.nDTriggerDir = USB3106A_AO_TRIGDIR_CHANGE;
		AOParam.nTriggerSens = 5;
		AOParam.nDelaySamps = 0;

		// 保留
		AOParam.nReserved0 = 0;
		AOParam.nReserved1 = 0;
		AOParam.nReserved2 = 0;

		ui->bnStartAO->setEnabled(true);
		ui->tbSampleRate->setEnabled(true);
		ui->tbSamplePerChan->setEnabled(true);
		ui->tbFrequency->setEnabled(false);
	}
}

/* 获取采集卡运行状态数据 */
void ARTWindow::getStatus() {
	// time = time + 0.01;
	if (USB3106A_AO_GetStatus(hArtDevice, &AOStatus) < 0)
	{
		showErrorMessage("AO_GetStatus Error");
		// getc(stdin);
	}

	std::string m = "bTaskDone=" + std::to_string(AOStatus.bTaskDone) + "bTriggered=" + std::to_string(AOStatus.bTriggered)
		+ "nSampsPerChanAcquired=" + std::to_string(AOStatus.nSampsPerChanAcquired);
	showLogMessage(m);
	//	printf("nSoftUnderflowCnt=%d nHardUnderflowCnt=%d\n", AOStatus.nSoftUnderflowCnt, AOStatus.nHardUnderflowCnt);

	if (AOStatus.bTaskDone) // 任务输出完成
	{
		timer->stop();
		time = 0;
		showLogMessage("Task done");
		bStatus = ARTFinish;
		on_bnStopAO_clicked();
		return;
		// writeAnalog();
	}

	readAnalog();
	now = std::chrono::system_clock::now();
	auto duration = ch::duration_cast<ch::microseconds>(now - start);
	time = double(duration.count()) * ch::microseconds::period::num / ch::microseconds::period::den;
	vTime.push_back(time);
	bStatus = ARTWorking;
}

/* 读取当前采集卡输出数值 */
void ARTWindow::readAnalog() {
	double AnalogData[2];
	USB3106A_AO_ReadbackAnalog(hArtDevice, AnalogData);
	//    qDebug("fangle1:%f", AnalogData[0]);
	//    qDebug("fangle2:%f", AnalogData[1]);
	if (AOParam.CHParam[0].bChannelEn) {
		fAnalogArray1.push_back(AnalogData[0]);
		std::string m = "Read AngleData:" + std::to_string(AnalogData[0]);
		showLogMessage(m);
	}
	else if (AOParam.CHParam[1].bChannelEn) {
		fAnalogArray2.push_back(AnalogData[1]);
		std::string m = "Read AngleData:" + std::to_string(AnalogData[1]);
		showLogMessage(m);
	}
}

/* 输出错误信息 */
void ARTWindow::showErrorMessage(const std::string& message) {
	QMessageBox::information(nullptr, "Error", QString(message.c_str()));
}

/* 输出日志信息 */
void ARTWindow::showLogMessage(const std::string& message) {
	ui->textBrowser->append(QString(message.c_str()));
}

/* 输出任务初始化 */
void ARTWindow::TaskInit() {
	bTaskFlag = true;
	if (USB3106A_AO_InitTask(hArtDevice, &AOParam, NULL) < 0) {
		showErrorMessage("AO_InitTask Error");
		return;
	}
	showLogMessage("_AO_InitTask OK");
}

/* 连续输出写入数据 */
void ARTWindow::WriteAnalog() {
	nAvailSampsPerChan = 0;
	nSampsPerChanWritten = 0;

	TaskInit();
	// fAngle = (double*)malloc(sizeof(double[nTotalSamps]));
	fAngle = new double[nTotalSamps];
	AO_CreateWave(fAngle, nWaveCount);

	if (fAngle == nullptr)
	{
		showErrorMessage("Out of memory!");
		on_bnCloseDevice_clicked();
		return;
	}

	showLogMessage("Data Created");
	int tmp = USB3106A_AO_WriteAnalog(hArtDevice, fAngle, nWriteSampsPerChan, &nSampsPerChanWritten,
		&nAvailSampsPerChan, fTimeout);
	//    qDebug("data:%f",fAngle[10]);
	//    qDebug("nAvailSampsPerChan:%d",nAvailSampsPerChan);
	//    qDebug("nSampsPerChanWritten:%d",nSampsPerChanWritten);
	if (tmp < 0) {
		std::cout << errno << std::endl;
		showErrorMessage("AO_WriteAnalog Error...\n");
	}
	showLogMessage("Press to start...");
}

/* 单点输出写入数据 */
void ARTWindow::writeAnalog(double AngleData) {
	nAvailSampsPerChan = 0;
	nSampsPerChanWritten = 0;
	if (ui->cbOneSample->isChecked())
		nWriteSampsPerChan = 1;
	else
		nWriteSampsPerChan = AOParam.nSampsPerChan;
	// TaskInit();
	double angleArray[1];
	angleArray[0] = AngleData;
	std::string m = "Write AngleData: " + std::to_string(angleArray[0]);
	showLogMessage(m);

	int tmp = USB3106A_AO_WriteAnalog(hArtDevice, angleArray, nWriteSampsPerChan, &nSampsPerChanWritten,
		&nAvailSampsPerChan, fTimeout);
	if (tmp < 0) {
		std::cout << errno << std::endl;
		showErrorMessage("AO_WriteAnalog Error...\n");
		on_bnCloseDevice_clicked();
	}
}


// 计算产生特定调制深度所需电压
double ARTWindow::calVolt(double amplitude) {
	double maxVolt = 150;              // 最大耐压150V
	double maxDisplacement = 17.4e-6;  // 最大形变17.4um
	double lambda = 532e-9;            // 光波长

	double voltage = amplitude * lambda / (2 * M_PI) / maxDisplacement * maxVolt;
	std::string m = "产生相位调制深度为:" + std::to_string(amplitude);
	showLogMessage(m);
	m = "设置调制电压为：" + std::to_string(voltage) + "\n";
	showLogMessage(m);
	return voltage;
}

