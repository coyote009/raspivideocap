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

// Standard port setting for the camera component
#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2

void raspi_video_capture::video_buffer_callback( MMAL_PORT_T *port,
                                                 MMAL_BUFFER_HEADER_T *buffer )
{
	MMAL_BUFFER_HEADER_T *new_buffer;
    raspi_video_capture *pcap = (raspi_video_capture *) port->userdata;

	if( pcap )
	{
		if( pcap->m_finished )
        {
			vcos_semaphore_post( &pcap->m_capture_done_sem );
			return;
		}

		if( buffer->length )
		{
			mmal_buffer_header_mem_lock( buffer );
 
			int data_size = pcap->m_monochrome ?
                pcap->m_img_buf.total() : pcap->m_img_buf.total() * 3;
			memcpy( pcap->m_img_buf.data, buffer->data, data_size );

			vcos_semaphore_post( &pcap->m_capture_done_sem );
			vcos_semaphore_wait( &pcap->m_capture_sem );

			mmal_buffer_header_mem_unlock( buffer );
		}
		else
		{
			vcos_log_error( "buffer null" );
		}
	}
	else
	{
		vcos_log_error( "Received a encoder buffer callback with no state" );
	}

	// release buffer back to the pool
	mmal_buffer_header_release( buffer );

	// and send one back to the port (if still open)
	if( port->is_enabled )
	{
		MMAL_STATUS_T status;

		new_buffer = mmal_queue_get( pcap->m_video_pool->queue );

		if( new_buffer )
        {
			status = mmal_port_send_buffer( port, new_buffer );
        }

		if( !new_buffer || status != MMAL_SUCCESS )
        {
			vcos_log_error("Unable to return a buffer to the encoder port");
        }
	}
}

bool raspi_video_capture::create_camera_component()
{
	MMAL_COMPONENT_T *camera;
	MMAL_ES_FORMAT_T *format;
	MMAL_PORT_T *video_port = NULL;
	MMAL_STATUS_T status;
	
	/* Create the component */
	status = mmal_component_create( MMAL_COMPONENT_DEFAULT_CAMERA, &camera );
	if( status != MMAL_SUCCESS )
	{
        vcos_log_error( "Failed to create camera component" );
        goto error;
	}
	
	if( !camera->output_num )
	{
        vcos_log_error( "Camera doesn't have output ports" );
        goto error;
	}
	
	video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];
	
	//  set up the camera configuration
	{
        MMAL_PARAMETER_CAMERA_CONFIG_T cam_config =
            {
                { MMAL_PARAMETER_CAMERA_CONFIG, sizeof(cam_config) },
                .max_stills_w = (uint32_t) m_width,
                .max_stills_h = (uint32_t) m_height,
                .stills_yuv422 = 0,
                .one_shot_stills = 0,
                .max_preview_video_w = (uint32_t) m_width,
                .max_preview_video_h = (uint32_t) m_height,
                .num_preview_video_frames = 3, /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
                .stills_capture_circular_buffer_height = 0,
                .fast_preview_resume = 0,
                .use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC
            };
        mmal_port_parameter_set( camera->control, &cam_config.hdr );
	}

	// Set the encode format on the video  port
	format = video_port->format;
	if( m_monochrome )
	{
		format->encoding_variant = MMAL_ENCODING_I420;
		format->encoding = MMAL_ENCODING_I420;
	}
	else
	{
		format->encoding = mmal_util_rgb_order_fixed(video_port) ?
            MMAL_ENCODING_BGR24 : MMAL_ENCODING_RGB24;
		format->encoding_variant = 0;
	}

	format->es->video.width = m_width;
	format->es->video.height = m_height;
	format->es->video.crop.x = 0;
	format->es->video.crop.y = 0;
	format->es->video.crop.width = m_width;
	format->es->video.crop.height = m_height;
	format->es->video.frame_rate.num = m_framerate;
	format->es->video.frame_rate.den = 1; /* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
	
	status = mmal_port_format_commit( video_port );
	if( status )
	{
        vcos_log_error("camera video format couldn't be set");
        goto error;
	}
	
    // Ensure there are enough buffers to avoid dropping frames
    // !!!!buffer_num must be set before enabling the port!!!!
    if( video_port->buffer_num < m_num_buffers )
    {
        video_port->buffer_num = m_num_buffers;
    }

	// PR : plug the callback to the video port 
	status = mmal_port_enable( video_port, video_buffer_callback );
	if( status )
	{
        vcos_log_error( "camera video callback2 error" );
        goto error;
	}

	//PR : create pool of message on video port
	MMAL_POOL_T *pool;
	pool = mmal_port_pool_create( video_port,
                                  video_port->buffer_num,
                                  video_port->buffer_size );
	if( !pool )
	{
        vcos_log_error( "Failed to create buffer header pool for video output port" );
        goto error;
	}
	
	/* Enable component */
	status = mmal_component_enable( camera );
	if( status )
	{
        vcos_log_error( "camera component couldn't be enabled" );
        goto error;
	}
	
	m_video_pool = pool;
	m_camera_component = camera;
	
	return true;

