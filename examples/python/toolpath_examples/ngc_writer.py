
# very simple G-code writer.
# Anders Wallin 2012

clearance_height= 20
feed_height = 10
feed = 200
plunge_feed = 100
metric = True

def line_to(x,y,z):
    print("G1 X% 8.6f Y% 8.6f Z% 8.6f F%.0f" % (x, y, z, feed))

def xy_line_to(x,y):
    print("G1 X% 8.4f Y% 8.4f " % (x, y))

# (endpoint, radius, center, cw?)
def xy_arc_to( x,y, r, cx,cy, cw ):
    if (cw):
        print("G2 X% 8.5f Y% 8.5f R% 8.5f" % (x, y, r))
    else:
        print("G3 X% 8.5f Y% 8.5f R% 8.5f" % (x, y, r))
    # FIXME: optional IJK format arcs
    
def xy_rapid_to(x,y):
    print("G0 X% 8.4f Y% 8.4f " % (x, y))

def pen_up():
    print("G0Z% 8.4f " % (clearance_height))

"""
def pen_down():
    print("G0Z% 8.4f" % (feed_height))
    plunge(0)
"""

def pen_down(z=0):
    print("G0Z% 8.4f" % (feed_height))
    plunge(z)

def plunge(z):
    print("G1 Z% 8.4f F% 8.0f" % (z, plunge_feed))

def preamble():
    if (metric):
        print("G21 F% 8.0f" % (feed)) # G20 F6 for inch
    else:
        print("G20 F% 8.0f" % (feed)) # G20 F6 for inch
        
    print("G64 P0.001") # linuxcnc blend mode
    pen_up()
    print("G0 X0 Y0") # this might not be a good idea!?

def postamble():
    pen_up()
    print("M2") # end of program

def comment(s=""):
    print("( ",s," )")
    
if __name__ == "__main__":
    print("Nothing to see here.")
