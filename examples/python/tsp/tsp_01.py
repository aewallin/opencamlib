import ocl
import camvtk
import time
import vtk
import datetime
import math
import gzip

# Travelling Salesman Problem
# a.k.a. drill-path optimization
#
# we have multiple points in the 2D plane to visit.
# the task is to optimize the distance travelled by the tool while
# visiting all points
#
# uses data from TSPLIB (not included in opencamlib)
#
# ocl.TSPSolver() is a wrapper for boost::metric_tsp_approx()
# https://www.boost.org/doc/libs/1_67_0/libs/graph/doc/metric_tsp_approx.html


def drawPoints(myscreen, pts):
    c=camvtk.PointCloud( pts ) 
    c.SetPoints()
    myscreen.addActor(c )

def drawTour(myscreen, pts, tour, tourColor = camvtk.red):
    for n in range(0,len(tour)-1):
        idx0 = n
        idx1 = n+1
        ip1 = pts[ tour[idx0] ]
        ip2 = pts[ tour[idx1] ]
        myscreen.addActor( camvtk.Line(p1=(ip1.x,ip1.y,ip1.z),p2=(ip2.x,ip2.y,ip2.z), color=tourColor) )

def TSPLIBReader( filename ):
    """ read a .tsp format TSPLIB file, and return list of points """
    #infile = open(filename,"r")
    infile = gzip.open(filename, 'rb')
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
            if len(words)!=0 and words[0]!="EOF" :
                points.append( ocl.Point( float( words[1] ),  float( words[2] ) ) )
            
                
    print("read ",len(points)," points from ", filename)
    return points

def TSPTourReader( filename ):
    infile = gzip.open(filename, 'rb')
    points = []
    found = 0
    n = 1
    for line in infile.readlines():
        if not found:
            for word in line.split():
                if word == "TOUR_SECTION":
                    found = 1
        else:
            words = line.split()
            if words[0]!="-1" and words[0]!="EOF" :
                points.append(  int( words[0] )-1 ) # NOTE our indexes start from 0, thus "-1"
    print(" read length ",len(points)," tour from ",filename)
    points.append( points[0] ) # end at the same point we started
    return points
    
def TSPTourLength(pts,tour):
    l = 0
    for n in range(0,len(tour)-1):
        start = n
        stop = n + 1
        #if stop == len(tour):
        #    stop = 0
        #print start," - ",stop," =: ",tour[start],"-",tour[stop]
        p1 = pts[ tour[start] ]
        p2 = pts[ tour[stop]  ]
        d = round( math.sqrt( (p1.x-p2.x)*(p1.x-p2.x) + (p1.y-p2.y)*(p1.y-p2.y) ) )
        l = l+d
        
    return l
