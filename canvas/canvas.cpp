#include "canvas.h"
#include <TVirtualX.h>


//------------------------------------------------------------------------------

//______________________________________________________________________________
QRootCanvas::QRootCanvas(QWidget* parent) : QWidget(parent, 0), fCanvas(0)
{
    // QRootCanvas constructor.

    // set options needed to properly update the canvas when resizing the widget
    // and to properly handle context menus and mouse move events
    setAttribute(Qt::WA_PaintOnScreen, false);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NativeWindow, true);
    setUpdatesEnabled(kFALSE);
    setMouseTracking(kTRUE);
    setMinimumSize(500, 400);

    // register the QWidget in TVirtualX, giving its native window id
    int wid = gVirtualX->AddWindow((ULong_t)winId(), width(), height());
    // create the ROOT TCanvas, giving as argument the QWidget registered id
    fCanvas = new TCanvas("Root hCanvas", width(), height(), wid);
    TQObject::Connect("TGPopupMenu", "PoppedDown()", "TCanvas", fCanvas, "Update()");
}

//______________________________________________________________________________
void QRootCanvas::mouseMoveEvent(QMouseEvent* e)
{
    // Handle mouse move events.

    if (fCanvas) {
        if (e->buttons() & Qt::LeftButton) {
            fCanvas->HandleInput(kButton1Motion, e->x(), e->y());
        }
        else if (e->buttons() & Qt::MidButton) {
            fCanvas->HandleInput(kButton2Motion, e->x(), e->y());
        }
        else if (e->buttons() & Qt::RightButton) {
            fCanvas->HandleInput(kButton3Motion, e->x(), e->y());
        }
        else {
            fCanvas->HandleInput(kMouseMotion, e->x(), e->y());
        }
    }
}

//______________________________________________________________________________
void QRootCanvas::mousePressEvent(QMouseEvent* e)
{
    // Handle mouse button press events.

    if (fCanvas) {
        switch (e->button()) {
        case Qt::LeftButton:
            fCanvas->HandleInput(kButton1Down, e->x(), e->y());
            break;
        case Qt::MidButton:
            fCanvas->HandleInput(kButton2Down, e->x(), e->y());
            break;
        case Qt::RightButton:
            // does not work properly on Linux...
            // ...adding setAttribute(Qt::WA_PaintOnScreen, true)
            // seems to cure the problem
            fCanvas->HandleInput(kButton3Down, e->x(), e->y());
            break;
        default:
            break;
        }
    }
}

//______________________________________________________________________________

void QRootCanvas::mouseReleaseEvent(QMouseEvent* e)
{
    // Handle mouse button release events.

    if (fCanvas) {
        switch (e->button()) {
        case Qt::LeftButton:
            fCanvas->HandleInput(kButton1Up, e->x(), e->y());
            break;
        case Qt::MidButton:
            fCanvas->HandleInput(kButton2Up, e->x(), e->y());
            break;
        case Qt::RightButton:
            // does not work properly on Linux...
            // ...adding setAttribute(Qt::WA_PaintOnScreen, true)
            // seems to cure the problem
            fCanvas->HandleInput(kButton3Up, e->x(), e->y());
            break;
        default:
            break;
        }
    }
}

//______________________________________________________________________________
void QRootCanvas::resizeEvent(QResizeEvent* event)
{
    // Handle resize events.

    if (fCanvas) {
        fCanvas->SetCanvasSize(event->size().width(), event->size().height());
        fCanvas->Resize();
        fCanvas->Update();
    }
}

//______________________________________________________________________________
void QRootCanvas::paintEvent(QPaintEvent*)
{
    // Handle paint events.

    if (fCanvas) {
        fCanvas->Resize();
        fCanvas->Update();
    }
}

//------------------------------------------------------------------------------

//______________________________________________________________________________
QMainCanvas::QMainCanvas(QWidget* parent) : QWidget(parent)
{
    // QMainCanvas constructor.

    auto* l = new QVBoxLayout(this);

    l->addWidget(canvas = new QRootCanvas(this));
    //l->addWidget(bTest = new QPushButton("&Draw Frame", this));
    l->addWidget(bClear = new QPushButton("Clear Canvas", this));

    //connect(bTest, SIGNAL(clicked()), this, SLOT(clicked1()));
    connect(bClear, SIGNAL(clicked()), this, SLOT(clickedClear()));

    fRootTimer = new QTimer(this);
    QObject::connect(fRootTimer, SIGNAL(timeout()), this, SLOT(handle_root_events()));
    fRootTimer->start(20);

    //    fFrameTimer = new QTimer( this );
    //    QObject::connect( fFrameTimer, SIGNAL(timeout()), this, SLOT(handle_frame_events()) );
}

/******************* slots *******************/
//______________________________________________________________________________
void QMainCanvas::clicked1()
{
    // Handle the "Draw Histogram" button clicked() event.

    static TH1F* h1f = nullptr;

    // Create a one dimensional histogram (one float per bin)
    // and fill it following the distribution in nFrequency
    canvas->getCanvas()->Clear();
    canvas->getCanvas()->cd();
    canvas->getCanvas()->SetBorderMode(0);
    canvas->getCanvas()->SetFillColor(0);
    canvas->getCanvas()->SetGrid();
    if (h1f == nullptr) {
        h1f = new TH1F("h1f", "Test random numbers", 200, 0, 10);
        new TFormula("form1", "abs(sin(x)/x)");
        TF1* sqroot = new TF1("sqroot", "x*gaus(0) + [3]*form1", 0, 10);
        sqroot->SetParameters(10, 4, 1, 20);
    }
    h1f->Reset();
    h1f->SetFillColor(kViolet + 2);
    h1f->SetFillStyle(3001);
    h1f->FillRandom("root", 10000);
    h1f->Draw();
    canvas->getCanvas()->Modified();
    canvas->getCanvas()->Update();
}

