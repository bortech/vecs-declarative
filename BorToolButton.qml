import QtQuick 2.0

Text {
    id: root

    signal clicked

    verticalAlignment: Text.AlignVCenter
    horizontalAlignment: Text.AlignHCenter

    font.pointSize: 10
    renderType: Text.NativeRendering

    color: mouseArea.containsMouse ? "white" : "lightgray"

    MouseArea {
        id: mouseArea
        hoverEnabled: true
        anchors.fill: parent
        onClicked: {
            root.clicked();
        }
    }
}
