/*
 * Settings.cpp
 *
 *  Created on: 22.02.2011
 *      Author: Mike Shatohin (brunql)
 *     Project: Lightpack
 *
 *  Lightpack is very simple implementation of the backlight for a laptop
 *
 *  Copyright (c) 2010, 2011 Mike Shatohin, mikeshatohin [at] gmail.com
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

#include "Settings.hpp"

#include <QtDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QSize>
#include <QPoint>
#include <QFileInfo>
#include <QDir>
#include <QUuid>

#include "debug.h"

using namespace SettingsScope;

#define MAIN_CONFIG_FILE_VERSION    "1.0"

//
// This strings keys and values must be accessible only in current file
//
namespace SettingsScope
{
namespace Main
{
namespace Key
{
// [General]
static const QString MainConfigVersion = "MainConfigVersion";
static const QString ProfileLast = "ProfileLast";
static const QString Language = "Language";
static const QString DebugLevel = "DebugLevel";
static const QString IsExpertModeEnabled = "IsExpertModeEnabled";
static const QString IsSwitchOffAtClosing = "IsSwitchOffAtClosing";
static const QString IsPingDeviceEverySecond = "IsPingDeviceEverySecond";
static const QString IsUpdateFirmwareMessageShown = "IsUpdateFirmwareMessageShown";
static const QString ConnectedDevice = "ConnectedDevice";
static const QString SupportedDevices = "SupportedDevices";

// [Hotkeys]
namespace Hotkeys
{
static const QString OnOffDeviceKey = "HotKeys/OnOffDeviceKey";
}

// [API]
namespace Api
{
static const QString IsEnabled = "API/IsEnabled";
static const QString Port = "API/Port";
static const QString IsAuthEnabled = "API/IsAuthEnabled";
static const QString AuthKey = "API/AuthKey";
}
// [SerialPort]
namespace SerialPort
{
static const QString Port = "SerialPort/Port";
static const QString BaudRate = "SerialPort/BaudRate";
}
namespace Adalight
{
static const QString NumberOfLeds = "Adalight/NumberOfLeds";
}
namespace PentawallMini
{
static const QString NumberOfLeds = "PentawallMini/NumberOfLeds";
}
namespace PentawallNet
{
static const QString NumberOfLeds = "PentawallNet/NumberOfLeds";
}
namespace Ardulight
{
static const QString NumberOfLeds = "Ardulight/NumberOfLeds";
}
namespace AlienFx
{
static const QString NumberOfLeds = "AlienFx/NumberOfLeds";
}
namespace Lightpack
{
static const QString NumberOfLeds = "Lightpack/NumberOfLeds";
}
namespace Virtual
{
static const QString NumberOfLeds = "Virtual/NumberOfLeds";
}
} /*Key*/

namespace Value
{
static const QString MainConfigVersion = MAIN_CONFIG_FILE_VERSION;

namespace ConnectedDevice
{
static const QString LightpackDevice = "Lightpack";
static const QString AlienFxDevice = "AlienFx";
static const QString AdalightDevice = "Adalight";
static const QString PentawallMiniDevice = "PentawallMini";
static const QString PentawallNetDevice = "PentawallNet";
static const QString ArdulightDevice = "Ardulight";
static const QString VirtualDevice = "Virtual";
}

} /*Value*/
} /*Main*/

namespace Profile
{
namespace Key
{
// [General]
static const QString LightpackMode = "LightpackMode";
static const QString IsBacklightEnabled = "IsBacklightEnabled";
// [Grab]
namespace Grab
{
static const QString Grabber = "Grab/Grabber";
static const QString IsAvgColorsEnabled = "Grab/IsAvgColorsEnabled";
static const QString IsSendDataOnlyIfColorsChanges = "Grab/IsSendDataOnlyIfColorsChanges";
static const QString Slowdown = "Grab/Slowdown";
static const QString MinimumLevelOfSensitivity = "Grab/MinimumLevelOfSensitivity";
}
// [MoodLamp]
namespace MoodLamp
{
static const QString IsLiquidMode = "MoodLamp/LiquidMode";
static const QString Color = "MoodLamp/Color";
static const QString Speed = "MoodLamp/Speed";
}
// [Device]
namespace Device
{
static const QString RefreshDelay = "Device/RefreshDelay";
static const QString Smooth = "Device/Smooth";
static const QString Brightness = "Device/Brightness";
static const QString ColorDepth = "Device/ColorDepth";
static const QString Gamma = "Device/Gamma";
}
// [LED_i]
namespace Led
{
static const QString Prefix = "LED_";
static const QString IsEnabled = "IsEnabled";
static const QString Size = "Size";
static const QString Position = "Position";
static const QString CoefRed = "CoefRed";
static const QString CoefGreen = "CoefGreen";
static const QString CoefBlue = "CoefBlue";
}
} /*Key*/

namespace Value
{

namespace LightpackMode
{
static const QString Ambilight = "Ambilight";
static const QString MoodLamp = "MoodLamp";
}

namespace GrabberType
{
static const QString Qt = "Qt";
static const QString QtEachWidget = "QtEachWidget";
static const QString WinAPI = "WinAPI";
static const QString WinAPIEachWidget = "WinAPIEachWidget";
static const QString X11 = "X11";
static const QString D3D9 = "D3D9";
static const QString MacCoreGraphics = "MacCoreGraphics";
}

} /*Value*/
} /*Profile*/
} /*SettingsScope*/

QMutex Settings::m_mutex;
QSettings * Settings::m_currentProfile;
QSettings * Settings::m_mainConfig; // LightpackMain.conf contains last profile

// Path to directory there store application generated stuff
QString Settings::m_applicationDirPath = "";

