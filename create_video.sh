#!/bin/bash

mencoder -mf fps=30  -ffourcc DX50 -ovc xvid -xvidencopts pass=1 mf://figures/*.png \
-o /dev/null

mencoder -mf fps=30  -ffourcc DX50 -ovc xvid -xvidencopts pass=2:bitrate=2000 mf://figures/*.png \
-o video_xvid.avi

# convert to x264
#mencoder video_xvid.avi -o crossing_x264.avi -ovc x264 -x264encopts bitrate=2000
