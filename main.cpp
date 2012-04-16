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
#include <QFile>

extern "C" {
#include <ges/ges.h>
}
#include "qmlapplicationviewer.h"
#include "qdeclarativevideoeditor.h"
#include "videoeditorimageprovider.h"
#include "videoeditoritem.h"

extern "C" {
#include "gstcapstricks.h"
}

/*
 * Need to check if we are in PR1.2 or before, if so we need to disable thumbnails as
 * something in the qml Image loading crashes. PR1.3 onwards should be fine.
 */
bool check_show_thumbnails() {

    /*
     * It seems the only way to get the current PR version is to look at the PR version
     * it to check the value hardcoded into the about applet binaries.
     */
    QFile file("/usr/lib/duicontrolpanel/applets/libaboutapplet.so");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return true;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        int index = line.indexOf("PR");
        if(index >= 0 && line.size() > index + 5 &&
                line.at(index+2).isDigit() && line.at(index+4).isDigit()) { //PRX.Y
            int major = line.at(index+2).digitValue();
            int minor = line.at(index+4).digitValue();
            if(major == 1) {
                if(minor <= 2) {
                    return false;
                }
                return true;
            } else if(major < 1) {
                return false;
            }
            break;
        }
    }

    return true;
}

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    gst_init(&argc, &argv);
    ges_init();
    gstcapstricks_init();

    qmlRegisterType<QDeclarativeVideoEditor>("VideoEditor", 1,0, "VideoEditor");
    qmlRegisterUncreatableType<VideoEditorItem>("VideoEditor", 1, 0, "VideoEditorItem", "Managed by VideoEditor in C++");

    QmlApplicationViewer viewer;

    viewer.engine()->addImageProvider("videoeditorimageprovider", new VideoEditorImageProvider());

    QDeclarativeContext *context = viewer.rootContext();
    uint XWinId = viewer.winId();
    context->setContextProperty("XWinId", XWinId);
    context->setContextProperty("showThumbnails", check_show_thumbnails());

    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationLockLandscape);
    viewer.setMainQmlFile(QLatin1String("qml/VideoEditor/main.qml"));
    viewer.showExpanded();

    return app->exec();
}
