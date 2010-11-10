/*
 * struct_rgb.h
 *
 *  Created on: 26.07.2010
 *      Author: Mike Shatohin (brunql)
 *     Project: AmbilightUSB
 *
 *  AmbilightUSB is very simple implementation of the backlight for a laptop
 *
 *  Copyright (c) 2010 Mike Shatohin, mikeshatohin [at] gmail.com
 *
 *  AmbilightUSB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  AmbilightUSB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

 
#ifndef RGB_H
#define RGB_H

struct sRGB
{
    unsigned char r;
    unsigned char g;
    unsigned char b;

    sRGB(){
        this->r = 0;
        this->g = 0;
        this->b = 0;
    }

    sRGB(unsigned char r, unsigned char g, unsigned char b){
        this->r = r;
        this->g = g;
        this->b = b;
    }

    unsigned char operator[] (int colorIndex){
        switch(colorIndex){
        case R: return this->r;
        case G: return this->g;
        case B: return this->b;
        }
        return -1;
    }
};

#endif // RGB_H
