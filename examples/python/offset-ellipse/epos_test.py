import ocl
import camvtk
import time
import vtk
import datetime
import math
        
if __name__ == "__main__":  
    p = ocl.Epos()
    print("initial Epos() is ",p, "with dia=",p.d)
    
    for n in range(0,30):
        print(p.d," : ",p)
        p.d = p.d - 0.25
        p.setD()
        
    raw_input("Press Enter to terminate") 
