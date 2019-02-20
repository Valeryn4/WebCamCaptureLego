#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QtMath>
#include <QString>
#include <QLocale>
#include <QFile>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QProcess>

#include "Viwe3dWidget.h"
#include "CaptureCamera.h"

#include <QColorDialog>
#include <Qt3DRender/QBlendEquationArguments>

#include "nlohmann/json.hpp"

#include <cmath>
#include <limits>
#include <iomanip>
#include <iostream>
#include <type_traits>
#include <algorithm>
#include <unordered_map>
#include <vector>




using Blending = Qt3DRender::QBlendEquationArguments::Blending;

enum class BlendingIndex
{
    Zero = 0, // 0,
    One , // 1,
    SourceColor , // 0x0300,
    SourceAlpha , // 0x0302,
    Source1Alpha,
    Source1Color,
    DestinationColor , // 0x0306,
    DestinationAlpha , // 0x0304,
    SourceAlphaSaturate , // 0x0308,
    ConstantColor , // 0x8001,
    ConstantAlpha , // 0x8003,
    OneMinusSourceColor , // 0x0301,
    OneMinusSourceAlpha , // 0x0303,
    OneMinusDestinationAlpha , // 0x0305,
    OneMinusDestinationColor , // 0x0307,
    OneMinusConstantColor , // 0x8002,
    OneMinusConstantAlpha , // 0x8004,
    OneMinusSource1Alpha,
    OneMinusSource1Color,
    OneMinusSource1Color0 , // OneMinusSource1Color // ### Qt 6: Remove
};

static std::vector<Blending> g_blendIndexToFlag =
{
    Blending::Zero, // 0,
    Blending::One, // 1,
    Blending::SourceColor , // 0x0300,
    Blending::SourceAlpha , // 0x0302,
    Blending::Source1Alpha,
    Blending::Source1Color,
    Blending::DestinationColor , // 0x0306,
    Blending::DestinationAlpha , // 0x0304,
    Blending::SourceAlphaSaturate , // 0x0308,
    Blending::ConstantColor , // 0x8001,
    Blending::ConstantAlpha , // 0x8003,
    Blending::OneMinusSourceColor , // 0x0301,
    Blending::OneMinusSourceAlpha , // 0x0303,
    Blending::OneMinusDestinationAlpha , // 0x0305,
    Blending::OneMinusDestinationColor , // 0x0307,
    Blending::OneMinusConstantColor , // 0x8002,
    Blending::OneMinusConstantAlpha , // 0x8004,
    Blending::OneMinusSource1Alpha,
    Blending::OneMinusSource1Color,
    Blending::OneMinusSource1Color0 ,
};

static std::vector<QString> g_blendingIndexToString =
{
    "Zero = 0", // 0",
    "One ", // 1",
    "SourceColor ", // 0x0300",
    "SourceAlpha ", // 0x0302",
    "Source1Alpha",
    "Source1Color",
    "DestinationColor ", // 0x0306",
    "DestinationAlpha ", // 0x0304",
    "SourceAlphaSaturate ", // 0x0308",
    "ConstantColor ", // 0x8001",
    "ConstantAlpha ", // 0x8003",
    "OneMinusSourceColor ", // 0x0301",
    "OneMinusSourceAlpha ", // 0x0303",
    "OneMinusDestinationAlpha ", // 0x0305",
    "OneMinusDestinationColor ", // 0x0307",
    "OneMinusConstantColor ", // 0x8002",
    "OneMinusConstantAlpha ", // 0x8004",
    "OneMinusSource1Alpha",
    "OneMinusSource1Color",
    "OneMinusSource1Color0 ",
};


