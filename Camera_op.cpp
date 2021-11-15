#include "Camera_op.h"

Camera_op::Camera_op(QWidget *parent)
	: QWidget(parent)
{
    List_disk= QDir::drives();
    QString sc;
    QDir dir;
    QDateTime current_date_time = QDateTime::currentDateTime();
    current_date = current_date_time.toString("yyyy-MM-dd-hh-mm-ss");
    //QString current_date = current_date_time.toString('yyyy-MM-dd hh:mm:ss ddd');
	ui.setupUi(this);
    for (unsigned int ii = 0; ii < List_disk.count(); ii++)
    {
        sc = List_disk.at(ii).absolutePath();
        sc = sc[0] + ":\\";
        ui.Cam_List_3->addItem(sc);
        if (ii == 0)
        {
            dir_save = sc[0] + ":\\Photomech_Image";
        }
    }
    ui.Cam_List_3->addItem("Discrete Save");
    ui.Cam_List_4->hide();
    ui.Cam_List_text_4->hide();
    ui.Cam_List_5->hide();
    ui.Cam_List_text_5->hide();
    ui.Cam_List_6->hide();
    ui.Cam_List_text_6->hide();
    ui.Cam_List_7->hide();
    ui.Cam_List_text_7->hide();
    if (!dir.exists(dir_save))
    {
        if (!dir.mkpath(dir_save))
        {
            QString mes1 = "Working path failed to creat.";
            QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        }
    }

    dir_save = dir_save + "\\";
    dir_save = dir_save + current_date;
    if (!dir.exists(dir_save))
    {
        if (!dir.mkpath(dir_save))
        {
            QString mes1 = "Working path failed to creat.";
            QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        }
    }
    m_nCurCameraIndex = -1;
    if_discret_save = FALSE;
    if_continue_save = FALSE;
    for (int i = 0; i < MAX_DEVICE_NUM; i++)
    {
        m_bCamCheck[i] = FALSE;
        m_pcMyCamera[i] = NULL;
        m_hGrabThread[i] = NULL;
        m_hSaveThread[i] = NULL;
        m_pSaveImageBuf[i] = NULL;
        m_hwndDisplay[i] = NULL;
        m_nSaveImageBufSize[i] = 0;
        save_file_num[i] = 0;
        m_bStartGrabbing[i] = FALSE;
        m_bOpenDevice[i] = FALSE;
        memset(&(m_stImageInfo[i]), 0, sizeof(MV_FRAME_OUT_INFO_EX));
        InitializeCriticalSection(&m_hSaveImageMux[i]);
    }
    m_hwndDisplay[0] = (HWND)this->ui.Camera_1->winId();
    m_hwndDisplay[1] = (HWND)this->ui.Camera_2->winId();
    m_hwndDisplay[2] = (HWND)this->ui.Camera_3->winId();
    m_hwndDisplay[3] = (HWND)this->ui.Camera_4->winId();
    sele_start();
	connect(ui.Open_new, SIGNAL(clicked()), this, SLOT(open_camera()));
    connect(ui.Open_all, SIGNAL(clicked()), this, SLOT(open_camera_all()));
    connect(ui.Sin_Save, SIGNAL(clicked()), this, SLOT(save_image_single_click()));
    connect(ui.Con_Save, SIGNAL(clicked()), this, SLOT(begin_con_save()));
    connect(ui.Con_Save_Stop, SIGNAL(clicked()), this, SLOT(stop_con_save()));
    connect(ui.re_cam, SIGNAL(clicked()), this, SLOT(refresh_camera()));
    connect(ui.Set_button, SIGNAL(clicked()), this, SLOT(set_camera()));
    connect(ui.Re_button, SIGNAL(clicked()), this, SLOT(reframe_camera()));
    connect(ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(reframe_camera()));
    connect(ui.Cam_List_2, SIGNAL(currentIndexChanged(int)), this, SLOT(reframe_camera()));
    connect(ui.Cam_List_3, SIGNAL(currentIndexChanged(int)), this, SLOT(re_savepath()));
}

