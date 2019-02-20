#include "Viwe3dWidget.h"

#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QMesh>
#include <Qt3DInput/QMouseEvent>
#include <QGuiApplication>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraLens>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QCommandLinkButton>
#include <QtGui/QScreen>
#include <Qt3DInput/QInputAspect>
#include <Qt3DInput/QMouseDevice>
#include <Qt3DInput/QMouseHandler>
#include <Qt3DInput/QMouseEvent>
#include <Qt3DRender/QMesh>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QSceneLoader>
#include <Qt3DRender/QPointLight>
#include <Qt3DCore/QAspectEngine>
#include <Qt3DRender/QRenderAspect>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QFirstPersonCameraController>
#include <Qt3DExtras/QPlaneMesh>
#include <Qt3DExtras/QCuboidMesh>
#include <Qt3DExtras/QTextureMaterial>
#include <Qt3DExtras/QNormalDiffuseSpecularMapMaterial>
#include <Qt3DExtras/QNormalDiffuseMapMaterial>
#include <Qt3DExtras/QDiffuseMapMaterial>
#include <Qt3DExtras/QDiffuseMapMaterial>
#include <Qt3DExtras/QPhongAlphaMaterial>
#include <Qt3DExtras/QPerVertexColorMaterial>
#include <Qt3DExtras/QGoochMaterial>
#include <Qt3DRender/QRenderCapture>
#include <Qt3DRender/QRenderCaptureReply>
#include <Qt3DRender/QPaintedTextureImage>
#include <Qt3DRender/QTextureImageData>
#include <QColor>
#include <QMaterial>
#include <QPainter>
#include <QImage>



class PrivatePaintedImage : public Qt3DRender::QPaintedTextureImage
{
public:
    PrivatePaintedImage(Qt3DCore::QNode *parent = nullptr) :
        Qt3DRender::QPaintedTextureImage(parent)
    {}
    ~PrivatePaintedImage() override
    {}

    void paint(QPainter * painter) override
    {
        //qDebug() << "draw frame";
        if (painter == nullptr)
            return;
        int w = painter->device()->width();
        int h = painter->device()->height();

        if (!m_image.isNull() && m_image.format() != QImage::Format::Format_Invalid)
            painter->drawImage(m_image.rect(), m_image, QRect(0,0,w,h));

    }

    void setImage(const QImage &img)
    {
        m_image = img.mirrored(true, false);

        setSize(img.size());
        update();
    }
    QImage m_image;
};


Viwe3dWidget::Viwe3dWidget(QWidget *parent) :
    QWidget(parent)
{

    initScene();
    initMouse();
    initBGImage();
    initGroup();
    initEntity();
    m_currentEntity = NONE;
    setCurrentEntity(LEGO_1X2_L);
}

Viwe3dWidget::Viwe3dWidget(const Viwe3dWidget::ListEntityRaw &list, QWidget *parent) :
    QWidget(parent)
{
    initScene();
    initBGImage();
    initGroup();

    for (auto &&v : list)
    {
        auto entity = createEntity(v.Type, v.Pos, v.Rot, v.Color);
        entity->setEnabled(true);
    }

    m_currentEntity = NONE;
}

void Viwe3dWidget::getScreen(bool showLego)
{
    //qDebug() << "getScreen";
    if (m_reply != nullptr)
        return;

    m_replyPrev = m_currentEntity;
    if (!showLego)
    {
        setCurrentEntityNone();
        m_showLego = true;
    }
    m_reply = m_capture->requestCapture();
    m_captureConnection = QObject::connect(m_reply, &Qt3DRender::QRenderCaptureReply::completed,
                                           this, &Viwe3dWidget::onCaptureCompleted);

}

QQuaternion Viwe3dWidget::getRotationEntity() const
{
    if (m_groupTransform == nullptr)
        return {};
    return m_groupTransform->rotation();
}

QVector3D Viwe3dWidget::getPositionEntity() const
{
    if (m_groupTransform == nullptr)
        return {};
    return m_groupTransform->translation();
}

Viwe3dWidget::LegoType Viwe3dWidget::getLegoType() const
{
    return m_currentEntity;
}

