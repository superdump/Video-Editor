/* VideoEditor Timeline
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

import QtQuick 1.1
import com.nokia.meego 1.0
import VideoEditor 1.0

Page {
    id: timeline
    property int screenWidthPx: 854;
    property int screenHeightPx: 480;
    property double screenWidthMM: 86.3382727
    property double minUsableWidthMM: 10.0
    property double zoomProportion: 1.0/3.0
    property double minGranularityNS: 1000000000.0 / 30.0
    property double maxGranularityNS: 1000000000.0 * 3.0 * 60.0 * 60.0
    property double screenHDPMM: screenWidthPx / screenWidthMM
    property double minUsableWidthPx: minUsableWidthMM * screenHDPMM

    orientationLock: PageOrientation.LockLandscape
    width: 854
    height: 480

    property int autoScrollMargin: width * 0.2
    property int autoScrollRate: 10
    property int autoScrollPeriod: 100 // ms

    VideoEditor {
        id: videoeditor

        winId: XWinId;

        onError: {
            console.debug("Error: " + message + " (" + debug + ")");
            progressDialog.close();
            messageTitleField.text = message + "\n(" + debug + ")";
            messageDialog.open();
        }

        onRenderComplete: {
            progressDialog.close();
            messageTitleField.text = "Export complete";
            messageDialog.open();
        }

        onDataChanged: {
            if (list.count == 1) {
                listScale.calculatedScale = list.width * timeline.zoomProportion / videoeditor.getObjDuration(0);
                listScale.scale = 1.0;
            }
        }

        onProgressChanged: {
            var curPos = position * listScale.currentScale;
            if (curPos < list.width / 2) {
                list.contentX = 0;
            } else if (curPos > list.listContentWidth - list.width / 2) {
                list.contentX = list.listContentWidth - list.width;
            } else {
                list.contentX = curPos - list.width / 2;
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "black"
    }

    Item {
        id: preview
        property bool isPlaying: false
        width: 512
        height: 288
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top

        Rectangle {
            id: previewBackground

            anchors.fill: parent

            // This is the RGB hex value for the color key in omapxvsink
            color: "#080810"
        }

        Text {
            id: previewText
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            anchors.verticalCenter: parent.verticalCenter
            text: "Video preview\nTap to play/pause"
            color: "white"
            font.bold: true
            font.pixelSize: 32
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (preview.isPlaying) {
                    console.log("Video preview: playing -> paused");
                    preview.isPlaying = false;
                    videoeditor.pause();
                    previewText.text = "Video preview\nTap to play/pause";
                } else {
                    console.log("Video preview: paused -> playing");
                    preview.isPlaying = true;
                    previewText.text = "";
                    videoeditor.play();
                }
            }
        }
    }


    QueryDialog {
        id: removeAllDialog
        acceptButtonText: "Clear"
        rejectButtonText: "Cancel"
        message: "Clear all clips?"
        onAccepted: {
            videoeditor.removeAll();
            close();
        }
    }
    QueryDialog {
        id: removeDialog
        acceptButtonText: "Remove"
        rejectButtonText: "Cancel"
        message: "Remove the selected clip?"
        onAccepted: {
            console.log("Removing item " + list.currentIndex);
            videoeditor.removeAt(list.currentIndex);
            close();
        }
    }

    Item {
        id: leftButtons
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: preview.left
        anchors.bottom: timelineBar.top

        Button {
            id: addMediaButton
            text: "Add Media"
            anchors.topMargin: 16
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            onClicked: {
                var component = Qt.createComponent("VideoGallery.qml")
                pageStack.push(component);
            }
        }

        Button {
            id: removeAllButton
            text: "Clear"
            anchors.topMargin: 16
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: addMediaButton.bottom
            onClicked: {
                if (list.count > 0) {
                    if (videoeditor.isRendering())
                        videoeditor.cancelRender();
                    if (videoeditor.isPlaying)
                        videoeditor.pause();
                    removeAllDialog.open();
                }
            }
        }

        Button {
            id: removeButton
            text: "Remove"
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            anchors.bottomMargin: 16
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            visible: list.count > 0
            onClicked: {
                if (list.count > 0) {
                    if (videoeditor.isRendering())
                        videoeditor.cancelRender();
                    if (videoeditor.isPlaying)
                        videoeditor.pause();
                    removeDialog.open();
                }
            }
        }
    }

    Dialog {
        id: progressDialog
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        status: DialogStatus.Closed
        visualParent: timeline

        title: Text {
            id: titleField
            color: "white"
            text: "Export progress:"
            font.pixelSize: 28
            anchors.horizontalCenter: parent.horizontalCenter
        }

        content: Item {
            width: parent.width
            height: 32
            ProgressBar {
                id: progressBar
                width: parent.width
                anchors.verticalCenter: parent.verticalCenter
                value: videoeditor.progress
            }
        }

        buttons: Button {
            text: "Cancel"
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: progressDialog.reject()
        }

        onRejected: videoeditor.cancelRender()
    }

    Dialog {
        id: messageDialog
        width: parent.width
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        status: DialogStatus.Closed
        visualParent: timeline

        title: Text {
            id: messageTitleField
            width: parent.width
            color: "white";
            font.pixelSize: 28
            horizontalAlignment: Text.AlignHCenter
        }

        content: Item {
            height: 8
        }

        buttons: Button {
            text: "OK"
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: messageDialog.close()
        }
    }

    Item {
        id: rightButtons
        anchors.bottom: timelineBar.top
        anchors.left: preview.right
        anchors.right: parent.right
        anchors.top: parent.top

        Button {
            id: exportButton
            text: "Export"
            anchors.topMargin: 16
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top

            onClicked: {
                progressDialog.open()
                videoeditor.render()
            }
        }
        Button {
            id: settingsButton
            text: "Settings"
            anchors.topMargin: 16
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: exportButton.bottom

            onClicked: {
                var component = Qt.createComponent("ExportSettingsPage.qml")
                pageStack.push(component);
            }
        }
        Button {
            id: fitButton
            text: "Fit"
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            anchors.bottomMargin: 16
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            visible: list.count > 0
            onClicked: {
                listScale.calculatedScale = list.width / videoeditor.duration;
                listScale.scale = 1.0;
            }
        }
    }

    Item {
        id: timelineBar

        anchors.top: preview.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        Rectangle {
            id: timelineBackground
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#000000"
                }

                GradientStop {
                    position: 1
                    color: "#303030"
                }
            }
            anchors.fill: parent
        }

        PinchArea {
            id: timelinePinch
            anchors.fill: parent
            pinch {
                minimumScale: minUsableWidthPx / (list.width * zoomProportion)
                maximumScale: 1 / zoomProportion
                target: listScale
                dragAxis: Pinch.NoDrag
            }
        }

        Item {
            id: listScale
            property double calculatedScale: 1.0
            property double currentScale: calculatedScale * scale
            property double minimumScale: minUsableWidthPx / maxGranularityNS
            property double maximumScale: minUsableWidthPx / minGranularityNS
        }

        Item {
            id: scrollBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 16

            height: 8

            property real position: list.contentX / list.listContentWidth
            property real pageSize: list.width < list.listContentWidth ? (list.width / list.listContentWidth) : 1

            visible: pageSize < 1

            Rectangle {
                anchors.fill: parent
                radius: (height / 2 - 1)
                color: "white"
                opacity: 0.2
            }

            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                x: scrollBar.position * (scrollBar.width - 2) + 1
                width: scrollBar.pageSize * (scrollBar.width - 2)
                height: parent.height - 2
                radius: height / 2 - 1
                color: "white"
                opacity: 0.6
            }
        }

        ListView {
            id: list
            model: videoeditor

            property double listContentWidth: listScale.currentScale * videoeditor.duration

            anchors.topMargin: 8
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            anchors.bottomMargin: 16
            anchors.top: scrollBar.bottom
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            orientation: ListView.Horizontal
            boundsBehavior: Flickable.StopAtBounds

            delegate: Item {
                id: delegateButton
                z: 1
                width: model.object.duration * listScale.currentScale
                height: list.height

                Rectangle {
                    id: highlight
                    anchors.fill: parent
                    color: delegateButton.ListView.isCurrentItem ? "#bfffffff" : "#4dffffff"
                    border.width: 1
                    border.color: delegateButton.ListView.isCurrentItem ? "#bf000000" : "#4d000000"
                    Image {
                        id: image
                        source: "image://videoeditorimageprovider/" + model.object.uri + "#1000%"
                        anchors.centerIn: parent
                        width: parent.width - 4
                        height: parent.height - 4
                        asynchronous: true
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: parent.height
                        clip: true
                    }
                    Text {
                        anchors.centerIn: parent
                        text: image.status === Image.Loading ? "Loading..." : ""
                        font.pixelSize: 20
                    }
                }

                Text {
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.margins: 16
                    font.pixelSize: 20
                    width: parent.width - 32
                    clip: true
                    color: "white"
                    text: model.object.fileName
                }

                Item {
                    id: inPoint

                    x: if(inPointDrag.drag.active) { x } else { 0 }
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    visible: inPointDrag.active || delegateButton.ListView.isCurrentItem

                    Rectangle {
                        id: inBall

                        z: 1002
                        width: 30
                        height: width
                        radius: width / 2
                        color: "blue"
                        anchors.verticalCenter: parent.top
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Rectangle {
                        id: inStick
                        width: 3

                        z: 1000
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter:  parent.horizontalCenter
                        color: "blue"
                    }

                    MouseArea {
                        id: inPointDrag
                        property double minInPoint: (-model.object.inPoint) * listScale.currentScale
                        property double maxInPoint: (model.object.maxDuration - model.object.inPoint) * listScale.currentScale

                        anchors.fill: inBall
                        anchors.centerIn: inBall

                        drag.axis: Drag.XAxis
                        drag.target: parent
                        drag.minimumX: minInPoint
                        drag.maximumX: maxInPoint;

                        onPositionChanged: {
                            inPointTimer.start();
                        }

                        onReleased: {
                            var clipped = Math.min(Math.max(inPoint.x, minInPoint), maxInPoint);
                            console.log("ip: " + inPoint.x + ", clipped: " + clipped + ", min: " + minInPoint + ", max: " + maxInPoint)
                            var pos = model.object.inPoint + (clipped / listScale.currentScale);
                            model.object.inPoint = pos;
                        }
                    }

                    Timer {
                        id: inPointTimer
                        interval: 200
                        repeat: true

                        onTriggered: {
                            if(inPointDrag.drag.active) {
                                if(inPoint.x >= list.x + list.width * 0.8 && list.listContentWidth - list.contentX > list.width) {
                                    list.contentX += 5
                                } else if(inPoint.x < list.x + list.width * 0.2 && list.contentX > 0) {
                                    list.contentX -= 5
                                }
                            } else {
                                stop();
                            }
                        }
                    }
                }


                Item {
                    id: endPoint

                    x: if(endPointDrag.drag.active) { x } else { parent.width }
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    visible: endPointDrag.drag.active || delegateButton.ListView.isCurrentItem

                    Rectangle {
                        id: endBall

                        z: 1002
                        width: 30
                        height: width
                        radius: width / 2
                        color: "blue"
                        anchors.verticalCenter: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter

                    }

                    Rectangle {
                        id: endStick
                        width: 3

                        z: 1000
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter:  parent.horizontalCenter
                        color: "blue"
                    }

                    MouseArea {
                        id: endPointDrag
                        property double maxEndPoint: listScale.currentScale * (model.object.maxDuration - model.object.inPoint)

                        anchors.fill: endBall
                        anchors.centerIn: endBall

                        drag.axis: Drag.XAxis
                        drag.target: parent
                        drag.minimumX: 0
                        drag.maximumX: maxEndPoint

                        onPositionChanged: {
                            endPointTimer.start();
                        }

                        onReleased: {
                            var clipped = Math.min(endPoint.x, maxEndPoint)
                            console.log("ep: " + endPoint.x + ", clipped: " + clipped + ", max: " + maxEndPoint)
                            var pos = clipped / listScale.currentScale;
                            model.object.duration = pos
                        }
                    }

                    Timer {
                        id: endPointTimer
                        interval: 200
                        repeat: true

                        onTriggered: {
                            if(endPointDrag.drag.active) {
                                if(endPoint.x >= list.x + list.width * 0.8 && list.listContentWidth - list.contentX > list.width) {
                                    list.contentX += 5
                                } else if(endPoint.x < list.x + list.width * 0.2 && list.contentX > 0) {
                                    list.contentX -= 5
                                }
                            } else {
                                stop();
                            }
                        }
                    }
                }

                MouseArea {
                    id: dragArea
                    anchors.fill: parent
                    property int positionStarted: 0
                    property int positionEnded: 0
                    property bool held: false
                    property double mousePos: list.x + (delegateButton.x - list.contentX) + mouseX
                    property double mousePosContent: positionStarted + mouseX
                    drag.axis: Drag.XAxis
                    enabled: delegateButton.ListView.isCurrentItem
                    onPressed: {
                        delegateButton.z = 2;
                        positionStarted = delegateButton.x;
                        fakeDel.curDragArea = dragArea
                        fakeDel.width = delegateButton.width
                        fakeDel.visible = true
                        delegateButton.opacity = 0.5;
                        list.interactive = false;
                        held = true;
                    }
                    onPositionChanged: {
                        dragTimer.start();
                        positionEnded = mousePosContent;
                    }
                    onReleased: {
                        if (held == true) {
                            delegateButton.x = positionStarted;
                            delegateButton.z = 1;
                            delegateButton.opacity = 1;
                            var newPosition = list.indexAt(positionEnded, 0);
                            if (newPosition !== index) {
                                videoeditor.move(index, newPosition);
                                list.currentIndex = newPosition;
                            }
                            dragTimer.stop();
                            fakeDel.visible = false
                            fakeDel.curDragArea = null
                            list.interactive = true;
                            held = false;
                        }
                    }
                    onClicked: {
                        list.currentIndex = -1;
                    }
                    onDoubleClicked: {
                        listScale.calculatedScale = list.width * timeline.zoomProportion / model.object.duration;
                        listScale.scale = 1.0;
                    }
                }
                Timer {
                    id: dragTimer
                    interval: autoScrollPeriod
                    repeat: true

                    onTriggered: {
                        if(dragArea.mousePos >= timeline.width - autoScrollMargin &&
                                list.listContentWidth - list.contentX > list.width) {
                            list.contentX = Math.max(0, Math.min(list.contentX + autoScrollRate,
                                                                 list.listContentWidth - list.width));
                        } else if(dragArea.mousePos < autoScrollMargin && list.contentX > 0) {
                            list.contentX = Math.min(Math.max(list.contentX - autoScrollRate, 0),
                                                     list.listContentWidth - list.width);
                        }
                    }
                }
                // This MouseArea is to workaround the button pressed state that causes
                // flicks and drags to momentarily flash the button as being selected
                MouseArea {
                    id: tapArea
                    anchors.fill: parent
                    enabled: delegateButton.ListView.isCurrentItem ? "false" : "true"
                    onClicked: {
                        list.currentIndex = index;
                    }
                    onDoubleClicked: {
                        listScale.calculatedScale = list.width * timeline.zoomProportion / model.object.duration;
                        listScale.scale = 1.0;
                    }
                }
            }

            Rectangle {
                id: fakeDel
                property variant curDragArea: null
                x: curDragArea ? (curDragArea.mousePos - list.x - width/2) : 0
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                visible: false
                opacity: 0.5
                color: "white"
            }

            Text {
                id: timelineText
                color: "#ffffff"
                text: list.count ? "" : "Add clips using the 'Add Media' button"
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: 22
            }
            Item {
                id: playhead

                x: if(dragMouseArea.drag.active) { x } else { videoeditor.position * listScale.currentScale - list.contentX }
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                visible: dragMouseArea.drag.active || (list.count &&
                         (videoeditor.position * listScale.currentScale >= list.contentX) &&
                         (videoeditor.position * listScale.currentScale <= list.contentX + list.width))

                Rectangle {
                    id: seekBall

                    z: 1002
                    width: 30
                    height: width
                    radius: width / 2
                    color: "red"
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter

                }

                Rectangle {
                    width: 3

                    z: 1001
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter:  parent.horizontalCenter
                    color: "red"
                }

                MouseArea {
                    id: dragMouseArea

                    anchors.fill: seekBall
                    anchors.centerIn: seekBall

                    drag.axis: Drag.XAxis
                    drag.target: parent
                    drag.minimumX: 0
                    drag.maximumX: Math.min(list.width, list.listContentWidth)

                    onPositionChanged: {
                        timelineMoveTimer.start();
                    }

                    onReleased: {
                        var pos = videoeditor.duration * (playhead.x + list.contentX) / list.listContentWidth;
                        console.debug("Sending seek to " + pos + " / " + videoeditor.duration);
                        videoeditor.seek(pos);
                    }
                }

                Timer {
                    id: timelineMoveTimer
                    interval: autoScrollPeriod
                    repeat: true

                    onTriggered: {
                        if(dragMouseArea.drag.active) {
                            if(playhead.x >= timeline.width - autoScrollMargin && list.listContentWidth - list.contentX > list.width) {
                                list.contentX = Math.max(0, Math.min(list.contentX + autoScrollRate,
                                                                     list.listContentWidth - list.width));
                            } else if(playhead.x < autoScrollMargin && list.contentX > 0) {
                                list.contentX = Math.min(Math.max(list.contentX - autoScrollRate, 0),
                                                         list.listContentWidth - list.width);
                            }
                        } else {
                            stop();
                        }
                    }
                }

            }
        }


        // The ListView does not clip items outside its own area. This looks a bit weird so
        // these two rectangles are to cover up those overflows.
        Rectangle {
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#000000"
                }

                GradientStop {
                    position: 1
                    color: "#303030"
                }
            }
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: list.left
        }

        Rectangle {
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#000000"
                }

                GradientStop {
                    position: 1
                    color: "#303030"
                }
            }
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: list.right
            anchors.right: parent.right
        }

    }
}
