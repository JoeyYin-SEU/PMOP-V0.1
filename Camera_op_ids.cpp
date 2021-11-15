#include "Camera_op_ids.h"

Camera_op_ids::Camera_op_ids(QWidget *parent)
	: QWidget(parent)
{
    m_begintime = std::chrono::system_clock::now();
    singlesave_num = 2;
    save_time_info = TRUE;
    List_disk = QDir::drives();
    QString sc;
    QDir dir;
    QDateTime current_date_time = QDateTime::currentDateTime();
    current_date = current_date_time.toString("yyyy-MM-dd-hh-mm-ss");
    //QString current_date = current_date_time.toString('yyyy-MM-dd hh:mm:ss ddd');
    ui.setupUi(this);

    scene_1 = new QGraphicsScene;
    scene_2 = new QGraphicsScene;
    scene_3 = new QGraphicsScene;
    scene_4 = new QGraphicsScene;
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
    if_discret_save = FALSE;
    if_continue_save = FALSE;
	m_vecDevices.clear();
    for (int ii = 0; ii < MAX_NUMBER_OF_DEVICES; ii++)
    {
        save_flag[ii] = 0;
        camera_width[ii] = 1;
        camera_height[ii] = 1;
    }
    refresh_camera();
    for (int ii = 0; ii < MAX_NUMBER_OF_DEVICES; ii++)
    {
        is_open[ii] = FALSE;
    }
    for (int ii = 0; ii < m_nValidCamNum; ii++)
    {
        offsetx[ii] = 0;
        offsety[ii] = 0;
        save_file_num[ii] = 0;
        if_single_save[ii] = FALSE;
        single_save_num[ii] = 0;
        dir_save = dir_save + "\\";
        dir_save_every[ii] = dir_save + device_name[ii];
        if (!dir.exists(dir_save_every[ii]))
        {
            if (!dir.mkpath(dir_save_every[ii]))
            {
                QString mes1 = "Working path failed to creat.";
                QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            }
        }
        dir_save_txt[ii] = dir_save + "\\";
        dir_save_txt[ii] = dir_save_txt[ii] + "info_"+ device_name[ii] +".txt";
        QFile file(dir_save_txt[ii]);
        if (!file.exists())
        {
            bool res2 = file.open(QIODevice::WriteOnly | QIODevice::Text );
        }
        file.close();
    }
    connect(ui.Open_new, SIGNAL(clicked()), this, SLOT(open_camera()));
    connect(ui.Open_all, SIGNAL(clicked()), this, SLOT(open_camera_all()));
    connect(ui.re_cam, SIGNAL(clicked()), this, SLOT(refresh_camera()));
    connect(ui.Cam_List_3, SIGNAL(currentIndexChanged(int)), this, SLOT(re_savepath()));
    connect(ui.Sin_Save, SIGNAL(clicked()), this, SLOT(save_image_single_click()));
    connect(ui.Con_Save, SIGNAL(clicked()), this, SLOT(begin_con_save()));
    connect(ui.Con_Save_Stop, SIGNAL(clicked()), this, SLOT(stop_con_save()));
    connect(ui.Cam_List, SIGNAL(currentIndexChanged(int)), this, SLOT(re_xy()));
    connect(ui.spin_Wid, SIGNAL(valueChanged(int)), this, SLOT(set_xyoff_w()));
    connect(ui.spin_Hei, SIGNAL(valueChanged(int)), this, SLOT(set_xyoff_h()));
    connect(ui.spin_OffX, SIGNAL(valueChanged(int)), this, SLOT(set_xyoff_x()));
    connect(ui.spin_OffY, SIGNAL(valueChanged(int)), this, SLOT(set_xyoff_y()));
    connect(ui.lineEdit_HZ, SIGNAL(textChanged(const QString&)), this, SLOT(set_hz()));
    connect(ui.lineEdit_EX, SIGNAL(textChanged(const QString&)), this, SLOT(set_ex()));
}
void Camera_op_ids::set_hz()
{
    int num = ui.Cam_List->currentIndex();
    if (num == -1)
    {
        return;
    }
    if (m_vecDevices.size() == 0)
    {
        return;
    }
    if (ui.comboBox->currentIndex() == 0)
    {
        double frameRateLimit = ui.lineEdit_HZ->text().toFloat();

        // Get the framerate range
        double framerateMin = m_nodeMap[ui.Cam_List->currentIndex()]
            ->FindNode<peak::core::nodes::FloatNode>(
                "AcquisitionFrameRate")
            ->Minimum();
        double framerateMax = m_nodeMap[ui.Cam_List->currentIndex()]
            ->FindNode<peak::core::nodes::FloatNode>(
                "AcquisitionFrameRate")
            ->Maximum();

        // Check framerate limit
        if (frameRateLimit < framerateMin)
        {
            frameRateLimit = framerateMin;
        }
        else if (frameRateLimit > framerateMax)
        {
            frameRateLimit = framerateMax;
        }

        // Set framerate to limit minus safety buffer of 0.5 frames
        m_nodeMap[ui.Cam_List->currentIndex()]
            ->FindNode<peak::core::nodes::FloatNode>("AcquisitionFrameRate")
            ->SetValue(frameRateLimit);
    }
    else
    {
        for (int ii = 0; ii < m_nValidCamNum; ii++)
        {
            double frameRateLimit = ui.lineEdit_HZ->text().toFloat();

            // Get the framerate range
            double framerateMin = m_nodeMap[ii]
                ->FindNode<peak::core::nodes::FloatNode>(
                    "AcquisitionFrameRate")
                ->Minimum();
            double framerateMax = m_nodeMap[ii]
                ->FindNode<peak::core::nodes::FloatNode>(
                    "AcquisitionFrameRate")
                ->Maximum();

            // Check framerate limit
            if (frameRateLimit < framerateMin)
            {
                frameRateLimit = framerateMin;
            }
            else if (frameRateLimit > framerateMax)
            {
                frameRateLimit = framerateMax;
            }

            // Set framerate to limit minus safety buffer of 0.5 frames
            m_nodeMap[ii]
                ->FindNode<peak::core::nodes::FloatNode>("AcquisitionFrameRate")
                ->SetValue(frameRateLimit);
        }
    }
}
void Camera_op_ids::set_ex()
{
    if (ui.lineEdit_EX->text().toFloat() < 100)
    {
        return;
    }
    int num = ui.Cam_List->currentIndex();
    if (num == -1)
    {
        return;
    }
    if (m_vecDevices.size() == 0)
    {
        return;
    }
    if (ui.comboBox->currentIndex() == 0)
    {
        m_nodeMap[ui.Cam_List->currentIndex()]->FindNode<peak::core::nodes::FloatNode>("ExposureTime")->SetValue(ui.lineEdit_EX->text().toFloat());
    }
    else
    {
        for (int ii = 0; ii < m_nValidCamNum; ii++)
        {
            m_nodeMap[ii]->FindNode<peak::core::nodes::FloatNode>("ExposureTime")->SetValue(ui.lineEdit_EX->text().toFloat());
        }
    }
}
void Camera_op_ids::set_xyoff_w()
{
    camera_width[ui.Cam_List->currentIndex()] = ui.spin_Wid->value();
}
void Camera_op_ids::set_xyoff_h()
{
    camera_height[ui.Cam_List->currentIndex()] = ui.spin_Hei->value();
}
void Camera_op_ids::set_xyoff_x()
{
    offsetx[ui.Cam_List->currentIndex()] = ui.spin_OffX->value();
}
void Camera_op_ids::set_xyoff_y()
{
    offsety[ui.Cam_List->currentIndex()] = ui.spin_OffY->value();
}
void Camera_op_ids::re_xy()
{
    int num = ui.Cam_List->currentIndex();
    if (num == -1)
    {
        return;
    }
    if (m_vecDevices.size() == 0)
    {
        return;
    }
    auto deviceElem = m_vecDevices.at(num);
    ui.spin_Wid->setMaximum(deviceElem->imageWidth);
    ui.spin_Hei->setMaximum(deviceElem->imageHeight);
    ui.spin_Wid->setMinimum(1);
    ui.spin_Hei->setMinimum(1);
    ui.spin_Wid->setValue(camera_width[num]);
    ui.spin_Hei->setValue(camera_height[num]);
    ui.spin_OffX->setValue(offsetx[num]);
    ui.spin_OffY->setValue(offsety[num]);
}
bool Camera_op_ids::OpenDevices()
{
    try
    {
        auto& deviceManager = peak::DeviceManager::Instance();
        deviceManager.Update();
        if (deviceManager.Devices().empty())
        {
            QMessageBox::critical(this, "Error", "No device found", QMessageBox::Ok);
            return false;
        }
        int opened = 0;
        int numberOfGevCameras = 0;
        int ii = 0;
        for (size_t i = 0; i < deviceManager.Devices().size(); ++i)
        {
            if (deviceManager.Devices().at(i)->IsOpenable())
            {
                auto device = deviceManager.Devices().at(i)->OpenDevice(peak::core::DeviceAccessType::Control);
                if (device)
                {
                    std::shared_ptr<DeviceContext> deviceElem = std::make_shared<DeviceContext>();
                    deviceElem->device = device;
                    auto dataStreams = device->DataStreams();
                    if (dataStreams.empty())
                    {
                        QMessageBox::critical(this, "Error", "Device has no DataStream", QMessageBox::Ok);
                        device.reset();
                        return false;
                    }

                    deviceElem->dataStream = dataStreams.at(0)->OpenDataStream();
                    deviceElem->nodemapRemoteDevice = device->RemoteDevice()->NodeMaps().at(0);
                    m_nodeMap[ii] = deviceElem->nodemapRemoteDevice;
                    try
                    {
                        deviceElem->nodemapRemoteDevice
                            ->FindNode<peak::core::nodes::EnumerationNode>("UserSetSelector")
                            ->SetCurrentEntry("Default");
                        deviceElem->nodemapRemoteDevice
                            ->FindNode<peak::core::nodes::CommandNode>("UserSetLoad")
                            ->Execute();
                        deviceElem->nodemapRemoteDevice
                            ->FindNode<peak::core::nodes::CommandNode>("UserSetLoad")
                            ->WaitUntilDone();
                    }
                    catch (peak::core::Exception const&)
                    {
                        // Cannot load userset default. Ingnore and try to continue.
                    }
                    //try
                    //{
                    //    // Set auto exposure continuous
                    //    m_nodeMap[ii]->FindNode<peak::core::nodes::EnumerationNode>("ExposureAuto")->SetCurrentEntry("Continuous");
                    //}
                    //catch (const peak::core::NotFoundException&)
                    //{
                    //    // Auto exposure is not available
                    //}
                    try
                    {
                        m_nodeMap[ii]->FindNode<peak::core::nodes::BooleanNode>("ChunkModeActive")->SetValue(true);
                        m_nodeMap[ii]->FindNode<peak::core::nodes::EnumerationNode>("ChunkSelector")->SetCurrentEntry("Width");
                        m_nodeMap[ii]->FindNode<peak::core::nodes::BooleanNode>("ChunkEnable")->SetValue(true);

                        m_nodeMap[ii]->FindNode<peak::core::nodes::EnumerationNode>("ChunkSelector")->SetCurrentEntry("Height");
                        m_nodeMap[ii]->FindNode<peak::core::nodes::BooleanNode>("ChunkEnable")->SetValue(true);
                        m_nodeMap[ii]->FindNode<peak::core::nodes::EnumerationNode>("ChunkSelector")->SetCurrentEntry("PixelFormat");
                        m_nodeMap[ii]->FindNode<peak::core::nodes::BooleanNode>("ChunkEnable")->SetValue(true);
                        m_nodeMap[ii]->FindNode<peak::core::nodes::EnumerationNode>("ChunkSelector")->SetCurrentEntry("ExposureTime");
                        m_nodeMap[ii]->FindNode<peak::core::nodes::BooleanNode>("ChunkEnable")->SetValue(true);
                    }
                    catch (const peak::core::NotFoundException&)
                    {
                        QMessageBox::information(this, "Exception", "Exception: The camera does not support ChunkData. ", QMessageBox::Ok);
                        
                    }
                    catch (const std::exception& e)
                    {
                        QMessageBox::information(this, "Exception", e.what(), QMessageBox::Ok);
                    }
                    // Get the payload size for correct buffer allocation
                    auto payloadSize = deviceElem->nodemapRemoteDevice
                        ->FindNode<peak::core::nodes::IntegerNode>("PayloadSize")
                        ->Value();

                    // Get the minimum number of buffers that must be announced
                    auto bufferCountMin = deviceElem->dataStream->NumBuffersAnnouncedMinRequired();

                    // Allocate and announce image buffers and queue them
                    for (size_t bufferCount = 0; bufferCount < bufferCountMin; ++bufferCount)
                    {
                        auto buffer = deviceElem->dataStream->AllocAndAnnounceBuffer(
                            static_cast<size_t>(payloadSize), nullptr);
                        deviceElem->dataStream->QueueBuffer(buffer);
                    }

                    // Get the sensor size
                    deviceElem->imageWidth = static_cast<int>(
                        deviceElem->nodemapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("Width")
                        ->Value());
                    deviceElem->imageHeight = static_cast<int>(
                        deviceElem->nodemapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("Height")
                        ->Value());

                    if ("GEV" == deviceElem->device->ParentInterface()->TLType())
                    {
                        numberOfGevCameras++;
                    }

                    // Push vector element into the device vector
                    m_vecDevices.push_back(deviceElem);

                    opened++;
                }
                if (MAX_NUMBER_OF_DEVICES == opened)
                {
                    break;
                }
                ii++;
            }
        }

        if (m_vecDevices.empty())
        {
            QMessageBox::critical(this, "Error", "No device openable - maybe already in use?", QMessageBox::Ok);
            return false;
        }

        /****************************************/
        /* GEV bandwidth management             */
        /****************************************/

        if (0 != numberOfGevCameras)
        {
            // Divide the maximum bandwidth by number of GEV cameras
            int64_t deviceLinkThroughputLimit = static_cast<int64_t>(MAXIMUM_THROUGHPUT_LIMIT / numberOfGevCameras);

            for (const auto& deviceElem : m_vecDevices)
            {
                if ("GEV" == deviceElem->device->ParentInterface()->TLType())
                {
                    // Try to adjust the DeviceLinkThoughputLimit
                    try
                    {
                        // Get range of the device link throughput limit
                        int64_t deviceLinkThroughputLimitRange_Min =
                            deviceElem->nodemapRemoteDevice
                            ->FindNode<peak::core::nodes::IntegerNode>("DeviceLinkThroughputLimit")
                            ->Minimum();
                        int64_t deviceLinkThroughputLimitRange_Max =
                            deviceElem->nodemapRemoteDevice
                            ->FindNode<peak::core::nodes::IntegerNode>("DeviceLinkThroughputLimit")
                            ->Maximum();

                        // Check the value
                        if (deviceLinkThroughputLimit < deviceLinkThroughputLimitRange_Min)
                        {
                            deviceLinkThroughputLimit = deviceLinkThroughputLimitRange_Min;
                        }
                        else if (deviceLinkThroughputLimit > deviceLinkThroughputLimitRange_Max)
                        {
                            deviceLinkThroughputLimit = deviceLinkThroughputLimitRange_Max;
                        }

                        // Set new throughput limit
                        deviceElem->nodemapRemoteDevice
                            ->FindNode<peak::core::nodes::IntegerNode>("DeviceLinkThroughputLimit")
                            ->SetValue(deviceLinkThroughputLimit);
                        // The maximum possible framerate changes depending on the throughput limit
                        double frameRateLimit = deviceElem->nodemapRemoteDevice
                            ->FindNode<peak::core::nodes::FloatNode>(
                                "DeviceLinkAcquisitionFrameRateLimit")
                            ->Value()
                            - 0.5;

                        // Get the framerate range
                        double framerateMin = deviceElem->nodemapRemoteDevice
                            ->FindNode<peak::core::nodes::FloatNode>(
                                "AcquisitionFrameRate")
                            ->Minimum();
                        double framerateMax = deviceElem->nodemapRemoteDevice
                            ->FindNode<peak::core::nodes::FloatNode>(
                                "AcquisitionFrameRate")
                            ->Maximum();

                        // Check framerate limit
                        if (frameRateLimit < framerateMin)
                        {
                            frameRateLimit = framerateMin;
                        }
                        else if (frameRateLimit > framerateMax)
                        {
                            frameRateLimit = framerateMax;
                        }

                        // Set framerate to limit minus safety buffer of 0.5 frames
                        deviceElem->nodemapRemoteDevice
                            ->FindNode<peak::core::nodes::FloatNode>("AcquisitionFrameRate")
                            ->SetValue(frameRateLimit);
                    }
                    catch (peak::core::Exception const&)
                    {
                        QMessageBox::information(this, "Warning",
                            "Unable to optimize bandwidth. Program will continue, but performance might not be "
                            "optimal.",
                            QMessageBox::Ok);
                    }
                }
            }
        }

        return true;
    }
    catch (const std::exception& e)
    {
        QMessageBox::information(this, "Exception", e.what(), QMessageBox::Ok);
    }

    return false;
}