unsigned int    __stdcall   WorkThread(void* pUser)
{
    if (pUser)
    {
        Camera_op* pCam = (Camera_op*)pUser;
        if (NULL == pCam)
        {
            return -1;
        }
        int nCurCameraIndex = pCam->m_nCurCameraIndex;
        pCam->m_nCurCameraIndex = -1;
        pCam->ThreadFun(nCurCameraIndex);

        return 0;
    }

    return -1;
}

void Camera_op::re_savepath()
{
    QString sc;
    QDir dir;
    if (ui.Cam_List_3->currentIndex() == (ui.Cam_List_3->count() - 1))
    {
        if_discret_save = TRUE;
    }
    else
    {
        if_discret_save = FALSE;
    }
    if (!if_discret_save)
    {
        sc = List_disk.at(ui.Cam_List_3->currentIndex()).absolutePath();
        dir_save = sc[0] + ":\\Photomech_Image";
        if (!dir.exists(dir_save))
        {
            if (!dir.mkpath(dir_save))
            {
                QString mes1 = "Working path failed to creat.";
                QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            }
        }

        dir_save = dir_save + "\\";
        dir_save = dir_save + current_date;
        if (!dir.exists(dir_save))
        {
            if (!dir.mkpath(dir_save))
            {
                QString mes1 = "Working path failed to creat.";
                QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            }
        }
    }
}
void Camera_op::refresh_camera()
{
    sele_start();
}

