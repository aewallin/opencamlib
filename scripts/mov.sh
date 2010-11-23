#!/bin/sh

# a scirpt for encoding jpeg-frames into a video suitable for uploading to youtube or vimeo

# oldest, not very good...
#mencoder mf://*.jpg -mf fps=25:type=jpg -ovc lavc -lavcopts vcodec=mpeg4 -ac copy -o output.avi -ffourcc DX50

# better.
mencoder mf://*.png -mf fps=25:type=png  -aspect 16:9 -of lavf -ovc lavc -lavcopts aglobal=1:vglobal=1:coder=0:vcodec=mpeg4:vbitrate=4500 -vf scale=1280:720 -ofps 30000/1001 -o OUTPUT3.mp4

# not so good.
#mencoder mf://*.jpg -mf w=800:h=600:fps=25:type=jpg -ovc lavc -lavcopts vcodec=mpeg4:mbd=2:trell -oac copy -o output4.avi


