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

#include "raspivideocap_core.h"
#include "raspivideocap.h"

RaspiVideoCapture::RaspiVideoCapture( int num_buffers ) :
    m_pcore( NULL ), m_num_buffers( num_buffers )
{
}

RaspiVideoCapture::~RaspiVideoCapture()
{
    if( m_pcore )
    {
        delete m_pcore;
        m_pcore = NULL;
    }
}

bool RaspiVideoCapture::open( int width, int height, int framerate, int monochrome,
                              int vflip, int hflip,
                              EXPOSURE_MODE exposure_mode,
                              float analog_gain, float digital_gain, int shutter_speed,
                              AWB_MODE awb_mode,
                              float awb_gains_r, float awb_gains_b )
{
    if( !m_pcore )
    {
        m_pcore = new raspi_video_capture( m_num_buffers );
    }

    return m_pcore->open( width, height, framerate, monochrome, vflip, hflip,
                          exposure_mode, analog_gain, digital_gain, shutter_speed,
                          awb_mode, awb_gains_r, awb_gains_b );
}

int RaspiVideoCapture::read( cv::Mat &img )
{
    if( m_pcore )
    {
        return m_pcore->read( img );
    }
    else
    {
        return 0;
    }
}

double RaspiVideoCapture::get( int property_id )
{
    if( m_pcore )
    {
        return  m_pcore->get( property_id );
    }
    else
    {
        return 0.0;
    }
}

int RaspiVideoCapture::isOpened()
{
    if( m_pcore )
    {
        return m_pcore->is_opened();
    }
    else
    {
        return 0;
    }
}

void RaspiVideoCapture::release()
{
    if( m_pcore )
    {
        m_pcore->release();
    }
}
