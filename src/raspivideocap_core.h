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

#ifndef RASPI_CAM_IF_CORE_H_
#define RASPI_CAM_IF_CORE_H_

#include <opencv2/opencv.hpp>
#include <opencv2/core/version.hpp>

#if CV_MAJOR_VERSION == 4
#define CV_CAP_PROP_FRAME_WIDTH  cv::CAP_PROP_FRAME_WIDTH
#define CV_CAP_PROP_FRAME_HEIGHT cv::CAP_PROP_FRAME_HEIGHT
#define CV_CAP_PROP_FPS          cv::CAP_PROP_FPS
#endif

#include "bcm_host.h"
#include "interface/vcos/vcos.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"

#include "RaspiCamControl.h"

class raspi_video_capture
{
protected:
    int m_num_buffers;
    
    int m_width;
    int m_height;
    int m_framerate;
    int m_monochrome;

    int m_finished;

	MMAL_COMPONENT_T *m_camera_component;
	MMAL_POOL_T *m_video_pool;

    cv::Mat m_img_buf;

	VCOS_SEMAPHORE_T m_capture_sem;
	VCOS_SEMAPHORE_T m_capture_done_sem;

    static void video_buffer_callback( MMAL_PORT_T *port,
                                       MMAL_BUFFER_HEADER_T *buffer );
    bool create_camera_component();

    void release_objects();

public:
    raspi_video_capture( int num_buffers ) :
        m_num_buffers( num_buffers ),
        m_camera_component( NULL ),
        m_video_pool( NULL )
        {}
    ~raspi_video_capture();

    bool open( int width, int height, int framerate, int monochrome,
               int vflip, int hflip,
               int exposure_mode,
               float analog_gain, float digital_gain, int shutter_speed,
               int awb_mode,
               float awb_gains_r, float awb_gains_b );
    int read( cv::Mat &img );
    double get( int property_id );
    int is_opened();
    void release();
};

#endif