QMap<SupportedDevices::DeviceType, QString> Settings::m_devicesTypeToNameMap;
QMap<SupportedDevices::DeviceType, QString> Settings::m_devicesTypeToKeyNumberOfLedsMap;

// Desktop should be initialized before call Settings::Initialize()
void Settings::Initialize( const QString & applicationDirPath, bool isDebugLevelObtainedFromCmdArgs)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    m_applicationDirPath = applicationDirPath;

    // Append to the end of dir path '/'
    if (m_applicationDirPath.lastIndexOf('/') != m_applicationDirPath.length() - 1)
        m_applicationDirPath += "/";

    m_mainConfig = new QSettings(m_applicationDirPath + "LightpackMain.conf", QSettings::IniFormat);
    m_mainConfig->setIniCodec("UTF-8");

    // TODO: if version < Main::Value::MainConfigVersion then clear lightpack main config file
    setNewOptionMain(Main::Key::MainConfigVersion,      Main::Value::MainConfigVersion, true /* rewrite */);
    setNewOptionMain(Main::Key::ProfileLast,            Main::ProfileNameDefault);
    setNewOptionMain(Main::Key::Language,               Main::LanguageDefault);
    setNewOptionMain(Main::Key::DebugLevel,             Main::DebugLevelDefault);
    setNewOptionMain(Main::Key::IsExpertModeEnabled,    Main::IsExpertModeEnabledDefault);
    setNewOptionMain(Main::Key::IsSwitchOffAtClosing,   Main::IsSwitchOffAtClosingDefault);
    setNewOptionMain(Main::Key::IsPingDeviceEverySecond,Main::IsPingDeviceEverySecond);
    setNewOptionMain(Main::Key::IsUpdateFirmwareMessageShown, Main::IsUpdateFirmwareMessageShown);
    setNewOptionMain(Main::Key::ConnectedDevice,        Main::ConnectedDeviceDefault);
    setNewOptionMain(Main::Key::SupportedDevices,       Main::SupportedDevices, true /* always rewrite this information to main config */);
    setNewOptionMain(Main::Key::Hotkeys::OnOffDeviceKey,Main::HotKeys::OnOffDeviceKeyDefault);
    setNewOptionMain(Main::Key::Api::IsEnabled,         Main::Api::IsEnabledDefault);
    setNewOptionMain(Main::Key::Api::Port,              Main::Api::PortDefault);
    setNewOptionMain(Main::Key::Api::IsAuthEnabled,     Main::Api::IsAuthEnabledDefault);
    // Generation AuthKey as new UUID
    setNewOptionMain(Main::Key::Api::AuthKey,           QUuid::createUuid().toString());

    // Serial device configuration
    setNewOptionMain(Main::Key::SerialPort::Port,       Main::SerialPort::PortDefault);
    setNewOptionMain(Main::Key::SerialPort::BaudRate,   Main::SerialPort::BaudRateDefault);

    // Init number of leds for each supported device
    setNewOptionMain(Main::Key::Adalight::NumberOfLeds,     Main::Adalight::NumberOfLedsDefault);
    setNewOptionMain(Main::Key::PentawallMini::NumberOfLeds,     Main::PentawallMini::NumberOfLedsDefault);
    setNewOptionMain(Main::Key::PentawallNet::NumberOfLeds,     Main::PentawallNet::NumberOfLedsDefault);
    setNewOptionMain(Main::Key::Ardulight::NumberOfLeds,    Main::Ardulight::NumberOfLedsDefault);
    setNewOptionMain(Main::Key::AlienFx::NumberOfLeds,      Main::AlienFx::NumberOfLedsDefault);
    setNewOptionMain(Main::Key::Lightpack::NumberOfLeds,    Main::Lightpack::NumberOfLedsDefault);
    setNewOptionMain(Main::Key::Virtual::NumberOfLeds,      Main::Virtual::NumberOfLedsDefault);

    if (isDebugLevelObtainedFromCmdArgs == false)
    {
        bool ok = false;
        int sDebugLevel = valueMain(Main::Key::DebugLevel).toInt(&ok);

        if (ok && sDebugLevel >= 0)
        {
            g_debugLevel = sDebugLevel;
            DEBUG_LOW_LEVEL << Q_FUNC_INFO << "debugLevel =" << g_debugLevel;
        } else {
            qWarning() << "DebugLevel in config has an invalid value, set the default" << Main::DebugLevelDefault;
            setValueMain(Main::Key::DebugLevel, Main::DebugLevelDefault);
            g_debugLevel = Main::DebugLevelDefault;
        }
    }

    QString profileLast = getLastProfileName();

    // Load last profile
    m_currentProfile = new QSettings(m_applicationDirPath + "Profiles/" + profileLast + ".ini", QSettings::IniFormat);
    m_currentProfile->setIniCodec("UTF-8");

    DEBUG_LOW_LEVEL << "Settings file:" << m_currentProfile->fileName();

    // Initialize m_devicesMap for mapping DeviceType on DeviceName
    initDevicesMap();

    // Initialize profile with default values without reset exists values
    initCurrentProfile(false);
}

//
//  Set all settings in current config to default values
//
void Settings::resetDefaults()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    initCurrentProfile(true /* = reset to default values */);
}

QStringList Settings::findAllProfiles()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    QFileInfo setsFile(Settings::getCurrentProfilePath());
    QFileInfoList iniFiles = setsFile.absoluteDir().entryInfoList(QStringList("*.ini"));

    QStringList settingsFiles;
    for(int i=0; i<iniFiles.count(); i++){
        QString compBaseName = iniFiles.at(i).completeBaseName();
        settingsFiles.append(compBaseName);
    }

    return settingsFiles;
}

