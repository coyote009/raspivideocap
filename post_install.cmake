execute_process( COMMAND bash "-c" "echo /opt/raspivideocap/lib > /etc/ld.so.conf.d/raspivideocap.conf" )
execute_process( COMMAND ldconfig )
