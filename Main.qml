import QtQuick
import QtQuick.Controls.Windows
import QtQuick.Controls
Window {
    width: 380
    height: 600
    visible: true
    title: qsTr("zwh meeting")

    property int count: 0
    Rectangle{
        width: 320
        height: 500
        id: loginBackground
        anchors.centerIn: parent
        color: "grey"
        gradient: Gradient {
            GradientStop {position: 0.50; color: "yellow"}
            GradientStop {position: 1.0; color: "green"}
        }
        Text {
            id: loginText
            text: qsTr("登录")
            font.pixelSize: 20
            font.bold: true

        }
        Text {
            id:accText
            anchors.top: loginText.bottom
            anchors.topMargin: 40
            text: qsTr("账号")
            font.bold: true
            font.pixelSize: 16
        }
        TextField {
            id:accEdit
            width: 300
            height: 30
            anchors.top: accText.bottom
            text: "请输入账号"
            font.italic: true
            font.pixelSize: 16
            color: "grey"
            wrapMode: TextEdit.NoWrap
            maximumLength: 16
            background: Rectangle{
                color: "transparent"
            }
            Rectangle{
                width: parent.width
                height: 1
                anchors.bottom: parent.bottom
                color: "grey"
            }


            onFocusChanged: {
                if(focus && text === "请输入账号"){
                    text = "";
                    color = "black"
                    font.italic = false
                }else if(!focus && text === "") {
                    text = "请输入账号"
                    color = "grey"
                    font.italic = true
                }
            }

        }
        Text {
            id:passText
            anchors.top: accText.bottom
            anchors.topMargin: 40
            text: qsTr("密码")
            font.bold: true
            font.pixelSize: 16
        }
        TextField {
            id:passEdit
            width: 300
            height: 30
            anchors.top: passText.bottom
            text: "请输入密码"
            font.italic: true
            font.pixelSize: 16
            color: "grey"
            wrapMode: TextEdit.NoWrap
            maximumLength: 16
            echoMode: "Password"
            background: Rectangle{
                color: "transparent"
            }

            Rectangle{
                width: parent.width
                height: 1
                anchors.bottom: parent.bottom
                color: "grey"
            }
            onFocusChanged: {
                if(focus && text === "请输入密码"){
                    text = "";
                    color = "black"
                    font.italic = false
                }else if(!focus && text === "") {
                    text = "请输入密码"
                    color = "grey"
                    font.italic = true
                }
            }
        }
        Item {
            id:login
            width: 200
            height: 50
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: passEdit.bottom
            anchors.topMargin: 20
            property bool canLogin: false
            Rectangle{
                id:loginButtonB
                anchors.fill: parent
                radius: 20
                color: "#80b7ff"
            }
            Text {
                id:loginButtonT
                anchors.centerIn: parent
                text: qsTr("登录")
                color: "white"
            }
            MouseArea{
                hoverEnabled: true
                anchors.fill: parent
                onEntered: {
                    if(accEdit.text !== "请输入账号" && passEdit.text !== "请输入密码" ){
                        login.canLogin = true
                        loginButtonB.color = "#0000ff"
                    }

                }
                onExited: {
                    if(accEdit.text !== "请输入账号" && passEdit.text !== "请输入密码"){
                        login.canLogin = false
                        loginButtonB.color = "#80b7ff"
                    }
                }
                onClicked: {

                    if(login.canLogin === true) {
                        console.log("zzzzz")
                        loginButtonT.text = "正在登录"
                    }
                }
            }



        }
    }
}
