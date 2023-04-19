//
// Created by jack-chen on 2022/11/16.
//

#include "algorithm.h"
#include <filesystem>
#include <set>

namespace fs = std::filesystem;


cv::Mat GLIA(double a, std::vector<double> modulation, const char* videoName)
{
    // GLIA 算法实现
    cv::VideoCapture cap;
    cap.open(videoName);
    cv::Mat frame;
    if (!cap.isOpened()) {
        printf("Open Video Failed!\n");
        return frame;
    }

    cv::Mat X;
    cv::Mat Y;
    cv::Mat tmp;
    int i = 0;
    double C, S;
    // 离散函数积分
    while (true) {
        cap >> frame;
        if (frame.empty()) {
            cap.release();
            break;
        }

        cv::cvtColor(frame, frame, CV_RGB2GRAY);
        frame.convertTo(frame, CV_64FC1);    // 转换图像格式，防止数据丢失

        C = cos(modulation.at(i));
        S = sin(modulation.at(i));

        if (X.empty() || Y.empty()) {
            cv::Mat X_mul = frame * C;
            X = X_mul.clone();
            cv::Mat Y_mul = frame * S;
            Y = Y_mul.clone();
        }
        else {
            X = X + frame * C;
            Y = Y + frame * S;
        }
        i++;
    }

    double M = 1 + std::cyl_bessel_jl(0, 2 * a);
    double N = 1 - std::cyl_bessel_jl(0, 2 * a);
    //计算相位
    cv::Mat dst1 = X / M;
    cv::Mat dst2 = Y / N;
    cv::Mat Phase = arctan2(dst1, dst2);;
    // cv::phase(X, Y, Phase,false);    // true输出角度 false输出弧度

    return Phase;
}

cv::Mat GLIA(double a, double fps, double time, std::vector<double> modulation)
{
    double timeCount = 0;
    cv::Mat X;
    cv::Mat Y;
    int count = 0;
    double C, S;
    std::string name;

    // 离散函数积分
    while (timeCount < time) {
        name = std::to_string(timeCount) + ".bmp";
        cv::Mat frame = cv::imread(name);   // 取一帧图像

        cv::cvtColor(frame, frame, CV_RGB2GRAY);
        frame.convertTo(frame, CV_64FC1);    // 转换图像格式，防止数据丢失


        C = cos(modulation.at(count));
        S = sin(modulation.at(count));

        if (X.empty() || Y.empty()) {
            cv::Mat X_mul = frame * C;
            X = X_mul.clone();
            cv::Mat Y_mul = frame * S;
            Y = Y_mul.clone();
        }
        else {
            X = X + frame * C;
            Y = Y + frame * S;
        }
        count++;
        timeCount = timeCount + 1 / fps;
    }

    double M = 1 + std::cyl_bessel_jl(0, 2 * a);
    double N = 1 - std::cyl_bessel_jl(0, 2 * a);
    //计算相位
    cv::Mat dst1 = X / M;
    cv::Mat dst2 = Y / N;

    cv::Mat out = arctan2(dst1, dst2);
    //    cv::phase(dst1, dst2, out, false);    // true输出角度 false输出弧度
    return out;
}

cv::Mat GLIA(double a, double f, const char* videoName)
{
    cv::VideoCapture cap;
    cap.open(videoName);
    cv::Mat frame;

    if (!cap.isOpened()) {
        printf("Open Video Failed!\n");
        return frame;
    }

    cv::Mat X;
    cv::Mat Y;
    cv::Mat tmp;
    int i = 0;
    double C, S;
    // 离散函数积分
    while (true) {
        cap >> frame;
        double t = cap.get(cv::CAP_PROP_POS_MSEC) / 1000;
        double modulation = a * sin(2 * M_PI * f * t);
        if (frame.empty()) {
            cap.release();
            break;
        }

        cv::cvtColor(frame, frame, CV_RGB2GRAY);
        frame.convertTo(frame, CV_64F);    // 转换图像格式，防止数据丢失

        C = cos(modulation);
        S = sin(modulation);

        if (X.empty() || Y.empty()) {
            cv::Mat X_mul = frame * C;
            X = X_mul.clone();
            cv::Mat Y_mul = frame * S;
            Y = Y_mul.clone();
        }
        else {
            X = X + frame * C;
            Y = Y + frame * S;
        }
        i++;
    }

    double M = 1 + std::cyl_bessel_jl(0, 2 * a);
    double N = 1 - std::cyl_bessel_jl(0, 2 * a);
    //计算相位
    cv::Mat dst1 = X / M;
    cv::Mat dst2 = Y / N;
    cv::Mat Phase = arctan2(dst1, dst2);;
    // cv::phase(X, Y, Phase,false);    // true输出角度 false输出弧度

    return Phase;
}

