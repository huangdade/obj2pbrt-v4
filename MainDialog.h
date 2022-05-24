#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>
#include <osg/Group>
#include <osgQOpenGL/osgQOpenGLWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainDialog; }
QT_END_NAMESPACE

class MainDialog : public QDialog
{
    Q_OBJECT

public:
    MainDialog(QWidget *parent = nullptr);
    ~MainDialog();

private:
    Ui::MainDialog *ui;

    osg::ref_ptr<osg::Group> root;
    void loadObjFile(QString);
    QString objFile;
    double scale;
    osg::Vec3d center;
    QString pbrtV3File;
    QString pbrtV4File;
    osgQOpenGLWidget* osgWidget;

private slots:
    void loadObjFile();
    void exportPbrtV3();
    void upgradeToPbrtV4();
    void render();
};
#endif // MAINDIALOG_H