void Settings::loadOrCreateProfile(const QString & profileName)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << profileName;

    QMutexLocker locker(&m_mutex);

    if (m_currentProfile != NULL)
    {
        // Copy current settings to new one
        QString profilesDir = QFileInfo(m_currentProfile->fileName()).absoluteDir().absolutePath();
        QString profileNewPath = profilesDir + "/" + profileName + ".ini";

        if (m_currentProfile->fileName() != profileNewPath)
            QFile::copy(m_currentProfile->fileName(), profileNewPath);

        delete m_currentProfile;
    }


    m_currentProfile = new QSettings(m_applicationDirPath + "Profiles/" + profileName + ".ini", QSettings::IniFormat );
    m_currentProfile->setIniCodec("UTF-8");

    locker.unlock();
    initCurrentProfile(false);
    locker.relock();

    DEBUG_LOW_LEVEL << "Settings file:" << m_currentProfile->fileName();

    m_mainConfig->setValue(Main::Key::ProfileLast, profileName);
}

void Settings::renameCurrentProfile(const QString & profileName)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "from" << getCurrentProfileName() << "to" << profileName;

    QMutexLocker locker(&m_mutex);

    if (m_currentProfile == NULL)
    {
        qWarning() << "void Settings::renameCurrentConfig(): fail, m_currentProfile not initialized";
        return;
    }

    // Copy current settings to new one
    QString profilesDir = QFileInfo( m_currentProfile->fileName() ).absoluteDir().absolutePath();
    QString profileNewPath = profilesDir + "/" + profileName + ".ini";

    if (m_currentProfile->fileName() != profileNewPath)
    {
        QFile::rename(m_currentProfile->fileName(), profileNewPath);

        delete m_currentProfile;

        // Update m_currentProfile point to new QSettings with configName
        m_currentProfile = new QSettings(m_applicationDirPath + "Profiles/" + profileName + ".ini", QSettings::IniFormat );
        m_currentProfile->setIniCodec("UTF-8");

        DEBUG_LOW_LEVEL << "Settings file renamed:" << m_currentProfile->fileName();

        m_mainConfig->setValue(Main::Key::ProfileLast, profileName);
    }

    m_currentProfile->sync();
}

void Settings::removeCurrentProfile()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    QMutexLocker locker(&m_mutex);

    if (m_currentProfile == NULL)
    {
        qWarning() << Q_FUNC_INFO << "current profile not loaded, nothing to remove";
        return;
    }

    bool result = QFile::remove( m_currentProfile->fileName() );

    if (result == false)
    {
        qWarning() << Q_FUNC_INFO << "QFile::remove(" << m_currentProfile->fileName() << ") fail";
        return;
    }

    delete m_currentProfile;
    m_currentProfile = NULL;

    m_mainConfig->setValue(Main::Key::ProfileLast, Main::ProfileNameDefault);
}

QString Settings::getCurrentProfileName()
{
    QMutexLocker locker(&m_mutex);

    DEBUG_MID_LEVEL << Q_FUNC_INFO << m_currentProfile->fileName();
    return QFileInfo(m_currentProfile->fileName()).completeBaseName();
}

QString Settings::getCurrentProfilePath()
{
    QMutexLocker locker(&m_mutex);

    DEBUG_MID_LEVEL << Q_FUNC_INFO << m_currentProfile->fileName();
    return m_currentProfile->fileName();
}

QString Settings::getApplicationDirPath()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << m_applicationDirPath;
    return m_applicationDirPath;
}

QPoint Settings::getDefaultPosition(int ledIndex)
{
    QPoint result;

    if (ledIndex > (MaximumNumberOfLeds::Default - 1))
    {
        int x = (ledIndex - MaximumNumberOfLeds::Default) * 10 /* px */;
        return QPoint(x, 0);
    }

    QRect screen = QApplication::desktop()->screenGeometry();

    int ledsCountDiv2 = MaximumNumberOfLeds::Default / 2;

    if (ledIndex < ledsCountDiv2)
    {
        result.setX(0);
    } else {
        result.setX(screen.width() - Profile::Led::SizeDefault.width());
    }

    int height = ledsCountDiv2 * Profile::Led::SizeDefault.height();

    int y = screen.height() / 2 - height / 2;

    result.setY(y + (ledIndex % ledsCountDiv2) * Profile::Led::SizeDefault.height());

    return result;
}

QString Settings::getLastProfileName()
{
    return valueMain(Main::Key::ProfileLast).toString();
}

QString Settings::getLanguage()
{
    return valueMain(Main::Key::Language).toString();
}

void Settings::setLanguage(const QString & language)
{
    setValueMain(Main::Key::Language, language);
}

int Settings::getDebugLevel()
{
    return valueMain(Main::Key::DebugLevel).toInt();
}

void Settings::setDebugLevel(int debugLvl)
{
    setValueMain(Main::Key::DebugLevel, debugLvl);
}

bool Settings::isApiEnabled()
{
    return valueMain(Main::Key::Api::IsEnabled).toBool();
}

void Settings::setIsApiEnabled(bool isEnabled)
{
    setValueMain(Main::Key::Api::IsEnabled, isEnabled);
}

int Settings::getApiPort()
{
    return valueMain(Main::Key::Api::Port).toInt();
}

void Settings::setApiPort(int apiPort)
{
    setValueMain(Main::Key::Api::Port, apiPort);
}

QString Settings::getApiAuthKey()
{
    QString apikey = valueMain(Main::Key::Api::AuthKey).toString();

    if (apikey == "")
    {
        qWarning() << Q_FUNC_INFO << "ApiKey is empty, generate new one.";

        apikey = QUuid::createUuid().toString();

        setApiKey(apikey);
    }

    return apikey;
}