void Camera_op::sele_start()
{
    QDir dir;
    ui.Cam_List->clear();
    ui.Cam_List_2->clear();
    //ui.Cam_List_2.
    memset(&m_stDevList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    int nRet = CMvCamera::EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &m_stDevList);
    if (MV_OK != nRet || m_stDevList.nDeviceNum == 0)
    {
        return;
    }
    m_nValidCamNum = 0;
    for (unsigned int i = 0; i < MAX_DEVICE_NUM; i++)
    {
        char chDeviceName[256] = { 0 };
        if (i < m_stDevList.nDeviceNum)
        {
            MV_CC_DEVICE_INFO* pDeviceInfo = m_stDevList.pDeviceInfo[i];
            if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
            {
                if (strcmp("", (LPCSTR)(pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName)) != 0)
                {
                    sprintf_s(chDeviceName, 256, "%s [%s]", pDeviceInfo->SpecialInfo.stGigEInfo.chModelName, pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName);
                }
                else
                {
                    sprintf_s(chDeviceName, 256, "%s (%s)", pDeviceInfo->SpecialInfo.stGigEInfo.chModelName, pDeviceInfo->SpecialInfo.stGigEInfo.chSerialNumber);
                }
            }
            else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE)
            {
                if (strcmp("", (LPCSTR)(pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName)) != 0)
                {
                    sprintf_s(chDeviceName, 256, "%s [%s]", pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName, pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
                }
                else
                {
                    sprintf_s(chDeviceName, 256, "%s (%s)", pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName, pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
                }
            }

            dir_save = dir_save + "\\";
            dir_save_every[i] = dir_save + QString(chDeviceName);
            if (!dir.exists(dir_save_every[i]))
            {
                if (!dir.mkpath(dir_save_every[i]))
                {
                    QString mes1 = "Working path failed to creat.";
                    QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                }
            }
            ui.Cam_List->addItem(chDeviceName);
            ui.Cam_List_2->addItem(chDeviceName);
            m_nValidCamNum++;
            m_bCamCheck[i] = TRUE;
        }
        else
        {
            sprintf_s(chDeviceName, 256, "Cam%d", i + 1);
            m_bCamCheck[i] = FALSE;
        }
        wchar_t strUserName[128] = { 0 };
    }
    ui.label->setText("Found " + QString::number(m_nValidCamNum) + " Camera(s).");
}
void Camera_op::open_camera_single(int flag_camera)
{

    int i = flag_camera;
    if (TRUE == m_bOpenDevice[i])
    {
        return;
    }
    BOOL bHaveCheck = FALSE;
    int nRet;
    if (m_bCamCheck[i])
    {
        bHaveCheck = TRUE;
        if (NULL == m_pcMyCamera[i])
        {
            m_pcMyCamera[i] = new CMvCamera;
        }
        nRet = m_pcMyCamera[i]->Open(m_stDevList.pDeviceInfo[i]);
        if (MV_OK != nRet)
        {

            QString mes1 = "Open Camera fail! DevIndex[";
            mes1 = mes1 + QString::number(i);
            mes1 = mes1 + "]";
            QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            delete(m_pcMyCamera[i]);
            m_pcMyCamera[i] = NULL;
        }
        else
        {
            m_bOpenDevice[i] = TRUE;
            // ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
            if (m_stDevList.pDeviceInfo[i]->nTLayerType == MV_GIGE_DEVICE)
            {
                unsigned int nPacketSize = 0;
                nRet = m_pcMyCamera[i]->GetOptimalPacketSize(&nPacketSize);
                if (nPacketSize > 0)
                {
                    nRet = m_pcMyCamera[i]->SetIntValue("GevSCPSPacketSize", nPacketSize);
                    if (nRet != MV_OK)
                    {
                        QString mes1 = "Set Packet Size fail! DevIndex[";
                        mes1 = mes1 + QString::number(i);
                        mes1 = mes1 + "]";
                        QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                    }
                }
                else
                {
                    QString mes1 = "Get Packet Size fail! DevIndex[";
                    mes1 = mes1 + QString::number(i);
                    mes1 = mes1 + "]";
                    QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

                }
            }
        }
    }

    if (FALSE == m_bOpenDevice[i] || TRUE == m_bStartGrabbing[i])
    {
        return;
    }

    nRet = MV_OK;

    if (m_pcMyCamera[i])
    {
        memset(&(m_stImageInfo[i]), 0, sizeof(MV_FRAME_OUT_INFO_EX));

        nRet = m_pcMyCamera[i]->StartGrabbing();
        if (MV_OK != nRet)
        {
            QString mes1 = "Start grabbing fail! DevIndex[";
            mes1 = mes1 + QString::number(i);
            mes1 = mes1 + "]";
            QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        }
        m_bStartGrabbing[i] = TRUE;
        unsigned int nCount = 0;
        while (-1 != m_nCurCameraIndex)
        {
            nCount++;
            if (nCount > 100)
            {
                return;
            }

            Sleep(0.01);
        }
        unsigned int nThreadID = 0;
        m_nCurCameraIndex = i;
        m_hGrabThread[i] = (void*)_beginthreadex(NULL, 0, WorkThread, this, 0, &nThreadID);
        if (NULL == m_hGrabThread[i])
        {
            QString mes1 = "Create grab thread fail! DevIndex[";
            mes1 = mes1 + QString::number(i);
            mes1 = mes1 + "]";
            QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        }
    }
}
void Camera_op::open_allcamera()
{

    int nRet;
    for (unsigned int i = 0; i < m_nValidCamNum; i++)
    {

        if (TRUE == m_bOpenDevice[i])
        {
            return;
        }
        BOOL bHaveCheck = FALSE;
        if (m_bCamCheck[i])
        {
            bHaveCheck = TRUE;
            if (NULL == m_pcMyCamera[i])
            {
                m_pcMyCamera[i] = new CMvCamera;
            }
            nRet = m_pcMyCamera[i]->Open(m_stDevList.pDeviceInfo[i]);
            if (MV_OK != nRet)
            {

                QString mes1 = "Open Camera fail! DevIndex[";
                mes1 = mes1 + QString::number(i);
                mes1 = mes1 + "]";
                QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                delete(m_pcMyCamera[i]);
                m_pcMyCamera[i] = NULL;
            }
            else
            {
                m_bOpenDevice[i] = TRUE;
                // ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
                if (m_stDevList.pDeviceInfo[i]->nTLayerType == MV_GIGE_DEVICE)
                {
                    unsigned int nPacketSize = 0;
                    nRet = m_pcMyCamera[i]->GetOptimalPacketSize(&nPacketSize);
                    if (nPacketSize > 0)
                    {
                        nRet = m_pcMyCamera[i]->SetIntValue("GevSCPSPacketSize", nPacketSize);
                        if (nRet != MV_OK)
                        {
                            QString mes1 = "Set Packet Size fail! DevIndex[";
                            mes1 = mes1 + QString::number(i);
                            mes1 = mes1 + "]";
                            QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                        }
                    }
                    else
                    {
                        QString mes1 = "Get Packet Size fail! DevIndex[";
                        mes1 = mes1 + QString::number(i);
                        mes1 = mes1 + "]";
                        QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

                    }
                }
            }
        }
    }

    for (unsigned int i = 0; i < m_nValidCamNum; i++)
    {
        if (FALSE == m_bOpenDevice[i] || TRUE == m_bStartGrabbing[i])
        {
            return;
        }

        nRet = MV_OK;

        if (m_pcMyCamera[i])
        {
            memset(&(m_stImageInfo[i]), 0, sizeof(MV_FRAME_OUT_INFO_EX));

            nRet = m_pcMyCamera[i]->StartGrabbing();
            if (MV_OK != nRet)
            {
                QString mes1 = "Start grabbing fail! DevIndex[";
                mes1 = mes1 + QString::number(i);
                mes1 = mes1 + "]";
                QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            }
            m_bStartGrabbing[i] = TRUE;
        }
    }

    for (unsigned int i = 0; i < m_nValidCamNum; i++)
    {
        if (m_pcMyCamera[i])
        {
            unsigned int nCount = 0;
            while (-1 != m_nCurCameraIndex)
            {
                nCount++;
                if (nCount > 100)
                {
                    return;
                }

                Sleep(2);
            }
            unsigned int nThreadID = 0;
            m_nCurCameraIndex = i;
            m_hGrabThread[i] = (void*)_beginthreadex(NULL, 0, WorkThread, this, 0, &nThreadID);
            if (NULL == m_hGrabThread[i])
            {
                QString mes1 = "Create grab thread fail! DevIndex[";
                mes1 = mes1 + QString::number(i);
                mes1 = mes1 + "]";
                QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            }
        }
    }
        
}
void Camera_op::open_camera()
{
    open_camera_single(ui.Cam_List->currentIndex());

}
void Camera_op::open_camera_all()
{
    open_allcamera();
}

void __stdcall Camera_op::ImageCallBack(unsigned char* pData, MV_FRAME_OUT_INFO* pFrameInfo, void* pUser)
{
	if (pFrameInfo)
	{
		// 输出时加上当前系统时间
		char   szInfo[128] = { 0 };
		SYSTEMTIME sys;
		GetLocalTime(&sys);
		sprintf_s(szInfo, 128, "[%d-%02d-%02d %02d:%02d:%02d:%04d] : GetOneFrame succeed, width[%d], height[%d]", sys.wYear, sys.wMonth,
			sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds, pFrameInfo->nWidth, pFrameInfo->nHeight);
		printf("%s\n", szInfo);
	}
}

void Camera_op::save_image_single_click()
{
    if (FALSE == m_bStartGrabbing)
    {
        return;
    }

    MV_SAVE_IMG_TO_FILE_PARAM stSaveFileParam;
    memset(&stSaveFileParam, 0, sizeof(MV_SAVE_IMG_TO_FILE_PARAM));

    for (int i = 0; i < MAX_DEVICE_NUM; i++)
    {
        if (m_pcMyCamera[i])
        {
            EnterCriticalSection(&m_hSaveImageMux[i]);
            if (m_pSaveImageBuf[i] == NULL || m_stImageInfo[i].enPixelType == 0)
            {
                QString mes1 = "Save Image fail! DevIndex[";
                mes1 = mes1 + QString::number(i);
                mes1 = mes1 + "]";
                QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                LeaveCriticalSection(&m_hSaveImageMux[i]);
                continue;
            }

            stSaveFileParam.enImageType = MV_Image_Bmp; // ch:需要保存的图像类型 | en:Image format to save
            stSaveFileParam.enPixelType = m_stImageInfo[i].enPixelType;  // ch:相机对应的像素格式 | en:Camera pixel type
            stSaveFileParam.nWidth = m_stImageInfo[i].nWidth;         // ch:相机对应的宽 | en:Width
            stSaveFileParam.nHeight = m_stImageInfo[i].nHeight;          // ch:相机对应的高 | en:Height
            stSaveFileParam.nDataLen = m_stImageInfo[i].nFrameLen;
            stSaveFileParam.pData = m_pSaveImageBuf[i];
            stSaveFileParam.iMethodValue = 2;
            sprintf_s(stSaveFileParam.pImagePath, 256, "Image%d_w%d_h%d_fn%03d.bmp", i, stSaveFileParam.nWidth, stSaveFileParam.nHeight, m_stImageInfo[i].nFrameNum);
            int nRet = m_pcMyCamera[i]->SaveImageToFile(&stSaveFileParam);
            LeaveCriticalSection(&m_hSaveImageMux[i]);
            if (MV_OK != nRet)
            {
                QString mes1 = "Save Image fail! DevIndex[";
                mes1 = mes1 + QString::number(i);
                mes1 = mes1 + "]";
                QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            }
        }
    }
}

void Camera_op::save_image_single(int nCurCameraIndex)
{
    if (FALSE == m_bStartGrabbing)
    {
        return;
    }

    MV_SAVE_IMG_TO_FILE_PARAM stSaveFileParam;
    memset(&stSaveFileParam, 0, sizeof(MV_SAVE_IMG_TO_FILE_PARAM));
        if (m_pcMyCamera[nCurCameraIndex])
        {
            EnterCriticalSection(&m_hSaveImageMux[nCurCameraIndex]);
            if (m_pSaveImageBuf[nCurCameraIndex] == NULL || m_stImageInfo[nCurCameraIndex].enPixelType == 0)
            {
                QString mes1 = "Save Image fail! DevIndex[";
                mes1 = mes1 + QString::number(nCurCameraIndex);
                mes1 = mes1 + "]";
                QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                LeaveCriticalSection(&m_hSaveImageMux[nCurCameraIndex]);
                //continue;
            }
            Thread_SaveImage *Sav = new Thread_SaveImage;
            connect(Sav, SIGNAL(finished()), Sav, SLOT(deleteLater()));
            Sav->pstImageInfo = &m_stImageInfo[nCurCameraIndex];
            Sav->pData = m_pSaveImageBuf[nCurCameraIndex];
            Sav->num = m_stImageInfo[nCurCameraIndex].nFrameNum;
            Sav->nCurCameraIndex = nCurCameraIndex;
            for (unsigned int ii = 0; ii < m_nValidCamNum; ii++)
            {
                Sav->dir_save_every_th[ii] = dir_save_every[ii];
            }
            Sav->start();
            save_file_num[nCurCameraIndex]++;
            //Convert2Mat(&m_stImageInfo[nCurCameraIndex], m_pSaveImageBuf[nCurCameraIndex], m_stImageInfo[nCurCameraIndex].nFrameNum, nCurCameraIndex);
            //stSaveFileParam.enImageType = MV_Image_Bmp; // ch:需要保存的图像类型 | en:Image format to save
            //stSaveFileParam.enPixelType = m_stImageInfo[i].enPixelType;  // ch:相机对应的像素格式 | en:Camera pixel type
            //stSaveFileParam.nWidth = m_stImageInfo[i].nWidth;         // ch:相机对应的宽 | en:Width
            //stSaveFileParam.nHeight = m_stImageInfo[i].nHeight;          // ch:相机对应的高 | en:Height
            //stSaveFileParam.nDataLen = m_stImageInfo[i].nFrameLen;
            //stSaveFileParam.pData = m_pSaveImageBuf[i];
            //stSaveFileParam.iMethodValue = 2;
            //sprintf_s(stSaveFileParam.pImagePath, 256, "Image%d_w%d_h%d_fn%03d.bmp", i, stSaveFileParam.nWidth, stSaveFileParam.nHeight, m_stImageInfo[i].nFrameNum);
            //int nRet = m_pcMyCamera[i]->SaveImageToFile(&stSaveFileParam);
            //LeaveCriticalSection(&m_hSaveImageMux[i]);
            //if (MV_OK != nRet)
            //{
            //    QString mes1 = "Save Image fail! DevIndex[";
            //    mes1 = mes1 + QString::number(i);
            //    mes1 = mes1 + "]";
            //    QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            //}
        }
}

int Camera_op::ThreadFun(int nCurCameraIndex)
{
    get_hik_frame();
    if (m_pcMyCamera[nCurCameraIndex])
    {
        MV_FRAME_OUT stImageOut = { 0 };
        MV_DISPLAY_FRAME_INFO stDisplayInfo = { 0 };
        while (m_bStartGrabbing[nCurCameraIndex])
        {
            int nRet = m_pcMyCamera[nCurCameraIndex]->GetImageBuffer(&stImageOut, 1000);
            if (nRet == MV_OK)
            {
                //用于保存图片
                EnterCriticalSection(&m_hSaveImageMux[nCurCameraIndex]);
                if (NULL == m_pSaveImageBuf[nCurCameraIndex] || stImageOut.stFrameInfo.nFrameLen > m_nSaveImageBufSize[nCurCameraIndex])
                {
                    if (m_pSaveImageBuf[nCurCameraIndex])
                    {
                        free(m_pSaveImageBuf[nCurCameraIndex]);
                        m_pSaveImageBuf[nCurCameraIndex] = NULL;
                    }

                    m_pSaveImageBuf[nCurCameraIndex] = (unsigned char*)malloc(sizeof(unsigned char) * stImageOut.stFrameInfo.nFrameLen);
                    if (m_pSaveImageBuf[nCurCameraIndex] == NULL)
                    {
                        EnterCriticalSection(&m_hSaveImageMux[nCurCameraIndex]);
                        return 0;
                    }
                    m_nSaveImageBufSize[nCurCameraIndex] = stImageOut.stFrameInfo.nFrameLen;
                }
                memcpy(m_pSaveImageBuf[nCurCameraIndex], stImageOut.pBufAddr, stImageOut.stFrameInfo.nFrameLen);
                memcpy(&(m_stImageInfo[nCurCameraIndex]), &(stImageOut.stFrameInfo), sizeof(MV_FRAME_OUT_INFO_EX));
                LeaveCriticalSection(&m_hSaveImageMux[nCurCameraIndex]);

                stDisplayInfo.hWnd = m_hwndDisplay[nCurCameraIndex];
                stDisplayInfo.pData = stImageOut.pBufAddr;
                stDisplayInfo.nDataLen = stImageOut.stFrameInfo.nFrameLen;
                stDisplayInfo.nWidth = stImageOut.stFrameInfo.nWidth;
                stDisplayInfo.nHeight = stImageOut.stFrameInfo.nHeight;
                stDisplayInfo.enPixelType = stImageOut.stFrameInfo.enPixelType;

                nRet = m_pcMyCamera[nCurCameraIndex]->DisplayOneFrame(&stDisplayInfo);
                if (MV_OK != nRet)
                {
                    QString mes1 = "Display one frame fail! DevIndex[";
                    mes1 = mes1 + QString::number(nCurCameraIndex);
                    mes1 = mes1 + "]";
                    QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                }

                nRet = m_pcMyCamera[nCurCameraIndex]->FreeImageBuffer(&stImageOut);
                if (MV_OK != nRet)
                {
                    QString mes1 = "Free image buffer fail! DevIndex[";
                    mes1 = mes1 + QString::number(nCurCameraIndex);
                    mes1 = mes1 + "]";
                    QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                }
                if ((if_continue_save)|| if_exsave(nCurCameraIndex))
                {
                    save_image_single(nCurCameraIndex);
                }
            }
        }
    }

    return MV_OK;
}
bool Camera_op::if_exsave(int nCurCameraIndex)
{
    unsigned int num_max=0;
    for (int i = 0; i < MAX_DEVICE_NUM; i++)
    {
        if (save_file_num[i] > num_max)
        {
            num_max = save_file_num[i];
        }
    }
    if (save_file_num[nCurCameraIndex] == num_max)
    {
        return false;
    }
    else
    {
        return true;
    }
}
void Camera_op::get_hik_frame()
{
    QString TE = "";
    for (unsigned int i = 0; i < m_nValidCamNum; i++)
    {
        if (m_bStartGrabbing[i])
        {
            QString txe = "Camera[";
            txe = txe + QString::number(i);
            txe = txe + "]-frame:";
            MVCC_FLOATVALUE stFloatValue = { 0 };
            int nRet = m_pcMyCamera[i]->GetFloatValue("ResultingFrameRate", &stFloatValue);
            if (MV_OK != nRet)
            {
                return ;
            }
            txe = txe + QString::number(stFloatValue.fCurValue);
            if (i < (m_nValidCamNum - 1))
            {
                txe = txe + "\n";
            }
            TE = TE + txe;
        }
    }
    ui.label->setText(TE);
}

void Camera_op::begin_con_save()
{
    if (if_continue_save)
    {
        return;
    }
    for (int i = 0; i < MAX_DEVICE_NUM; i++)
    {
        save_file_num[i] = 0;
    }
    if_continue_save = TRUE;
}

void Camera_op::stop_con_save()
{
    if (!if_continue_save)
    {
        return;
    }
    if_continue_save = FALSE;
}

bool Camera_op::Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char* pData, unsigned int num,int nCurCameraIndex)
{
    cv::Mat srcImage;
    if (pstImageInfo->enPixelType == PixelType_Gvsp_Mono8)
    {
        srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC1, pData);
    }
    else if (pstImageInfo->enPixelType == PixelType_Gvsp_RGB8_Packed)
    {
        RGB2BGR(pData, pstImageInfo->nWidth, pstImageInfo->nHeight);
        srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC3, pData);
    }
    else
    {
        printf("unsupported pixel format\n");
        return false;
    }

    if (NULL == srcImage.data)
    {
        return false;
    }
    QString dir_n = dir_save_every[nCurCameraIndex] + "\\Fn";
    dir_n = dir_n + QString::number(num);
    dir_n = dir_n + ".bmp";
    //save converted image in a local file
    try {
#if defined (VC9_COMPILE)
        cvSaveImage("MatImage.bmp", &(IplImage(srcImage)));
#else
        cv::imwrite(dir_n.toStdString(), srcImage);
#endif
    }
    catch (cv::Exception& ex) {
        fprintf(stderr, "Exception saving image to bmp format: %s\n", ex.what());
    }

    srcImage.release();

    return true;
}

