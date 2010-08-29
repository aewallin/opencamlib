import ocl
import math

print ocl.revision()

# cylinder
c = ocl.CylCutter(2.345, 5)
print c
d = c.offsetCutter(0.1)
print d
print

# ball
c = ocl.BallCutter(2.345, 6)
print c
d = c.offsetCutter(0.1)
print d
print

# bull
c = ocl.BullCutter(2.345, 0.123, 6)
print c
d = c.offsetCutter(0.1)
print d
print

# cone
c = ocl.ConeCutter(2.345, math.pi/6)
print c
d = c.offsetCutter(0.1)
print d