void Settings::setApiKey(const QString & apiKey)
{
    setValueMain(Main::Key::Api::AuthKey, apiKey);
}

bool Settings::isApiAuthEnabled()
{
    return valueMain(Main::Key::Api::IsAuthEnabled).toBool();
}

void Settings::setIsApiAuthEnabled(bool isEnabled)
{
    setValueMain(Main::Key::Api::IsAuthEnabled, isEnabled);
}

bool Settings::isExpertModeEnabled()
{   
    return valueMain(Main::Key::IsExpertModeEnabled).toBool();
}

void Settings::setExpertModeEnabled(bool isEnabled)
{
    setValueMain(Main::Key::IsExpertModeEnabled, isEnabled);
}

bool Settings::isSwitchOffAtClosing()
{
    return valueMain(Main::Key::IsSwitchOffAtClosing).toBool();
}

void Settings::setSwitchOffAtClosing(bool isEnabled)
{
    setValueMain(Main::Key::IsSwitchOffAtClosing, isEnabled);
}

bool Settings::isPingDeviceEverySecond()
{
    return valueMain(Main::Key::IsPingDeviceEverySecond).toBool();
}

void Settings::setPingDeviceEverySecond(bool isEnabled)
{
    setValueMain(Main::Key::IsPingDeviceEverySecond, isEnabled);
}

bool Settings::isUpdateFirmwareMessageShown()
{
    return valueMain(Main::Key::IsUpdateFirmwareMessageShown).toBool();
}

void Settings::setUpdateFirmwareMessageShown(bool isShown)
{
    setValueMain(Main::Key::IsUpdateFirmwareMessageShown, isShown);
}

SupportedDevices::DeviceType Settings::getConnectedDevice()
{
    QString deviceName = valueMain(Main::Key::ConnectedDevice).toString();

    if (m_devicesTypeToNameMap.values().contains(deviceName) == false)
    {
        qWarning() << Q_FUNC_INFO << Main::Key::ConnectedDevice << "in main config contains crap or unsupported device,"
                   << "reset it to default value:" << Main::ConnectedDeviceDefault;

        setConnectedDevice(SupportedDevices::DefaultDevice);
        deviceName = Main::ConnectedDeviceDefault;
    }

    return m_devicesTypeToNameMap.key(deviceName, SupportedDevices::DefaultDevice);
}

void Settings::setConnectedDevice(SupportedDevices::DeviceType device)
{
    QString deviceName = m_devicesTypeToNameMap.value(device, Main::ConnectedDeviceDefault);

    setValueMain(Main::Key::ConnectedDevice, deviceName);
}

QString Settings::getConnectedDeviceName()
{
    return m_devicesTypeToNameMap.value(getConnectedDevice(), Main::ConnectedDeviceDefault);
}

void Settings::setConnectedDeviceName(const QString & deviceName)
{
    if (deviceName == "")
        return; // silent return

    if (m_devicesTypeToNameMap.values().contains(deviceName) == false)
    {
        qCritical() << Q_FUNC_INFO << "Failure during check the device name" << deviceName << "in m_devicesMap. The main config has not changed.";
        return;
    }

    setValueMain(Main::Key::ConnectedDevice, deviceName);
}

QStringList Settings::getSupportedDevices()
{
    return Main::SupportedDevices.split(',');
}

QKeySequence Settings::getOnOffDeviceKey()
{
    if( valueMain(Main::Key::Hotkeys::OnOffDeviceKey) == "Undefined") {
        return QKeySequence();
    }
    QKeySequence returnSequence = QKeySequence( valueMain(Main::Key::Hotkeys::OnOffDeviceKey).toString() );
    return returnSequence;
}

void Settings::setOnOffDeviceKey(const QKeySequence &keySequence)
{
    if( keySequence.isEmpty() ) {
        setValueMain(Main::Key::Hotkeys::OnOffDeviceKey, Main::HotKeys::OnOffDeviceKeyDefault);
    } else {
        setValueMain(Main::Key::Hotkeys::OnOffDeviceKey, keySequence.toString());
    }
}

QString Settings::getSerialPortName()
{
    return valueMain(Main::Key::SerialPort::Port).toString();
}

void Settings::setSerialPortName(const QString & port)
{
    setValueMain(Main::Key::SerialPort::Port, port);
}

QString Settings::getSerialPortBaudRate()
{
    // TODO: validate baudrate reading from settings file
    return valueMain(Main::Key::SerialPort::BaudRate).toString();
}

void Settings::setSerialPortBaudRate(const QString & baud)
{
    // TODO: validator
    setValueMain(Main::Key::SerialPort::BaudRate, baud);
}

QStringList Settings::getSupportedSerialPortBaudRates()
{
    QStringList list;

    // TODO: Add more baud rates if need it
    list.append("230400");
    list.append("115200");
    list.append("57600");
    list.append("9600");
    return list;
}

bool Settings::isConnectedDeviceUsesSerialPort()
{
    switch (getConnectedDevice())
    {
    case SupportedDevices::AdalightDevice:
        return true;
    case SupportedDevices::PentawallMiniDevice:
        return true;
    case SupportedDevices::PentawallNetDevice:
        return true;
    case SupportedDevices::ArdulightDevice:
        return true;
    default:
        return false;
    }
}

void Settings::setNumberOfLeds(SupportedDevices::DeviceType device, int numberOfLeds)
{
    QString key = m_devicesTypeToKeyNumberOfLedsMap.value(device);

    if (key == "")
    {
        qCritical() << Q_FUNC_INFO << "Device type not recognized, device ==" << device << "numberOfLeds ==" << numberOfLeds;
        return;
    }

    setValueMain(key, numberOfLeds);
}

