#include "Photomech.h"
#include <fstream>
#include<opencv2/opencv.hpp>
#include<opencv2/calib3d/calib3d.hpp>
#include <opencv2\imgproc\types_c.h>
#include <QtWidgets/QApplication>
using namespace cv;
using namespace std;

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
    Photomech w;
    w.show();
    return a.exec();
}

