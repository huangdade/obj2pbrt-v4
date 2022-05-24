#include "MainDialog.h"
#include "./ui_MainDialog.h"

#include <osgQOpenGL/osgQOpenGLWidget>
#include <osg/Group>
#include <osg/Multisample>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <QSettings>
#include <QFileDialog>
#include <osgDB/ReadFile>
#include <QDebug>
#include <QProcess>
#include "obj2pbrt.h"
#include <stdio.h>
#include <osg/ComputeBoundsVisitor>
#include <osg/PositionAttitudeTransform>
#include <osg/Quat>
#include <QMessageBox>

MainDialog::MainDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);

    ui->exportPbrtV3Btn->setEnabled(false);
    ui->upgradePbrtV4Btn->setEnabled(false);
    ui->renderBtn->setEnabled(false);

    osgWidget = new osgQOpenGLWidget();
    ui->verticalLayout->addWidget(osgWidget, 1);
}

MainDialog::~MainDialog()
{
    delete ui;
}

void MainDialog::loadObjFile(QString filename)
{
    qDebug() << "osg size:" << osgWidget->width() << osgWidget->height();
    osgViewer::Viewer* viewer = osgWidget->getOsgViewer();
    if (!root.valid())
    {
        qDebug() << "init root";
        root = new osg::Group();
        viewer->setSceneData(root.get());
        // 修正长宽比
        osg::Camera* camera = viewer->getCamera();
        camera->setProjectionMatrixAsPerspective(45, 1.0 * osgWidget->width() / osgWidget->height(), 0.1, 1000);
    }

    osg::ref_ptr<osg::Node> objNode = osgDB::readNodeFile(filename.toStdString());

    // 计算包围盒
    osg::ComputeBoundsVisitor boundVisitor;
    objNode->accept(boundVisitor);
    osg::BoundingBox boundingBox = boundVisitor.getBoundingBox();
    // 计算缩放和平移，将其居中
    this->scale = 20.0 / boundingBox.radius();
    this->center = boundingBox.center();

    // 使模型居中
    osg::ref_ptr<osg::PositionAttitudeTransform> pat = new osg::PositionAttitudeTransform();
    pat->setPosition(-this->center * this->scale);
    pat->setScale(osg::Vec3(this->scale, this->scale, this->scale));
    pat->addChild(objNode);

    root->addChild(pat);

//    // 验证一下是否真的居中了
//    osg::ComputeBoundsVisitor boundVisitor2;
//    root->accept(boundVisitor2);
//    osg::BoundingBox boundingBox2 = boundVisitor2.getBoundingBox();
//    osg::Vec3d center2 = boundingBox2.center();
//    qDebug() << "check center: " << center2.x() << center2.y() << center2.z();

    osgGA::TrackballManipulator* manipulator = new osgGA::TrackballManipulator();
    viewer->setCameraManipulator(manipulator, true);
    manipulator->setTrackballSize(0.3);
    manipulator->setAllowThrow(false);
    manipulator->setWheelZoomFactor(-0.2);

    objFile = filename;
    ui->exportPbrtV3Btn->setEnabled(true);
}

void MainDialog::loadObjFile()
{
    // 把上次访问的文件存储到ini配置文件，避免丢失
    QSettings setting("./Setting.ini", QSettings::IniFormat);
    QString lastPath = setting.value("LastFilePath").toString();

    QString filename = QFileDialog::getOpenFileName(this,
                                                    QStringLiteral("选择您的模型文件"),
                                                    lastPath,
                                                    QStringLiteral("OBJ文件(*.obj)"));
    if (!filename.isEmpty())
    {
        lastPath = QFileInfo(filename).dir().path();
        setting.setValue("LastFilePath", lastPath);

        // 文件选择成功
        loadObjFile(filename);
    }
}

void MainDialog::exportPbrtV3()
{
    // 提取camera参数
    osgViewer::Viewer* viewer = osgWidget->getOsgViewer();
    osg::Camera* camera = viewer->getCamera();
    osg::Vec3d eye;
    osg::Vec3d lookAtTarget;
    osg::Vec3d up;
    camera->getViewMatrixAsLookAt(eye, lookAtTarget, up, 1.0);

    // camera fov
    double fov, _;
    camera->getProjectionMatrixAsPerspective(fov, _, _, _);


    QFileInfo fileInfo(objFile);
    QString saveAs = QFileDialog::getSaveFileName(nullptr,
                                                  "另存为",
                                                  fileInfo.absolutePath() + "/" + fileInfo.baseName() + "-v3.pbrt",
                                                  "pbrt-v3文件(*.pbrt)");

    if (!saveAs.isEmpty())
    {
        std::vector<std::string> args;
        args.push_back("obj2pbrt");
        args.push_back(objFile.toStdString());
        args.push_back(saveAs.toStdString());
        obj2pbrt(args, eye, lookAtTarget, up, fov, scale, this->center);
        pbrtV3File = saveAs;
        ui->upgradePbrtV4Btn->setEnabled(true);
    }
}

void MainDialog::upgradeToPbrtV4()
{
    QFileInfo fileInfo(pbrtV3File);
    QString baseName = fileInfo.baseName();
    if (baseName.endsWith("-v3"))
    {
        baseName = baseName.left(baseName.size() - 1) + "4";
    }
    else
    {
        baseName += "-v4";
    }
    QString saveAs = QFileDialog::getSaveFileName(nullptr,
                                                  "另存为",
                                                  fileInfo.absolutePath() + "/" + baseName + ".pbrt",
                                                  "pbrt-v4文件(*.pbrt)");

    // QString cmd = "pbrt\\pbrt.exe " + pbrtV3File + " --upgrade > " + saveAs;
    QProcess p;
    p.start("pbrt\\pbrt.exe " + pbrtV3File  + " --upgrade");
    p.waitForFinished();
    QByteArray output = p.readAll();
    QFile file(saveAs);
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(output);
        file.close();
    }

    ui->renderBtn->setEnabled(true);

    pbrtV4File = saveAs;
}

void MainDialog::render()
{
    QFileInfo fileInfo(pbrtV4File);
    QString saveAs = QFileDialog::getSaveFileName(nullptr,
                                                  "另存为",
                                                  fileInfo.absolutePath() + "/" + fileInfo.baseName() + ".png",
                                                  "图片(*.png)");
    QProcess p;
//    p.start("cmd.exe");
//    p.write("pbrt\\pbrt.exe " + pbrtV4File + " --outfile " + saveAs);
//    QProcess::startDetached();
    p.startDetached("pbrt\\pbrt.exe " + pbrtV4File + " --outfile " + saveAs);
    // p.start("pbrt\\pbrt.exe " + pbrtV4File + " --outfile " + saveAs);
    // p.waitForFinished();
    // QMessageBox::information(nullptr, "成功", "渲染图片成功");
}

