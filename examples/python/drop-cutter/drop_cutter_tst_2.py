import ocl
import camvtk
import time

if __name__ == "__main__":  
    cutter = ocl.CylCutter(.1, 5)
    print(cutter)
    a=ocl.Point(1,0,0)
    b=ocl.Point(0,1,0)
    c=ocl.Point(0,0,1)
    t = ocl.Triangle(a,b,c)
    print("triangle created  t=", t)
    cl = ocl.CLPoint(0.2,0.2,0)

    cutter.dropCutter(cl,t)
    print("CL= ", cl)