template<class T>
typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
    almost_equal(T x, T y, int ulp)
{
    // the machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return std::abs(x-y) <= std::numeric_limits<T>::epsilon() * std::abs(x+y) * ulp
    // unless the result is subnormal
           || std::abs(x-y) < std::numeric_limits<T>::min();
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_frameID(0)
{

    ui->setupUi(this);
    m_viweWidget = nullptr;

    /**
     * @brief comboBoxLegoType
     * Заполняем комбобокс
     */

    //qDebug() << "OK 1";

    auto comboBoxLegoType = ui->comboBoxLegoType;
    bool legoIsL = true;
    for (int i = 0; i < int(Viwe3dWidget::LegoType::COUNT); ++i)
    {
        QString prefix = "LEGO ";
        QString suffix = legoIsL ? " L" : " S";
        legoIsL = legoIsL ? false : true;

        QString v1;
        QString v2;
        int mid = int(Viwe3dWidget::LegoType::LEGO_2X2_L);
        if (i >= mid)
        {
            v1 = "2X";
            int tmp = (i - mid);

            int v2Val = tmp == 0 ? 2 : qFloor(tmp / 2.) + 2;
            v2.setNum(v2Val);
        }
        else
        {
            v1 = "1X";
            int v2Val = i == 0 ? 1 : qFloor(i / 2.) + 1;
            v2.setNum(v2Val);

        }

        QString name = prefix + v1 + v2 + suffix;
        comboBoxLegoType->addItem(name);
    }

     ui->comboBoxLegoType->setCurrentIndex(int(Viwe3dWidget::NONE));

    for (size_t i = 0; i < g_blendIndexToFlag.size(); ++i)
    {
        ui->comboBoxBlendingSRC->addItem(g_blendingIndexToString[i]);
        ui->comboBoxBlendingDest->addItem(g_blendingIndexToString[i]);
    }

    ui->comboBoxBlendingSRC->addItem("NONE");
    ui->comboBoxBlendingSRC->hide(); //TODO HIDE
    ui->comboBoxBlendingSRC->setCurrentText("NONE");
    ui->comboBoxBlendingDest->addItem("NONE");
    ui->comboBoxBlendingDest->hide(); // TODO HIDE
    ui->comboBoxBlendingDest->setCurrentText("NONE");

    //TODO HIDE
    ui->label_BLENDING_DST->hide();
    ui->label_BLENDING_src->hide();
    ui->groupBox_12->hide();
    ui->groupBox_13->hide();


    ui->horizontalSliderShininess->setValue(100);
    ui->horizontalSliderAlpha->setValue(50);
    /*
     * Добавляем последнюю NONE кнопку
     * */

    //qDebug() << "OK 2";
    m_viweWidget = ui->viweWidget;
    comboBoxLegoType->addItem("NONE");

    QObject::connect(m_viweWidget, &Viwe3dWidget::entityRotationTo, this, &MainWindow::setRotationLego);
    QObject::connect(m_viweWidget, &Viwe3dWidget::enditytTransformTo, this, &MainWindow::setPositionLego);
    QObject::connect(m_viweWidget, &Viwe3dWidget::screenComplited, this, &MainWindow::screenCompleted);



    //qDebug() << "OK 3";

    auto capture = new CaptureCamera(this);
    QObject::connect(capture, &CaptureCamera::frame, [this](const QImage &img, quint64 frameId)
    {
        //qDebug() << "frame processing";
        this->frameProcessing(img, frameId);
    });

    //qDebug() << "OK 4";

}

MainWindow::~MainWindow()
{

    delete ui;
}

void MainWindow::on_comboBoxLegoType_currentIndexChanged(int index)
{
    if (m_viweWidget == nullptr)
        return;
    int max = int(Viwe3dWidget::LegoType::COUNT);
    if (index >= max || index < 0)
        return;

    m_viweWidget->setCurrentEntity(Viwe3dWidget::LegoType(index));
}

void MainWindow::on_pushButtonHideLego_clicked()
{
    if (m_viweWidget == nullptr)
        return;
    m_viweWidget->setCurrentEntityNone();
    ui->comboBoxLegoType->setCurrentIndex(int(Viwe3dWidget::LegoType::NONE));
}



void MainWindow::on_doubleSpinBoxRotX_valueChanged(double arg1)
{
    if (m_viweWidget == nullptr)
        return;
    auto quanterion = m_viweWidget->getRotationEntity();
    auto fX = quanterion.x();
    if (almost_equal(double(fX), arg1, 4))
        return;

    quanterion.setX(float(arg1));
    quanterion.normalize();
    m_viweWidget->setEntityRotationTo(quanterion);

}

void MainWindow::on_doubleSpinBoxRotY_valueChanged(double arg1)
{
    if (m_viweWidget == nullptr)
        return;
    auto quanterion = m_viweWidget->getRotationEntity();
    auto fY = quanterion.y();
    if (almost_equal(double(fY), arg1, 4))
        return;

    quanterion.setY(float(arg1));
    quanterion.normalize();
    m_viweWidget->setEntityRotationTo(quanterion);

}

void MainWindow::on_doubleSpinBoxRotZ_valueChanged(double arg1)
{
    if (m_viweWidget == nullptr)
        return;
    auto quanterion = m_viweWidget->getRotationEntity();
    auto fZ = quanterion.z();
    if (almost_equal(double(fZ), arg1, 4))
        return;

    quanterion.setZ(float(arg1));
    quanterion.normalize();
    m_viweWidget->setEntityRotationTo(quanterion);
}

void MainWindow::setRotationLego(const QQuaternion &q)
{
    ui->doubleSpinBoxRotX->setValue(double(q.x()));
    ui->doubleSpinBoxRotY->setValue(double(q.y()));
    ui->doubleSpinBoxRotZ->setValue(double(q.z()));

}

void MainWindow::on_doubleSpinBoxTransformX_valueChanged(double arg1)
{
    if (m_viweWidget == nullptr)
        return;
    auto transfom = m_viweWidget->getPositionEntity();
    auto fX = transfom.x();
    if (almost_equal(arg1, double(fX), 4))
        return;
    transfom.setX(float(arg1));
    m_viweWidget->setEntityPositionTo(transfom);
}

void MainWindow::on_doubleSpinBoxTransformY_valueChanged(double arg1)
{
    if (m_viweWidget == nullptr)
        return;
    auto transfom = m_viweWidget->getPositionEntity();
    auto fY = transfom.y();
    if (almost_equal(arg1, double(fY), 4))
        return;
    transfom.setY(float(arg1));
    m_viweWidget->setEntityPositionTo(transfom);
}

void MainWindow::on_doubleSpinBoxTransformZ_valueChanged(double arg1)
{
    if (m_viweWidget == nullptr)
        return;
    auto transfom = m_viweWidget->getPositionEntity();
    auto fZ = transfom.z();
    if (almost_equal(arg1, double(fZ), 4))
        return;
    transfom.setZ(float(arg1));
    m_viweWidget->setEntityPositionTo(transfom);
}

void MainWindow::setPositionLego(const QVector3D &v)
{
    ui->doubleSpinBoxTransformX->setValue(double(v.x()));
    ui->doubleSpinBoxTransformY->setValue(double(v.y()));
    ui->doubleSpinBoxTransformZ->setValue(double(v.z()));
}

void MainWindow::on_pushButtonScreenShoot_clicked()
{
    if (m_viweWidget == nullptr)
        return;
    m_viweWidget->getScreen();
}

void MainWindow::screenCompleted(QImage img1, QImage img2)
{
    if (m_viweWidget == nullptr)
        return;
    QString currentPath = QDir::currentPath();
    QDir dir;
    dir.cd(currentPath);
    QString path = currentPath + "/screen/" + QDateTime::currentDateTime().toString("ddMMyy_HHmmss");
    auto type = m_viweWidget->getLegoType();

    if (!dir.exists(path))
    {
        dir.mkpath(path);
    }
    else
    {
        Q_ASSERT(false);
    }

    //IMG1
    {
        dir.cd(currentPath);
        QString name = path + QString("/screen_0") + QString(".png");
        QFile file(name);

        Q_ASSERT(!file.exists());
        auto b = img1.save(name);
        Q_ASSERT(b);

        qDebug() << "file is saved : " << file.fileName();
        dir.cd(currentPath);
    }

    QString img_path = "";
    //IMG2
    {
        dir.cd(currentPath);
        QString name = path + QString("/screen_1") + QString(".png");
        QFile file(name);

        Q_ASSERT(!file.exists());
        auto b = img2.save(name);
        img_path = name;
        Q_ASSERT(b);

        qDebug() << "file is saved : " << file.fileName();
        dir.cd(currentPath);
    }

    //FileLog
    auto rot = m_viweWidget->getRotationEntity();
    auto pos = m_viweWidget->getPositionEntity();
    {
        dir.cd(currentPath);
        QString name = path + QString("/cords") + QString(".txt");
        QFile file(name);

        Q_ASSERT(!file.exists());

        auto b = file.open(QIODevice::WriteOnly | QIODevice::Text);
        Q_ASSERT(b);

        QTextStream out(&file);


        out << QString("\"pos\":{\n"
                       "  \"X\":\"%1\",\n"
                       "  \"Y\":\"%2\",\n"
                       "  \"Z\":\"%3\"\n"
                       "},\n"
                       "\"rot\":{\n"
                       "  \"X\":\"%4\",\n"
                       "  \"Y\":\"%5\",\n"
                       "  \"Z\":\"%6\",\n"
                       "  \"W\":\"%7\"\n"
                       "}")
               .arg(pos.x()).arg(pos.y()).arg(pos.z())
               .arg(rot.x()).arg(rot.y()).arg(rot.z()).arg(rot.scalar());

        qDebug() << "file is saved : " << file.fileName();
        dir.cd(currentPath);
    }

    //Load json
    QProcess process;
    //TODO SCTIPT

    QString script = QString("./LEGONFIRE/source/tester_turn.py -wm last.h5 -wt weights.0387.hdf5 -i %1 -o \"\" ").arg(img_path);
    qDebug() << "Run script: " << script;
    process.start(script); //TODO Сюда питон!
    process.waitForFinished(-1); // will wait forever until finished

    QString out = process.readAllStandardOutput();
    QString err = process.readAllStandardError();

    QString read_json = "";
    //Читаем json
    QString json_path = img_path + ".json";
    qDebug() << "Read Json: " << json_path;
    QFile file(img_path + ".json");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        read_json = file.readAll();
    }
    else
    {

        json_path = ":/file.json";
        qDebug() << "Read TEST Json: " << json_path;
        file.setFileName(json_path);
        Q_ASSERT(file.open(QIODevice::ReadOnly | QIODevice::Text));

        read_json = file.readAll();

        qDebug() << "Test json: \n" << read_json;

    }

    read_json.remove('\n');
    read_json.remove('\t');

    using json = nlohmann::json;
    using PosRot = QPair<QVector3D, QQuaternion>;
    using ListLego = Viwe3dWidget::ListEntityRaw;

    ListLego listLego;

   // QVector<PosRot> vecPosRot;

    try
    {
        auto j = json::parse(read_json.toStdString());
        qDebug() << "parse success";
        for (auto &&j_obj : j)
        {
            qDebug() << " obj: " << QString::fromStdString( j_obj.dump() );

            QVector3D res_pos;
            QQuaternion res_rot;

            auto j_pos = j_obj.at("pos");
            auto j_rot = j_obj.at("rot");
            qDebug() << "get position and rotation json";

            auto obj_to_float = [](const json &obj)
            {
                auto str = obj.get<std::string>();
                return QString::fromStdString(str).toFloat();
            };

            res_pos = QVector3D(obj_to_float(j_pos.at("X")),
                                obj_to_float(j_pos.at("Y")),
                                obj_to_float(j_pos.at("Z")));

            qDebug() << " position parse success ";
            res_rot = QQuaternion(obj_to_float(j_rot.at("W")),
                                  obj_to_float(j_rot.at("X")),
                                  obj_to_float(j_rot.at("Y")),
                                  obj_to_float(j_rot.at("Z")));

            qDebug() << "quantareon parse success";
            //vecPosRot.append(PosRot(res_pos, res_rot));
            listLego.append({res_pos, res_rot, type, QColor("blue") });
        }


    }
    catch (json::parse_error &e)
    {
        qDebug() << e.what() << "\n byte:" << e.byte;
    }
    catch (json::exception &e)
    {
         qDebug() << e.what();
    }
    qDebug() << "Print: \n" << out << "\n ERR:\n" << err;
    //qDebug() << "RES:\n" << listLego;

    if (!listLego.isEmpty())
    {
        auto dialog = new QDialog(this);
        QVBoxLayout *mainLayout = new QVBoxLayout;
        dialog->setLayout(mainLayout);

        listLego.append({pos, rot, type, QColor("red") });

        auto widget = new Viwe3dWidget(listLego, dialog);

        //widget->setEntityRotationTo(vecPosRot.at(0).second);
        //widget->setEntityPositionTo(vecPosRot.at(0).first);
        widget->frameUpdate(img2.convertToFormat(QImage::Format_RGB16).mirrored(true, false));
        mainLayout->addWidget(widget);
        dialog->exec();
    }
    else
    {
        auto dialog = new QDialog(this);
        QVBoxLayout *mainLayout = new QVBoxLayout;
        dialog->setLayout(mainLayout);

        ListLego list;
        list.append({pos, rot, type, QColor("red")});

        auto widget = new Viwe3dWidget(list, dialog);
        widget->frameUpdate(img2.convertToFormat(QImage::Format_RGB16).mirrored(true, false));
        mainLayout->addWidget(widget);
        dialog->exec();
    }

}

