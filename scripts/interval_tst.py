import ocl
import camvtk
import time
import vtk
import datetime
import math

        
if __name__ == "__main__":  
    print ocl.revision()
    i = ocl.Interval()
    print i
    i = ocl.Interval(0.123, 0.456)
    print i
    i.updateLower(-0.222)
    print i
    i.updateLower(0)
    print i
    i.updateUpper(0)
    print i
    i.updateUpper(2)
    print i
