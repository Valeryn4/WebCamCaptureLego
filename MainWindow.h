#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QQuaternion>
#include <QVector3D>
#include <QtMultimedia/QtMultimedia>
#include <QtMultimediaWidgets/QtMultimediaWidgets>

namespace Ui {
class MainWindow;
}

class Viwe3dWidget;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_comboBoxLegoType_currentIndexChanged(int index);

    void on_pushButtonHideLego_clicked();

    void on_doubleSpinBoxRotX_valueChanged(double arg1);

    void on_doubleSpinBoxRotY_valueChanged(double arg1);

    void on_doubleSpinBoxRotZ_valueChanged(double arg1);

    void setRotationLego(const QQuaternion &q);

    void on_doubleSpinBoxTransformX_valueChanged(double arg1);

    void on_doubleSpinBoxTransformY_valueChanged(double arg1);

    void on_doubleSpinBoxTransformZ_valueChanged(double arg1);

    void setPositionLego(const QVector3D &v);

    void on_pushButtonScreenShoot_clicked();

    void screenCompleted(QImage img1, QImage img2);

    void on_toolButtonDiffuse_clicked();

    void on_toolButtonSpecular_clicked();

    void on_toolButtonAmbiend_clicked();

    void on_horizontalSliderAlpha_sliderMoved(int position);

    void on_horizontalSliderShininess_sliderMoved(int position);

    void on_comboBoxBlendingSRC_currentIndexChanged(int index);

    void on_comboBoxBlendingDest_currentIndexChanged(int index);


private:

    bool floatEqual(double dX, double dY);
    void frameProcessing(const QImage &img, quint64 frameID);
    Ui::MainWindow *ui;
    Viwe3dWidget *m_viweWidget;
    size_t m_frameID;

};

#endif // MAINWINDOW_H
