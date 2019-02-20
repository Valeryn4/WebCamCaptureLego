#ifndef CAPTURECAMERA_H
#define CAPTURECAMERA_H


#include <QObject>
#include <QtMultimedia/QCamera>
#include <QtMultimedia/QCameraImageCapture>
#include <QtMultimedia/QtMultimedia>
#include <QtMultimediaWidgets/QtMultimediaWidgets>
#include <QImage>

class PrivateCapture;
class CaptureCamera : public QObject
{
    Q_OBJECT
public:
    explicit CaptureCamera(QObject *parent = nullptr);
    ~CaptureCamera();

signals:
    void frame(QImage img, quint64 frameID);

public slots:
    void startCapture();
    void stopCapture();

private:
    QCamera *m_camera;
    QCameraImageCapture *m_capture;

};

#endif // CAPTURECAMERA_H