cv::Mat fGLIA(double a, std::vector<double> modulation, const char* videoName)
{
    // fGLIA 算法
    cv::VideoCapture cap;
    cap.open(videoName);
    cv::Mat frame;
    if (!cap.isOpened()) {
        printf("Open Video Failed!\n");
        return frame;
    }

    cv::Mat X;
    cv::Mat Y;
    cv::Mat I;
    int i = 0;
    double C, S;

    while (true) {
        cap >> frame;
        if (frame.empty()) {
            cap.release();
            break;
        }
        cv::cvtColor(frame, frame, CV_RGB2GRAY);
        frame.convertTo(frame, CV_64F);    // 转换图像格式，防止数据丢失
        if (I.empty()) {
            I = frame.clone();
            I.convertTo(I, CV_64FC1);
        }
        else {
            I = I + frame;
        }
        i++;
    }
    cv::Mat mean = I / i;

    // 离散函数积分
    i = 0;
    cap.open(videoName);
    if (!cap.isOpened()) {
        printf("Open Video Failed!\n");
        return frame;
    }
    while (true) {
        cap >> frame;
        if (frame.empty()) {
            cap.release();
            break;
        }

        cv::cvtColor(frame, frame, CV_RGB2GRAY);
        frame.convertTo(frame, CV_64F);    // 转换图像格式，防止数据丢失
        frame = frame - mean;

        C = cos(modulation.at(i));
        S = sin(modulation.at(i));

        if (X.empty() || Y.empty()) {
            cv::Mat X_mul = frame * C;
            X = X_mul.clone();
            cv::Mat Y_mul = frame * S;
            Y = Y_mul.clone();
        }
        else {
            X = X + frame * C;
            Y = Y + frame * S;
        }
        i++;
    }

    long double J2a = std::cyl_bessel_jl(0, 2 * a);
    long double J1a = std::cyl_bessel_jl(0, a);
    double M = 1 + J2a - 2 * J1a * J1a;
    double N = 1 - J2a;
    //计算相位
    cv::Mat dst1 = X / M;
    cv::Mat dst2 = Y / N;
    cv::Mat Phase = arctan2(dst1, dst2);;
    // cv::phase(X, Y, Phase,false);    // true输出角度 false输出弧度

    return Phase;
}

cv::Mat fGLIA(double a, double fps, double time, std::vector<double> modulation)
{
    double timeCount = 0;
    cv::Mat X;
    cv::Mat Y;
    cv::Mat I;
    int count = 0;
    double C, S;
    std::string name;
    // 计算光强均值
    while (timeCount < time) {
        name = std::to_string(timeCount) + ".png";
        cv::Mat frame = cv::imread(name, cv::IMREAD_ANYCOLOR);   // 取一帧图像

        // cv::cvtColor(mFrame, mFrame, CV_RGB2GRAY);
        frame.convertTo(frame, CV_64FC1);    // 转换图像格式，防止数据丢失
        if (I.empty()) {
            I = frame.clone();
            I.convertTo(I, CV_64FC1);
        }
        else {
            I = I + frame;
        }
        count++;
        timeCount = timeCount + 1 / fps;
    }
    // 扣除直流误差
    cv::Mat mean = I / count;

    // 离散函数积分
    count = 0;  timeCount = 0;
    while (timeCount < time) {
        name = std::to_string(timeCount) + ".png";
        cv::Mat frame = cv::imread(name, cv::IMREAD_ANYCOLOR);   // 取一帧图像

        // std::cout << mFrame.channels() << std::endl;
        // cv::cvtColor(mFrame, mFrame, CV_RGB2GRAY);
        frame.convertTo(frame, CV_64FC1);    // 转换图像格式，防止数据丢失
        frame = frame - mean;

        C = cos(modulation.at(count));
        S = sin(modulation.at(count));

        if (X.empty() || Y.empty()) {
            cv::Mat X_mul = frame * C;
            X = X_mul.clone();
            X.convertTo(X, CV_64FC1);
            cv::Mat Y_mul = frame * S;
            Y = Y_mul.clone();
            Y.convertTo(Y, CV_64FC1);
        }
        else {
            X = X + frame * C;
            Y = Y + frame * S;
        }
        count++;
        timeCount = timeCount + 1 / fps;
    }

    long double J2a = std::cyl_bessel_jl(0, 2 * a);
    long double J1a = std::cyl_bessel_jl(0, a);
    double M = 1 + J2a - 2 * J1a * J1a;
    double N = 1 - J2a;

    //计算相位
    cv::Mat dst1 = X / M;
    cv::Mat dst2 = Y / N;

    cv::Mat out = arctan2(dst1, dst2);
    //    cv::Mat out;
    //    cv::phase(dst1, dst2, out, false);    // true输出角度 false输出弧度
    //    std::cout << X.at<double>(151, 80) << std::endl;
    //    std::cout << Y.at<double>(151, 80) << std::endl;
    //    std::cout << dst1.at<double>(151, 80) << std::endl;
    //    std::cout << dst2.at<double>(151, 80) << std::endl;
    //    std::cout << out.at<double>(151, 80) << std::endl;
    return out;
}

