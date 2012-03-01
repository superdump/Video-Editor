#include <QtGui/QApplication>
#include <QDeclarativeEngine>
#include <QDeclarativeComponent>
extern "C" {
    #include <ges/ges.h>
}
#include "qmlapplicationviewer.h"
#include "qdeclarativevideoeditor.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    gst_init(&argc, &argv);
    ges_init();

    qmlRegisterType<QDeclarativeVideoEditor>("VideoEditor", 1,0, "VideoEditor");

    QmlApplicationViewer viewer;
    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer.setMainQmlFile(QLatin1String("qml/VideoEditor/main.qml"));
    viewer.showExpanded();

    return app->exec();
}
