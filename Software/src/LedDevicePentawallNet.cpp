/*
 * LedDevicePentawallNet.cpp
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

#include "LedDevicePentawallNet.hpp"
#include "LightpackMath.hpp"
#include "Settings.hpp"
#include "debug.h"
#include "stdio.h"

using namespace SettingsScope;

LedDevicePentawallNet::LedDevicePentawallNet(QObject * parent) : ILedDevice(parent)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    m_gamma = Settings::getDeviceGamma();
    m_brightness = Settings::getDeviceBrightness();

    // TODO: think about init m_savedColors in all ILedDevices

    DEBUG_LOW_LEVEL << Q_FUNC_INFO << "initialized";
}


LedDevicePentawallNet::~LedDevicePentawallNet()
{
    if (m_PentawallNetDevice != NULL)
        m_PentawallNetDevice->close();

    delete m_PentawallNetDevice;
}

void LedDevicePentawallNet::setColors(const QList<QRgb> & colors)
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

void LedDevicePentawallNet::offLeds()
{
    int count = m_colorsSaved.count();
    m_colorsSaved.clear();

    for (int i = 0; i < count; i++)
        m_colorsSaved << 0;

    setColors(m_colorsSaved);
}

void LedDevicePentawallNet::setRefreshDelay(int /*value*/)
{
    emit commandCompleted(true);
}

void LedDevicePentawallNet::setColorDepth(int /*value*/)
{
    emit commandCompleted(true);
}

void LedDevicePentawallNet::setSmoothSlowdown(int /*value*/)
{
    emit commandCompleted(true);
}

void LedDevicePentawallNet::setGamma(double value)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << value;

    m_gamma = value;
    setColors(m_colorsSaved);
}

void LedDevicePentawallNet::setBrightness(int percent)
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO << percent;

    m_brightness = percent;
    setColors(m_colorsSaved);
}

void LedDevicePentawallNet::requestFirmwareVersion()
{
    emit firmwareVersion("unknown (PentawallNet device)");
    emit commandCompleted(true);
}

void LedDevicePentawallNet::updateDeviceSettings()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    setGamma(Settings::getDeviceGamma());
    setBrightness(Settings::getDeviceBrightness());
}

void LedDevicePentawallNet::open()
{
    DEBUG_LOW_LEVEL << Q_FUNC_INFO;

    m_gamma = Settings::getDeviceGamma();
    m_brightness = Settings::getDeviceBrightness();

    m_PentawallNetDevice = new AbstractSerial();

    m_PentawallNetDevice->setDeviceName(Settings::getSerialPortName());

    bool ok = m_PentawallNetDevice->open(AbstractSerial::WriteOnly | AbstractSerial::Unbuffered);

    // Ubuntu 10.04: on every second attempt to open the device leads to failure
    if (ok == false)
    {
        // Try one more time
        ok = m_PentawallNetDevice->open(AbstractSerial::WriteOnly | AbstractSerial::Unbuffered);
    }

    if (ok)
    {
        DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Serial device" << m_PentawallNetDevice->deviceName() << "open";

        ok = m_PentawallNetDevice->setBaudRate(Settings::getSerialPortBaudRate());
        if (ok)
        {
            ok = m_PentawallNetDevice->setDataBits(AbstractSerial::DataBits8);
            if (ok)
            {
                DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Baud rate  :" << m_PentawallNetDevice->baudRate();
                DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Data bits  :" << m_PentawallNetDevice->dataBits();
                DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Parity     :" << m_PentawallNetDevice->parity();
                DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Stop bits  :" << m_PentawallNetDevice->stopBits();
                DEBUG_LOW_LEVEL << Q_FUNC_INFO << "Flow       :" << m_PentawallNetDevice->flowControl();
            } else {
                qWarning() << Q_FUNC_INFO << "Set data bits 8 fail";
            }
        } else {
            qWarning() << Q_FUNC_INFO << "Set baud rate" << Settings::getSerialPortBaudRate() << "fail";
        }

    } else {
        qWarning() << Q_FUNC_INFO << "Serial device" << m_PentawallNetDevice->deviceName() << "open fail";
    }

    emit openDeviceSuccess(ok);
}

bool LedDevicePentawallNet::writeBuffer(const QByteArray & buff)
{
    DEBUG_MID_LEVEL << Q_FUNC_INFO << "Hex:" << buff.toHex();

    if (m_PentawallNetDevice->isOpen() == false)
        return false;




	char c = 0;
//	m_PentawallNetDevice->write("\x00",1);
    for (char i = 0; i < m_colorsBuffer.count(); i++)
    {

    	c = i+1;
    	 //qDebug() << Q_FUNC_INFO << "Test" << QString::number(c) << "TEST";
    	m_PentawallNetDevice->write("\x42");
    	write_escaped(c);
    	write_escaped(buff[3*i]);
    	write_escaped(buff[3*i+1]);
    	write_escaped(buff[3*i+2]);
    }



//	write_escaped(buff[0]);
//	write_escaped(buff[1]);
//	write_escaped(buff[2]);
	//m_PentawallNetDevice->write(&c,1);
	//c = buff[1];
	//m_PentawallNetDevice->write(&c,1);
	//c = buff[2];
	//m_PentawallNetDevice->write(&c,1);
//    m_PentawallNetDevice->write(""+(char) 0x42,1);
//    m_PentawallNetDevice->write(""+(char) 0x00,1);
//    m_PentawallNetDevice->write(""+(char) 0x,1);
//    m_PentawallNetDevice->write(""+(char) 0x00,1);
//    m_PentawallNetDevice->write(""+(char) 0x00,1);
    //int bytesWritten = m_PentawallNetDevice->write(buff);

//    if (bytesWritten != buff.count())
//    {
//        qWarning() << Q_FUNC_INFO << "bytesWritten != buff.count():" << bytesWritten << buff.count();
//        return false;
//    }

    return true;
}

void LedDevicePentawallNet::resizeColorsBuffer(int buffSize)
{
    if (m_colorsBuffer.count() == buffSize)
        return;

    m_colorsBuffer.clear();

    if (buffSize > MaximumNumberOfLeds::PentawallNet)
    {
        qCritical() << Q_FUNC_INFO << "buffSize > MaximumNumberOfLeds::PentawallNet" << buffSize << ">" << MaximumNumberOfLeds::PentawallNet;

        buffSize = MaximumNumberOfLeds::PentawallNet;
    }

    for (int i = 0; i < buffSize; i++)
    {
        m_colorsBuffer << StructRgb();
    }

}



void LedDevicePentawallNet::write_escaped(char byte)
{

	if(byte == 0x23)
	{
		m_PentawallNetDevice->write("\x65");
		m_PentawallNetDevice->write("\x01");
	}
	else if(byte == 0x42)
	{
		m_PentawallNetDevice->write("\x65");
		m_PentawallNetDevice->write("\x02");
	}
	else if(byte == 0x65)
	{
		m_PentawallNetDevice->write("\x65");
		m_PentawallNetDevice->write("\x03");
	}
	else if(byte == 0x66)
	{
		m_PentawallNetDevice->write("\x65");
		m_PentawallNetDevice->write("\x04");
		}
	else
	{
		m_PentawallNetDevice->write(&byte,1);
	}
}
