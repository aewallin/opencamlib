import oellipse5_tst as oe
import math

Nsteps = 500
ystart = -0.2
ystop = 2.2

ystep = (ystop-ystart)/(Nsteps-1)
fiangle = 55

def radian(deg):
    return (float(deg)/360)*2*math.pi

for n in range(0,Nsteps):
    yc = ystart + n*ystep
    fname = "frames/oet"+ ('%05d' % n)+".png"
    oe.main(ycoord=yc, filename=fname, theta=70, fi=radian(fiangle))
    #fiangle = fiangle + 2