bool MainWindow::floatEqual(double dX, double dY)
{
    const double dEpsilon = 0.000001; // or some other small number
    return fabs(dX - dY) <= dEpsilon * fabs(dX);

}

void MainWindow::frameProcessing(const QImage &img, quint64 frameID)
{
    if (frameID == 0)
        m_frameID = 0;
    if (frameID < m_frameID)
        return;
    if (img.isNull())
        return;

    m_viweWidget->frameUpdate(img);
}

void MainWindow::on_toolButtonDiffuse_clicked()
{
    if (m_viweWidget == nullptr)
        return;
    QColor color = QColorDialog::getColor(Qt::white, this, "", QColorDialog::ShowAlphaChannel);
    if (!color.isValid())
        return;

    QImage icon(QSize(25,25), QImage::Format::Format_RGB32);
    icon.fill(color);
    ui->toolButtonDiffuse->setIcon(QIcon(QPixmap::fromImage(icon)));
    m_viweWidget->setDiffuse(color);
}

void MainWindow::on_toolButtonSpecular_clicked()
{
    if (m_viweWidget == nullptr)
        return;

    QColor color = QColorDialog::getColor(Qt::white, this, "", QColorDialog::ShowAlphaChannel);
    if (!color.isValid())
        return;

    QImage icon(QSize(25,25), QImage::Format::Format_RGB32);
    icon.fill(color);
    ui->toolButtonSpecular->setIcon(QIcon(QPixmap::fromImage(icon)));
    m_viweWidget->setSpecular(color);
}

