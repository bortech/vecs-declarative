import QtQuick 2.0

Item {
    id: root

    property int value: 0
    property int count: 4
    property color color: "#1d5c97"

    width: 200
    height: 25

    Rectangle {
        id: bar

        color: root.color
        anchors.verticalCenter: root.verticalCenter
        width: root.width
        height: 5
    }

    Rectangle {
        id: handle

        color: root.color
        anchors.verticalCenter: root.verticalCenter
        width: 15
        height: 15
        radius: width / 2

        x: root.width / root.count * root.value + (root.width / root.count / 2 - width / 2)
        Behavior on x { PropertyAnimation {} }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            root.value = mouseX / (root.width / root.count);
        }
    }
}