cv::Mat fGLIA(double a, double f, const char* videoName)
{
    cv::VideoCapture cap;
    cap.open(videoName);
    cv::Mat frame;
    if (!cap.isOpened()) {
        printf("Open Video Failed!\n");
        return frame;
    }

    cv::Mat X;
    cv::Mat Y;
    cv::Mat I;
    int count = 0;
    double C, S;

    while (true) {
        cap >> frame;
        if (frame.empty()) {
            cap.release();
            break;
        }
        cv::cvtColor(frame, frame, CV_RGB2GRAY);
        frame.convertTo(frame, CV_64F);    // 转换图像格式，防止数据丢失
        if (I.empty()) {
            I = frame.clone();
            I.convertTo(I, CV_64FC1);
        }
        else {
            I = I + frame;
        }
        count++;
    }
    cv::Mat mean = I / count;

    // 离散函数积分
    count = 0;
    cap.open(videoName);
    if (!cap.isOpened()) {
        printf("Open Video Failed!\n");
        return frame;
    }
    while (true) {
        cap >> frame;
        if (frame.empty()) {
            cap.release();
            break;
        }
        double t = cap.get(cv::CAP_PROP_POS_MSEC) / 1000;
        double modulation = a * sin(2 * M_PI * f * t);

        cv::cvtColor(frame, frame, CV_RGB2GRAY);
        frame.convertTo(frame, CV_64F);    // 转换图像格式，防止数据丢失
        frame = frame - mean;

        C = cos(modulation);
        S = sin(modulation);

        if (X.empty() || Y.empty()) {
            cv::Mat X_mul = frame * C;
            X = X_mul.clone();
            cv::Mat Y_mul = frame * S;
            Y = Y_mul.clone();
        }
        else {
            X = X + frame * C;
            Y = Y + frame * S;
        }
        count++;
    }

    long double J2a = std::cyl_bessel_jl(0, 2 * a);
    long double J1a = std::cyl_bessel_jl(0, a);
    double M = 1 + J2a - 2 * J1a * J1a;
    double N = 1 - J2a;
    //计算相位
    cv::Mat dst1 = X / M;
    cv::Mat dst2 = Y / N;
    cv::Mat Phase = arctan2(dst1, dst2);;
    // cv::phase(X, Y, Phase,false);    // true输出角度 false输出弧度

    return Phase;
}

cv::Mat fGLIA(double a, double f, double fps, const std::string& filePath)
{
    // cv::Mat frame;
    fs::path path = filePath;
    std::set<fs::path> sorted_by_name;
    // 遍历文件夹下所有图像并排序
    for (auto& entry : fs::directory_iterator(path))
        sorted_by_name.insert(entry.path());
    cv::Mat X;
    cv::Mat Y;
    cv::Mat I;
    int count = 0;
    double C, S;

    for (auto& fileName : sorted_by_name) {
        std::cout << fileName << std::endl;
        cv::Mat frame = cv::imread(fileName.string(), cv::IMREAD_ANYCOLOR);

        if (frame.empty()) {
            std::cout << "Error! " << "Img" << fileName << "is empty!" << std::endl;
            return frame;
        }

        // cv::cvtColor(frame, frame, CV_RGB2GRAY);
        frame.convertTo(frame, CV_64FC1);    // 转换图像格式，防止数据丢失
        if (I.empty()) {
            I = frame.clone();
            I.convertTo(I, CV_64FC1);
        }
        else {
            I = I + frame;
        }
        count++;
    }
    // 扣除直流误差
    cv::Mat mean = I / count;

    // 离散函数积分
    count = 0; double timeCount = 0;
    for (auto& fileName : sorted_by_name) {
        cv::Mat frame = cv::imread(fileName.string(), cv::IMREAD_ANYCOLOR);
        // cv::cvtColor(frame, frame, CV_RGB2GRAY);

        if (frame.empty()) {
            std::cout << "Error! " << "Img" << fileName << "is empty!" << std::endl;
            return frame;
        }

        frame.convertTo(frame, CV_64FC1);    // 转换图像格式，防止数据丢失
        frame = frame - mean;

        double modulation = a * sin(2 * M_PI * f * timeCount);
        C = cos(modulation);
        S = sin(modulation);

        if (X.empty() || Y.empty()) {
            cv::Mat X_mul = frame * C;
            X = X_mul.clone();
            X.convertTo(X, CV_64FC1);
            cv::Mat Y_mul = frame * S;
            Y = Y_mul.clone();
            Y.convertTo(Y, CV_64FC1);
        }
        else {
            X = X + frame * C;
            Y = Y + frame * S;
        }
        timeCount = timeCount + 1 / fps;
        count++;
    }

    long double J2a = std::cyl_bessel_jl(0, 2 * a);
    long double J1a = std::cyl_bessel_jl(0, a);
    double M = 1 + J2a - 2 * J1a * J1a;
    double N = 1 - J2a;
    //计算相位
    cv::Mat dst1 = X / M;
    cv::Mat dst2 = Y / N;

    cv::Mat out = arctan2(dst1, dst2);
    // cv::phase(X, Y, Phase,false);    // true输出角度 false输出弧度
    return out;
}