void Viwe3dWidget::mouseClicked(Qt3DInput::QMouseEvent *mouse)
{
    if (mouse->buttons() == Qt::NoButton)
        return;

    m_mousePosition = QVector2D(mouse->x(), mouse->y());
}

void Viwe3dWidget::mouseMoved(Qt3DInput::QMouseEvent *mouse)
{
    if (mouse->buttons() == Qt::NoButton)
        return;
    if (m_currentEntity == NONE)
        return;

    //TODO вращение
    if (mouse->buttons() == Qt::LeftButton)
    {
        QVector2D diff = QVector2D(mouse->x(), mouse->y()) - m_mousePosition;
        m_mousePosition = QVector2D(mouse->x(), mouse->y());
        float angle = diff.length() / 2.0f;

        QVector3D axis = {diff.y(), diff.x(), 0.f};

        QQuaternion rotation = QQuaternion::fromAxisAndAngle(axis, angle) * m_groupTransform->rotation();

        setEntityRotationTo(rotation);
        emit entityRotationTo(m_currentEntityRotation);
    }
    //TODO перемещение
    else if (mouse->buttons() == Qt::RightButton)
    {
        QVector2D diff = QVector2D(mouse->x(), mouse->y()) - m_mousePosition;
        m_mousePosition = QVector2D(mouse->x(), mouse->y());
        diff *= 0.01f;

        QVector3D translation = m_groupTransform->translation() + QVector3D(diff.x(), -diff.y(), 0);

        setEntityPositionTo(translation);
        emit enditytTransformTo(translation);
    }


    if (m_textureImg != nullptr)
         m_textureImg->update();


}

void Viwe3dWidget::wheelEvent(Qt3DInput::QWheelEvent *wheel)
{
    auto dt = wheel->angleDelta();
    float y = dt.y() * 0.005f;
    auto pos = m_groupTransform->translation();
    pos = QVector3D(pos.x(), pos.y(), pos.z() + y);

    setEntityPositionTo(pos);
    emit enditytTransformTo(pos);
}

void Viwe3dWidget::setEntityRotationTo(const QQuaternion &q)
{
    if (m_groupTransform == nullptr)
        return;
    m_currentEntityRotation = q;
    m_groupTransform->setRotation(q);
}

void Viwe3dWidget::setEntityPositionTo(const QVector3D &v)
{
    if (m_groupTransform == nullptr)
        return;
   m_groupTransform->setTranslation(v);
}

void Viwe3dWidget::setCurrentEntity(Viwe3dWidget::LegoType type)
{
    if (m_entityMap.empty())
        return;

    if (m_currentEntity != NONE)
    {
        m_entityMap[m_currentEntity].first->setEnabled(false);
    }

    if (type == NONE)
    {
        return;
    }

    m_entityMap[type].first->setEnabled(true);
    m_currentEntity = type;

}

void Viwe3dWidget::setCurrentEntityNone()
{
    setCurrentEntity(NONE);
}

void Viwe3dWidget::frameUpdate(QImage img)
{
    //qDebug() << "frame update";
    if (m_textureImg == nullptr)
        return;
    m_textureImg->setImage(img);

    //qDebug() << "frame update OK";
    m_textureImg->update();
}

void Viwe3dWidget::setColor(const QColor &color)
{

    for (int i = 0; i < int(LegoType::COUNT); ++i)
    {
        auto compVect = m_entityMap[LegoType(i)].first->components();
        for (auto &comp : compVect)
        {
            auto obj = qobject_cast<Qt3DExtras::QPhongAlphaMaterial*>(comp);
            if (obj != nullptr)
            {
                obj->setDiffuse(color);
            }
        }
    }
}

void Viwe3dWidget::setAlpha(float a)
{
    for (int i = 0; i < int(LegoType::COUNT); ++i)
    {
        auto compVect = m_entityMap[LegoType(i)].first->components();
        for (auto &comp : compVect)
        {
            auto obj = qobject_cast<Qt3DExtras::QPhongAlphaMaterial*>(comp);
            if (obj != nullptr)
            {
                obj->setAlpha(a);
            }
        }
    }
}

