/*!
 * \file    acquisitionworker.h
 * \author  IDS Imaging Development Systems GmbH
 * \date    2021-03-15
 * \since   1.1.6
 *
 * \brief   The AcquisitionWorker class is used in a worker thread to capture
 *          images from the device continuously and do an image conversion into
 *          a desired pixel format.
 *
 * \version 1.1.0
 *
 * Copyright (C) 2020 - 2021, IDS Imaging Development Systems GmbH.
 *
 * The information in this document is subject to change without notice
 * and should not be construed as a commitment by IDS Imaging Development Systems GmbH.
 * IDS Imaging Development Systems GmbH does not assume any responsibility for any errors
 * that may appear in this document.
 *
 * This document, or source code, is provided solely as an example of how to utilize
 * IDS Imaging Development Systems GmbH software libraries in a sample application.
 * IDS Imaging Development Systems GmbH does not assume any responsibility
 * for the use or reliability of any portion of this document.
 *
 * General permission to copy or modify is hereby granted.
 */

#ifndef ACQUISITIONWORKER_H
#define ACQUISITIONWORKER_H


#include <qgraphicsview.h>
#include <peak_ipl/peak_ipl.hpp>

#include <peak/peak.hpp>

#include <QImage>
#include <QLabel>
#include <QObject>

#include <cstdint>


class MainWindow;


class AcquisitionWorker : public QObject
{
    Q_OBJECT

public:
    AcquisitionWorker(QWidget* parent, QGraphicsView* displayWindow,
        std::shared_ptr<peak::core::DataStream> dataStream, std::shared_ptr<peak::core::NodeMap> Nodemap, peak::ipl::PixelFormatName pixelFormat,
        size_t imageWidth, size_t imageHeight, std::chrono::time_point<std::chrono::system_clock> begin_time);
    ~AcquisitionWorker();

    void Stop();

private:
    QWidget* m_parent;
    QGraphicsView* m_displayWindow;
    std::shared_ptr<peak::core::DataStream> m_dataStream;
    std::shared_ptr<peak::core::NodeMap> m_nodemapRemoteDevice;
    peak::ipl::PixelFormatName m_pixelFormat;
    size_t m_imageWidth;
    size_t m_imageHeight;
    std::chrono::time_point<std::chrono::system_clock> m_begintime;

    bool m_running;

    unsigned int m_frameCounter;
    unsigned int m_errorCounter;
    bool m_customNodesAvailable;

public slots:
    void Start();

signals:
    void ImageReceived(QImage image,double time_now, double conversionTime_ms);
    void UpdateCounters(double frameTime_ms, double conversionTime_ms, unsigned int frameCounter,
        unsigned int errorCounter, int incomplete, int dropped, int lost, bool showCustomNodes);
};

#endif // ACQUISITIONWORKER_H