void MainWindow::on_toolButtonAmbiend_clicked()
{
    if (m_viweWidget == nullptr)
        return;

    if (m_viweWidget == nullptr)
        return;

    QColor color = QColorDialog::getColor(Qt::white, this, "", QColorDialog::ShowAlphaChannel);
    if (!color.isValid())
        return;

    QImage icon(QSize(25,25), QImage::Format::Format_RGB32);
    icon.fill(color);
    ui->toolButtonAmbiend->setIcon(QIcon(QPixmap::fromImage(icon)));
    m_viweWidget->setAmbiend(color);
}

void MainWindow::on_horizontalSliderAlpha_sliderMoved(int position)
{
    if (m_viweWidget == nullptr)
        return;
    float alpha = position / 100.f;
    m_viweWidget->setAlpha(alpha > 1.f ? 1.f : alpha);
}

void MainWindow::on_horizontalSliderShininess_sliderMoved(int position)
{
    if (m_viweWidget == nullptr)
        return;
    float shin = position / 100.f;
    m_viweWidget->setShininess(shin > 1.f ? 1.f : shin);
}

void MainWindow::on_comboBoxBlendingSRC_currentIndexChanged(int index)
{

    if (m_viweWidget == nullptr)
        return;
    if (size_t(index) < g_blendIndexToFlag.size() )
        m_viweWidget->setBlendingSrc(g_blendIndexToFlag[size_t(index)]);
}

void MainWindow::on_comboBoxBlendingDest_currentIndexChanged(int index)
{
    if (m_viweWidget == nullptr)
        return;
    if (size_t(index) < g_blendIndexToFlag.size() )
        m_viweWidget->setBlendingDst(g_blendIndexToFlag[size_t(index)]);
}