void Viwe3dWidget::setSpecular(const QColor &color)
{
    for (int i = 0; i < int(LegoType::COUNT); ++i)
    {
        auto compVect = m_entityMap[LegoType(i)].first->components();
        for (auto &comp : compVect)
        {
            auto obj = qobject_cast<Qt3DExtras::QPhongAlphaMaterial*>(comp);
            if (obj != nullptr)
            {
                obj->setSpecular(color);
            }
        }
    }
}

void Viwe3dWidget::setShininess(float s)
{
    for (int i = 0; i < int(LegoType::COUNT); ++i)
    {
        auto compVect = m_entityMap[LegoType(i)].first->components();
        for (auto &comp : compVect)
        {
            auto obj = qobject_cast<Qt3DExtras::QPhongAlphaMaterial*>(comp);
            if (obj != nullptr)
            {
                obj->setShininess(s);
            }
        }
    }
}

void Viwe3dWidget::setDiffuse(const QColor &color)
{
    for (int i = 0; i < int(LegoType::COUNT); ++i)
    {
        auto compVect = m_entityMap[LegoType(i)].first->components();
        for (auto &comp : compVect)
        {
            auto obj = qobject_cast<Qt3DExtras::QPhongAlphaMaterial*>(comp);
            if (obj != nullptr)
            {
                obj->setDiffuse(color);
            }
        }
    }
}

void Viwe3dWidget::setAmbiend(const QColor &color)
{
    for (int i = 0; i < int(LegoType::COUNT); ++i)
    {
        auto compVect = m_entityMap[LegoType(i)].first->components();
        for (auto &comp : compVect)
        {
            auto obj = qobject_cast<Qt3DExtras::QPhongAlphaMaterial*>(comp);
            if (obj != nullptr)
            {
                obj->setAmbient(color);
            }
        }
    }
}

void Viwe3dWidget::setBlendingSrc(Viwe3dWidget::Blending blend)
{
    for (int i = 0; i < int(LegoType::COUNT); ++i)
    {
        auto compVect = m_entityMap[LegoType(i)].first->components();
        for (auto &comp : compVect)
        {
            auto obj = qobject_cast<Qt3DExtras::QPhongAlphaMaterial*>(comp);
            if (obj != nullptr)
            {
                obj->setSourceRgbArg(blend);
            }
        }

    }
}

void Viwe3dWidget::setBlendingDst(Viwe3dWidget::Blending blend)
{
    for (int i = 0; i < int(LegoType::COUNT); ++i)
    {
        auto compVect = m_entityMap[LegoType(i)].first->components();
        for (auto &comp : compVect)
        {
            auto obj = qobject_cast<Qt3DExtras::QPhongAlphaMaterial*>(comp);
            if (obj != nullptr)
            {
                obj->setDestinationRgbArg(blend);


            }
        }
    }
}

void Viwe3dWidget::onCaptureCompleted()
{
    QObject::disconnect(m_captureConnection);
    auto img = m_reply->image();


    //TODO CONVERTING
    /*
    if (img.hasAlphaChannel()
            && img.data_ptr())
    {
            switch (img.format()) {
            case QImage::Format_RGBA8888:
            case QImage::Format_RGBA8888_Premultiplied:
                img = img.convertToFormat(QImage::Format_RGBX8888);
                break;
            case QImage::Format_A2BGR30_Premultiplied:
                img = img.convertToFormat(QImage::Format_BGR30);
                break;
            case QImage::Format_A2RGB30_Premultiplied:
                img = img.convertToFormat(QImage::Format_RGB30);
                break;
            default:
                img = img.convertToFormat(QImage::Format_RGB32);
                break;
            }
        }

        */

    //img = img.convertToFormat(QImage::Format_RGB888);
    delete m_reply;
    m_reply = nullptr;

    setCurrentEntity(m_replyPrev);

    if (m_showLego)
    {
        getScreen(true);
        m_showLego = false;
        m_screenNoLego = img;
    }
    else
    {
        m_screenLego = img;
        emit screenComplited(m_screenLego, m_screenNoLego);
    }
}

