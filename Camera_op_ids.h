#pragma once
#include "acquisitionworker.h"
#include <QApplication>
#include <QWidget>
#include "ui_Camera_op_ids.h"
#include <windows.h>
#include <stdio.h>
#include <qmessagebox.h>
#include <process.h>
#include <opencv2/opencv.hpp>
#include <QDir>
#include <QDateTime>
#include <qthread.h>
#include <vector>
#include <peak_ipl/peak_ipl.hpp>
#include <peak/peak.hpp>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QWidget>
#include <cstdint>
#include <qtextstream.h>
#define VERSION "1.0.0"
#define MAX_NUMBER_OF_DEVICES 4
#define MAXIMUM_THROUGHPUT_LIMIT 125000000

struct DeviceContext
{
	std::shared_ptr<peak::core::Device> device;
	std::shared_ptr<peak::core::DataStream> dataStream;
	std::shared_ptr<peak::core::NodeMap> nodemapRemoteDevice;
	peak::ipl::PixelFormatName pixelFormat;
	int imageWidth;
	int imageHeight;
	AcquisitionWorker* acquisitionWorker;
	QThread acquisitionThread;
};


class Save_Thread : public QThread
{
	Q_OBJECT
public:
	Save_Thread(QObject* parent = nullptr);
	~Save_Thread();
	QImage img;
	QString save_file;
	unsigned int num;
	BOOL mode;
protected:
	void run() override;
	QString get_num(unsigned int number);
};

class Camera_op_ids : public QWidget
{
	Q_OBJECT

public:
	Camera_op_ids(QWidget *parent = Q_NULLPTR);
	~Camera_op_ids();

private:
	Ui::Camera_op_ids ui;
	std::chrono::time_point<std::chrono::system_clock> m_begintime;
	std::vector<std::shared_ptr<DeviceContext>> m_vecDevices;
	std::shared_ptr<peak::core::Device> m_device = nullptr;
	std::shared_ptr<peak::core::NodeMap> m_nodeMap[MAX_NUMBER_OF_DEVICES];
	std::shared_ptr<peak::core::DataStream> m_dataStream = nullptr;
	std::vector<std::shared_ptr<peak::core::DeviceDescriptor>> m_devices;
	std::string m_openedCamera = "";
	unsigned int    m_nValidCamNum;
	bool open_camera_single(unsigned int num);
	bool close_camera_single(unsigned int num);
	bool OpenDevices();
	bool is_open[MAX_NUMBER_OF_DEVICES];
	QFileInfoList List_disk;
	QFileInfoList List_disk_discret[MAX_NUMBER_OF_DEVICES];
	QString current_date;
	QString dir_save;
	QString dir_save_every[MAX_NUMBER_OF_DEVICES];
	QString dir_save_txt[MAX_NUMBER_OF_DEVICES];
	BOOL if_single_save[MAX_NUMBER_OF_DEVICES];
	BOOL if_continue_save;
	BOOL if_discret_save;
	QString device_name[MAX_NUMBER_OF_DEVICES];
	unsigned int single_save_num[MAX_NUMBER_OF_DEVICES];
	unsigned int save_file_num[MAX_NUMBER_OF_DEVICES];
	unsigned int camera_width[MAX_NUMBER_OF_DEVICES];
	unsigned int camera_height[MAX_NUMBER_OF_DEVICES];
	unsigned int offsetx[MAX_NUMBER_OF_DEVICES];
	unsigned int offsety[MAX_NUMBER_OF_DEVICES];
	double camera_hz[MAX_NUMBER_OF_DEVICES];
	double camera_extime[MAX_NUMBER_OF_DEVICES];
	QGraphicsScene* scene_1;
	QGraphicsScene* scene_2;
	QGraphicsScene* scene_3;
	QGraphicsScene* scene_4;
	int singlesave_num;
	int save_flag[MAX_NUMBER_OF_DEVICES];
	BOOL save_time_info;

private slots:
	void refresh_camera();
	void open_camera();
	void open_camera_all();
	void UpdateDisplay_1(QImage image, double time_now, double conversionTime_ms);
	void UpdateDisplay_2(QImage image, double time_now, double conversionTime_ms);
	void UpdateDisplay_3(QImage image, double time_now, double conversionTime_ms);
	void UpdateDisplay_4(QImage image, double time_now, double conversionTime_ms);
	void UpdateCounters_1(double frameTime_ms, double conversionTime_ms, unsigned int frameCounter,
		unsigned int errorCounter, int incomplete, int dropped, int lost, bool showCustomNodes);
	void UpdateCounters_2(double frameTime_ms, double conversionTime_ms, unsigned int frameCounter,
		unsigned int errorCounter, int incomplete, int dropped, int lost, bool showCustomNodes);
	void UpdateCounters_3(double frameTime_ms, double conversionTime_ms, unsigned int frameCounter,
		unsigned int errorCounter, int incomplete, int dropped, int lost, bool showCustomNodes);
	void UpdateCounters_4(double frameTime_ms, double conversionTime_ms, unsigned int frameCounter,
		unsigned int errorCounter, int incomplete, int dropped, int lost, bool showCustomNodes);
	void re_savepath();
	void re_xy();
	void set_ex();
	void set_hz();
	void set_xyoff_w();
	void set_xyoff_h();
	void set_xyoff_x();
	void set_xyoff_y();
	void save_image_single_click();
	void begin_con_save();
	void stop_con_save();
};
