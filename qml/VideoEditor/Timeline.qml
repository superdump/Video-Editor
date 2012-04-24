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

    Connections {
        target: Qt.application
        onActiveChanged: {
            if (!Qt.application.active) {
                if(videoeditor.isPlaying) {
                    videoeditor.pause();
                }
            }
        }
    }

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
            renderCompleteDialog.open();
        }

        onDataChanged: {
            if (list.count == 1 && listScale.recalculateOnAdd && duration !== -1) {
                listScale.setCalculatedScale(list.width * timeline.zoomProportion / videoeditor.getObjDuration(0));
                listScale.setScale(1.0, listScale.scale, list.contentX);
                listScale.recalculateOnAdd = false;
            }
        }

        onProgressChanged: {
            var curPos = position * listScale.currentScale;
            if (list.listContentWidth <= list.width || curPos < list.width / 2) {
                list.updateContentX(0);
            } else if (curPos > list.listContentWidth - list.width / 2) {
                list.updateContentX(list.listContentWidth - list.width);
            } else {
                list.updateContentX(curPos - list.width / 2);
            }
        }
    }

    RenderCompleteDialog {
        id: renderCompleteDialog;

        filename: videoeditor.filename;
    }

    Rectangle {
        anchors.fill: parent
        color: "black"
    }

    Item {
        id: preview
        width: 512
        height: 288
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top

        function pause() {
            console.log("Video preview: playing -> paused");
            videoeditor.pause();
        }
        function play() {
            console.log("Video preview: paused -> playing");
            videoeditor.play();
        }

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
            visible: !videoeditor.isPlaying
            text: "Video preview\nTap to play/pause"
            color: "white"
            font.bold: true
            font.pixelSize: 32
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (videoeditor.isPlaying) {
                    preview.pause();
                } else {
                    preview.play();
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
                preview.pause();
                if (list.count == 0) {
                    listScale.recalculateOnAdd = true;
                }
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
                    preview.pause();
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
            visible: list.count > 0 && list.currentIndex >= 0
            onClicked: {
                if (list.count > 0 && list.currentIndex >= 0) {
                    preview.pause();
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
            onClicked: progressDialog.doReject()
        }

        function reject() {
            //Do nothing to prevent outside click rejecting our dialog
        }

        function doReject() {
            videoeditor.cancelRender();
            close();
            rejected();
        }
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
                preview.pause();
                if(videoeditor.render()) {
                    progressDialog.open()
                }
            }
        }
        Button {
            id: menuButton
            iconSource: "image://theme/icon-m-toolbar-view-menu-white"
            anchors.topMargin: 16
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: exportButton.bottom

            onClicked: {
                preview.pause();
                menu.open();
            }
        }
        ButtonColumn {
            id: scaleButtons
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            anchors.bottomMargin: 16
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            visible: list.count > 0

            exclusive: false

            ButtonRow {
                anchors.left: parent.left
                anchors.right: parent.right
                exclusive: false
                Button {
                    iconSource: "image://theme/icon-m-toolbar-up-white"
                    checkable: false
                    onClicked: {
                        listScale.setCalculatedScale(listScale.currentScale * 1.1);

                    }
                }
                Button {
                    iconSource: "image://theme/icon-m-toolbar-down-white"
                    checkable: false
                    onClicked: {
                        listScale.setCalculatedScale(listScale.currentScale / 1.1);
                        listScale.setScale(1.0, listScale.scale, list.contentX);
                    }
                }
            }

            Button {
                id: fitButton
                text: "Fit"
                checkable: false
                onClicked: {
                    listScale.setCalculatedScale(list.width / videoeditor.duration);
                    listScale.setScale(1.0, listScale.scale, list.contentX);
                }
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
            property double oldScale;
            property double oldX;

            pinch {
                minimumScale: minUsableWidthPx / (list.width * zoomProportion)
                maximumScale: 1 / zoomProportion
                //target: listScale
                dragAxis: Pinch.NoDrag
            }
            onPinchStarted: {
                fakeDel.visible = false;
                oldScale = listScale.scale;
                oldX = list.contentX;
            }
            onPinchUpdated: {
                fakeDel.visible = false;
                listScale.setScale(pinch.scale, oldScale, oldX);
            }
            onPinchFinished: {
                fakeDel.visible = false;
            }
        }

        Item {
            id: listScale
            property bool recalculateOnAdd: true
            property double calculatedScale: 1.0
            property double currentScale: calculatedScale * scale
            property double minimumScale: minUsableWidthPx / maxGranularityNS
            property double maximumScale: minUsableWidthPx / minGranularityNS

            function setCalculatedScale(cScale) {
                list.updateContentX(0);
                calculatedScale = cScale;
            }

            function setScale(newscale, oldscale, oldX) {
                list.updateContentX(0);
                scale = newscale * oldscale;

                //TODO this seems to break the contentX placement, disable it for now
                //Will make the scaling always take the timeline to 0 position
                //                var newX = oldX * (scale / oldscale);
                //                list.contentX = newX;
            }
        }

        ListView {
            id: list
            model: videoeditor

            property double listContentWidth: listScale.currentScale * videoeditor.duration
            property double oldContentX: 0;

            anchors.topMargin: 16
            anchors.bottomMargin: 16
            anchors.leftMargin: 32
            anchors.rightMargin: 32
            anchors.top: parent.top
            anchors.bottom: scrollBar.top
            anchors.left: parent.left
            anchors.right: parent.right

            orientation: ListView.Horizontal
            boundsBehavior: Flickable.StopAtBounds

            function updateContentX(cx) {
                oldContentX = cx;
                contentX = cx;
            }

            onCurrentItemChanged: {
                if(currentIndex >= 0 && list.listContentWidth > list.width) {
                    contentX = oldContentX;
                }
            }

            onMovementEnded: {
                oldContentX = contentX;
            }

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
                        visible: showThumbnails
                        source: showThumbnails ? "image://videoeditorimageprovider/" + model.object.uri + "#33%" : ""
                        anchors.centerIn: parent
                        width: parent.width - 4
                        height: parent.height - 4
                        asynchronous: true
                        fillMode: Image.PreserveAspectFit
                        sourceSize.height: parent.height
                        clip: true
                    }
                    Text {
                        visible: showThumbnails
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

                    onMousePosChanged: {
                        fakeDel.x = mousePos - list.x - width/2;
                    }

                    onPressed: {
                        delegateButton.z = 2;
                        positionStarted = delegateButton.x;
                        fakeDel.displayObj(delegateButton.width, dragArea, "white");
                        delegateButton.opacity = 0.5;
                        list.interactive = false;
                        held = true;
                        dragTimer.start();
                    }
                    onPositionChanged: {
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
                                if(newPosition === -1) {
                                    list.currentIndex = list.count-1;
                                } else {
                                    list.currentIndex = newPosition;
                                }
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
                        listScale.setCalculatedScale(list.width * timeline.zoomProportion / model.object.duration);
                        listScale.setScale(1.0, listScale.scale, list.contentX);
                    }
                }
                Timer {
                    id: dragTimer
                    interval: autoScrollPeriod
                    repeat: true

                    onTriggered: {
                        if(dragArea.mousePos >= timeline.width - autoScrollMargin &&
                                list.listContentWidth - list.contentX > list.width) {
                            list.updateContentX(Math.max(0, Math.min(list.contentX + autoScrollRate,
                                                                     list.listContentWidth - list.width)));
                        } else if(dragArea.mousePos < autoScrollMargin && list.contentX > 0) {
                            list.updateContentX(Math.min(Math.max(list.contentX - autoScrollRate, 0),
                                                         list.listContentWidth - list.width));
                        }
                    }
                }

                Item {
                    id: inPoint

                    x: if(inPointDrag.drag.active) { x } else { 0 }
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    visible: delegateButton.ListView.isCurrentItem && !inPointDrag.drag.active

                    Rectangle {
                        id: inBall

                        z: 1002
                        width: 30
                        height: width
                        radius: width / 2
                        color: "#3465a4"
                        anchors.top: parent.top
                        anchors.horizontalCenter: parent.horizontalCenter

                    }

                    Rectangle {
                        id: inStick
                        width: 3

                        z: 1000
                        anchors.top: inBall.bottom
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter:  parent.horizontalCenter
                        color: "#3465a4"
                    }

                    MouseArea {
                        id: inPointDrag

                        width: minUsableWidthPx
                        height: width
                        anchors.centerIn: inBall
                        preventStealing: true

                        property int positionEnded: 0
                        property bool held: false
                        property double initMousePos: width/2
                        property double mousePosContent: mouseX - initMousePos
                        property double mousePos: list.x + (delegateButton.x - list.contentX) + mousePosContent
                        property double originPoint: model.object.inPoint * listScale.currentScale
                        property double maxinPoint: listScale.currentScale * model.object.maxDuration - originPoint
                        drag.axis: Drag.XAxis
                        enabled: delegateButton.ListView.isCurrentItem
                        onPressed: {
                            initMousePos = mouseX
                            fakeinPoint.x = mousePos - list.x
                            fakeinPoint.visible = true
                            fakeDel.x = delegateButton.x - list.contentX - originPoint;
                            fakeDel.displayObj(model.object.maxDuration * listScale.currentScale, null, "#3465a4");
                            list.interactive = false;
                            held = true;
                        }
                        onPositionChanged: {
                            inPointTimer.start();
                            fakeinPoint.x = mousePos - list.x
                            positionEnded = mousePosContent;
                            if (fakeDel.x < 0 && positionEnded < originPoint) {
                                console.log("HACK: Setting inpoint to 0 to allow setting any inpoint");
                                model.object.duration += originPoint;
                                model.object.inPoint = 0;
                                fakeDel.x = delegateButton.x - list.contentX;
                            }
                        }
                        onReleased: {
                            if (held == true) {
                                inPointTimer.stop();
                                var clipped = Math.max(-originPoint, Math.min(positionEnded, maxinPoint));
                                model.object.inPoint += clipped / listScale.currentScale;
                                fakeDel.visible = false;
                                fakeDel.curDragArea = null;
                                fakeinPoint.visible = false
                                list.interactive = true;
                                held = false;
                            }
                        }
                    }
                    Timer {
                        id: inPointTimer
                        interval: autoScrollPeriod
                        repeat: true

                        onTriggered: {
                            if(inPointDrag.mousePos >= timeline.width - autoScrollMargin &&
                                    list.listContentWidth - list.contentX > list.width) {
                                list.updateContentX(Math.max(0, Math.min(list.contentX + autoScrollRate,
                                                                         list.listContentWidth - list.width)));
                            } else if(inPointDrag.mousePos < autoScrollMargin && list.contentX > 0) {
                                list.updateContentX(Math.min(Math.max(list.contentX - autoScrollRate, 0),
                                                             list.listContentWidth - list.width));
                            }
                            fakeDel.x = delegateButton.x - list.contentX - inPointDrag.originPoint;
                        }
                    }
                }


                Item {
                    id: endPoint

                    x: if(endPointDrag.drag.active) { x } else { parent.width }
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    visible: delegateButton.ListView.isCurrentItem && !endPointDrag.drag.active

                    Rectangle {
                        id: endBall

                        z: 1002
                        width: 30
                        height: width
                        radius: width / 2
                        color: "#3465a4"
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter

                    }

                    Rectangle {
                        id: endStick
                        width: 3

                        z: 1000
                        anchors.top: parent.top
                        anchors.bottom: endBall.top
                        anchors.horizontalCenter:  parent.horizontalCenter
                        color: "#3465a4"
                    }

                    MouseArea {
                        id: endPointDrag

                        width: minUsableWidthPx
                        height: width
                        anchors.centerIn: endBall
                        preventStealing: true

                        property int positionEnded: 0
                        property bool held: false
                        property double initMousePos: width/2
                        property double mousePosContent: delegateButton.width + (mouseX - initMousePos)
                        property double mousePos: list.x + (delegateButton.x - list.contentX) + mousePosContent
                        property double origendPoint: delegateButton.width
                        property double maxEndPoint: listScale.currentScale * (model.object.maxDuration - model.object.inPoint)
                        drag.axis: Drag.XAxis
                        enabled: delegateButton.ListView.isCurrentItem
                        onPressed: {
                            initMousePos = mouseX
                            fakeEndPoint.x = mousePos - list.x
                            fakeEndPoint.visible = true
                            fakeDel.x = delegateButton.x - list.contentX - model.object.inPoint * listScale.currentScale;
                            fakeDel.displayObj(model.object.maxDuration * listScale.currentScale, null, "#3465a4");
                            list.interactive = false;
                            held = true;
                        }
                        onPositionChanged: {
                            endPointTimer.start();
                            fakeEndPoint.x = mousePos - list.x
                            positionEnded = mousePosContent;
                            if (fakeDel.x + fakeDel.width > list.width && positionEnded > origendPoint) {
                                console.log("HACK: Setting to full duration to allow setting endpoint");
                                model.object.duration = model.object.maxDuration - model.object.inPoint;
                            }
                        }
                        onReleased: {
                            if (held == true) {
                                endPointTimer.stop();
                                var clipped = Math.max(0, Math.min(positionEnded, maxEndPoint));
                                model.object.duration = Math.max(0, clipped / listScale.currentScale);
                                fakeDel.visible = false;
                                fakeDel.curDragArea = null;
                                fakeEndPoint.visible = false
                                list.interactive = true;
                                held = false;
                            }
                        }
                    }
                    Timer {
                        id: endPointTimer
                        interval: autoScrollPeriod
                        repeat: true

                        onTriggered: {
                            if(endPointDrag.mousePos >= timeline.width - autoScrollMargin &&
                                    list.listContentWidth - list.contentX > list.width) {
                                list.updateContentX(Math.max(0, Math.min(list.contentX + autoScrollRate,
                                                                         list.listContentWidth - list.width)));
                            } else if(endPointDrag.mousePos < autoScrollMargin && list.contentX > 0) {
                                list.updateContentX(Math.min(Math.max(list.contentX - autoScrollRate, 0),
                                                             list.listContentWidth - list.width));
                            }
                            fakeDel.x = delegateButton.x - list.contentX - model.object.inPoint * listScale.currentScale;
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
                        listScale.setCalculatedScale(list.width * timeline.zoomProportion / model.object.duration);
                        listScale.setScale(1.0, listScale.scale, list.contentX);
                    }
                }
            }


            // Fake objects for drag and drop
            Item {
                id: fakeinPoint
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                visible: false
                opacity: 0.5
                Rectangle {
                    id: fakeinBall

                    z: 1002
                    width: 30
                    height: width
                    radius: width / 2
                    color: "#3465a4"
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Rectangle {
                    id: fakeinStick
                    width: 3

                    z: 1000
                    anchors.top: fakeinBall.bottom
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter:  parent.horizontalCenter
                    color: "#3465a4"
                }
            }
            Item {
                id: fakeEndPoint
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                visible: false
                opacity: 0.5
                Rectangle {
                    id: fakeEndBall

                    z: 1002
                    width: 30
                    height: width
                    radius: width / 2
                    color: "#3465a4"
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Rectangle {
                    id: fakeEndStick
                    width: 3

                    z: 1000
                    anchors.top: parent.top
                    anchors.bottom: fakeEndBall.top
                    anchors.horizontalCenter:  parent.horizontalCenter
                    color: "#3465a4"
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

                function displayObj(displayWidth, inputArea, delColor) {
                    width = displayWidth;

                    if (typeof inputArea !== 'undefined') {
                        curDragArea = inputArea;
                    }

                    if (typeof delColor !== 'undefined') {
                        color = delColor;
                    } else {
                        color = "white";
                    }

                    visible = true;
                }
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

                x: dragMouseArea.drag.active ? x : videoeditor.position * listScale.currentScale - list.contentX
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
                    color: "#cc0000"
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.horizontalCenter: parent.horizontalCenter

                }

                Rectangle {
                    width: 3

                    z: 1001
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.horizontalCenter:  parent.horizontalCenter
                    color: "#cc0000"
                }

                MouseArea {
                    id: dragMouseArea

                    width: minUsableWidthPx
                    height: width
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
                                list.updateContentX(Math.max(0, Math.min(list.contentX + autoScrollRate,
                                                                         list.listContentWidth - list.width)));
                            } else if(playhead.x < autoScrollMargin && list.contentX > 0) {
                                list.updateContentX(Math.min(Math.max(list.contentX - autoScrollRate, 0),
                                                             list.listContentWidth - list.width));
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

        Item {
            id: scrollBar
            anchors.bottom: parent.bottom
            anchors.left: list.left
            anchors.right: list.right
            anchors.topMargin: 16
            anchors.bottomMargin: 16
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
    }

    Menu {
        id: menu
        visualParent: timeline
        content: MenuLayout {
            MenuItem {
                text: "Settings"
                onClicked: {
                    var component = Qt.createComponent("SettingsPage.qml")
                    pageStack.push(component);
                }
            }
            MenuItem {
                text: "Report an issue"
                onClicked: {
                    Qt.openUrlExternally("https://github.com/superdump/Video-Editor/issues/new");
                }
            }
            MenuItem {
                text: "About"
                onClicked: {
                    var component = Qt.createComponent("About.qml")
                    pageStack.push(component);
                }
            }
        }
    }
}
