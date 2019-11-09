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

#include <opencv2/opencv.hpp>
#include "raspivideocap.h"

int main( int argc, char **argv )
{
    //cv::Size size_in( 640, 480 );
    cv::Size size_in( 1440, 1440 );
    int fps = 30;
    int gray = 0;
    int hflip = 0;
    int vflip = 0;
    int num_buffers = 10;

    if( argc >= 3 )
    {
        size_in.width = atoi( argv[1] );
        size_in.height = atoi( argv[2] );

        if( argc >= 4 )
        {
            fps = atoi( argv[3] );

            if( argc >= 5 )
            {
                gray = atoi( argv[4] );

                if( argc >= 6 )
                {
                    hflip = atoi( argv[5] );
                    
                    if( argc >= 7 )
                    {
                        vflip = atoi( argv[6] );
                    
                        if( argc >= 8 )
                        {
                            num_buffers = atoi( argv[7] );
                        }
                    }
                }
            }
        }
    }

    printf( "Size = %d x %d fps = %d gray = %d "
            "hflip = %d vflip = %d num_buffers = %d\n",
            size_in.width, size_in.height, fps,
            gray, hflip, vflip, num_buffers );
    
    RaspiVideoCapture cap( num_buffers );
    if( !cap.open( size_in.width, size_in.height, fps, gray, hflip, vflip
                   //, RaspiVideoCapture::EXPOSUREMODE_OFF, 100, 0.5, 30000
            ) )
    {
        fprintf( stderr, "Failed to open camera\n" );
        return 1;
    }

    cv::TickMeter tm;
    int frame = 0;
    tm.start();
    while( 1 )
    {
        cv::Mat img_in;
        cap.read( img_in );

        cv::imshow( "img_in", img_in );

        frame++;
        
        if( cv::waitKey( 1 ) == 27 )
        {
            break;
        }
    }

    tm.stop();
    double time = tm.getTimeSec();
    printf( "Average %f [sec/frame] %f [fps]\n",
            time / frame, frame / time );
    
    return 0;
}