void Camera_op_ids::refresh_camera()
{
    peak::Library::Initialize();
    QDir dir;
    ui.Cam_List->clear();
    ui.Cam_List_2->clear();
    m_vecDevices.clear();
    try
    {
        auto& deviceManager = peak::DeviceManager::Instance();
        deviceManager.Update();
        int opened = 0;
        int numberOfGevCameras = 0;
        m_nValidCamNum = 0;
        std::vector<std::string> serialNumbers; 
        m_devices = deviceManager.Devices();
        std::shared_ptr<peak::core::DeviceDescriptor> a = deviceManager.Devices().at(0);
        for (const auto& deviceDescriptor : deviceManager.Devices())
        {
            if (std::find(serialNumbers.begin(), serialNumbers.end(), deviceDescriptor->SerialNumber())
                == serialNumbers.end())
            {
                serialNumbers.push_back(deviceDescriptor->SerialNumber());
                std::string chDeviceName = deviceDescriptor->DisplayName();
                device_name[m_nValidCamNum] = QString::fromStdString(chDeviceName);
                m_nValidCamNum++;
                ui.Cam_List->addItem(QString::fromStdString(chDeviceName));
                ui.Cam_List_2->addItem(QString::fromStdString(chDeviceName));
            }
        }
        for (int ii = 0; ii < m_nValidCamNum; ii++)
        {
            dir_save = dir_save + "\\";
            dir_save_every[ii] = dir_save + device_name[ii];
            if (!dir.exists(dir_save_every[ii]))
            {
                if (!dir.mkpath(dir_save_every[ii]))
                {
                    QString mes1 = "Working path failed to creat.";
                    QMessageBox::critical(NULL, "critical", mes1, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
                }
            }
        }
        if (m_nValidCamNum == 0)
        {
            ui.label->show();
            ui.label_2->hide();
            ui.label_3->hide();
            ui.label_4->hide();
        }
        if (m_nValidCamNum == 1)
        {
            ui.Camera_1->show();
            ui.Camera_2->hide();
            ui.Camera_3->hide();
            ui.Camera_4->hide();
            ui.label->show();
            ui.label_2->hide();
            ui.label_3->hide();
            ui.label_4->hide();
            ui.horizontalLayout->setStretch(0, 1);
            ui.horizontalLayout->setStretch(1, 0);
            ui.verticalLayout->setStretch(0, 1);
            ui.verticalLayout->setStretch(1, 0);
        }
        if (m_nValidCamNum == 2)
        {
            ui.Camera_1->show();
            ui.Camera_2->show();
            ui.Camera_3->hide();
            ui.Camera_4->hide();
            ui.label->show();
            ui.label_2->show();
            ui.label_3->hide();
            ui.label_4->hide();
            ui.horizontalLayout->setStretch(0, 1);
            ui.horizontalLayout->setStretch(1, 1);
            ui.verticalLayout->setStretch(0, 1);
            ui.verticalLayout->setStretch(1, 0);
        }
        OpenDevices();
        int ii = 0;
        for (const auto& deviceElem : m_vecDevices)
        {
            ui.spin_Wid->setMaximum(deviceElem->imageWidth);
            ui.spin_Hei->setMaximum(deviceElem->imageHeight);
            ui.spin_Wid->setMinimum(1);
            ui.spin_Hei->setMinimum(1);
            if (camera_width[ii] == 1 & camera_height[ii] == 1)
            {
                ui.spin_Wid->setValue(deviceElem->imageWidth);
                ui.spin_Hei->setValue(deviceElem->imageHeight);
                camera_width[ii] = deviceElem->imageWidth;
                camera_height[ii] = deviceElem->imageHeight;
            }
            ii++;
        }
    }
    catch (const std::exception& e)
    {
    }
    ui.horizontalLayout_4->setStretch(0, 0);
    ui.horizontalLayout_4->setStretch(1, 1);
}

void Camera_op_ids::open_camera()
{   
    if (m_nValidCamNum == 0)
    {
        return;
    }
    if (is_open[ui.Cam_List->currentIndex()])
    {
        close_camera_single(ui.Cam_List->currentIndex());
        is_open[ui.Cam_List->currentIndex()] = FALSE;
        return;
    }

    if (ui.Cam_List->currentIndex() >= MAX_NUMBER_OF_DEVICES)
    {
        QMessageBox::information(this, "Exception", "MAX CAMERA MUST LESS THAN 4", QMessageBox::Ok);
    }
    try
    {
        refresh_camera();
        if (open_camera_single(ui.Cam_List->currentIndex()))
        {
            is_open[ui.Cam_List->currentIndex()] = TRUE;
        }
        else
        {
            is_open[ui.Cam_List->currentIndex()] = FALSE;
        }
    }
    catch (const std::exception& e)
    {
        QMessageBox::information(this, "Exception", e.what(), QMessageBox::Ok);
    }
}
bool Camera_op_ids::open_camera_single(unsigned int num)
{
        
    try
    {
        auto deviceElem = m_vecDevices[num];
        deviceElem->nodemapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("Width")->SetValue((int64_t)camera_width[num]);
        deviceElem->nodemapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("Height")->SetValue((int64_t)camera_height[num]);
        deviceElem->nodemapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("OffsetX")->SetValue((int64_t)offsetx[num]);
        deviceElem->nodemapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("OffsetY")->SetValue((int64_t)offsety[num]);
        deviceElem->imageWidth = camera_width[num];
        deviceElem->imageHeight = camera_height[num];
        deviceElem->pixelFormat = peak::ipl::PixelFormatName::Mono8;
        if(num==0)
        {
            deviceElem->acquisitionWorker = new AcquisitionWorker(this, ui.Camera_1,
                deviceElem->dataStream, m_nodeMap[0], deviceElem->pixelFormat, static_cast<size_t>(deviceElem->imageWidth),
                static_cast<size_t>(deviceElem->imageHeight), m_begintime);
        }
        if (num == 1)
        {
            deviceElem->acquisitionWorker = new AcquisitionWorker(this, ui.Camera_2,
                deviceElem->dataStream, m_nodeMap[1], deviceElem->pixelFormat, static_cast<size_t>(deviceElem->imageWidth),
                static_cast<size_t>(deviceElem->imageHeight), m_begintime);
        }
        if (num == 2)
        {
            deviceElem->acquisitionWorker = new AcquisitionWorker(this, ui.Camera_3,
                deviceElem->dataStream, m_nodeMap[2], deviceElem->pixelFormat, static_cast<size_t>(deviceElem->imageWidth),
                static_cast<size_t>(deviceElem->imageHeight), m_begintime);
        }
        if (num == 3)
        {
            deviceElem->acquisitionWorker = new AcquisitionWorker(this, ui.Camera_4,
                deviceElem->dataStream, m_nodeMap[3], deviceElem->pixelFormat, static_cast<size_t>(deviceElem->imageWidth),
                static_cast<size_t>(deviceElem->imageHeight), m_begintime);
        }
        deviceElem->acquisitionWorker->moveToThread(&deviceElem->acquisitionThread);
        if (num == 0)
        {
            connect(deviceElem->acquisitionWorker, &AcquisitionWorker::ImageReceived, this,
                &Camera_op_ids::UpdateDisplay_1);
            connect(deviceElem->acquisitionWorker, &AcquisitionWorker::UpdateCounters, this,
                &Camera_op_ids::UpdateCounters_1);
        }
        if (num == 1)
        {
            connect(deviceElem->acquisitionWorker, &AcquisitionWorker::ImageReceived, this,
                &Camera_op_ids::UpdateDisplay_2);
            connect(deviceElem->acquisitionWorker, &AcquisitionWorker::UpdateCounters, this,
                &Camera_op_ids::UpdateCounters_2);
        }
        if (num == 2)
        {
            connect(deviceElem->acquisitionWorker, &AcquisitionWorker::ImageReceived, this,
                &Camera_op_ids::UpdateDisplay_3);
            connect(deviceElem->acquisitionWorker, &AcquisitionWorker::UpdateCounters, this,
                &Camera_op_ids::UpdateCounters_3);
        }
        if (num == 3)
        {
            connect(deviceElem->acquisitionWorker, &AcquisitionWorker::ImageReceived, this,
                &Camera_op_ids::UpdateDisplay_4);
            connect(deviceElem->acquisitionWorker, &AcquisitionWorker::UpdateCounters, this,
                &Camera_op_ids::UpdateCounters_4);
        }
        connect(&deviceElem->acquisitionThread, &QThread::started, deviceElem->acquisitionWorker,
            &AcquisitionWorker::Start);
        deviceElem->acquisitionThread.start();
        deviceElem->nodemapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("TLParamsLocked")
            ->SetValue(1);
        deviceElem->dataStream->StartAcquisition();
        deviceElem->nodemapRemoteDevice->FindNode<peak::core::nodes::CommandNode>("AcquisitionStart")
            ->Execute();
        return TRUE;
    }
    catch (const std::exception& e)
    {
        QMessageBox::information(this, "Exception", e.what(), QMessageBox::Ok);
        return FALSE;
    }

}
bool Camera_op_ids::close_camera_single(unsigned int num)
{
    auto deviceElem = m_vecDevices[num];
    if (deviceElem->acquisitionWorker)
    {
        deviceElem->acquisitionWorker->Stop();
        deviceElem->acquisitionThread.quit();
        deviceElem->acquisitionThread.wait();

        delete deviceElem->acquisitionWorker;
        deviceElem->acquisitionWorker = nullptr;
    }
    try
    {
        if (deviceElem->dataStream->IsGrabbing())
        {
            deviceElem->nodemapRemoteDevice->FindNode<peak::core::nodes::CommandNode>("AcquisitionStop")
                ->Execute();
            deviceElem->nodemapRemoteDevice->FindNode<peak::core::nodes::CommandNode>("AcquisitionStop")
                ->WaitUntilDone();
        }
    }
    catch (const std::exception& e)
    {
        QMessageBox::information(this, "Exception", e.what(), QMessageBox::Ok);
    }
    if (deviceElem->dataStream)
    {
        try
        {
            if (deviceElem->dataStream->IsGrabbing())
            {
                deviceElem->dataStream->KillWait();
                deviceElem->dataStream->StopAcquisition(peak::core::AcquisitionStopMode::Default);
            }

            deviceElem->dataStream->Flush(peak::core::DataStreamFlushMode::DiscardAll);

            for (const auto& buffer : deviceElem->dataStream->AnnouncedBuffers())
            {
                deviceElem->dataStream->RevokeBuffer(buffer);
            }
            deviceElem->nodemapRemoteDevice->FindNode<peak::core::nodes::IntegerNode>("TLParamsLocked")
                ->SetValue(0);

            deviceElem->dataStream = nullptr;
            deviceElem->nodemapRemoteDevice = nullptr;
            deviceElem->device = nullptr;
        }
        catch (const std::exception& e)
        {
            QMessageBox::information(this, "Exception", e.what(), QMessageBox::Ok);
        }
    }
    m_vecDevices[num]= nullptr;
    return TRUE;
}
void Camera_op_ids::open_camera_all()
{
    bool if_rf = FALSE;
    for (int ii = 0; ii < m_nValidCamNum; ii++)
    {
        if (is_open[ii])
            if_rf = TRUE;
    }
    if (if_rf)
    {
        for (int ii = 0; ii < m_nValidCamNum; ii++)
        {
            close_camera_single(ii);
            is_open[ii] = FALSE;
        }
        return;
    }
    try
    {
        refresh_camera();
        for(unsigned int ii=0;ii< m_nValidCamNum;ii++)
        {
            if (open_camera_single(ii))
            {
                is_open[ii] = TRUE;
            }
            else
            {
                is_open[ii] = FALSE;
            }

        }
    }
    catch (const std::exception& e)
    {
        QMessageBox::information(this, "Exception", e.what(), QMessageBox::Ok);
    }
}

void Camera_op_ids::UpdateCounters_1(double frameTime_ms, double conversionTime_ms, unsigned int frameCounter,
    unsigned int errorCounter, int incomplete, int dropped, int lost, bool showCustomNodes)
{
    QString str;
    auto deviceElem = m_vecDevices[0];
    double frameRate_Hz = 0.0;
    if (frameTime_ms > 0)
    {
        frameRate_Hz = 1000.0 / frameTime_ms;
    }

    double m_frameRate_Hz = frameRate_Hz;
    double m_conversionTime_ms = conversionTime_ms;

    QString strText;

    if (showCustomNodes)
    {
        strText.sprintf(
            "Framerate: %.1f fps, ExposureTime: %.1f us, acquired: %d, errors: %d, incomplete: %d, dropped: %d, lost: %d",
            m_frameRate_Hz, m_conversionTime_ms, frameCounter, errorCounter, incomplete, dropped, lost);
    }
    else
    {
        strText.sprintf("Framerate: %.1f fps, ExposureTime: %.1f us, acquired: %d, errors: %d", m_frameRate_Hz,
            m_conversionTime_ms, frameCounter, errorCounter);
    }

    ui.label->setText(strText);
}
void Camera_op_ids::UpdateCounters_2(double frameTime_ms, double conversionTime_ms, unsigned int frameCounter,
    unsigned int errorCounter, int incomplete, int dropped, int lost, bool showCustomNodes)
{
    QString str;

    double frameRate_Hz = 0.0;
    if (frameTime_ms > 0)
    {
        frameRate_Hz = 1000.0 / frameTime_ms;
    }

    double m_frameRate_Hz = frameRate_Hz;
    double m_conversionTime_ms = conversionTime_ms;

    QString strText;

    if (showCustomNodes)
    {
        strText.sprintf(
            "Framerate: %.1f fps, ExposureTime: %.1f us, acquired: %d, errors: %d, incomplete: %d, dropped: %d, lost: %d",
            m_frameRate_Hz, m_conversionTime_ms, frameCounter, errorCounter, incomplete, dropped, lost);
    }
    else
    {
        strText.sprintf("Framerate: %.1f fps, ExposureTime: %.1f us, acquired: %d, errors: %d", m_frameRate_Hz,
            m_conversionTime_ms, frameCounter, errorCounter);
    }

    ui.label_2->setText(strText);
}
void Camera_op_ids::UpdateCounters_3(double frameTime_ms, double conversionTime_ms, unsigned int frameCounter,
    unsigned int errorCounter, int incomplete, int dropped, int lost, bool showCustomNodes)
{
    QString str;

    double frameRate_Hz = 0.0;
    if (frameTime_ms > 0)
    {
        frameRate_Hz = 1000.0 / frameTime_ms;
    }

    double m_frameRate_Hz = frameRate_Hz;
    double m_conversionTime_ms = conversionTime_ms;

    QString strText;

    if (showCustomNodes)
    {
        strText.sprintf(
            "Framerate: %.1f fps, ExposureTime: %.1f us, acquired: %d, errors: %d, incomplete: %d, dropped: %d, lost: %d",
            m_frameRate_Hz, m_conversionTime_ms, frameCounter, errorCounter, incomplete, dropped, lost);
    }
    else
    {
        strText.sprintf("Framerate: %.1f fps, ExposureTime: %.1f us, acquired: %d, errors: %d", m_frameRate_Hz,
            m_conversionTime_ms, frameCounter, errorCounter);
    }

    ui.label_3->setText(strText);
}
void Camera_op_ids::UpdateCounters_4(double frameTime_ms, double conversionTime_ms, unsigned int frameCounter,
    unsigned int errorCounter, int incomplete, int dropped, int lost, bool showCustomNodes)
{
    QString str;

    double frameRate_Hz = 0.0;
    if (frameTime_ms > 0)
    {
        frameRate_Hz = 1000.0 / frameTime_ms;
    }

    double m_frameRate_Hz = frameRate_Hz;
    double m_conversionTime_ms = conversionTime_ms;

    QString strText;

    if (showCustomNodes)
    {
        strText.sprintf(
            "Framerate: %.1f fps, ExposureTime: %.1f us, acquired: %d, errors: %d, incomplete: %d, dropped: %d, lost: %d",
            m_frameRate_Hz, m_conversionTime_ms, frameCounter, errorCounter, incomplete, dropped, lost);
    }
    else
    {
        strText.sprintf("Framerate: %.1f fps, ExposureTime: %.1f us, acquired: %d, errors: %d", m_frameRate_Hz,
            m_conversionTime_ms, frameCounter, errorCounter);
    }

    ui.label_4->setText(strText);
}
void Camera_op_ids::UpdateDisplay_1(QImage image, double time_now, double conversionTime_ms)
{
    scene_1->clear();
    scene_1->addPixmap(QPixmap::fromImage(image).scaled(ui.Camera_1->width() - 8, ui.Camera_1->height() - 8, Qt::KeepAspectRatio));
    ui.Camera_1->setScene(scene_1);
    if (if_continue_save)
    {
        Save_Thread* Sav = new Save_Thread;
        connect(Sav, SIGNAL(finished()), Sav, SLOT(deleteLater()));
        Sav->img = image;
        Sav->save_file = dir_save_every[0];
        Sav->num = save_file_num[0];
        Sav->mode = FALSE;
        save_file_num[0] ++;
        Sav->start();
        if (save_time_info)
        {
            QFile file(dir_save_txt[0]);
            file.open(QIODevice::ReadWrite);
            file.close();
            if (file.open(QIODevice::ReadWrite | QIODevice::Text))
            {
                QTextStream stream(&file);
                stream.seek(file.size());
                stream << (int)time_now << "\t\t" << conversionTime_ms << "\n";
                file.close();
            }
        }
    }
    else if(if_single_save[0])
    {
        save_flag[0] = save_flag[0] - 1;
        if (save_flag[0] == 0)
        {
            if_single_save[0] = FALSE;
        }
        Save_Thread* Sav = new Save_Thread;
        connect(Sav, SIGNAL(finished()), Sav, SLOT(deleteLater()));
        Sav->img = image;
        Sav->save_file = dir_save_every[0];
        Sav->num = single_save_num[0];
        Sav->mode = TRUE;
        single_save_num[0] ++;
        Sav->start();
        if (save_time_info)
        {
            QFile file(dir_save_txt[0]);
            file.open(QIODevice::ReadWrite);
            file.close();
            if (file.open(QIODevice::ReadWrite | QIODevice::Text))
            {
                QTextStream stream(&file);
                stream.seek(file.size());
                stream << (int)time_now << "\t\t" << conversionTime_ms << "\n";
                file.close();
            }
        }
    }
}
void Camera_op_ids::UpdateDisplay_2(QImage image, double time_now, double conversionTime_ms)
{
    scene_2->clear();
    scene_2->addPixmap(QPixmap::fromImage(image).scaled(ui.Camera_1->width() - 8, ui.Camera_1->height() - 8, Qt::KeepAspectRatio));
    ui.Camera_2->setScene(scene_2);
    if (if_continue_save)
    {
        Save_Thread* Sav = new Save_Thread;
        connect(Sav, SIGNAL(finished()), Sav, SLOT(deleteLater()));
        Sav->img = image;
        Sav->save_file = dir_save_every[1];
        Sav->num = save_file_num[1];
        Sav->mode = FALSE;
        save_file_num[1] ++;
        Sav->start();
        if (save_time_info)
        {
            QFile file(dir_save_txt[1]);
            file.open(QIODevice::ReadWrite);
            file.close();
            if (file.open(QIODevice::ReadWrite | QIODevice::Text))
            {
                QTextStream stream(&file);
                stream.seek(file.size());
                stream << (int)time_now << "\t\t" << conversionTime_ms << "\n";
                file.close();
            }
        }
    }
    else if(if_single_save[1])
    {
        save_flag[1] = save_flag[1] - 1;
        if (save_flag[1] == 0)
        {
            if_single_save[1] = FALSE;
        }
        Save_Thread* Sav = new Save_Thread;
        connect(Sav, SIGNAL(finished()), Sav, SLOT(deleteLater()));
        Sav->img = image;
        Sav->save_file = dir_save_every[1];
        Sav->num = single_save_num[1];
        Sav->mode = TRUE;
        single_save_num[1] ++;
        Sav->start();
        if (save_time_info)
        {
            QFile file(dir_save_txt[1]);
            file.open(QIODevice::ReadWrite);
            file.close();
            if (file.open(QIODevice::ReadWrite | QIODevice::Text))
            {
                QTextStream stream(&file);
                stream.seek(file.size());
                stream << (int)time_now << "\t\t" << conversionTime_ms << "\n";
                file.close();
            }
        }
    }
}
void Camera_op_ids::UpdateDisplay_3(QImage image, double time_now, double conversionTime_ms)
{
    scene_3->addPixmap(QPixmap::fromImage(image).scaled(ui.Camera_1->width() - 8, ui.Camera_1->height() - 8, Qt::KeepAspectRatio));
    ui.Camera_3->setScene(scene_3);
    if (if_continue_save)
    {
        Save_Thread* Sav = new Save_Thread;
        connect(Sav, SIGNAL(finished()), Sav, SLOT(deleteLater()));
        Sav->img = image;
        Sav->save_file = dir_save_every[2];
        Sav->num = save_file_num[2];
        Sav->mode = FALSE;
        save_file_num[2] ++;
        Sav->start();
        if (save_time_info)
        {
            QFile file(dir_save_txt[2]);
            file.open(QIODevice::ReadWrite);
            file.close();
            if (file.open(QIODevice::ReadWrite | QIODevice::Text))
            {
                QTextStream stream(&file);
                stream.seek(file.size());
                stream << (int)time_now << "\t\t" << conversionTime_ms << "\n";
                file.close();
            }
        }
    }
    else if(if_single_save[2])
    {
        save_flag[2] = save_flag[2] - 1;
        if (save_flag[2] == 0)
        {
            if_single_save[2] = FALSE;
        }
        Save_Thread* Sav = new Save_Thread;
        connect(Sav, SIGNAL(finished()), Sav, SLOT(deleteLater()));
        Sav->img = image;
        Sav->save_file = dir_save_every[2];
        Sav->num = single_save_num[2];
        Sav->mode = TRUE;
        single_save_num[2] ++;
        Sav->start();
        if (save_time_info)
        {
            QFile file(dir_save_txt[2]);
            file.open(QIODevice::ReadWrite);
            file.close();
            if (file.open(QIODevice::ReadWrite | QIODevice::Text))
            {
                QTextStream stream(&file);
                stream.seek(file.size());
                stream << (int)time_now << "\t\t" << conversionTime_ms << "\n";
                file.close();
            }
        }
    }
}
void Camera_op_ids::UpdateDisplay_4(QImage image, double time_now, double conversionTime_ms)
{
    scene_4->addPixmap(QPixmap::fromImage(image).scaled(ui.Camera_1->width() - 8, ui.Camera_1->height() - 8, Qt::KeepAspectRatio));
    ui.Camera_4->setScene(scene_4);
    if (if_continue_save)
    {
        Save_Thread* Sav = new Save_Thread;
        connect(Sav, SIGNAL(finished()), Sav, SLOT(deleteLater()));
        Sav->img = image;
        Sav->save_file = dir_save_every[3];
        Sav->num = save_file_num[3];
        Sav->mode = FALSE;
        save_file_num[3] ++;
        Sav->start();
        if (save_time_info)
        {
            QFile file(dir_save_txt[3]);
            file.open(QIODevice::ReadWrite);
            file.close();
            if (file.open(QIODevice::ReadWrite | QIODevice::Text))
            {
                QTextStream stream(&file);
                stream.seek(file.size());
                stream << (int)time_now << "\t\t" << conversionTime_ms << "\n";
                file.close();
            }
        }
    }
    else if (if_single_save[3])
    {
        save_flag[3]--;
        if (save_flag[3] == 0)
        {
            if_single_save[3] = FALSE;
        }
        Save_Thread* Sav = new Save_Thread;
        connect(Sav, SIGNAL(finished()), Sav, SLOT(deleteLater()));
        Sav->img = image;
        Sav->save_file = dir_save_every[3];
        Sav->num = single_save_num[3];
        Sav->mode = TRUE;
        single_save_num[3] ++;
        Sav->start();
        if (save_time_info)
        {
            QFile file(dir_save_txt[3]);
            file.open(QIODevice::ReadWrite);
            file.close();
            if (file.open(QIODevice::ReadWrite | QIODevice::Text))
            {
                QTextStream stream(&file);
                stream.seek(file.size());
                stream << (int)time_now << "\t\t" << conversionTime_ms << "\n";
                file.close();
            }
        }
    }
}

void Camera_op_ids::re_savepath()
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

void Camera_op_ids::save_image_single_click()
{
    for (int ii = 0; ii < m_nValidCamNum; ii++)
    {
        if_single_save[ii] = TRUE;
        save_flag[ii] = singlesave_num;
    }
}

void Camera_op_ids::begin_con_save()
{
    if (if_continue_save)
    {
        return;
    }
    if_continue_save = TRUE;
}

void Camera_op_ids::stop_con_save()
{
    if (!if_continue_save)
    {
        return;
    }
    if_continue_save = FALSE;
}

Camera_op_ids::~Camera_op_ids()
{

    for (int ii = 0; ii < m_nValidCamNum; ii++)
    {
        close_camera_single(ii);
        is_open[ii] = FALSE;
    }
    m_vecDevices.clear();
}




void Save_Thread::run()
{
    if (mode)
    {
        QString save_name = save_file + "\\Single_Save_";
        save_name = save_name + get_num(num);
        save_name = save_name + ".bmp";
        img.save(save_name);
    }
    else
    {
        QString save_name = save_file + "\\Continue_Save_";
        save_name = save_name + get_num(num);
        save_name = save_name + ".bmp";
        img.save(save_name);
    }
}
QString Save_Thread::get_num(unsigned int number)
{
    if (num < 10)
        return "00000" + QString::number(number);
    if ((num < 100) & (num>=10))
        return "0000" + QString::number(number);
    if ((num < 1000) &(num >= 100))
        return "000" + QString::number(number);
    if ((num < 10000) & (num >= 1000))
        return "00" + QString::number(number);
    if ((num < 100000) &(num >= 10000))
        return "0" + QString::number(number);
    if ((num < 1000000) & (num >= 100000))
        return QString::number(number);
}

Save_Thread::Save_Thread(QObject* parent)
{

}

Save_Thread::~Save_Thread()
{

}