#ifndef PRIVATECAPTURE_H
#define PRIVATECAPTURE_H

#include <QObject>
#include <QtMultimedia/QCamera>
#include <QtMultimedia/QCameraImageCapture>
#include <QtMultimedia/QtMultimedia>
#include <QtMultimediaWidgets/QtMultimediaWidgets>
#include <QImage>

class PrivateCapture : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    explicit PrivateCapture(QObject *parent = nullptr);

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
            QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;

    bool present(const QVideoFrame &frame);

signals:
    void frame(QImage img, quint64 id);

private:
    quint64 m_frameId = 0;
    QImage imageFromVideoFrame(const QVideoFrame &frame);

};


#endif // PRIVATECAPTURE_H