void Viwe3dWidget::addEntity(const QString &fileName, Viwe3dWidget::LegoType type)
{
    //qDebug() << "add Entity " << fileName;
    Qt3DRender::QMesh *legoMesh = new Qt3DRender::QMesh();
    legoMesh->setSource(QUrl(fileName));

    //  transform
    auto legoTransform  = new Qt3DCore::QTransform();

    legoTransform->setScale(0.2f);
    legoTransform->setTranslation(QVector3D(0,0,0));

    auto *legoMaterial = new Qt3DExtras::QPhongAlphaMaterial();
    legoMaterial->setDiffuse(QColor::fromRgbF(0.5, 0.5, 0.5, 0.));
    legoMaterial->setAmbient(QColor::fromRgbF(0.5, 0.5, 0.5, 0.));
    legoMaterial->setSpecular(QColor::fromRgbF(0.5, 0.5, 0.5, 0.));
    legoMaterial->setShininess(0.f);
    legoMaterial->setAlpha(0.5f);

    //legoMaterial->setSourceAlphaArg(Blending::ConstantAlpha );
    //legoMaterial->setSourceRgbArg(Blending::One);
    //legoMaterial->setDestinationAlphaArg(Blending::SourceAlpha);
    //legoMaterial->setDestinationRgbArg(Blending::One);

    /**
    Qt3DReder::QBlendEquationArguments::Blending sourceRgbArg() const;
    Qt3DRender::QBlendEquationArguments::Blending destinationRgbArg() const;
    Qt3DRender::QBlendEquationArguments::Blending sourceAlphaArg() const;
    Qt3DRender::QBlendEquationArguments::Blending destinationAlphaArg() const;
    Qt3DRender::QBlendEquation::BlendFunction blendFunctionArg() const;
      */

    // entity
    auto legoEntity = new Qt3DCore::QEntity(m_groupEntity);
    legoEntity->addComponent(legoMesh);
    legoEntity->addComponent(legoMaterial);
    legoEntity->addComponent(legoTransform);

    m_entityMap.insert(type, {legoEntity, legoTransform});
    legoEntity->setEnabled(false);
}

Qt3DCore::QEntity *Viwe3dWidget::createEntity(Viwe3dWidget::LegoType type, const QVector3D &pos, const QQuaternion &rot, const QColor &color)
{
    QString filename;

    switch (type) {
    case LEGO_1X1_S:
        filename = QStringLiteral("qrc:/LegoObject/LEGO-1X1-S_simple.stl");// = LEGO_1X1_S);
        break;
        case LEGO_1X1_L:
        filename = QStringLiteral("qrc:/LegoObject/LEGO-1X1-L_simple.stl");// = LEGO_1X1_L);

        break;
        case LEGO_1X2_S:
        filename = QStringLiteral("qrc:/LegoObject/LEGO-1X2-S_simple.stl");// = LEGO_1X2_S);
        break;
        case LEGO_1X2_L:
        filename = QStringLiteral("qrc:/LegoObject/LEGO-1X2-L_simple.stl");// = LEGO_1X2_L);
        break;
        case LEGO_1X3_S:
        filename = QStringLiteral("qrc:/LegoObject/LEGO-1X3-S_simple.stl");// = LEGO_1X3_S);
        break;
        case LEGO_1X3_L:
        filename = QStringLiteral("qrc:/LegoObject/LEGO-1X3-L_simple.stl");// = LEGO_1X3_L);
        break;
        case LEGO_1X4_S:
        filename = QStringLiteral("qrc:/LegoObject/LEGO-1X4-S_simple.stl");// = LEGO_1X4_S);
        break;
        case LEGO_1X4_L:
        filename = QStringLiteral("qrc:/LegoObject/LEGO-1X4-L_simple.stl");// = LEGO_1X4_L);
        break;
        case LEGO_2X2_S:
        filename = QStringLiteral("qrc:/LegoObject/LEGO-2X2-S_simple.stl");// = LEGO_2X2_S);
        break;
        case LEGO_2X2_L:
        filename = QStringLiteral("qrc:/LegoObject/LEGO-2X2-L_simple.stl");// = LEGO_2X2_L);
        break;
        case LEGO_2X3_S:
        filename = QStringLiteral("qrc:/LegoObject/LEGO-2X3-S_simple.stl");// = LEGO_2X3_S);
        break;
        case LEGO_2X3_L:
        filename = QStringLiteral("qrc:/LegoObject/LEGO-2X3-L_simple.stl");// = LEGO_2X3_L);
        break;
        case LEGO_2X4_S:
        filename = QStringLiteral("qrc:/LegoObject/LEGO-2X4-S_simple.stl");// = LEGO_2X4_S);
        break;
        case LEGO_2X4_L:
        filename = QStringLiteral("qrc:/LegoObject/LEGO-2X4-L_simple.stl");// = LEGO_2X4_L);
        break;
    default:
        break;
    }

    //qDebug() << "add Entity " << fileName;
    Qt3DRender::QMesh *legoMesh = new Qt3DRender::QMesh();
    legoMesh->setPrimitiveType(Qt3DRender::QGeometryRenderer::PrimitiveType::LineLoop);
    legoMesh->setSource(QUrl(filename));
    legoMesh->vertexCount();

    //  transform
    auto legoTransform  = new Qt3DCore::QTransform();

    legoTransform->setScale(0.2f);
    legoTransform->setTranslation(pos);
    legoTransform->setRotation(rot);

    auto *legoMaterial = new Qt3DExtras::QGoochMaterial(); //new Qt3DExtras::QPhongAlphaMaterial();
    //legoMaterial->setAlpha(0.1f);
    //legoMaterial->setBeta(0.1f);
    //legoMaterial->setWarm(QColor("red"));
    //legoMaterial->setCool(QColor("blue"));

    legoMaterial->setDiffuse(color);
    //legoMaterial->setAmbient(color);
    legoMaterial->setSpecular(color);
    legoMaterial->setShininess(0.f);
    legoMaterial->setAlpha(0.5f);

    // entity
    auto legoEntity = new Qt3DCore::QEntity(m_rootEntity);
    legoEntity->addComponent(legoMesh);
    legoEntity->addComponent(legoMaterial);
    legoEntity->addComponent(legoTransform);

    legoEntity->setEnabled(false);
    return legoEntity;
}

