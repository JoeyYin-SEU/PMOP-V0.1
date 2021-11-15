#include "Photomech.h"

Photomech::Photomech(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    connect(ui.actionLoad_image_s, SIGNAL(triggered()), this, SLOT(OpenImg()));
    connect(ui.actionControl, SIGNAL(triggered()), this, SLOT(OpenCamera()));
}

void Photomech::OpenImg()
{
    QString OpenFile;
    QImage image;
    OpenFile = QFileDialog::getOpenFileName(this,
        "Please choose an image file",
        Work_Path,
        "Image Files(*.jpg *.png *.bmp *.pgm *.pbm *.fdi);;All(*.*)");
    if (OpenFile.size() == 0)
    {
        return;
    }
    Image_open = imread(OpenFile.toStdString());
    if(Photomech::Show_single(Image_open, true))
    {
        ui.Im_info->setText(Get_Singlename(OpenFile));
        Work_Path = Get_Workpath(OpenFile);
    }
}

void Photomech::OpenCamera()
{
    Cam_S = new DiaCamSel;
    Cam_S->exec();
    if (Cam_S->modu != -1)
    {
        if (Cam_S->modu == 0)
        {
            Cam_Window = new Camera_op;
            Cam_Window->show();
        }
        else if (Cam_S->modu == 1)
        {
            Cam_Window2 = new Camera_op_ids;
            Cam_Window2->show();
        }
    }
}
bool Photomech::Show_single(Mat img, bool if_fir)
{
    if (img.empty())
    {
        QMessageBox::information(this, tr("Error"), "Failed to read picture", QMessageBox::Ok | QMessageBox::Cancel);
        return false;
    }
        
    if (if_fir)
    {
        int width = img.cols;
        int height = img.rows;
        int i, j;
        QImage* Image = new QImage(width, height, QImage::Format_RGB888);
        for (i = 0; i < height; i++)
        {
            unsigned char* ptr = img.ptr<unsigned char>(i);
            for (j = 0; j < width * 3; j += 3)
            {
                Image->bits()[(Image->bytesPerLine() * i) + (j + 2)] = ptr[j];
                Image->bits()[(Image->bytesPerLine() * i) + (j + 1)] = ptr[j + 1];
                Image->bits()[(Image->bytesPerLine() * i) + (j)] = ptr[j + 2];
            }
        }
        QRect deskRect = QGuiApplication::primaryScreen()->availableGeometry();
        
        deskRect.setWidth(deskRect.width() - (this->frameGeometry().width() - ui.Show_Im->width()));
        deskRect.setHeight(deskRect.height() - (this->frameGeometry().height() - ui.Show_Im->height()));
        QSize Show_size;
        if ((width < deskRect.width()) && (height < deskRect.height()))
        {
            Show_size.setWidth(width);
            Show_size.setHeight(height);
        }
        else
        {
            double scale_w = (double)width / (double)deskRect.width();
            double scale_h = (double)height / (double)deskRect.height();
            if (scale_w > scale_h)
            {
                double w_pixel = (double)deskRect.width() * 0.8+1;
                double h_pixel = (double)deskRect.width() * (double)height / (double)width * 0.8+1;
                Show_size.setWidth((int)w_pixel);
                Show_size.setHeight((int)h_pixel);
            }
            else
            {
                double w_pixel = (double)deskRect.height() * (double)width / (double)height * 0.8+1;
                double h_pixel = (double)deskRect.height() * 0.8+1;
                Show_size.setWidth((int)w_pixel);
                Show_size.setHeight((int)h_pixel);
            }
        }
        QGraphicsScene* scene = new QGraphicsScene;
        scene->addPixmap(QPixmap::fromImage(*Image).scaled(Show_size));
        ui.Show_Im->setScene(scene);
        ui.Show_Im->show();
        int a1 = this->menuBar()->height();
        int a2 = ui.menuFile->height();
        this->resize((Show_size.width() + (this->frameGeometry().width() - ui.Show_Im->width())), (Show_size.height() + ui.Im_info->height()+
            (this->frameGeometry().height() - ui.menuFile->height() - ui.Show_Im->height())));
        int dx = (QGuiApplication::primaryScreen()->availableGeometry().width() - (Show_size.width()+ (this->frameGeometry().width() - ui.Show_Im->width()))) / 2;
        int dy = (QGuiApplication::primaryScreen()->availableGeometry().height() - (Show_size.height()+ (this->frameGeometry().height() - ui.Show_Im->height()))) / 2;
        this->move(dx, dy);
        return true;
    }
    return true;
}

QString Photomech::Get_Workpath(QString File)
{
    int first = File.lastIndexOf("/");
    return File.left(first+1);
}
QString Photomech::Get_Singlename(QString File)
{
    int first = File.lastIndexOf("/");
    return File.right(File.length() - first - 1);
}
void Photomech::mouseMoveEvent(QMouseEvent* event)
{
    int a = 1;
}