if __name__ == "__main__":  
    
    tspname = ["a280" ,  2579 ]
    tspname = ["eil51" , 426 ]
    tspname = ["berlin52" , 7542 ]
    tspname = ["bier127" , 118282 ]
    tspname = ["brd14051" , 469385 ] # 90s
    tspname = ["ch130" , 6110 ] # 
    tspname = ["ch150" , 6528 ] #
    tspname = ["d198" , 15780 ] #
    tspname = ["d493" , 35002 ]
    tspname = ["d657" , 48912 ]
    tspname = ["d1291" , 50801] # 0.7s
    tspname = ["d1655" , 62128] # 1.23s
    tspname = ["d2103" , 80450] # 1.98s
    tspname = ["d15112" , 1573084] # 110s
    tspname = ["d18512" , 645238] # 160s, 3.9-4.3 Gb
    tspname = ["eil51" ,  426 ]
    tspname = ["eil76" ,  538 ]
    tspname = ["eil101" ,  629 ]
    tspname = ["fl417" ,  11861 ]
    tspname = ["fl1400" ,  20127 ] # 0.92s
    tspname = ["fl1577" ,  22249 ] # 1.09s
    tspname = ["fl3795" ,  28772 ] # 6.5s
    tspname = ["fnl4461" ,  182566 ] # 9.2s
    tspname = ["gil262" ,  2378 ] 
    tspname = ["kroA100" ,  21282 ] 
    tspname = ["kroB100" ,  22141] 
    tspname = ["kroC100" ,  20749] 
    tspname = ["kroD100" ,  21294] 
    tspname = ["kroE100" ,  22068] 
    tspname = ["kroA150" ,  26524] 
    tspname = ["kroB150" ,  26130] 
    tspname = ["kroA200" ,  29368] 
    tspname = ["kroB200" ,  29437]
    tspname = ["lin105" ,  14379]
    tspname = ["lin318" ,  42029]
    tspname = ["linhp318" ,  41345] 
    tspname = ["nrw1379" ,  56638] # 0.87
    tspname = ["p654" ,  34643] 
    tspname = ["pcb442" , 50778] 
    tspname = ["pcb1173" , 56892]
    tspname = ["pcb3038" , 137694] # 4s
    tspname = ["pr76" , 108159]
    tspname = ["pr107" , 44303] 
    tspname = ["pr124" , 59030] 
    tspname = ["pr136" , 96772] 
    tspname = ["pr144" , 58537] 
    tspname = ["pr152" , 73682] 
    tspname = ["pr226" , 80369] 
    tspname = ["pr264" , 49135] 
    tspname = ["pr299" , 48191] 
    tspname = ["pr439" , 107217] 
    tspname = ["pr1002" , 259045]
    tspname = ["pr2392" , 378032] 
    tspname = ["pr2392" , 378032] 
    tspname = ["rat99" , 1211] 
    tspname = ["rat195" , 2323] 
    tspname = ["rat575" , 6773] 
    tspname = ["rat783" , 8806] 
    tspname = ["rd100" , 7910]
    tspname = ["rd400" , 15281] 
    tspname = ["rl1304" , 252948] 
    tspname = ["rl1323" , 270199]
    tspname = ["rl1889" , 316536] # 1.64s 
    tspname = ["rl5915" , 565530] # 15.95s
    tspname = ["rl5934" , 556045] # 16.05s
    tspname = ["rl11849" , 923288] # 68s
    tspname = ["st70" , 675] # 
    tspname = ["ts225" , 126643] #
    tspname = ["tsp225" , 3916] #
    tspname = ["u159" , 42080] 
    tspname = ["u574" , 36905] 
    tspname = ["u724" , 41910] 
    tspname = ["u1060" , 224094] 
    tspname = ["u1432" , 152970] 
    tspname = ["u1817" , 57201] 
    
    tspname = ["u2319" , 234256] # grid
    tspname = ["vm1084" , 239297] 
     
    
    tspname = ["vm1748" , 336556]
    tspname = ["u2152" , 64253]  # pcb
    tspname = ["usa13509" , 19982859] # 86s
    opt_len = tspname[1]
    
    tspfile = "tsplib/"+tspname[0]+".tsp.gz"
    #tourfile = "tsplib/"+tspname+".opt.tour.gz"
    pts = TSPLIBReader( tspfile )
    #tour = TSPTourReader( tourfile )
    #print tour
    #opt_len = TSPTourLength(pts,tour)
    print(" optimal tour length = ",opt_len)

    myscreen = camvtk.VTKScreen()
    drawPoints(myscreen, pts)
    #drawTour(myscreen, pts, tour, camvtk.green)
    tsp = ocl.TSPSolver()
    n=0
    for p in pts:
        tsp.addPoint( p.x , p.y)
        n=n+1
    print(" added ",n," points")
    tsp.run()
    r = tsp.getOutput()
    l = tsp.getLength()
    print
    
    drawTour(myscreen, pts, r)
    #tsp.reset()
    print(" tour length= ",l, " ratio = ",float(l)/float(opt_len))

    myscreen.camera.SetPosition(0.0, 0.0, 200)
    myscreen.camera.SetViewUp(0.0, 0.0, 0)
    myscreen.camera.SetFocalPoint(50, 50, 0)
    myscreen.render()
    
    w2if = vtk.vtkWindowToImageFilter()
    w2if.SetInput(myscreen.renWin)
    lwr = vtk.vtkPNGWriter()
    lwr.SetInput( w2if.GetOutput() )

    myscreen.iren.Start()
