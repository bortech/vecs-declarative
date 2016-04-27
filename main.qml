import QtQuick 2.6
import QtQuick.Window 2.2
import com.vecs.device 1.0
import "helper.js" 1.0 as Helper

Window {
    id: mainWindow
    width: 640
    height: 480
    visible: true
    title: "Virtual Environment Control Sensors"

    Rectangle {
        anchors.fill: parent        

        Rectangle {
            id: toolBar
            width: parent.width
            anchors.top: parent.top
            height: 36
            color: "#1d5c97"

            Row {
                leftPadding: 10
                spacing: 10

                Image {
                    id: busyIndicator
                    anchors.verticalCenter: parent.verticalCenter
                    visible: vecs.discovering
                    width:25
                    height:25
                    source: "busy_dark.png"
                    fillMode: Image.PreserveAspectFit
                    NumberAnimation on rotation { duration: 3000; from:0; to: 360; loops: Animation.Infinite}
                }

                BorToolButton {
                    height: toolBar.height
                    text: "Discover sensors"
                    onClicked: {
                        vecs.startScan();
                    }
                }

                BorToolButton {
                    height: toolBar.height
                    text: "Start session"
                    onClicked: {
                        vecs.startSession();
                    }
                }
            }
        }

        ListView {
            id: listDevices
            width: parent.width
            anchors.top: toolBar.bottom
            anchors.bottom: statusBar.top

            model: vecs.model
            delegate: Rectangle {
                id: deviceDelegate
                width: parent.width
                height: column.height

                Connections {
                    target: modelData
                    onKeyPressed: {
                        beepButton.checked = false;
                    }
                }

                Column {
                    id: column
                    Row {
                        id: row
                        padding: 10
                        spacing: 10

                        Image {
                            id: vecsIcon
                            height: 48
                            fillMode: Image.PreserveAspectFit
                            anchors.verticalCenter: row.verticalCenter
                            source: "vecs_icon.svg"
                        }

                        Column {
                            width: 150
                            Text { text: modelData.address; font.bold: true }
                            Text { text: "<b>rssi:</b> " + modelData.rssi + "dBm" }
                            Text { text: "<b>status:</b> " + (modelData.connectionState === 2 ? "connected" : "disconnected") }
                            Text { text: "<b>role:</b> " + Helper.deviceRoleWrap(modelData.role) }
                        }

                        BorCheckButton {
                            anchors.verticalCenter: row.verticalCenter
                            text: {
                                switch (modelData.connectionState) {
                                case VecsDevice.StateDisconnected:
                                    return "Connect";
                                case VecsDevice.StateConnecting:
                                    return "Connecting...";
                                case VecsDevice.StateConnected:
                                    return "Disconnect";
                                }
                            }

                            checked: modelData.connectionState === VecsDevice.StateConnected

                            onCheckedChanged: {
                                if (!checked) {
                                    setupButton.checked = false;
                                }
                            }

                            onClicked:  {
                                switch (modelData.connectionState) {
                                case VecsDevice.StateDisconnected:
                                    modelData.connectToDevice();
                                    break;
                                case VecsDevice.StateConnected:
                                    modelData.disconnectFromDevice();                                    
                                    break;
                                }
                            }
                        }
                        BorCheckButton {
                            id: setupButton
                            anchors.verticalCenter: row.verticalCenter
                            text: "Settings"
                            onClicked: checked = !checked
                        }
                        BorDelayButton {
                            id: beepButton
                            anchors.verticalCenter: row.verticalCenter
                            visible: modelData.connectionState === VecsDevice.StateConnected
                            delay: 500
                            text: "Beep"
                            onActivated: {
                                modelData.keyRequest();
                            }
                        }
                        BorCheckButton {
                            id: mpuStartButton
                            anchors.verticalCenter: row.verticalCenter
                            visible: modelData.connectionState === VecsDevice.StateConnected
                            text: checked ? "Stop" : "Start"
                            checked: modelData.mpuState
                            onClicked: {
                                if (!modelData.mpuState) {
                                    modelData.mpuStart();
                                } else {
                                    modelData.mpuStop();
                                }

                            }
                        }                        
                    }
                    Row {
                        id: rowSettings

                        leftPadding: 10
                        bottomPadding: 5
                        spacing: 10

                        property bool visibleState: setupButton.checked

                        visible: false
                        opacity: 0

                        Column {
                            Row {
                                Text { width: roleSlider.width / 4; text: "Undefined"; horizontalAlignment: Text.AlignHCenter; font.pointSize: 9 }
                                Text { width: roleSlider.width / 4; }
                                Text { width: roleSlider.width / 4; text: "Patient (Hand)"; horizontalAlignment: Text.AlignHCenter; font.pointSize: 9 }
                                Text { width: roleSlider.width / 4; }
                            }
                            BorSlider {
                                id: roleSlider
                                count: 4
                                value: modelData.role
                                onValueChanged: {
                                    modelData.setRole(value);
                                }
                            }
                            Row {
                                Text { width: roleSlider.width / 4; }
                                Text { width: roleSlider.width / 4; text: "Doctor"; horizontalAlignment: Text.AlignHCenter; font.pointSize: 9 }
                                Text { width: roleSlider.width / 4; }
                                Text { width: roleSlider.width / 4; text: "Patient (Back)"; horizontalAlignment: Text.AlignHCenter; font.pointSize: 9 }
                            }
                        }
                        Column {
                            Text {
                                font.pointSize: 10
                                text: "Accelerometer range:"
                            }
                            BorSlider {
                                id: accelSlider
                                count: 4
                                value: modelData.accelRange
                                onValueChanged: {
                                    modelData.setAccelRange(value);
                                }
                            }
                            Row {
                                Text { width: accelSlider.width / 4; text: "±2G"; horizontalAlignment: Text.AlignHCenter; font.pointSize: 9 }
                                Text { width: accelSlider.width / 4; text: "±4G"; horizontalAlignment: Text.AlignHCenter; font.pointSize: 9 }
                                Text { width: accelSlider.width / 4; text: "±8G"; horizontalAlignment: Text.AlignHCenter; font.pointSize: 9 }
                                Text { width: accelSlider.width / 4; text: "±16G"; horizontalAlignment: Text.AlignHCenter; font.pointSize: 9 }
                            }
                        }
                        Column {
                            Text {
                                font.pointSize: 10
                                text: "Gyroscope range:"
                            }
                            BorSlider {
                                id: gyroSlider
                                count: 4
                                value: modelData.gyroRange
                                onValueChanged: {
                                    modelData.setGyroRange(value);
                                }
                            }
                            Row {
                                Text { width: gyroSlider.width / 4; text: "±250°/s"; horizontalAlignment: Text.AlignHCenter; font.pointSize: 9 }
                                Text { width: gyroSlider.width / 4; text: "±500°/s"; horizontalAlignment: Text.AlignHCenter; font.pointSize: 9 }
                                Text { width: gyroSlider.width / 4; text: "±1000°/s"; horizontalAlignment: Text.AlignHCenter; font.pointSize: 9 }
                                Text { width: gyroSlider.width / 4; text: "±2000°/s"; horizontalAlignment: Text.AlignHCenter; font.pointSize: 9 }
                            }
                        }

                        onOpacityChanged: {
                            visible = (opacity != 0);
                        }

                        states: [
                            State {
                                when: rowSettings.visibleState;
                                PropertyChanges {   target: rowSettings; opacity: 1.0    }
                            },
                            State {
                                when: !rowSettings.visibleState;
                                PropertyChanges {   target: rowSettings; opacity: 0.0    }
                            }
                        ]
                        transitions: Transition {
                            NumberAnimation { property: "opacity"; duration: 500}
                        }
                    }
                    Row {
                        id: rowInfo

                        property bool visibleState: modelData.connectionState === VecsDevice.StateConnected

                        visible: false
                        opacity: 0

                        leftPadding: 10
                        spacing: 10

                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            Text { text: "<b>battery</b>: " + modelData.batteryLevel + "%" }
                        }

                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            Text { text: "<b>single clicks:</b> " + modelData.singleClickCount }
                            Text { text: "<b>double clicks:</b> " + modelData.doubleClickCount }
                            Text { text: "<b>long clicks:</b> " + modelData.longClickCount }
                        }

                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            Text { text: "<b>MPU Rate:</b> " + modelData.mpuRate + "Hz"}
                            Text { text: "<b>Gyro Range:</b> ±" + Helper.gyroRangeWrap(modelData.gyroRange) + "°/s" }
                            Text { text: "<b>Accel Range:</b> ±" + Helper.accelRangeWrap(modelData.accelRange) + "G" }
                        }

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: "<b>Accel:</b>"
                        }

                        BorGauge {
                            maxAbsValue: 32768
                            height: 50
                            value: modelData.accelX
                        }
                        BorGauge {
                            maxAbsValue: 32768
                            height: 50
                            value: modelData.accelY
                        }
                        BorGauge {
                            maxAbsValue: 32768
                            height: 50
                            value: modelData.accelZ
                        }

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: "<b>Gyro:</b>"
                        }
                        BorGauge {
                            maxAbsValue: 32768
                            height: 50
                            value: modelData.gyroX
                        }
                        BorGauge {
                            maxAbsValue: 32768
                            height: 50
                            value: modelData.gyroY
                        }
                        BorGauge {
                            maxAbsValue: 32768
                            height: 50
                            value: modelData.gyroZ
                        }

                        onOpacityChanged: {
                            visible = (opacity != 0);
                        }

                        states: [
                            State {
                                when: rowInfo.visibleState;
                                PropertyChanges {   target: rowInfo; opacity: 1.0    }
                            },
                            State {
                                when: !rowInfo.visibleState;
                                PropertyChanges {   target: rowInfo; opacity: 0.0    }
                            }
                        ]
                        transitions: Transition {
                            NumberAnimation { property: "opacity"; duration: 500}
                        }                        
                    }
                }
            }
            populate: Transition {
                NumberAnimation { property: "opacity"; from: 0; to: 1.0; duration: 500 }
            }
        }

        Rectangle {
            id: statusBar
            width: parent.width
            anchors.bottom: parent.bottom
            height: 25
            color: "#1d5c97"            

            Text {
                anchors.centerIn: parent
                font.pointSize: 10
                text: vecs.message
                color: "white"
            }
        }
    }    
}