cv::Mat fGLIA_step(double a, std::vector<double> modulation, const std::string& filePath)
{
    fs::path path = filePath;
    std::set<fs::path> sorted_by_name;
    // 遍历文件夹下所有图像并排序
    for (auto& entry : fs::directory_iterator(path))
        sorted_by_name.insert(entry.path());

    cv::Mat X;
    cv::Mat Y;
    cv::Mat I;
    int count = 0;
    double C, S;

    for (auto& fileName : sorted_by_name) {
        std::cout << fileName << std::endl;
        cv::Mat frame = cv::imread(fileName.string(), cv::IMREAD_ANYCOLOR);

        if (frame.empty()) {
            std::cout << "Error! " << "Img" << fileName << "is empty!" << std::endl;
            return frame;
        }

        // cv::cvtColor(frame, frame, CV_RGB2GRAY);
        frame.convertTo(frame, CV_64FC1);    // 转换图像格式，防止数据丢失
        if (I.empty()) {
            I = frame.clone();
            I.convertTo(I, CV_64FC1);
        }
        else {
            I = I + frame;
        }
        count++;
    }
    cv::Mat mean = I / count;

    // 离散函数积分
    count = 0;
    for (auto& fileName : sorted_by_name) {
        cv::Mat frame = cv::imread(fileName.string(), cv::IMREAD_ANYCOLOR);

        if (frame.empty()) {
            std::cout << "Error! " << "Img" << fileName << "is empty!" << std::endl;
            return frame;
        }

        // cv::cvtColor(frame, frame, CV_RGB2GRAY);
        frame.convertTo(frame, CV_64FC1);    // 转换图像格式，防止数据丢失
        frame = frame - mean;

        C = cos(modulation.at(count));
        S = sin(modulation.at(count));

        if (X.empty() || Y.empty()) {
            cv::Mat X_mul = frame * C;
            X = X_mul.clone();
            X.convertTo(X, CV_64FC1);
            cv::Mat Y_mul = frame * S;
            Y = Y_mul.clone();
            X.convertTo(X, CV_64FC1);
        }
        else {
            X = X + frame * C;
            Y = Y + frame * S;
        }
        count++;
    }

    long double J2a = std::cyl_bessel_jl(0, 2 * a);
    long double J1a = std::cyl_bessel_jl(0, a);
    double M = 1 + J2a - 2 * J1a * J1a;
    double N = 1 - J2a;
    //计算相位
    cv::Mat dst1 = X / M;
    cv::Mat dst2 = Y / N;
    cv::Mat Phase = arctan2(dst1, dst2);;
    // cv::phase(X, Y, Phase,false);    // true输出角度 false输出弧度

    return Phase;
}

cv::Mat arctan2(cv::Mat X, cv::Mat Y)
{
    cv::Mat* dst = nullptr;

    int i, j;
    int rows = X.rows;
    int cols = X.cols;
    if (rows != Y.rows || cols != Y.cols) {
        printf("Error Input Mat!\n");
        return X;
    }

    dst = new cv::Mat(rows, cols, CV_64FC1);
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            double x = X.at<double>(i, j);
            double y = Y.at<double>(i, j);
            dst->at<double>(i, j) = std::atan2(y, x);
        }
    }

    return *dst;
}

// End of File