int Settings::getNumberOfLeds(SupportedDevices::DeviceType device)
{
    QString key = m_devicesTypeToKeyNumberOfLedsMap.value(device);

    if (key == "")
    {
        qCritical() << Q_FUNC_INFO << "Device type not recognized, device ==" << device;
        return MaximumNumberOfLeds::Default;
    }

    // TODO: validator on maximum number of leds for current 'device'
    return valueMain(key).toInt();
}

int Settings::getGrabSlowdown()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    return getValidGrabSlowdown(value(Profile::Key::Grab::Slowdown).toInt());
}

void Settings::setGrabSlowdown(int value)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    setValue(Profile::Key::Grab::Slowdown, getValidGrabSlowdown(value));
}

bool Settings::isBacklightEnabled()
{
    return value(Profile::Key::IsBacklightEnabled).toBool();
}

void Settings::setIsBacklightEnabled(bool isEnabled)
{
    setValue(Profile::Key::IsBacklightEnabled, isEnabled);
}

bool Settings::isGrabAvgColorsEnabled()
{
    return value(Profile::Key::Grab::IsAvgColorsEnabled).toBool();
}

void Settings::setGrabAvgColorsEnabled(bool isEnabled)
{
    setValue(Profile::Key::Grab::IsAvgColorsEnabled, isEnabled);
}

bool Settings::isSendDataOnlyIfColorsChanges()
{
    return value(Profile::Key::Grab::IsSendDataOnlyIfColorsChanges).toBool();
}

void Settings::setSendDataOnlyIfColorsChanges(bool isEnabled)
{
    setValue(Profile::Key::Grab::IsSendDataOnlyIfColorsChanges, isEnabled);
}

int Settings::getGrabMinimumLevelOfSensitivity()
{
    return value(Profile::Key::Grab::MinimumLevelOfSensitivity).toInt();
}

void Settings::setGrabMinimumLevelOfSensitivity(int value)
{
    setValue(Profile::Key::Grab::MinimumLevelOfSensitivity, value);
}

int Settings::getDeviceRefreshDelay()
{
    return getValidDeviceRefreshDelay(value(Profile::Key::Device::RefreshDelay).toInt());
}

void Settings::setDeviceRefreshDelay(int value)
{
    setValue(Profile::Key::Device::RefreshDelay, getValidDeviceRefreshDelay(value));
}

int Settings::getDeviceBrightness()
{
    return getValidDeviceBrightness(value(Profile::Key::Device::Brightness).toInt());
}

void Settings::setDeviceBrightness(int value)
{
    setValue(Profile::Key::Device::Brightness, getValidDeviceBrightness(value));
}

int Settings::getDeviceSmooth()
{
    return getValidDeviceSmooth(value(Profile::Key::Device::Smooth).toInt());
}

void Settings::setDeviceSmooth(int value)
{
    setValue(Profile::Key::Device::Smooth, getValidDeviceSmooth(value));
}

int Settings::getDeviceColorDepth()
{
    return getValidDeviceColorDepth(value(Profile::Key::Device::ColorDepth).toInt());
}

void Settings::setDeviceColorDepth(int value)
{
    setValue(Profile::Key::Device::ColorDepth, getValidDeviceColorDepth(value));
}

double Settings::getDeviceGamma()
{
    return getValidDeviceGamma(value(Profile::Key::Device::Gamma).toDouble());
}

void Settings::setDeviceGamma(double gamma)
{
    setValue(Profile::Key::Device::Gamma, getValidDeviceGamma(gamma));
}

Grab::GrabberType Settings::getGrabberType()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    QString strGrabber = value(Profile::Key::Grab::Grabber).toString();

    if (strGrabber == Profile::Value::GrabberType::Qt)
        return Grab::QtGrabber;
    if (strGrabber == Profile::Value::GrabberType::QtEachWidget)
        return Grab::QtEachWidgetGrabber;

#ifdef WINAPI_GRAB_SUPPORT
    if (strGrabber == Profile::Value::GrabberType::WinAPI)
        return Grab::WinAPIGrabber;
    if (strGrabber == Profile::Value::GrabberType::WinAPIEachWidget)
        return Grab::WinAPIEachWidgetGrabber;
#endif

#ifdef D3D9_GRAB_SUPPORT
    if (strGrabber == Profile::Value::GrabberType::D3D9)
        return Grab::D3D9Grabber;
#endif

#ifdef X11_GRAB_SUPPORT
    if (strGrabber == Profile::Value::GrabberType::X11)
        return Grab::X11Grabber;
#endif

#ifdef MAC_OS_CG_GRAB_SUPPORT
    if (strGrabber == Profile::Value::GrabberType::MacCoreGraphics)
        return Grab::MacCoreGraphicsGrabber;
#endif

    qWarning() << Q_FUNC_INFO << Profile::Key::Grab::Grabber << "contains invalid value:" << strGrabber << ", reset it to default:" << Profile::Grab::GrabberDefaultString;
    setGrabberType(Profile::Grab::GrabberDefault);

    return Profile::Grab::GrabberDefault;
}

