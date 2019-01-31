import ocl
import camvtk
import time
import vtk
import datetime
import math
import gzip
import csv


def readPts():
    spamReader = csv.reader(open('tsplib/graph.txt', 'rb'), delimiter=',', quotechar='|')
    pts=[]
    for row in spamReader:
        pts.append( ocl.Point( float(row[0]), float(row[1]) ) )
    return pts

def drawTour(myscreen, pts, tour, tourColor = camvtk.red):
    for n in range(0,len(tour)-1):
        idx0 = n
        idx1 = n+1
        ip1 = pts[ tour[idx0] ]
        ip2 = pts[ tour[idx1] ]
        myscreen.addActor( camvtk.Line(p1=(ip1.x,ip1.y,ip1.z),p2=(ip2.x,ip2.y,ip2.z), color=tourColor) )

def drawPoints(myscreen, pts):
    c=camvtk.PointCloud( pts ) 
    c.SetPoints()
    myscreen.addActor(c )
    

if __name__ == "__main__":  
    
    myscreen = camvtk.VTKScreen()
    
    
    tsp = ocl.TSPSolver()
    pts = readPts()
    drawPoints(myscreen, pts)
    print("got ",len(pts)," points")
    #exit()
    for p in pts:
        tsp.addPoint( p.x , p.y)
    start_time = time.time()
    tsp.run()
    run_time = time.time() - start_time
    l = tsp.getLength()
    print("tour length ",l)
    out = tsp.getOutput()
    for o in out:
        print(o,)
    
    drawTour(myscreen, pts, out)

    myscreen.camera.SetPosition(0.0, 0.0, 200)
    myscreen.camera.SetViewUp(0.0, 0.0, 0)
    myscreen.camera.SetFocalPoint(50, 50, 0)
    myscreen.render()

    myscreen.iren.Start()



import ocl
import camvtk
import time
import vtk
import datetime
import math
import gzip
import csv


def readPts():
    spamReader = csv.reader(open('tsplib/graph.txt', 'rb'), delimiter=',', quotechar='|')
    pts=[]
    for row in spamReader:
        pts.append( ocl.Point( float(row[0]), float(row[1]) ) )
    return pts

def drawTour(myscreen, pts, tour, tourColor = camvtk.red):
    for n in range(0,len(tour)-1):
        idx0 = n
        idx1 = n+1
        ip1 = pts[ tour[idx0] ]
        ip2 = pts[ tour[idx1] ]
        myscreen.addActor( camvtk.Line(p1=(ip1.x,ip1.y,ip1.z),p2=(ip2.x,ip2.y,ip2.z), color=tourColor) )

def drawPoints(myscreen, pts):
    c=camvtk.PointCloud( pts ) 
    c.SetPoints()
    myscreen.addActor(c )
    

if __name__ == "__main__":  
    
    myscreen = camvtk.VTKScreen()
    
    
    tsp = ocl.TSPSolver()
    pts = readPts()
    drawPoints(myscreen, pts)
    print("got ",len(pts)," points")
    #exit()
    for p in pts:
        tsp.addPoint( p.x , p.y)
    start_time = time.time()
    tsp.run()
    run_time = time.time() - start_time
    l = tsp.getLength()
    print("tour length ",l)
    out = tsp.getOutput()
    for o in out:
        print(o,)
    
    drawTour(myscreen, pts, out)

    myscreen.camera.SetPosition(0.0, 0.0, 200)
    myscreen.camera.SetViewUp(0.0, 0.0, 0)
    myscreen.camera.SetFocalPoint(50, 50, 0)
    myscreen.render()

    myscreen.iren.Start()



