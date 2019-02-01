import ocl
import math

print ocl.version()

# cylinder
c = ocl.CylCutter(2.345, 5)
d = c.offsetCutter(0.1)
print c
print "offset: ",d
print

# ball
c = ocl.BallCutter(2.345, 6)
d = c.offsetCutter(0.1)
print c
print "offset: ",d
print

# bull
c = ocl.BullCutter(2.345, 0.123, 6)
d = c.offsetCutter(0.1)
print c
print "offset: ",d
print

# cone
c = ocl.ConeCutter(2.345, math.pi/6)
d = c.offsetCutter(0.1)
print c
print "offset: ",d

# TODO: add compound-cutters here below.
