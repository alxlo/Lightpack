/*
 * LedDevicePentawallNet.hpp
 *
 *  Created on: 25.03.2012
 *      Author: Alexander Lorz
 *     Project: Lightpack
 *
 *  based on th Adalight code by Timur Sattarov && Mike Shatohin
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

#pragma once

#include "ILedDevice.hpp"
#include "StructRgb.hpp"
#include "abstractserial.h"

class LedDevicePentawallNet : public ILedDevice
{
    Q_OBJECT
public:
    LedDevicePentawallNet(QObject * parent = 0);
    ~LedDevicePentawallNet();

public slots:
    void open();
    void setColors(const QList<QRgb> & /*colors*/);
    void offLeds();
    void setRefreshDelay(int /*value*/);
    void setColorDepth(int /*value*/);
    void setSmoothSlowdown(int /*value*/);
    void setGamma(double /*value*/);
    void setBrightness(int /*value*/);
    void requestFirmwareVersion();
    void updateDeviceSettings();

private:
    bool writeBuffer(const QByteArray & buff);
    void resizeColorsBuffer(int buffSize);
    void write_escaped(char byte);
private:
    AbstractSerial *m_PentawallNetDevice;

    QByteArray m_writeBuffer;

    double m_gamma;
    int m_brightness;

    QList<QRgb> m_colorsSaved;
    QList<StructRgb> m_colorsBuffer;
};
