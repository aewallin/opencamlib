import oellipse5_tst as oe

Nsteps = 5
ystart = -0.2
ystop = 2.2

ystep = (ystop-ystart)/(Nsteps-1)
for n in xrange(0,Nsteps):
    yc = ystart + n*ystep
    fname = "frames/oe"+ ('%05d' % n)+".png"
    oe.main(ycoord=yc, filename=fname)
