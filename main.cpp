/* VideoEditor main
 * Copyright (C) 2012 Thiago Sousa Santos <thiago.sousa.santos@collabora.co.uk>
 * Copyright (C) 2012 Robert Swain <robert.swain@collabora.co.uk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <QtGui/QApplication>
#include <QDeclarativeView>
#include <QDeclarativeEngine>
#include <QDeclarativeComponent>
#include <QDeclarativeContext>
#include <QDebug>
extern "C" {
    #include <ges/ges.h>
}
#include "qmlapplicationviewer.h"
#include "qdeclarativevideoeditor.h"

extern "C" {
    #include "gstcapstricks.h"
}

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    gst_init(&argc, &argv);
    ges_init();
    gstcapstricks_init();

    qmlRegisterType<QDeclarativeVideoEditor>("VideoEditor", 1,0, "VideoEditor");

    QmlApplicationViewer viewer;

    QDeclarativeContext *context = viewer.rootContext();
    uint XWinId = viewer.winId();
    context->setContextProperty("XWinId", XWinId);

    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationLockLandscape);
    viewer.setMainQmlFile(QLatin1String("qml/VideoEditor/main.qml"));
    viewer.showExpanded();

    return app->exec();
}
