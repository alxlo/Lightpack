/*
 * ApiServer.hpp
 *
 *  Created on: 07.09.2011
 *     Authors: Andrey Isupov && Mike Shatohin
 *     Project: Lightpack
 *
 *  Lightpack is very simple implementation of the backlight for a laptop
 *
 *  Copyright (c) 2011 Mike Shatohin, mikeshatohin [at] gmail.com
 *
 *  Lightpack is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Lightpack is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QSet>
#include <QRgb>
#include "ApiServerSetColorTask.hpp"
#include "SettingsWindow.hpp"
#include "debug.h"

struct ClientInfo
{
    bool isAuthorized;
    // Think about it. May be we need to save gamma,
    // smooth and brightness and after success lock send
    // this values to device?
};

class ApiServer : public QTcpServer
{
    Q_OBJECT

public:
    ApiServer(QObject *parent = 0);
    ApiServer(quint16 port, QObject *parent = 0);

    void firstStart();

public:
    static const char * ApiVersion;   
    static const char * CmdUnknown;
    static const char * CmdExit;

    static const char * CmdApiKey;
    static const char * CmdApiKeyResult_Ok;
    static const char * CmdApiKeyResult_Fail;
    static const char * CmdApiCheck_AuthRequired;

    static const char * CmdGetStatus;
    static const char * CmdResultStatus_On;
    static const char * CmdResultStatus_Off;
    static const char * CmdResultStatus_DeviceError;
    static const char * CmdResultStatus_Unknown;

    static const char * CmdGetStatusAPI;
    static const char * CmdResultStatusAPI_Busy;
    static const char * CmdResultStatusAPI_Idle;

    static const char * CmdGetProfiles;
    static const char * CmdResultProfiles;

    static const char * CmdGetProfile;
    static const char * CmdResultProfile;

    static const char * CmdLock;
    static const char * CmdResultLock_Success;
    static const char * CmdResultLock_Busy;

    static const char * CmdUnlock;
    static const char * CmdResultUnlock_Success;
    static const char * CmdResultUnlock_NotLocked;

    // Set-commands works only after success lock
    static const char * CmdSetResult_Ok;
    static const char * CmdSetResult_Error;
    static const char * CmdSetResult_Busy;
    static const char * CmdSetResult_NotLocked;

    static const char * CmdSetColor;
    static const char * CmdSetGamma;
    static const char * CmdSetBrightness;
    static const char * CmdSetSmooth;
    static const char * CmdSetProfile;

    static const char * CmdSetStatus;
    static const char * CmdSetStatus_On;
    static const char * CmdSetStatus_Off;

    static const int SignalWaitTimeoutMs;

signals:
    void requestBacklightStatus();
    void startParseSetColorTask(QByteArray buffer);
    void updateLedsColors(const QList<QRgb> & colors);
    void updateGamma(double value);
    void updateBrightness(int value);
    void updateSmooth(int value);
    void updateProfile(QString profileName);
    void updateStatus(Backlight::Status status);
    void updateDeviceLockStatus(Api::DeviceLockStatus status);
    void errorOnStartListening(QString errorMessage);
    void clearColorBuffers();

public slots:
    void enableApiServer(bool isEnabled);
    void enableApiAuth(bool isEnabled);
    void updateApiPort(int port);
    void updateApiKey(QString key);

protected:
    void incomingConnection(int socketDescriptor);

private slots:
    void clientDisconnected();
    void clientProcessCommands();
    void resultBacklightStatus(Backlight::Status status);
    void taskSetColorIsSuccess(bool isSuccess);

private:
    void initPrivateVariables();
    void initApiSetColorTask();
    void startListening();
    void stopListening();
    void writeData(QTcpSocket* client, const QString & data);

private:
    int m_apiPort;
    QString m_apiAuthKey;
    bool m_isAuthEnabled;

    QTcpSocket *m_lockedClient;
    QMap <QTcpSocket*, ClientInfo> m_clients;

    QThread *m_apiSetColorTaskThread;
    ApiServerSetColorTask *m_apiSetColorTask;

    QTime m_time;

    bool m_isTaskSetColorDone;
    bool m_isTaskSetColorParseSuccess;

    bool m_isRequestBacklightStatusDone;
    Backlight::Status m_backlightStatusResult;
};
