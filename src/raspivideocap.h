/*
  Copyright 2019 coyote009

  This file is part of raspivideocap.

  raspivideocap is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  raspivideocap is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with raspivideocap.  If not, see <http://www.gnu.org/licenses/>.

 */

#ifndef RASPI_CAM_IF_H_
#define RASPI_CAM_IF_H_

#include <opencv2/opencv.hpp>

class raspi_video_capture;
class RaspiVideoCapture
{
public:
    enum EXPOSURE_MODE
    {
        EXPOSUREMODE_OFF,
        EXPOSUREMODE_AUTO,
        EXPOSUREMODE_NIGHT,
        EXPOSUREMODE_NIGHTPREVIEW,
        EXPOSUREMODE_BACKLIGHT,
        EXPOSUREMODE_SPOTLIGHT,
        EXPOSUREMODE_SPORTS,
        EXPOSUREMODE_SNOW,
        EXPOSUREMODE_BEACH,
        EXPOSUREMODE_VERYLONG,
        EXPOSUREMODE_FIXEDFPS,
        EXPOSUREMODE_ANTISHAKE,
        EXPOSUREMODE_FIREWORKS,
        EXPOSUREMODE_MAX = 0x7fffffff
    };

    enum AWB_MODE
    {
        AWBMODE_OFF,
        AWBMODE_AUTO,
        AWBMODE_SUNLIGHT,
        AWBMODE_CLOUDY,
        AWBMODE_SHADE,
        AWBMODE_TUNGSTEN,
        AWBMODE_FLUORESCENT,
        AWBMODE_INCANDESCENT,
        AWBMODE_FLASH,
        AWBMODE_HORIZON,
        AWBMODE_MAX = 0x7fffffff
    };
    
protected:
    raspi_video_capture *m_pcore;
    int m_num_buffers;

public:
    RaspiVideoCapture( int num_buffers = 10 );
    ~RaspiVideoCapture();
    bool open( int width, int height, int framerate, int monochrome = 0,
               int vflip = 0, int hflip = 0,
               EXPOSURE_MODE exposure_mode = EXPOSUREMODE_AUTO,
               float analog_gain = 0, float digital_gain = 0, int shutter_speed = 0,
               AWB_MODE awb_mode = AWBMODE_AUTO,
               float awb_gains_r = 0.0, float awb_gains_b = 0.0 );
    int read( cv::Mat &img );
    double get( int property_id );
    int isOpened();
    void release();
};

#endif
