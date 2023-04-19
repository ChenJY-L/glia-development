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
 * ��ȡmain�������
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
	showLogMessage("ϵͳ��ʼ�����");
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_bnShow2D_clicked()
{
	// ����2Dͼ��
	cv::Mat Phase;
	if (!bStepModel) {
		nAmplitude = hArtWidget->nAmplitude;
		// ����Ƶ�� = ������ / ���ڲ����� * ������
		nFrequency = hArtWidget->AOParam.fSampleRate / hArtWidget->AOParam.nSampsPerChan * hArtWidget->nWaveCount;

		Phase = fGLIA(nAmplitude, nFrequency, hCamWidget->nFps, hCamWidget->sFileName).clone();
	}
	else {
		Phase = fGLIA_step(nAmplitude, vModulation, hCamWidget->sFileName);
	}

	// �����˲�
	//cv::GaussianBlur(Phase, Phase, cv::Size(3,3), 0, 0);

	Mat2Vec(Phase);

	// Mesh mFrame
	hCanvas->surf(vPhaseX, vPhaseY, vPhaseZ);
	hCanvas->setTitle("�㷨������");
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
		// �ɼ�ʵ������
		ui->tabWidget->setCurrentIndex(1);
		hArtWidget->startAO();  // �����ɼ���
		long start = clock();
		hCamWidget->grabStart();   // �������
		long end = clock();
		double time = ((double)(end - start) / CLOCKS_PER_SEC);
		qDebug() << time;
		timer->start(1);
		// std::cout << double(end - start) << std::endl;
		showLogMessage("�����ɼ�ģʽ");
	}
	else if (ARTStatus != ARTInited) {
		// �������ȴ򿪲ɼ���
		QMessageBox::information(nullptr, "Error", "���ȴ򿪲ɼ���");
	}
	else {
		// ����������豸
		QMessageBox::information(nullptr, "Error", "�������ģ��");
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
		// ��������û���������
		int count = hCamWidget->nFps * hArtWidget->nWaveCount / hArtWidget->nFrequency;
		int cyclePoints = count / hArtWidget->nWaveCount;
		nAmplitude = hArtWidget->nVoltage;
		// �ɼ��������ʼ��
		hArtWidget->TaskInit();
		hCamWidget->grabStart();

		for (int i = 0; i < count; i++) {
			// ������Ƶ�ѹ
			vModulation.push_back((double)(nAmplitude * sin(2 * M_PI * (i % cyclePoints) / cyclePoints))
				+ hArtWidget->nBiasVoltage);
			// д��ɼ���
			hArtWidget->writeAnalog(vModulation[i]);
			// �ɼ��������ѹ
			hArtWidget->startAO();
			// ���زɼ�����ѹ
			hArtWidget->readAnalog();
			// ���Ƶ��Ʋ���
			hCanvas->plot(hArtWidget->vTime, hArtWidget->fAnalogArray1);
			// ����ɼ�ͼ��
			hCamWidget->grabOneImg();
			// ���õȴ�ʱ��
			QTime time;
			time.start();
			while (time.elapsed() < 500)             //�ȴ�500ms
				QCoreApplication::processEvents();   //�����¼�
			// �ɼ���ֹͣ���
			hArtWidget->stopStepAO();
		}
		// ֹͣ����ɼ���
		hArtWidget->stopAO();
		hCamWidget->grabStop();
		QMessageBox::information(nullptr, "��ʾ", "�ɼ����");
		ui->bnShow2D->setEnabled(true);
	}
	else if (ARTStatus != ARTStepInited) {
		// �������ȴ򿪲ɼ���
		QMessageBox::information(nullptr, "Error", "���ȴ򿪲ɼ���");
	}
	else {
		// ����������豸
		QMessageBox::information(nullptr, "Error", "�뿪�����ģ����������");
	}

}

void MainWindow::timer_UpdateFrame()
{
	// nTimeCount = nTimeCount + 0.1;

	/* ����Rootͼ�� */
	int status = hArtWidget->bStatus;
	if (status == ARTWorking) {
		// hCanvas->plot(vDy, 1, 1);
		hArtWidget->getCurrentAnalog(vVoltData, 0);
		hCanvas->plot(hArtWidget->vTime, vVoltData, 1, 1);
		hCanvas->setTitle("���Ƶ�ѹ");
	}
	else if (status == ARTFinish) {
		timer->stop();
		std::vector<double>().swap(vVoltData);
		hCamWidget->grabStop();
		hArtWidget->bStatus = ARTInited;
		// �����������
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

		QMessageBox::information(nullptr, "��ʾ", "�ɼ����");
		ui->bnShow2D->setEnabled(true);
	}
}

void MainWindow::on_cbDoubleCam_clicked()
{
	if (ui->cbDoubleCam->isChecked()) {
		auto widgetCam1 = new QWidget();
		ui->tabWidget->addTab(widgetCam1, "���1");
		hCam1 = new CamWindow(widgetCam1);
		showLogMessage("����˫��λģʽ");
	}
	else {
		delete hCam1;
		ui->tabWidget->removeTab(4);
		showLogMessage("�ر�˫��λģʽ");
	}
}

void MainWindow::Mat2Vec(cv::Mat src)
{

	// ��Mat����ת��Ϊ�����������������ڵ�ͨ��ͼ����Ч
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