void Settings::setGrabberType(Grab::GrabberType grabberType)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << grabberType;

    QString strGrabber;
    switch (grabberType)
    {
    case Grab::QtGrabber:
        strGrabber = Profile::Value::GrabberType::Qt;
        break;

    case Grab::QtEachWidgetGrabber:
        strGrabber = Profile::Value::GrabberType::QtEachWidget;
        break;

#ifdef WINAPI_GRAB_SUPPORT
    case Grab::WinAPIGrabber:
        strGrabber = Profile::Value::GrabberType::WinAPI;
        break;
    case Grab::WinAPIEachWidgetGrabber:
        strGrabber = Profile::Value::GrabberType::WinAPIEachWidget;
        break;
#endif

#ifdef D3D9_GRAB_SUPPORT
    case Grab::D3D9Grabber:
        strGrabber = Profile::Value::GrabberType::D3D9;
        break;
#endif

#ifdef X11_GRAB_SUPPORT
    case Grab::X11Grabber:
        strGrabber = Profile::Value::GrabberType::X11;
        break;
#endif

#ifdef MAC_OS_CG_GRAB_SUPPORT
    case Grab::MacCoreGraphicsGrabber:
        strGrabber = Profile::Value::GrabberType::MacCoreGraphics;
        break;
#endif

    default:
        qWarning() << Q_FUNC_INFO << "Switch on grabberType =" << grabberType << "failed. Reset to default value.";
        strGrabber = Profile::Grab::GrabberDefaultString;
    }
    setValue(Profile::Key::Grab::Grabber, strGrabber);
}

Lightpack::Mode Settings::getLightpackMode()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    QString strMode = value(Profile::Key::LightpackMode).toString();

    if (strMode == Profile::Value::LightpackMode::Ambilight)
    {
        return Lightpack::AmbilightMode;
    }
    else if (strMode == Profile::Value::LightpackMode::MoodLamp)
    {
        return Lightpack::MoodLampMode;
    }
    else
    {
        qWarning() << Q_FUNC_INFO << "Read fail. Reset to default value = " << Lightpack::Default;

        setLightpackMode(Lightpack::Default);
        return Lightpack::Default;
    }
}

void Settings::setLightpackMode(Lightpack::Mode mode)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << mode;

    if (mode == Lightpack::AmbilightMode)
    {
        setValue(Profile::Key::LightpackMode, Profile::Value::LightpackMode::Ambilight);
    }
    else if (mode == Lightpack::MoodLampMode)
    {
        setValue(Profile::Key::LightpackMode, Profile::Value::LightpackMode::MoodLamp);
    }
    else
    {
        qCritical() << Q_FUNC_INFO << "Invalid value =" << mode;
    }
}

bool Settings::isMoodLampLiquidMode()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    return value(Profile::Key::MoodLamp::IsLiquidMode).toBool();
}

void Settings::setMoodLampLiquidMode(bool value)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    setValue(Profile::Key::MoodLamp::IsLiquidMode, value );
}

QColor Settings::getMoodLampColor()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    return QColor(value(Profile::Key::MoodLamp::Color).toString());
}

void Settings::setMoodLampColor(QColor value)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << value.name();
    setValue(Profile::Key::MoodLamp::Color, value.name() );
}

int Settings::getMoodLampSpeed()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    return getValidMoodLampSpeed(value(Profile::Key::MoodLamp::Speed).toInt());
}

void Settings::setMoodLampSpeed(int value)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;
    setValue(Profile::Key::MoodLamp::Speed, getValidMoodLampSpeed(value));
}

double Settings::getLedCoefRed(int ledIndex)
{
    return getValidLedCoef(ledIndex, Profile::Key::Led::CoefRed);
}

double Settings::getLedCoefGreen(int ledIndex)
{
    return getValidLedCoef(ledIndex, Profile::Key::Led::CoefGreen);
}

double Settings::getLedCoefBlue(int ledIndex)
{
    return getValidLedCoef(ledIndex, Profile::Key::Led::CoefBlue);
}

void Settings::setLedCoefRed(int ledIndex, double value)
{
    setValidLedCoef(ledIndex, Profile::Key::Led::CoefRed, value);
}

void Settings::setLedCoefGreen(int ledIndex, double value)
{
    setValidLedCoef(ledIndex, Profile::Key::Led::CoefGreen, value);
}

void Settings::setLedCoefBlue(int ledIndex, double value)
{
    setValidLedCoef(ledIndex, Profile::Key::Led::CoefBlue, value);
}

QSize Settings::getLedSize(int ledIndex)
{
    return value(Profile::Key::Led::Prefix + QString::number(ledIndex + 1) + "/" + Profile::Key::Led::Size).toSize();
}

void Settings::setLedSize(int ledIndex, QSize size)
{
    setValue(Profile::Key::Led::Prefix + QString::number(ledIndex + 1) + "/" + Profile::Key::Led::Size, size);
}

QPoint Settings::getLedPosition(int ledIndex)
{
    return value(Profile::Key::Led::Prefix + QString::number(ledIndex + 1) + "/" + Profile::Key::Led::Position).toPoint();
}

void Settings::setLedPosition(int ledIndex, QPoint position)
{
    setValue(Profile::Key::Led::Prefix + QString::number(ledIndex + 1) + "/" + Profile::Key::Led::Position, position);
}

bool Settings::isLedEnabled(int ledIndex)
{
    QVariant result = value(Profile::Key::Led::Prefix + QString::number(ledIndex + 1) + "/" + Profile::Key::Led::IsEnabled);
    if (result.isNull())
        return Profile::Led::IsEnabledDefault;
    else
        return result.toBool();
}

void Settings::setLedEnabled(int ledIndex, bool isEnabled)
{
    setValue(Profile::Key::Led::Prefix + QString::number(ledIndex + 1) + "/" + Profile::Key::Led::IsEnabled, isEnabled);
}

int Settings::getValidDeviceRefreshDelay(int value)
{
    if (value < Profile::Device::RefreshDelayMin)
        value = Profile::Device::RefreshDelayMin;
    else if (value > Profile::Device::RefreshDelayMax)
        value = Profile::Device::RefreshDelayMax;
    return value;
}

int Settings::getValidDeviceBrightness(int value)
{
    if (value < Profile::Device::BrightnessMin)
        value = Profile::Device::BrightnessMin;
    else if (value > Profile::Device::BrightnessMax)
        value = Profile::Device::BrightnessMax;
    return value;
}

