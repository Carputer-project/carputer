import QtQuick 2.15
import QtQuick.Controls 2.15
Item {
    focus: true;
    property var internalWiFiManager: null
    Rectangle {
        anchors.fill: parent;
        color: bgDark;
        Column {
            anchors.fill: parent;
            anchors.margins: 10;
            spacing: 10;
            // Header
            Rectangle {
                width: parent.width; height: 50;
                color: bgCard; radius: 8;
                Text {
                    anchors.centerIn: parent;
                    text: "INTERNAL WIFI";
                    color: carBlue; font.pixelSize: 22; font.bold: true;
                }
            }
            // WiFi Status
            Rectangle {
                width: parent.width; height: 100;
                color: bgCard; radius: 8;
                Column {
                    anchors.fill: parent;
                    anchors.margins: 10;
                    spacing: 5;
                    Text {
                        text: internalWiFiManager ? (internalWiFiManager.connected ? "Connected" : "Not Connected") : "N/A";
                        color: internalWiFiManager && internalWiFiManager.connected ? themeManager.statusGreen : textSecondary;
                        font.pixelSize: 16; font.bold: true;
                    }
                    Text {
                        text: internalWiFiManager ? "SSID: " + internalWiFiManager.ssid : "";
                        color: textPrimary; font.pixelSize: 12;
                    }
                    Text {
                        text: internalWiFiManager ? "IP: " + internalWiFiManager.ipAddress : "";
                        color: textPrimary; font.pixelSize: 12;
                    }
                    Text {
                        text: internalWiFiManager ? "Signal: " + internalWiFiManager.signalStrength + " dBm" : "";
                        color: textPrimary; font.pixelSize: 12;
                    }
                }
            }
            // Connect to Carputer_ECU button
            Button {
                width: parent.width; height: 50;
                text: "Connect to Carputer_ECU (Static IP)"
                onClicked: {
                    if (internalWiFiManager)
                        internalWiFiManager.connectToCarputerECU()
                }
            }
            // Status text
            Rectangle {
                width: parent.width; height: 50; color: bgCard; radius: 8;
                Text {
                    anchors.centerIn: parent;
                    text: internalWiFiManager ? internalWiFiManager.statusText : "";
                    color: textSecondary; font.pixelSize: 12;
                }
            }
        }
    }
}