void Camera_op::set_camera()
{
    if (m_bOpenDevice[ui.Cam_List_2->currentIndex()])
    {
        int nRet = m_pcMyCamera[ui.Cam_List_2->currentIndex()]->SetEnumValue("ExposureMode", MV_EXPOSURE_MODE_TIMED);
        if (MV_OK != nRet)
        {
            return;
        }
        nRet = m_pcMyCamera[ui.Cam_List_2->currentIndex()]->SetEnumValue("ExposureAuto", MV_EXPOSURE_AUTO_MODE_OFF);
        nRet = m_pcMyCamera[ui.Cam_List_2->currentIndex()]->SetFloatValue("ExposureTime", ui.Cam_us_in->text().toFloat());

        nRet = m_pcMyCamera[ui.Cam_List_2->currentIndex()]->SetBoolValue("AcquisitionFrameRateEnable", true);
        if (MV_OK != nRet)
        {
            return;
        }

        nRet = m_pcMyCamera[ui.Cam_List_2->currentIndex()]->SetFloatValue("AcquisitionFrameRate", ui.Cam_hz_in->text().toFloat());
        reframe_camera();
    }
}
void Camera_op::reframe_camera_fir()
{
    ui.Cam_hz_in->setText("......");
    ui.Cam_us_in->setText("......");
    qApp->processEvents();
    Sleep(1);
}
void Camera_op::reframe_camera()
{
    int a = ui.Cam_List_2->currentIndex();
    if (ui.Cam_List_2->currentIndex() == -1)
    {
        return;
    }
    if (m_bOpenDevice[ui.Cam_List_2->currentIndex()])
    {
        MVCC_FLOATVALUE stFloatValue = { 0 };
        int nRet = m_pcMyCamera[ui.Cam_List_2->currentIndex()]->GetFloatValue("ResultingFrameRate", &stFloatValue);
        if (MV_OK != nRet)
        {
            return;
        }
        ui.Cam_hz_in->setText(QString::number(stFloatValue.fCurValue));

        MVCC_FLOATVALUE stFloatValue2 = { 0 };
        nRet = m_pcMyCamera[ui.Cam_List_2->currentIndex()]->GetFloatValue("ExposureTime", &stFloatValue2);
        if (MV_OK != nRet)
        {
            return;
        }
        ui.Cam_us_in->setText(QString::number(stFloatValue2.fCurValue));
    }
}
int Camera_op::RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight)
{
    if (NULL == pRgbData)
    {
        return MV_E_PARAMETER;
    }

    for (unsigned int j = 0; j < nHeight; j++)
    {
        for (unsigned int i = 0; i < nWidth; i++)
        {
            unsigned char red = pRgbData[j * (nWidth * 3) + i * 3];
            pRgbData[j * (nWidth * 3) + i * 3] = pRgbData[j * (nWidth * 3) + i * 3 + 2];
            pRgbData[j * (nWidth * 3) + i * 3 + 2] = red;
        }
    }

    return MV_OK;
}