int Settings::getValidDeviceSmooth(int value)
{
    if (value < Profile::Device::SmoothMin)
        value = Profile::Device::SmoothMin;
    else if (value > Profile::Device::SmoothMax)
        value = Profile::Device::SmoothMax;
    return value;
}

int Settings::getValidDeviceColorDepth(int value)
{
    if (value < Profile::Device::ColorDepthMin)
        value = Profile::Device::ColorDepthMin;
    else if (value > Profile::Device::ColorDepthMax)
        value = Profile::Device::ColorDepthMax;
    return value;
}

double Settings::getValidDeviceGamma(double value)
{
    if (value < Profile::Device::GammaMin)
        value = Profile::Device::GammaMin;
    else if (value > Profile::Device::GammaMax)
        value = Profile::Device::GammaMax;
    return value;
}

int Settings::getValidGrabSlowdown(int value)
{
    if (value < Profile::Grab::SlowdownMin)
        value = Profile::Grab::SlowdownMin;
    else if (value > Profile::Grab::SlowdownMax)
        value = Profile::Grab::SlowdownMax;
    return value;
}

int Settings::getValidMoodLampSpeed(int value)
{
    if (value < Profile::MoodLamp::SpeedMin)
        value = Profile::MoodLamp::SpeedMin;
    else if (value > Profile::MoodLamp::SpeedMax)
        value = Profile::MoodLamp::SpeedMax;
    return value;
}

void Settings::setValidLedCoef(int ledIndex, const QString & keyCoef, double coef)
{
    if (coef < Profile::Led::CoefMin || coef > Profile::Led::CoefMax){
        QString error = "Error: outside the valid values (coef < " +
                QString::number(Profile::Led::CoefMin) + " || coef > " + QString::number(Profile::Led::CoefMax) + ").";

        qWarning() << Q_FUNC_INFO << "Settings bad value"
                   << "[" + Profile::Key::Led::Prefix + QString::number(ledIndex + 1) + "]"
                   << keyCoef
                   << error
                   << "Convert to double error. Set it to default value" << keyCoef << "=" << Profile::Led::CoefDefault;
        coef = Profile::Led::CoefDefault;        
    }
    Settings::setValue(Profile::Key::Led::Prefix + QString::number(ledIndex + 1) + "/" + keyCoef, coef);
}

double Settings::getValidLedCoef(int ledIndex, const QString & keyCoef)
{
    bool ok = false;
    double coef = Settings::value(Profile::Key::Led::Prefix + QString::number(ledIndex + 1) + "/" + keyCoef).toDouble(&ok);
    QString error;
    if (ok == false){
        error = "Error: Convert to double.";
    } else if (coef < Profile::Led::CoefMin || coef > Profile::Led::CoefMax){
        QString error = "Error: outside the valid values (coef < " +
                QString::number(Profile::Led::CoefMin) + " || coef > " + QString::number(Profile::Led::CoefMax) + ").";
    } else {
        // OK
        return coef;
    }
    // Have an error
    qWarning() << Q_FUNC_INFO << "Settings bad value"
               << "[" + Profile::Key::Led::Prefix + QString::number(ledIndex + 1) + "]"
               << keyCoef
               << error
               << "Set it to default value" << keyCoef << "=" << Profile::Led::CoefDefault;
    coef = Profile::Led::CoefDefault;
    Settings::setValue(Profile::Key::Led::Prefix + QString::number(ledIndex + 1) + "/" + keyCoef, coef);
    return coef;
}

//
//  Check and/or initialize settings
//
void Settings::initCurrentProfile(bool isResetDefault)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << isResetDefault;

    // [General]
    setNewOption(Profile::Key::LightpackMode, Profile::LightpackModeDefault, isResetDefault);
    setNewOption(Profile::Key::IsBacklightEnabled, Profile::IsBacklightEnabledDefault, isResetDefault);
    // [Grab]
    setNewOption(Profile::Key::Grab::Grabber,       Profile::Grab::GrabberDefaultString, isResetDefault);
    setNewOption(Profile::Key::Grab::IsAvgColorsEnabled, Profile::Grab::IsAvgColorsEnabledDefault, isResetDefault);
    setNewOption(Profile::Key::Grab::IsSendDataOnlyIfColorsChanges, Profile::Grab::IsSendDataOnlyIfColorsChangesDefault, isResetDefault);
    setNewOption(Profile::Key::Grab::Slowdown,      Profile::Grab::SlowdownDefault, isResetDefault);
    setNewOption(Profile::Key::Grab::MinimumLevelOfSensitivity, Profile::Grab::MinimumLevelOfSensitivityDefault, isResetDefault);
    // [MoodLamp]
    setNewOption(Profile::Key::MoodLamp::IsLiquidMode,  Profile::MoodLamp::IsLiquidMode, isResetDefault);
    setNewOption(Profile::Key::MoodLamp::Color,         Profile::MoodLamp::ColorDefault, isResetDefault);
    setNewOption(Profile::Key::MoodLamp::Speed,         Profile::MoodLamp::SpeedDefault, isResetDefault);
    // [Device]
    setNewOption(Profile::Key::Device::RefreshDelay,Profile::Device::RefreshDelayDefault, isResetDefault);
    setNewOption(Profile::Key::Device::Brightness,  Profile::Device::BrightnessDefault, isResetDefault);
    setNewOption(Profile::Key::Device::Smooth,      Profile::Device::SmoothDefault, isResetDefault);
    setNewOption(Profile::Key::Device::Gamma,       Profile::Device::GammaDefault, isResetDefault);
    setNewOption(Profile::Key::Device::ColorDepth,  Profile::Device::ColorDepthDefault, isResetDefault);

    QPoint ledPosition;

    for (int i = 0; i < MaximumNumberOfLeds::AbsoluteMaximum; i++)
    {
        ledPosition = getDefaultPosition(i);

        setNewOption(Profile::Key::Led::Prefix + QString::number(i + 1) + "/" + Profile::Key::Led::IsEnabled,
                     Profile::Led::IsEnabledDefault, isResetDefault);
        setNewOption(Profile::Key::Led::Prefix + QString::number(i + 1) + "/" + Profile::Key::Led::Position,
                     ledPosition, isResetDefault);
        setNewOption(Profile::Key::Led::Prefix + QString::number(i + 1) + "/" + Profile::Key::Led::Size,
                     Profile::Led::SizeDefault, isResetDefault);

        setNewOption(Profile::Key::Led::Prefix + QString::number(i + 1) + "/" + Profile::Key::Led::CoefRed,
                     Profile::Led::CoefDefault, isResetDefault);
        setNewOption(Profile::Key::Led::Prefix + QString::number(i + 1) + "/" + Profile::Key::Led::CoefGreen,
                     Profile::Led::CoefDefault, isResetDefault);
        setNewOption(Profile::Key::Led::Prefix + QString::number(i + 1) + "/" + Profile::Key::Led::CoefBlue,
                     Profile::Led::CoefDefault, isResetDefault);
    }

    QMutexLocker locker(&m_mutex);
    m_currentProfile->sync();
}


