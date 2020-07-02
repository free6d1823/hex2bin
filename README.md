* To build the project
 $ make

* To convert a hex file to gray image
 $ ./hex2bin -r  input0_buffer.hex -o gray0.y

* To convert 3 hex files to RGB image (packet mode, rgb48be)
 $ ./hex2bin -r  input0_buffer.hex -g input1_buffer.hex -b input2_buffer.hex -o input.rgb

* To display a gray image with ffplay
 $ ffplay -video_size 180x180 -pixel_format gray16be gray0.y

* To display a rgb-16bit image with ffplay
ffplay -video_size 180x180 -pixel_format rgb48be input.rgb 
