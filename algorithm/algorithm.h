/*
 * �������������Ϊ��Ŀ�ṩ�㷨�ӿڣ������ش���֮�������
 * ���ڣ�2023/3/9
 * �汾: 2.0
 * ����Գ���¼���
 * ����˵��
 * ����������㷨�ӿ�
 * ע������
 * 1. ��ȡͼ��ʱ����Ҫʹ�ò����޸Ķ�ͼ����
 * 2.
 */

#ifndef TEST_ALGORITHM_H
#define TEST_ALGORITHM_H

#define _USE_MATH_DEFINES
#include <cmath>

#include <opencv2/imgproc/types_c.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

cv::Mat GLIA(double a, std::vector<double> modulation, const char* videoName);

cv::Mat GLIA(double a, double fps, double time, std::vector<double> modulation);

cv::Mat GLIA(double a, double f, const char* videoName);

cv::Mat fGLIA(double a, double f, const char* videoName);

cv::Mat fGLIA(double a, double fps, double time, std::vector<double> modulation);

cv::Mat fGLIA(double a, std::vector<double> modulation, const char* videoName);

cv::Mat fGLIA(double a, double f, double fps, const std::string& filePath);

cv::Mat fGLIA_step(double a, std::vector<double> modulation, const std::string& filePath);

cv::Mat arctan2(cv::Mat X, cv::Mat Y);

#endif //TEST_ALGORITHM_H
