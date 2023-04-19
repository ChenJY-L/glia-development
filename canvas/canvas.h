
#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <QWidget>
#include <QPushButton>
#include <QLayout>
#include <QTimer>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>

#include <cstdlib>

#include <TCanvas.h>
//#include <TVirtualX.h>
#include <TSystem.h>
#include <TFormula.h>
#include <TF1.h>
#include <TH1.h>
#include <TFrame.h>
#include <TTimer.h>
#include <TGraph.h>
#include <TGraph2D.h>
#include <TStyle.h>
#include <TRint.h>
#include <TStyle.h>


class QPaintEvent;
class QResizeEvent;
class QMouseEvent;
class QPushButton;
class QTimer;
class TCanvas;

class QRootCanvas : public QWidget
{
    Q_OBJECT

public:
    QRootCanvas(QWidget* parent = nullptr);
    virtual ~QRootCanvas() {}
    TCanvas* getCanvas() { return fCanvas; }

protected:
    TCanvas* fCanvas;

    void    mouseMoveEvent(QMouseEvent* e) override;
    void    mousePressEvent(QMouseEvent* e) override;
    void    mouseReleaseEvent(QMouseEvent* e) override;
    void    paintEvent(QPaintEvent* e) override;
    void    resizeEvent(QResizeEvent* e) override;
};

class QMainCanvas : public QWidget
{
    Q_OBJECT

public:
    QMainCanvas(QWidget* parent = nullptr);
    virtual ~QMainCanvas() {}
    virtual void changeEvent(QEvent* e);

    void setTitle(const std::string& name) { canvas->getCanvas()->SetTitle(name.c_str()); }

    void divide(int nx, int ny);

    void setSize(int wide, int height);

    void plot(const std::vector<double>& Dy, int LineWidth = 1, int position = 1);

    void plot(std::vector<double> Dx, std::vector<double> Dy,
        int LineWidth = 1, int position = 1);

    void plot(std::vector<double> Dx, std::vector<double> Dy,
        char* xAxis, char* yAxis, int LineWidth = 1, int position = 1);

    void surf(std::vector<double> Dx, std::vector<double> Dy,
        std::vector<double> Dz, Option_t* option = "surf1", int position = 1);
public slots:
    void clicked1();
    void clickedClear();
    void handle_root_events();

protected:
    QRootCanvas* canvas;
    QPushButton* bTest;
    QPushButton* bClear;
    QTimer* fRootTimer;
    // QTimer         *fFrameTimer{};
};

#endif