error:
    if( camera )
    {
        if( pool )
        {
            mmal_port_pool_destroy( video_port, pool );
        }
        mmal_component_destroy( camera );
    }

   return false;
}

void raspi_video_capture::release_objects()
{
    vcos_semaphore_delete( &m_capture_sem );
    vcos_semaphore_delete( &m_capture_done_sem );

    if( m_camera_component )
    {
        if( m_video_pool )
        {
            mmal_port_disable( m_camera_component->output[MMAL_CAMERA_VIDEO_PORT] );
            mmal_port_pool_destroy( m_camera_component->output[MMAL_CAMERA_VIDEO_PORT],
                                    m_video_pool );
            m_video_pool = NULL;
        }
        
        mmal_component_disable( m_camera_component );
        mmal_component_destroy( m_camera_component );

        m_camera_component = NULL;
    }
}

raspi_video_capture::~raspi_video_capture()
{
    release();
}

bool raspi_video_capture::open( int width, int height, int framerate, int monochrome,
                                int vflip, int hflip,
                                int exposure_mode,
                                float analog_gain, float digital_gain, int shutter_speed,
                                int awb_mode,
                                float awb_gains_r, float awb_gains_b )
{
    if( is_opened() )
    {
        return false;
    }
    
	bcm_host_init();

    m_width = width;
    m_height = height;
    m_framerate = framerate;
    m_monochrome = monochrome;

    m_finished = 0;

    if( monochrome )
    {
        m_img_buf.create( height, width, CV_8UC1 );
    }
    else
    {
        m_img_buf.create( height, width, CV_8UC3 );
    }

	vcos_semaphore_create( &m_capture_sem, "Capture-Sem", 0 );
	vcos_semaphore_create( &m_capture_done_sem, "Capture-Done-Sem", 0 );

	// create camera
	if( !create_camera_component() )
	{
	   vcos_log_error("%s: Failed to create camera component", __func__);
       release_objects();
	   return false;
	}

	RASPICAM_CAMERA_PARAMETERS camera_parameters;
    raspicamcontrol_set_defaults( &camera_parameters );
    camera_parameters.vflip = vflip;
    camera_parameters.hflip = hflip;
    camera_parameters.exposureMode = (MMAL_PARAM_EXPOSUREMODE_T) exposure_mode;
    camera_parameters.analog_gain = analog_gain;
    camera_parameters.digital_gain = digital_gain;
    camera_parameters.shutter_speed = shutter_speed;
    camera_parameters.awbMode = (MMAL_PARAM_AWBMODE_T) awb_mode;
    camera_parameters.awb_gains_r = awb_gains_r;
    camera_parameters.awb_gains_b = awb_gains_b;
	raspicamcontrol_set_all_parameters( m_camera_component, &camera_parameters );

	MMAL_PORT_T *camera_video_port;
	camera_video_port = m_camera_component->output[MMAL_CAMERA_VIDEO_PORT];

	// assign data to use for callback
	camera_video_port->userdata = (struct MMAL_PORT_USERDATA_T *) this;

	// start capture
	if( mmal_port_parameter_set_boolean( camera_video_port, MMAL_PARAMETER_CAPTURE, 1) !=
        MMAL_SUCCESS )
	{
        vcos_log_error("%s: Failed to start capture", __func__);
        release_objects();
        return false;
	}

	// Send all the buffers to the video port
		
	int num = mmal_queue_length( m_video_pool->queue );
	for( int q = 0; q < num; q++ )
	{
		MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get( m_video_pool->queue );
		
		if( !buffer )
        {
			vcos_log_error( "Unable to get a required buffer %d from pool queue", q );
        }
		
		if( mmal_port_send_buffer( camera_video_port, buffer ) != MMAL_SUCCESS )
        {
			vcos_log_error( "Unable to send a buffer to encoder output port (%d)", q );
        }
	}

	vcos_semaphore_wait( &m_capture_done_sem );

	return true;
}

int raspi_video_capture::read( cv::Mat &img )
{
    if( !is_opened() )
    {
        return 0;
    }
    
	vcos_semaphore_post( &m_capture_sem );
	vcos_semaphore_wait( &m_capture_done_sem );

    img = m_img_buf;

    return 1;
}

double raspi_video_capture::get( int property_id )
{
    if( !is_opened() )
    {
        return 0.0;
    }
    
    switch( property_id )
    {
    case CV_CAP_PROP_FRAME_WIDTH:  return ((double) m_width);
    case CV_CAP_PROP_FRAME_HEIGHT: return ((double) m_height);
    case CV_CAP_PROP_FPS:          return m_framerate;
    default:                       return 0.0;
    }
}

int raspi_video_capture::is_opened()
{
    return ( m_camera_component ? 1 : 0 );
}

void raspi_video_capture::release()
{
    if( is_opened() )
    {
        // Unblock the callback.
        m_finished = 1;
        vcos_semaphore_post( &m_capture_sem );
        vcos_semaphore_wait( &m_capture_done_sem );

        release_objects();
    }
}