void Viwe3dWidget::initScene()
{
    //qDebug() << " Render step 1";
    m_position = QVector3D(0,0,0);
    Qt3DExtras::Qt3DWindow *view = new Qt3DExtras::Qt3DWindow();
    m_view = view;
    Q_ASSERT(m_view != nullptr);
    view->defaultFrameGraph()->setClearColor(QColor::fromRgb(255,255,255, 255));
    QWidget *container = QWidget::createWindowContainer(view);
    QSize screenSize = view->screen()->size();
    container->setMinimumSize(QSize(1280, 720));
    container->setMaximumSize(screenSize);


    //qDebug() << " Render step 2";
    Q_ASSERT(container != nullptr);

    QGridLayout* gridLayout = new QGridLayout(this);
    gridLayout->addWidget(container);


    //qDebug() << " Render step 3";
    Qt3DInput::QInputAspect *input = new Qt3DInput::QInputAspect;

    view->registerAspect(input);

    // Root entity
    m_rootEntity = new Qt3DCore::QEntity();
    view->setRootEntity(m_rootEntity);

    //Capture
    m_capture = new Qt3DRender::QRenderCapture(m_rootEntity);

    view->activeFrameGraph()->setParent(m_capture);
    view->setActiveFrameGraph(m_capture);


    //qDebug() << " Render step 4";
    // Camera
    Qt3DRender::QCamera *cameraEntity = view->camera();

    cameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    cameraEntity->setPosition(QVector3D(0, 0, 20.0f));
    cameraEntity->setUpVector(QVector3D(0, 1, 0));
    cameraEntity->setViewCenter(QVector3D(0, 0, 0));


    //qDebug() << " Render step 5";
    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DRender::QPointLight *light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor("white");
    light->setIntensity(1);
    Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform(lightEntity);
    lightEntity->addComponent(light);
    lightTransform->setTranslation(cameraEntity->position());
    lightEntity->addComponent(lightTransform);


}

