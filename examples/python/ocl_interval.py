import ocl
import camvtk
import time
import vtk
import datetime
import math

        
if __name__ == "__main__":  
    print(ocl.version())
    i = ocl.Interval()
    print(i)
    i = ocl.Interval(0.123, 0.456)
    print(i)
    cc = ocl.CCPoint(1,2,3)
    cc.type = ocl.CCType.VERTEX
    i.updateLower(-0.222,cc)
    print(i)
    i.updateLower(0,cc)
    print(i)
    i.updateUpper(0,cc)
    print(i)
    i.updateUpper(2,cc)
    print(i)
