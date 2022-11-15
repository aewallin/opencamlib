import ocl
import time
import datetime
import math
        
if __name__ == "__main__":  
    print(ocl.version())
    s = ocl.STLSurf()
    ocl.STLReader("../../stl/gnu_tux_mod.stl", s)
    print("STL surface read,", s.size(), "triangles")
    zh=1.75145
    cutter_diams = [1.4]
    length = 5
    loops = []
    for diam in cutter_diams:
        #cutter = ocl.CylCutter( diam , length )
        cutter = ocl.BallCutter( diam , length )
        #cutter = ocl.BullCutter( diam , diam/5, length )
        wl = ocl.Waterline()
        wl.setSTL(s)
        wl.setCutter(cutter)
        wl.setZ(zh)
        wl.setSampling(0.1)
        #wl.setThreads(5)
        t_before = time.time() 
        wl.run()
        t_after = time.time()
        calctime = t_after-t_before
        print(" Waterline done in ", calctime," s")
        cutter_loops = wl.getLoops()
        for l in cutter_loops:
            loops.append(l)

    print("All waterlines done. Got", len(loops)," loops in total.")
    # draw the loops
    for loop in loops:
        for p in loop:
          # todo, use rapid, retract before loops etc, this is just for testing purposes.
          print("G01 X{:.5f}".format(p.x), "Y{:.5f}".format(p.y), "Z{:.5f}".format(p.z))
    
    print("done.")
