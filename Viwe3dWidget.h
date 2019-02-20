#ifndef VIWE3DWIDGET_H
#define VIWE3DWIDGET_H

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DInput/QMouseEvent>
#include <Qt3DRender/QRenderCapture>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DRender/QBlendEquationArguments>

#include <QHash>
#include <QVector>
#include <QVector2D>
#include <QWidget>
#include <QString>
#include <QQuaternion>
#include <QImage>


class PrivatePaintedImage;


class Viwe3dWidget : public QWidget
{
    Q_OBJECT
public:
    enum LegoType
    {
        LEGO_1X1_L = 0,
        LEGO_1X1_S,
        LEGO_1X2_L,
        LEGO_1X2_S,
        LEGO_1X3_L,
        LEGO_1X3_S,
        LEGO_1X4_L,
        LEGO_1X4_S,
        LEGO_2X2_L,
        LEGO_2X2_S,
        LEGO_2X3_L,
        LEGO_2X3_S,
        LEGO_2X4_L,
        LEGO_2X4_S,

        COUNT,
        NONE = COUNT
    };


    struct EntityRaw
    {
        QVector3D Pos;
        QQuaternion Rot;
        LegoType Type;
        QColor Color;
    };
    using ListEntityRaw = QList<EntityRaw>;

    explicit Viwe3dWidget(QWidget *parent = nullptr);
    explicit Viwe3dWidget(const ListEntityRaw &list, QWidget *parent = nullptr);
    using Blending = Qt3DRender::QBlendEquationArguments::Blending;




    using EntityAndTransform = QPair<Qt3DCore::QEntity*, Qt3DCore::QTransform*>;
    using EntityMap = QHash<LegoType,EntityAndTransform>;

    void getScreen(bool showLego = false);

    QQuaternion getRotationEntity() const;
    QVector3D getPositionEntity() const;
    LegoType getLegoType() const;


signals:
    void entityRotationTo(const QQuaternion &q);
    void enditytTransformTo(const QVector3D &v);
    void screenComplited(QImage img1, QImage img2);

public slots:
    void mouseClicked(Qt3DInput::QMouseEvent *mouse); //TODO - кнопки!
    void mouseMoved(Qt3DInput::QMouseEvent *mouse);
    void wheelEvent(Qt3DInput::QWheelEvent *wheel); //TODO КОЛЕСО!

    void setEntityRotationTo(const QQuaternion &q);
    void setEntityPositionTo(const QVector3D &v);
    void setCurrentEntity(LegoType type);
    void setCurrentEntityNone();
    void frameUpdate(QImage img);
    void setColor(const QColor &color);
    void setAlpha(float a);
    void setSpecular(const QColor &color);
    void setShininess(float s);
    void setDiffuse(const QColor &color);
    void setAmbiend(const QColor &color);
    void setBlendingSrc(Blending blend);
    void setBlendingDst(Blending blend);
    void onCaptureCompleted();

private:
    void addEntity(const QString &fileName, LegoType type);
    Qt3DCore::QEntity* createEntity(LegoType type, const QVector3D &pos, const QQuaternion& rot, const QColor &color);
    void initScene();
    void initMouse();
    void initBGImage();
    void initGroup();
    void initEntity();

    friend class PrivatePaintedImage;
    PrivatePaintedImage *m_textureImg = nullptr;

    Qt3DCore::QEntity *m_rootEntity = nullptr;
    Qt3DRender::QRenderCapture *m_capture = nullptr;
    Qt3DRender::QRenderCaptureReply *m_reply = nullptr;

    Qt3DCore::QEntity *m_groupEntity = nullptr;
    Qt3DCore::QTransform *m_groupTransform = nullptr;
    Qt3DExtras::Qt3DWindow *m_view = nullptr;
    EntityMap m_entityMap = {};
    LegoType m_currentEntity = LegoType::NONE;
    LegoType m_replyPrev = LegoType::NONE;
    QVector2D m_mousePosition = {0,0};
    QQuaternion m_currentEntityRotation = {0,0,0,0};
    QVector3D m_position = {0,0,0};

    //size_t __padding : 32;
    QImage m_screenLego;
    QImage m_screenNoLego;


    QMetaObject::Connection m_captureConnection;
    bool m_showLego = false;

    //size_t __padding : 32;

};

#endif // VIWE3DWIDGET_H
