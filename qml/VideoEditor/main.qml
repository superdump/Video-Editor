import QtQuick 1.1
import com.nokia.meego 1.0

PageStackWindow {
    id: appWindow
    showStatusBar: false
    showToolBar: false

    initialPage: timeline

    Timeline {
        orientationLock: PageOrientation.LockLandscape
        id: timeline
    }

    TimelineList {
        id: timelineListModel
    }
}
