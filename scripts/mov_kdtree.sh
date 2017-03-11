#!/bin/sh

#first convert to JPEG
#mogrify -format jpg -quality 97 *.png

# a script for encoding jpeg-frames into a video suitable for uploading to youtube or vimeo

# oldest, not very good...
#mencoder mf://*.jpg -mf fps=25:type=jpg -ovc lavc -lavcopts vcodec=mpeg4 -ac copy -o output.avi -ffourcc DX50

# -aspect 16:9
#  -mf fps=25:type=png   -
# better.
mencoder mf://*.png -mf type=png   -of lavf -ovc lavc -lavcopts aglobal=1:vglobal=1:coder=0:vcodec=mpeg4:vbitrate=24000000 -vf scale=1280:720 -ofps 30000/1001 -o OUTPUTp_24long.mp4

# not so good.
#mencoder mf://*.jpg -mf w=800:h=600:fps=25:type=jpg -ovc lavc -lavcopts vcodec=mpeg4:mbd=2:trell -oac copy -o output4.avi


