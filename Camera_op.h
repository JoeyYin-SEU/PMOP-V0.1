#pragma once

#include <QWidget>
#include "ui_Camera_op.h"
#include "MvCamera.h"
#include <windows.h>
#include <stdio.h>
#include <qmessagebox.h>
#include <process.h>
#include <opencv2/opencv.hpp>
#include <QDir>
#include <QDateTime>
#include <qthread.h>
#include <vector>

using namespace std;
using namespace cv;

#define WM_TIMER_GRAB_INFO      1
#define MAX_DEVICE_NUM          4

#ifndef VOID
#define VOID void
typedef char CHAR;
typedef short SHORT;
typedef long LONG;
#if !defined(MIDL_PASS)
typedef int INT;
#endif
#endif
#define FAR                 far
#define NEAR                near
#ifndef CONST
#define CONST               const
#endif

typedef CHAR* PCHAR, * LPCH, * PCH;
typedef CONST CHAR* LPCCH, * PCCH;

typedef _Null_terminated_ CHAR* NPSTR, * LPSTR, * PSTR;
typedef _Null_terminated_ PSTR* PZPSTR;
typedef _Null_terminated_ CONST PSTR* PCZPSTR;
typedef _Null_terminated_ CONST CHAR* LPCSTR, * PCSTR;
typedef _Null_terminated_ PCSTR* PZPCSTR;
typedef _Null_terminated_ CONST PCSTR* PCZPCSTR;

typedef _NullNull_terminated_ CHAR* PZZSTR;
typedef _NullNull_terminated_ CONST CHAR* PCZZSTR;

typedef  CHAR* PNZCH;
typedef  CONST CHAR* PCNZCH;

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;
typedef FLOAT* PFLOAT;

typedef int                 INT;
typedef unsigned int        UINT;
typedef unsigned int* PUINT;
#define FALSE   0
#define TRUE    1
#define NULL    0

#define IDM_ABOUTBOX                    0x0010
#define IDD_ABOUTBOX                    100
#define IDS_ABOUTBOX                    101
#define IDD_MULTIPLECAMERA_DIALOG       102
#define IDR_MAINFRAME                   128
#define IDC_CAMERA_INIT_STATIC          1000
#define IDC_OPEN_DEVICES_BUTTON         1001
#define IDC_CLOSE_DEVICES_BUTTON        1002
#define IDC_GRAB_CONTROL_STATIC         1003
#define IDC_START_GRABBING_BUTTON       1004
#define IDC_STOP_GRABBING_BUTTON        1005
#define IDC_SOFTWARE_MODE_BUTTON        1006
#define IDC_SOFTWARE_ONCE_BUTTON        1007
#define IDC_HARDWARE_MODE_BUTTON        1008
#define IDC_CONTINUS_MODE_RADIO         1009
#define IDC_TRIGGER_MODE_RADIO          1010
#define IDC_ENUM_DEVICES_BUTTON         1011
#define IDC_IMAGE_SHOW_STATIC           1012
#define IDC_SAVE_IMAGE_BUTTON           1013
#define IDC_FRAME_COUNT1_STATIC         1031
#define IDC_FRAME_COUNT2_STATIC         1032
#define IDC_FRAME_COUNT3_STATIC         1033
#define IDC_FRAME_COUNT4_STATIC         1034
#define IDC_FRAME_COUNT5_STATIC         1035
#define IDC_FRAME_COUNT6_STATIC         1036
#define IDC_FRAME_COUNT7_STATIC         1037
#define IDC_FRAME_COUNT8_STATIC         1038
#define IDC_FRAME_COUNT9_STATIC         1039
#define IDC_OUTPUT_INFO_LIST            1049
#define IDC_DISPLAY1_STATIC             1051
#define IDC_DISPLAY2_STATIC             1052
#define IDC_DISPLAY3_STATIC             1053
#define IDC_DISPLAY4_STATIC             1054
#define IDC_DISPLAY5_STATIC             1055
#define IDC_DISPLAY6_STATIC             1056
#define IDC_DISPLAY7_STATIC             1057
#define IDC_DISPLAY8_STATIC             1058
#define IDC_DISPLAY9_STATIC             1059
#define IDC_CAM1_CHECK                  1071
#define IDC_CAM2_CHECK                  1072
#define IDC_CAM3_CHECK                  1073
#define IDC_CAM4_CHECK                  1074
#define IDC_CAM5_CHECK                  1075
#define IDC_CAM6_CHECK                  1076
#define IDC_CAM7_CHECK                  1077
#define IDC_CAM8_CHECK                  1078
#define IDC_CAM9_CHECK                  1079

