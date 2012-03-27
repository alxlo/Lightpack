/*
 * LedDevicePentawallMini.cpp
 *
 *  Created on: 25.03.2012
 *      Author: Alexander Lorz
 *     Project: Lightpack
 *
 *  based on the Adalight code by Mike Shatohin
 *  and PentalightMini code by sebseb7 (https://github.com/sebseb7/PentawallHD)
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

#include "LedDevicePentawallMini.hpp"
#include "LightpackMath.hpp"
#include "Settings.hpp"
#include "debug.h"
#include "stdio.h"

using namespace SettingsScope;

LedDevicePentawallMini::LedDevicePentawallMini(QObject * parent) : ILedDevice(parent)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    m_gamma = Settings::getDeviceGamma();
    m_brightness = Settings::getDeviceBrightness();

    // TODO: think about init m_savedColors in all ILedDevices

    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "initialized";
}

LedDevicePentawallMini::~LedDevicePentawallMini()
{
    if (m_PentawallMiniDevice != NULL)
        m_PentawallMiniDevice->close();

    delete m_PentawallMiniDevice;
}

void LedDevicePentawallMini::setColors(const QList<QRgb> & colors)
{
    // Save colors for showing changes of the brightness
    m_colorsSaved = colors;

    resizeColorsBuffer(colors.count());

    LightpackMath::gammaCorrection(m_gamma, colors, m_colorsBuffer);
    LightpackMath::brightnessCorrection(m_brightness, m_colorsBuffer);

    m_writeBuffer.clear();

    for (int i = 0; i < m_colorsBuffer.count(); i++)
    {
        StructRgb color = m_colorsBuffer[i];

        m_writeBuffer.append(color.r);
        m_writeBuffer.append(color.g);
        m_writeBuffer.append(color.b);
    }

    bool ok = writeBuffer(m_writeBuffer);

    emit commandCompleted(ok);
}

void LedDevicePentawallMini::offLeds()
{
    int count = m_colorsSaved.count();
    m_colorsSaved.clear();

    for (int i = 0; i < count; i++)
        m_colorsSaved << 0;

    setColors(m_colorsSaved);
}

void LedDevicePentawallMini::setRefreshDelay(int /*value*/)
{
    emit commandCompleted(true);
}

void LedDevicePentawallMini::setColorDepth(int /*value*/)
{
    emit commandCompleted(true);
}

void LedDevicePentawallMini::setSmoothSlowdown(int /*value*/)
{
    emit commandCompleted(true);
}

void LedDevicePentawallMini::setGamma(double value)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << value;

    m_gamma = value;
    setColors(m_colorsSaved);
}

void LedDevicePentawallMini::setBrightness(int percent)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << percent;

    m_brightness = percent;
    setColors(m_colorsSaved);
}

void LedDevicePentawallMini::requestFirmwareVersion()
{
    emit firmwareVersion("unknown (PentawallMini device)");
    emit commandCompleted(true);
}

void LedDevicePentawallMini::updateDeviceSettings()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    setGamma(Settings::getDeviceGamma());
    setBrightness(Settings::getDeviceBrightness());
}

void LedDevicePentawallMini::open()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    m_gamma = Settings::getDeviceGamma();
    m_brightness = Settings::getDeviceBrightness();

    m_PentawallMiniDevice = new AbstractSerial();

    m_PentawallMiniDevice->setDeviceName(Settings::getSerialPortName());

    bool ok = m_PentawallMiniDevice->open(AbstractSerial::WriteOnly | AbstractSerial::Unbuffered);

    // Ubuntu 10.04: on every second attempt to open the device leads to failure
    if (ok == false)
    {
        // Try one more time
        ok = m_PentawallMiniDevice->open(AbstractSerial::WriteOnly | AbstractSerial::Unbuffered);
    }

    if (ok)
    {
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Serial device" << m_PentawallMiniDevice->deviceName() << "open";

        ok = m_PentawallMiniDevice->setBaudRate(Settings::getSerialPortBaudRate());
        if (ok)
        {
            ok = m_PentawallMiniDevice->setDataBits(AbstractSerial::DataBits8);
            if (ok)
            {
                DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Baud rate  :" << m_PentawallMiniDevice->baudRate();
                DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Data bits  :" << m_PentawallMiniDevice->dataBits();
                DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Parity     :" << m_PentawallMiniDevice->parity();
                DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Stop bits  :" << m_PentawallMiniDevice->stopBits();
                DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Flow       :" << m_PentawallMiniDevice->flowControl();
            } else {
                qWarning() << Q_FUNC_INFO << "Set data bits 8 fail";
            }
        } else {
            qWarning() << Q_FUNC_INFO << "Set baud rate" << Settings::getSerialPortBaudRate() << "fail";
        }

    } else {
        qWarning() << Q_FUNC_INFO << "Serial device" << m_PentawallMiniDevice->deviceName() << "open fail";
    }

    emit openDeviceSuccess(ok);
}

bool LedDevicePentawallMini::writeBuffer(const QByteArray & buff)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << "Hex:" << buff.toHex();

    if (m_PentawallMiniDevice->isOpen() == false)
        return false;

	m_PentawallMiniDevice->write("\x42");



	m_PentawallMiniDevice->write("\x00",1);


//	write_escaped(buff[0]);

	write_escaped(buff[0]);
	write_escaped(buff[1]);
	write_escaped(buff[2]);
	//m_PentawallMiniDevice->write(&c,1);
	//c = buff[1];
	//m_PentawallMiniDevice->write(&c,1);
	//c = buff[2];
	//m_PentawallMiniDevice->write(&c,1);
//    m_PentawallMiniDevice->write(""+(char) 0x42,1);
//    m_PentawallMiniDevice->write(""+(char) 0x00,1);
//    m_PentawallMiniDevice->write(""+(char) 0x,1);
//    m_PentawallMiniDevice->write(""+(char) 0x00,1);
//    m_PentawallMiniDevice->write(""+(char) 0x00,1);
    //int bytesWritten = m_PentawallMiniDevice->write(buff);

//    if (bytesWritten != buff.count())
//    {
//        qWarning() << Q_FUNC_INFO << "bytesWritten != buff.count():" << bytesWritten << buff.count();
//        return false;
//    }

    return true;
}

void LedDevicePentawallMini::resizeColorsBuffer(int buffSize)
{
    if (m_colorsBuffer.count() == buffSize)
        return;

    m_colorsBuffer.clear();

    if (buffSize > MaximumNumberOfLeds::PentawallMini)
    {
        qCritical() << Q_FUNC_INFO << "buffSize > MaximumNumberOfLeds::PentawallMini" << buffSize << ">" << MaximumNumberOfLeds::PentawallMini;

        buffSize = MaximumNumberOfLeds::PentawallMini;
    }

    for (int i = 0; i < buffSize; i++)
    {
        m_colorsBuffer << StructRgb();
    }

}



void LedDevicePentawallMini::write_escaped(char byte)
{

	if(byte == 0x23)
	{
		m_PentawallMiniDevice->write("\x65");
		m_PentawallMiniDevice->write("\x01");
	}
	else if(byte == 0x42)
	{
		m_PentawallMiniDevice->write("\x65");
		m_PentawallMiniDevice->write("\x02");
	}
	else if(byte == 0x65)
	{
		m_PentawallMiniDevice->write("\x65");
		m_PentawallMiniDevice->write("\x03");
	}
	else if(byte == 0x66)
	{
		m_PentawallMiniDevice->write("\x65");
		m_PentawallMiniDevice->write("\x04");
		}
	else
	{
		m_PentawallMiniDevice->write(&byte,1);
	}
}