void Settings::setNewOption(const QString & name, const QVariant & value,
                                        bool isForceSetOption, QSettings * settings /*= m_currentProfile*/)
{
    QMutexLocker locker(&m_mutex);

    if (isForceSetOption)
    {
        settings->setValue(name, value);
    } else {
        if (settings->contains(name) == false)
        {
            DEBUG_LOW_LEVEL << "Settings:"<< name << "not found."
                    << "Set it to default value: " << value.toString();

            settings->setValue(name, value);
        }
        // else option exists do nothing
    }
}

void Settings::setNewOptionMain(const QString & name, const QVariant & value, bool isForceSetOption /*= false*/)
{
    setNewOption(name, value, isForceSetOption, m_mainConfig);
}

void Settings::setValueMain(const QString & key, const QVariant & value)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << key;

    QMutexLocker locker(&m_mutex);

    if (m_mainConfig == NULL)
    {
        qWarning() << Q_FUNC_INFO << "m_mainConfig == NULL";
        return;
    }
    m_mainConfig->setValue(key, value);
}

QVariant Settings::valueMain(const QString & key)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << key;

    QMutexLocker locker(&m_mutex);

    if (m_mainConfig == NULL)
    {
        qWarning() << Q_FUNC_INFO << "m_mainConfig == NULL";
        return QVariant();
    }
    return m_mainConfig->value(key);
}

void Settings::setValue(const QString & key, const QVariant & value)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << key;

    QMutexLocker locker(&m_mutex);

    if (m_currentProfile == NULL)
    {
        qWarning() << Q_FUNC_INFO << "m_currentProfile == NULL";
        return;
    }
    m_currentProfile->setValue(key, value);
}

QVariant Settings::value(const QString & key)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << key;

    QMutexLocker locker(&m_mutex);

    if (m_currentProfile == NULL)
    {
        qWarning() << Q_FUNC_INFO << "m_currentProfile == NULL";
        return QVariant();
    }
    return m_currentProfile->value(key);
}

void Settings::initDevicesMap()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    m_devicesTypeToNameMap[SupportedDevices::AdalightDevice]  = Main::Value::ConnectedDevice::AdalightDevice;
    m_devicesTypeToNameMap[SupportedDevices::PentawallMiniDevice]  = Main::Value::ConnectedDevice::PentawallMiniDevice;
    m_devicesTypeToNameMap[SupportedDevices::PentawallNetDevice]  = Main::Value::ConnectedDevice::PentawallNetDevice;
    m_devicesTypeToNameMap[SupportedDevices::ArdulightDevice] = Main::Value::ConnectedDevice::ArdulightDevice;
    m_devicesTypeToNameMap[SupportedDevices::LightpackDevice] = Main::Value::ConnectedDevice::LightpackDevice;
    m_devicesTypeToNameMap[SupportedDevices::VirtualDevice]   = Main::Value::ConnectedDevice::VirtualDevice;

    m_devicesTypeToKeyNumberOfLedsMap[SupportedDevices::AdalightDevice]  = Main::Key::Adalight::NumberOfLeds;
    m_devicesTypeToKeyNumberOfLedsMap[SupportedDevices::PentawallMiniDevice]  = Main::Key::PentawallMini::NumberOfLeds;
    m_devicesTypeToKeyNumberOfLedsMap[SupportedDevices::PentawallNetDevice]  = Main::Key::PentawallNet::NumberOfLeds;
    m_devicesTypeToKeyNumberOfLedsMap[SupportedDevices::ArdulightDevice] = Main::Key::Ardulight::NumberOfLeds;
    m_devicesTypeToKeyNumberOfLedsMap[SupportedDevices::LightpackDevice] = Main::Key::Lightpack::NumberOfLeds;
    m_devicesTypeToKeyNumberOfLedsMap[SupportedDevices::VirtualDevice]   = Main::Key::Virtual::NumberOfLeds;

#ifdef ALIEN_FX_SUPPORTED
    m_devicesTypeToNameMap[SupportedDevices::AlienFxDevice]   = Main::Value::ConnectedDevice::AlienFxDevice;
    m_devicesTypeToKeyNumberOfLedsMap[SupportedDevices::AlienFxDevice]   = Main::Key::AlienFx::NumberOfLeds;
#endif
}