class Camera_op : public QWidget
{
	Q_OBJECT

public:
	Camera_op(QWidget *parent = Q_NULLPTR);
	~Camera_op();
	int modu;
	static void __stdcall ImageCallBack(unsigned char* pData, MV_FRAME_OUT_INFO* pFrameInfo, void* pUser);
	void open_camera_single(int flag_camera);
	int m_nCurCameraIndex;
	int ThreadFun(int nCurCameraIndex);
	BOOL if_continue_save;
	BOOL if_discret_save;
	MV_CC_DEVICE_INFO_LIST m_stDevList;
	bool Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char* pData, unsigned int num, int nCurCameraIndex);
	int RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight);
	QString dir_save;
	QString dir_save_every[MAX_DEVICE_NUM];


private:
	Ui::Camera_op ui;
	void sele_start();
	QFileInfoList List_disk;
	QFileInfoList List_disk_discret[MAX_DEVICE_NUM];
	QString current_date;
	unsigned int    m_nValidCamNum;
	BOOL            m_bOpenDevice[MAX_DEVICE_NUM];
	BOOL            m_bStartGrabbing[MAX_DEVICE_NUM];
	MV_FRAME_OUT_INFO_EX   m_stImageInfo[MAX_DEVICE_NUM];
	BOOL            m_bCamCheck[MAX_DEVICE_NUM];
	CMvCamera*		m_pcMyCamera[MAX_DEVICE_NUM];
	CRITICAL_SECTION        m_hSaveImageMux[MAX_DEVICE_NUM];
	unsigned char* m_pSaveImageBuf[MAX_DEVICE_NUM];
	unsigned int            m_nSaveImageBufSize[MAX_DEVICE_NUM];
	HWND            m_hwndDisplay[MAX_DEVICE_NUM];
	BOOL thread_exflag;
	unsigned int save_file_num[MAX_DEVICE_NUM];
	void* m_hGrabThread[MAX_DEVICE_NUM];
	void* m_hSaveThread[MAX_DEVICE_NUM];
	void* m_handle = NULL;
	void* m_handle_1 = NULL;
	void* m_handle_2 = NULL;
	void* m_handle_3 = NULL;
	void* m_handle_4 = NULL;
	void get_hik_frame();
	void save_image_single(int nCurCameraIndex);
	void open_allcamera();
	bool if_exsave(int nCurCameraIndex);

private slots:
	void open_camera();
	void save_image_single_click();
	void open_camera_all();
	void begin_con_save();
	void stop_con_save();
	void refresh_camera();
	void reframe_camera();
	void reframe_camera_fir();
	void set_camera();
	void re_savepath();
};


class Thread_SaveImage : public QThread
{
	Q_OBJECT
public:
	Thread_SaveImage(QObject* parent = nullptr);
	MV_FRAME_OUT_INFO_EX* pstImageInfo;
	unsigned char* pData;
	unsigned int num;
	int nCurCameraIndex;
	QString dir_save_every_th[MAX_DEVICE_NUM];
	int RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight);
//signals:
//	void myThreadSignal(const int);
//	//×Ô¶¨Òå²Û
//public slots:
//	void myThreadSlot(const int);
protected:
	void run() override;
};