//------------------------------------------------------------------------------
void QMainCanvas::clickedClear()
{
    canvas->getCanvas()->Clear();
    canvas->getCanvas()->Modified();
    canvas->getCanvas()->Update();
}

//______________________________________________________________________________
void QMainCanvas::handle_root_events()
{
    //call the inner loop of ROOT
    gSystem->ProcessEvents();
}

//______________________________________________________________________________
void QMainCanvas::changeEvent(QEvent* e)
{
    if (e->type() == QEvent::WindowStateChange) {
        QWindowStateChangeEvent* event = static_cast<QWindowStateChangeEvent*>(e);
        if ((event->oldState() & Qt::WindowMaximized) ||
            (event->oldState() & Qt::WindowMinimized) ||
            (event->oldState() == Qt::WindowNoState &&
                this->windowState() == Qt::WindowMaximized)) {
            if (canvas->getCanvas()) {
                canvas->getCanvas()->Resize();
                canvas->getCanvas()->Update();
            }
        }
    }
}

//______________________________________________________________________________
void QMainCanvas::setSize(int wide, int height)
{
    canvas->setMinimumSize(wide, height);
}

//______________________________________________________________________________
void QMainCanvas::divide(int nx, int ny)
{
    canvas->getCanvas()->Divide(nx, ny);
}

//______________________________________________________________________________
void QMainCanvas::plot(const std::vector<double>& Dy, int LineWidth, int position)
{
    // »æÖÆ1DÍ¼Ïñ
    int n = int(Dy.size());
    std::vector<double> Dx;

    for (int i = 0; i < n; i++) {
        Dx.push_back(i);
    }

    plot(Dx, Dy, LineWidth, position);

}

//______________________________________________________________________________
void QMainCanvas::plot(std::vector<double> Dx, std::vector<double> Dy,
    int LineWidth, int position)
{
    // static TGraph *Graph = nullptr;
    int n = int(Dx.size());

    if (n == Dy.size()) {
        canvas->getCanvas()->Clear();
        canvas->getCanvas()->SetBorderMode(0);
        canvas->getCanvas()->SetFillColor(0);
        canvas->getCanvas()->SetGrid();
        canvas->getCanvas()->cd(position);
        TGraph* Graph = nullptr;
        Graph = new TGraph(n, Dx.data(), Dy.data());
        Graph->SetTitle("Modulation Voltage;t;V;");
        Graph->SetLineWidth(LineWidth);
        Graph->SetFillColor(kViolet + 2);
        Graph->SetFillStyle(3001);
        Graph->Draw("AL");
        /*        if(Graph == nullptr) {

                }*/
        canvas->getCanvas()->Modified();
        canvas->getCanvas()->Update();
    }
    else {
        printf("Input Data Error\n");
    }
    //Graph = nullptr;
}

//______________________________________________________________________________
void QMainCanvas::plot(std::vector<double> Dx, std::vector<double> Dy,
    char* xAxis, char* yAxis, int LineWidth, int position)
{
    static TGraph* Graph = nullptr;
    int n = int(Dx.size());

    if (n == Dy.size()) {
        if (Graph == nullptr) {


            // Create a one dimensional histogram (one float per bin)
            // and fill it following the distribution in function sqroot.
            canvas->getCanvas()->cd(position);
            canvas->getCanvas()->Clear();
            canvas->getCanvas()->SetBorderMode(0);
            canvas->getCanvas()->SetFillColor(0);
            canvas->getCanvas()->SetGrid();

            Graph = new TGraph(n, Dx.data(), Dy.data());
            Graph->GetXaxis()->SetTitle(xAxis);
            Graph->GetYaxis()->SetTitle(yAxis);
            Graph->SetLineWidth(LineWidth);
            Graph->SetFillColor(kViolet + 2);
            Graph->SetFillStyle(3001);
            Graph->Draw();

            canvas->getCanvas()->Update();
            canvas->getCanvas()->Modified();
        }
    }
    else {
        printf("Input Data Error\n");
    }
}

//______________________________________________________________________________
void QMainCanvas::surf(std::vector<double> Dx, std::vector<double> Dy,
    std::vector<double> Dz, Option_t* option, int position)
{
    static TGraph2D* Graph2D = nullptr;
    int n = int(Dx.size());
    // std::cout << n << Dy.size() << Dz.size();
    if (n == Dy.size() && n == Dz.size()) {
        canvas->getCanvas()->cd(position);
        canvas->getCanvas()->Clear();
        canvas->getCanvas()->SetBorderMode(0);
        canvas->getCanvas()->SetFillColor(0);
        canvas->getCanvas()->SetGrid();

        if (Graph2D == nullptr) {
            Graph2D = new TGraph2D(n, Dx.data(), Dy.data(), Dz.data());
            Graph2D->SetTitle("Phase;x;y;phase/rad");
            gStyle->SetPalette(1);
            Graph2D->Draw(option);
        }
        else {
            delete Graph2D;
            Graph2D = new TGraph2D(n, Dx.data(), Dy.data(), Dz.data());
            Graph2D->SetTitle("Phase;x;y;phase/rad");
            gStyle->SetPalette(1);
            Graph2D->Draw(option);
        }

        canvas->getCanvas()->Update();
        canvas->getCanvas()->Show();
    }
    else {
        printf("Input Data Error\n");
    }
}

//______________________________________________________________________________

//End of file