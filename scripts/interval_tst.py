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
