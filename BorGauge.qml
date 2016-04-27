import QtQuick 2.0

Rectangle {
    id: root

    property int value: 0
    property int maxAbsValue: 100
    property color color: "#1d5c97"

    clip: true

    width: 10
    height: 100

    border.color: color
    border.width: 1

    Rectangle {
        id: bar
        color: root.color
        width: root.width
        height: Math.abs(value) / maxAbsValue * root.height / 2
        y: root.height / 2 - ((value > 0) ? height : 0)
    }
}
