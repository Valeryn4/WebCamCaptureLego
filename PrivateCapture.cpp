#include "PrivateCapture.h"



PrivateCapture::PrivateCapture(QObject *parent) :
    QAbstractVideoSurface(parent),
    m_frameId(0)
{}

QList<QVideoFrame::PixelFormat> PrivateCapture::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    //qDebug() << "support pixel formats";
    Q_UNUSED(handleType);
    /*
    return QList<QVideoFrame::PixelFormat>()
            << QVideoFrame::Format_ARGB32
            << QVideoFrame::Format_ARGB32_Premultiplied
            << QVideoFrame::Format_RGB32
            << QVideoFrame::Format_RGB24
            << QVideoFrame::Format_RGB565
            << QVideoFrame::Format_RGB555
            << QVideoFrame::Format_ARGB8565_Premultiplied
            << QVideoFrame::Format_BGRA32
            << QVideoFrame::Format_BGRA32_Premultiplied
            << QVideoFrame::Format_BGR32
            << QVideoFrame::Format_BGR24
            << QVideoFrame::Format_BGR565
            << QVideoFrame::Format_BGR555
            << QVideoFrame::Format_BGRA5658_Premultiplied
            << QVideoFrame::Format_AYUV444
            << QVideoFrame::Format_AYUV444_Premultiplied
            << QVideoFrame::Format_YUV444
            << QVideoFrame::Format_YUV420P
            << QVideoFrame::Format_YV12
            << QVideoFrame::Format_UYVY
            << QVideoFrame::Format_YUYV
            << QVideoFrame::Format_NV12
            << QVideoFrame::Format_NV21
            << QVideoFrame::Format_IMC1
            << QVideoFrame::Format_IMC2
            << QVideoFrame::Format_IMC3
            << QVideoFrame::Format_IMC4
            << QVideoFrame::Format_Y8
            << QVideoFrame::Format_Y16
            << QVideoFrame::Format_Jpeg
            << QVideoFrame::Format_CameraRaw
            << QVideoFrame::Format_AdobeDng;
            */
    Q_UNUSED(handleType);
        return QList<QVideoFrame::PixelFormat>()
            << QVideoFrame::Format_RGB24
            << QVideoFrame::Format_Jpeg
            << QVideoFrame::Format_RGB32
            << QVideoFrame::Format_ARGB32
        ;
}

bool PrivateCapture::present(const QVideoFrame &frame)
{
    m_frameId++;
    if ((m_frameId % 10) == 0)
    {

        return true;
    }
    //qDebug() << "present";
    if (frame.isValid()) {
        QImage img = imageFromVideoFrame(frame);

        img = img.convertToFormat(QImage::Format_RGB888);
        if (surfaceFormat().scanLineDirection() == QVideoSurfaceFormat::BottomToTop)
            img = img.transformed(QTransform().scale(1,-1).translate(0, img.width()));


        //qDebug() << "emit frame";
        emit this->frame(img.copy(), m_frameId);

    }

    return true;
}

QImage PrivateCapture::imageFromVideoFrame(const QVideoFrame &buffer)
{
    QImage img;
    QVideoFrame frame(buffer);  // make a copy we can call map (non-const) on
    frame.map(QAbstractVideoBuffer::ReadOnly);
    QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(
                frame.pixelFormat());
    // BUT the frame.pixelFormat() is QVideoFrame::Format_Jpeg, and this is
    // mapped to QImage::Format_Invalid by
    // QVideoFrame::imageFormatFromPixelFormat
    if (imageFormat != QImage::Format_Invalid) {
        img = QImage(frame.bits(),
                     frame.width(),
                     frame.height(),
                     // frame.bytesPerLine(),
                     imageFormat);
    } else {
        // e.g. JPEG
        int nbytes = frame.mappedBytes();
        img = QImage::fromData(frame.bits(), nbytes);
    }
    frame.unmap();
    return img;

}
