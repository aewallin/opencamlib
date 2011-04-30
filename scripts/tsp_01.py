import ocl
import camvtk
import time
import vtk
import datetime
import math


def drawPoints(myscreen, pts):
    c=camvtk.PointCloud( pts ) 
    c.SetPoints()
    myscreen.addActor(c )

def drawTour(myscreen, pts, tour):
    for n in range(0,len(tour)-1):
        idx0 = n
        idx1 = n+1
        ip1 = pts[ tour[idx0] ]
        ip2 = pts[ tour[idx1] ]
        myscreen.addActor( camvtk.Line(p1=(ip1.x,ip1.y,ip1.z),p2=(ip2.x,ip2.y,ip2.z), color=camvtk.red) )

def TSPLIBReader( filename ):
    """ read a .tsp format TSPLIB file, and return list of points """
    infile = open(filename,"r")
    points = []
    found = 0
    n = 1
    for line in infile.readlines():
        if not found:
            for word in line.split():
                if word == "NODE_COORD_SECTION":
                    found = 1
        else:
            words = line.split()
            if words[0]!="EOF":
                points.append( ocl.Point( int( words[1] ),  int( words[2] ) ) )
            
                
    print "read ",len(points)," points from ", filename
    return points
    

if __name__ == "__main__":  
    #pts = TSPLIBReader( "tsplib/eil51.tsp" )
    pts = TSPLIBReader( "tsplib/a280.tsp" )
    print " got ", len(pts), " points"
    myscreen = camvtk.VTKScreen()
    drawPoints(myscreen, pts)
    
    tsp = ocl.TSPSolver()
    n=0
    for p in pts:
        tsp.addPoint( p.x , p.y)
        n=n+1
    print " added ",n," points"
    tsp.run()
    r = tsp.getOutput()
    #print r
    drawTour(myscreen, pts, r)
    #tsp.reset()
    

    myscreen.camera.SetPosition(0.0, 0.0, 200)
    myscreen.camera.SetViewUp(0.0, 0.0, 0)
    myscreen.camera.SetFocalPoint(50, 50, 0)
    myscreen.render()
    
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )

    myscreen.iren.Start()
