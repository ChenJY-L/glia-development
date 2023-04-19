#pragma execution_character_set("utf-8")
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include "QTimer"
#include <ctime>
#include <QTime>
#include "QDebug"

int arc;
char** arv;

/*
 * 获取main函数入参
 */
static void get_main_para(int argc, char** argv)
{
	printf("[info] Get main parameters\n");
	arc = argc;
	arv = argv;
}

__attribute__((section(".init_array"))) \
void (*p_get_main_para_test_t)(int, char* []) = &get_main_para;


MainWindow::MainWindow(QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	timer = new QTimer(this);
	timer->setTimerType(Qt::PreciseTimer);
	connect(timer, SIGNAL(timeout()), this, SLOT(timer_UpdateFrame()));
	timer->stop();

	/* Make handle for cam */
	hCamWidget = new CamWindow(ui->widgetCam);
	hCam1 = nullptr;
	/* Make handle for root widget */
	hRootApp = new TApplication("app", &arc, arv);
	hCanvas = new QMainCanvas(ui->widgetRoot);
	// hCanvas  = new QMainCanvas(nullptr);
	hCanvas->resize(hCanvas->sizeHint());
	// hCanvas->setWindowTitle("Qt Example - hCanvas");
	// hCanvas->setGeometry( 100, 100, 699, 499 );
	hCanvas->show();
	// hCanvas->resize(500, 500);
	hCanvas->divide(1, 2);
	/* Make handle for vModulation */
	hArtWidget = new ARTWindow(ui->widgetArt);
	//nTimeCount = 0;
	nAmplitude = 2.4;
	nFrequency = 10;
	showLogMessage("系统初始化完成");
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_bnShow2D_clicked()
{
	// 绘制2D图像
	cv::Mat Phase;
	if (!bStepModel) {
		nAmplitude = hArtWidget->nAmplitude;
		// 调制频率 = 采样率 / 周期采样点 * 周期数
		nFrequency = hArtWidget->AOParam.fSampleRate / hArtWidget->AOParam.nSampsPerChan * hArtWidget->nWaveCount;

		Phase = fGLIA(nAmplitude, nFrequency, hCamWidget->nFps, hCamWidget->sFileName).clone();
	}
	else {
		Phase = fGLIA_step(nAmplitude, vModulation, hCamWidget->sFileName);
	}

	// 数据滤波
	//cv::GaussianBlur(Phase, Phase, cv::Size(3,3), 0, 0);

	Mat2Vec(Phase);

	// Mesh mFrame
	hCanvas->surf(vPhaseX, vPhaseY, vPhaseZ);
	hCanvas->setTitle("算法解调结果");
	// Clear vector
	std::vector<double>().swap(vPhaseX);
	std::vector<double>().swap(vPhaseY);
	std::vector<double>().swap(vPhaseZ);
	std::vector<double>().swap(vModulation);
	ui->bnShow2D->setEnabled(false);
}

void MainWindow::on_bnStartGrab_clicked()
{
	bStepModel = false;
	int ARTStatus = hArtWidget->bStatus;
	int CamStatus = hCamWidget->bStatus;
	int Cam1Status;
	if (ui->cbDoubleCam->isChecked()) {
		Cam1Status = hCam1->bStatus;
	}

	// std::cout << "ARTStatus:" << ARTStatus << std::endl << "CamStatus:" << CamStatus << std::endl;

	if (ARTStatus == ARTInited && (CamStatus == CamInited || Cam1Status == CamInited)) {
		// 采集实验数据
		ui->tabWidget->setCurrentIndex(1);
		hArtWidget->startAO();  // 启动采集卡
		long start = clock();
		hCamWidget->grabStart();   // 启动相机
		long end = clock();
		double time = ((double)(end - start) / CLOCKS_PER_SEC);
		qDebug() << time;
		timer->start(1);
		// std::cout << double(end - start) << std::endl;
		showLogMessage("连续采集模式");
	}
	else if (ARTStatus != ARTInited) {
		// 报错：请先打开采集卡
		QMessageBox::information(nullptr, "Error", "请先打开采集卡");
	}
	else {
		// 报错：打开相机设备
		QMessageBox::information(nullptr, "Error", "请检查相机模块");
	}
}

void MainWindow::on_bnStartStepGrab_clicked()
{
	bStepModel = true;
	int ARTStatus = hArtWidget->bStatus;
	int CamStatus = hCamWidget->bStatus;
	int Cam1Status;
	if (ui->cbDoubleCam->isChecked()) {
		Cam1Status = hCam1->bStatus;
	}

	if (ARTStatus == ARTStepInited && (CamStatus == CamStepInited || Cam1Status == CamStepInited)) {
		// 点数计算没有梳理清楚
		int count = hCamWidget->nFps * hArtWidget->nWaveCount / hArtWidget->nFrequency;
		int cyclePoints = count / hArtWidget->nWaveCount;
		nAmplitude = hArtWidget->nVoltage;
		// 采集卡相机初始化
		hArtWidget->TaskInit();
		hCamWidget->grabStart();

		for (int i = 0; i < count; i++) {
			// 计算调制电压
			vModulation.push_back((double)(nAmplitude * sin(2 * M_PI * (i % cyclePoints) / cyclePoints))
				+ hArtWidget->nBiasVoltage);
			// 写入采集卡
			hArtWidget->writeAnalog(vModulation[i]);
			// 采集卡输出电压
			hArtWidget->startAO();
			// 读回采集卡电压
			hArtWidget->readAnalog();
			// 绘制调制波形
			hCanvas->plot(hArtWidget->vTime, hArtWidget->fAnalogArray1);
			// 相机采集图像
			hCamWidget->grabOneImg();
			// 设置等待时间
			QTime time;
			time.start();
			while (time.elapsed() < 500)             //等待500ms
				QCoreApplication::processEvents();   //处理事件
			// 采集卡停止输出
			hArtWidget->stopStepAO();
		}
		// 停止相机采集卡
		hArtWidget->stopAO();
		hCamWidget->grabStop();
		QMessageBox::information(nullptr, "提示", "采集完成");
		ui->bnShow2D->setEnabled(true);
	}
	else if (ARTStatus != ARTStepInited) {
		// 报错：请先打开采集卡
		QMessageBox::information(nullptr, "Error", "请先打开采集卡");
	}
	else {
		// 报错：打开相机设备
		QMessageBox::information(nullptr, "Error", "请开启相机模块软触发功能");
	}

}

void MainWindow::timer_UpdateFrame()
{
	// nTimeCount = nTimeCount + 0.1;

	/* 更新Root图像 */
	int status = hArtWidget->bStatus;
	if (status == ARTWorking) {
		// hCanvas->plot(vDy, 1, 1);
		hArtWidget->getCurrentAnalog(vVoltData, 0);
		hCanvas->plot(hArtWidget->vTime, vVoltData, 1, 1);
		hCanvas->setTitle("调制电压");
	}
	else if (status == ARTFinish) {
		timer->stop();
		std::vector<double>().swap(vVoltData);
		hCamWidget->grabStop();
		hArtWidget->bStatus = ARTInited;
		// 保存调制数据
		std::string fileName = hCamWidget->sFileName + ".txt";
		std::ofstream f(fileName, std::ios::app);
		if (!f.is_open()) {
			std::cout << "file open failed" << std::endl;
		}
		else {
			f << "t" << " V" << std::endl;
			for (int i = 0; i < hArtWidget->vTime.size(); i++) {
				f << hArtWidget->vTime[i] << " " << hArtWidget->fAnalogArray1[i] << std::endl;
			}
			f.close();
		}

		QMessageBox::information(nullptr, "提示", "采集完成");
		ui->bnShow2D->setEnabled(true);
	}
}

void MainWindow::on_cbDoubleCam_clicked()
{
	if (ui->cbDoubleCam->isChecked()) {
		auto widgetCam1 = new QWidget();
		ui->tabWidget->addTab(widgetCam1, "相机1");
		hCam1 = new CamWindow(widgetCam1);
		showLogMessage("启动双机位模式");
	}
	else {
		delete hCam1;
		ui->tabWidget->removeTab(4);
		showLogMessage("关闭双机位模式");
	}
}

void MainWindow::Mat2Vec(cv::Mat src)
{

	// 将Mat矩阵转换为三个列向量，仅对于单通道图像有效
	int row = src.rows;
	int col = src.cols;

	vPhaseZ = (std::vector<double>)(src.reshape(1, 1));

	int i = 1, j = 1;
	while (true) {
		vPhaseX.push_back(i);
		vPhaseY.push_back(j);

		i++;

		if (i > col) {
			j++;
			i = 1;
		}
		if (j > row) {
			break;
		}
	}
}

void MainWindow::showLogMessage(const std::string& m) {
	ui->tbBrowser->append(QString(m.c_str()));
}