void Viwe3dWidget::initMouse()
{
    Qt3DInput::QMouseDevice *mouse = new Qt3DInput::QMouseDevice(m_rootEntity);


    Qt3DInput::QMouseHandler *handler = new Qt3DInput::QMouseHandler(m_rootEntity);
    handler->setSourceDevice(mouse);

    //qDebug() << " Render step 7";
    QObject::connect(handler, &Qt3DInput::QMouseHandler::positionChanged,
                     this, &Viwe3dWidget::mouseMoved);
    QObject::connect(handler, &Qt3DInput::QMouseHandler::clicked,
                     this, &Viwe3dWidget::mouseClicked);
    QObject::connect(handler, &Qt3DInput::QMouseHandler::pressed,
                     this, &Viwe3dWidget::mouseClicked);
    QObject::connect(handler, &Qt3DInput::QMouseHandler::wheel,
                     this, &Viwe3dWidget::wheelEvent);
}

void Viwe3dWidget::initBGImage()
{

    m_textureImg = new PrivatePaintedImage();
    QImage empty(m_view->size(), QImage::Format_RGB888);
    empty.fill(QColor::fromRgbF(0,0,0,1));
    m_textureImg->setImage(empty);

    auto planeImageMesh = new Qt3DExtras::QPlaneMesh();
    planeImageMesh->setHeight(100);
    planeImageMesh->setWidth(180);

    auto planeImageMaterial = new Qt3DExtras::QTextureMaterial();
    planeImageMaterial->texture()->addTextureImage(m_textureImg);

    auto planeImageTransform = new Qt3DCore::QTransform();
    planeImageTransform->setScale(1.0f);
    planeImageTransform->setTranslation(QVector3D(0.0f, 0.0f, -100.0f));
    planeImageTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), 90.0f));


    auto planeEntity = new Qt3DCore::QEntity(m_rootEntity);
    planeEntity->addComponent(planeImageMesh);
    planeEntity->addComponent(planeImageMaterial);
    planeEntity->addComponent(planeImageTransform);

    planeEntity->setEnabled(true);

}

void Viwe3dWidget::initGroup()
{

    m_groupEntity = new Qt3DCore::QEntity(m_rootEntity);


    m_groupTransform = new Qt3DCore::QTransform();
    m_position = QVector3D(0, 0, 0); //TODO - начальная позиция!(-10,5,0)
    m_groupTransform->setTranslation(m_position);

    m_groupEntity->addComponent(m_groupTransform);

}

void Viwe3dWidget::initEntity()
{
    addEntity(QStringLiteral("qrc:/LegoObject/LEGO-1X1-S_simple.stl"), LEGO_1X1_S);
    addEntity(QStringLiteral("qrc:/LegoObject/LEGO-1X1-L_simple.stl"), LEGO_1X1_L);
    addEntity(QStringLiteral("qrc:/LegoObject/LEGO-1X2-S_simple.stl"), LEGO_1X2_S);
    addEntity(QStringLiteral("qrc:/LegoObject/LEGO-1X2-L_simple.stl"), LEGO_1X2_L);
    addEntity(QStringLiteral("qrc:/LegoObject/LEGO-1X3-S_simple.stl"), LEGO_1X3_S);
    addEntity(QStringLiteral("qrc:/LegoObject/LEGO-1X3-L_simple.stl"), LEGO_1X3_L);
    addEntity(QStringLiteral("qrc:/LegoObject/LEGO-1X4-S_simple.stl"), LEGO_1X4_S);
    addEntity(QStringLiteral("qrc:/LegoObject/LEGO-1X4-L_simple.stl"), LEGO_1X4_L);


    addEntity(QStringLiteral("qrc:/LegoObject/LEGO-2X2-S_simple.stl"), LEGO_2X2_S);
    addEntity(QStringLiteral("qrc:/LegoObject/LEGO-2X2-L_simple.stl"), LEGO_2X2_L);
    addEntity(QStringLiteral("qrc:/LegoObject/LEGO-2X3-S_simple.stl"), LEGO_2X3_S);
    addEntity(QStringLiteral("qrc:/LegoObject/LEGO-2X3-L_simple.stl"), LEGO_2X3_L);
    addEntity(QStringLiteral("qrc:/LegoObject/LEGO-2X4-S_simple.stl"), LEGO_2X4_S);
    addEntity(QStringLiteral("qrc:/LegoObject/LEGO-2X4-L_simple.stl"), LEGO_2X4_L);

}
