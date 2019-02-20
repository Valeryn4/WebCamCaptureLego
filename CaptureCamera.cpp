#include "CaptureCamera.h"
#include "PrivateCapture.h"

#include <QDialog>
#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QWidget>



CaptureCamera::CaptureCamera(QObject *parent) :
    QObject(parent),
    m_camera(nullptr),
    m_capture(nullptr)
{
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();


    {
        auto dialog = new QDialog();
        auto layoutDialog = new QVBoxLayout();
        dialog->setLayout(layoutDialog);
        auto selecDialog = new QDialogButtonBox();
        layoutDialog->addWidget(selecDialog);

        for (auto &&cameraInfo : cameras)
        {
            auto button = new QPushButton(cameraInfo.description(), selecDialog);

            QObject::connect(button, &QPushButton::clicked, [=]()
            {
                m_camera = new QCamera(cameraInfo, this);
                dialog->accept();

            });

            selecDialog->addButton(button, QDialogButtonBox::ButtonRole::AcceptRole);
        }

        dialog->exec();
    }

    Q_ASSERT(m_camera != nullptr);

    m_capture = new QCameraImageCapture(m_camera);

    m_camera->setCaptureMode( QCamera::CaptureVideo );
    auto captureProcessing = new PrivateCapture(this);
    m_camera->setViewfinder(captureProcessing);

    QObject::connect(captureProcessing, &PrivateCapture::frame, this, &CaptureCamera::frame);

    startCapture();

    {
        QList<QSize> resolutions = m_capture->supportedResolutions();
        qDebug() << "resolution count " << resolutions.size();
        for (auto &&size : resolutions)
        {
            qDebug() << " Resoution " << size;
        }

        QCameraViewfinderSettings viewfinderSettings = m_camera->viewfinderSettings();

        qDebug() << "Max FPS: " << viewfinderSettings.maximumFrameRate()
                 << "\nMinFPS: " << viewfinderSettings.minimumFrameRate()
                 << "\nResolution: " << viewfinderSettings.resolution();
        //viewfinderSettings.setMinimumFrameRate(1.0);
        //viewfinderSettings.setMaximumFrameRate(15.0);

        {
            auto dialog = new QDialog();
            auto layoutDialog = new QVBoxLayout();
            dialog->setLayout(layoutDialog);
            auto selecDialog = new QDialogButtonBox();
            layoutDialog->addWidget(selecDialog);

            for (auto &&res : resolutions)
            {
                auto button = new QPushButton(QString("%1x%2").arg(res.width()).arg(res.height()), selecDialog);

                QObject::connect(button, &QPushButton::clicked, [&]()
                {
                    viewfinderSettings.setResolution(res);
                    dialog->accept();

                });

                selecDialog->addButton(button, QDialogButtonBox::ButtonRole::AcceptRole);
            }

            dialog->exec();
        }

        m_camera->setViewfinderSettings(viewfinderSettings);
    }

}

CaptureCamera::~CaptureCamera()
{
    m_camera->stop();
    m_camera->unload();
}

void CaptureCamera::startCapture()
{

    //qDebug() << "OK 5";
    Q_ASSERT(m_camera->isAvailable());
    m_camera->load();
    m_camera->start();
}

void CaptureCamera::stopCapture()
{
    m_camera->stop();
    m_camera->unload();
}

