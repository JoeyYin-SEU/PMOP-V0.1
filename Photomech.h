#pragma once
#ifdef WIN32  
#pragma execution_character_set("utf-8")  
#endif

#include <QtWidgets/QMainWindow>
#include <QLabel>
#include <qfiledialog.h>
#include <QScreen>
#include <qapplication.h>
#include "ui_Photomech.h"
#include <vector>
#include <opencv2/opencv.hpp>
#include <qmessagebox.h>
#include <QTextCodec>
#include <QGraphicsScene>
#include "DiaCamSel.h"
#include "Camera_op.h"
#include "Camera_op_ids.h"
using namespace cv;

class Photomech : public QMainWindow
{
    Q_OBJECT

public:
    Photomech(QWidget *parent = Q_NULLPTR);
    void mouseMoveEvent(QMouseEvent* event);
    QString Get_Workpath(QString File);
    QString Get_Singlename(QString File);

private:
    Ui::PhotomechClass ui;
    bool Show_single(Mat img, bool if_fir = false);
    Mat Image_open;
    int Screen_wid;
    int Screen_hei;
    QString Work_Path="";
    DiaCamSel* Cam_S;
    Camera_op* Cam_Window;
    Camera_op_ids* Cam_Window2;

private slots:
    void OpenImg();
    void OpenCamera();
};