Camera_op::~Camera_op()
{
}



Thread_SaveImage::Thread_SaveImage(QObject* parent)
{

}

void Thread_SaveImage::run()
{

    cv::Mat srcImage;
    if (pstImageInfo->enPixelType == PixelType_Gvsp_Mono8)
    {
        srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC1, pData);
    }
    else if (pstImageInfo->enPixelType == PixelType_Gvsp_RGB8_Packed)
    {
        RGB2BGR(pData, pstImageInfo->nWidth, pstImageInfo->nHeight);
        srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC3, pData);
    }
    else
    {
        printf("unsupported pixel format\n");
    }

    if (NULL == srcImage.data)
    {
    }
    QString dir_n = dir_save_every_th[nCurCameraIndex] + "\\Fn";
    dir_n = dir_n + QString::number(num);
    dir_n = dir_n + ".bmp";
    //save converted image in a local file
    try {
#if defined (VC9_COMPILE)
        cvSaveImage("MatImage.bmp", &(IplImage(srcImage)));
#else
        cv::imwrite(dir_n.toStdString(), srcImage);
#endif
    }
    catch (cv::Exception& ex) {
        fprintf(stderr, "Exception saving image to bmp format: %s\n", ex.what());
    }

    srcImage.release();
}
int Thread_SaveImage::RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight)
{
    if (NULL == pRgbData)
    {
        return MV_E_PARAMETER;
    }

    for (unsigned int j = 0; j < nHeight; j++)
    {
        for (unsigned int i = 0; i < nWidth; i++)
        {
            unsigned char red = pRgbData[j * (nWidth * 3) + i * 3];
            pRgbData[j * (nWidth * 3) + i * 3] = pRgbData[j * (nWidth * 3) + i * 3 + 2];
            pRgbData[j * (nWidth * 3) + i * 3 + 2] = red;
        }
    }

    return MV_OK;
}