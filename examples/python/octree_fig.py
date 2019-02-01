import ocl
import pyocl
import camvtk
import time
import datetime
import vtk
import math
"""
This scripts draws a picture which enumerates the vertices, edges, and faces
of an octree node, as used in the octree cutting simulation and
marching-cubes.
"""
def main():  
    print(ocl.version())
    myscreen = camvtk.VTKScreen()   
    myscreen.camera.SetPosition(-8, -4, 25)
    myscreen.camera.SetFocalPoint(0,0, 0)   
    arpos=-1.5
    camvtk.drawArrows(myscreen,center=(arpos,arpos,arpos))
    camvtk.drawOCLtext(myscreen)
    octtext = camvtk.Text()
    octtext.SetPos( (70, myscreen.height-600) )
    myscreen.addActor( octtext)
    octtext.SetText("Octree")
    vertex = [       ocl.Point( 1, 1,-1),   #// 0
                     ocl.Point(-1, 1,-1),   #// 1
                     ocl.Point(-1,-1,-1),   #// 2
                     ocl.Point( 1,-1,-1),   #// 3
                     ocl.Point( 1, 1, 1),   #// 4
                     ocl.Point(-1, 1, 1),   #// 5
                     ocl.Point(-1,-1, 1),   #// 6
                     ocl.Point( 1,-1, 1)    #// 7
                    ]
    
    n=0
    for v in vertex:
        myscreen.addActor( camvtk.Sphere(center=(v.x,v.y,v.z), radius=0.1,color=camvtk.red))
        v=v
        t = camvtk.Text3D(color=camvtk.red, center=(v.x+0.1,v.y+0.1,v.z), text=str(n), scale=0.2, camera=myscreen.camera)
        myscreen.addActor(t)
        n=n+1
    
    edgeTable = [ [0,1] ,
                  [1,2] ,
                  [2,3] ,
                  [3,0] ,
                  [4,5] ,
                  [5,6] ,
                  [6,7] ,
                  [7,4] ,
                  [0,4] ,
                  [1,5] ,
                  [2,6] ,
                  [3,7] ,  
                ]
    
    # draw the edges as tubes
    ne = 0
    for e in edgeTable:

        ep1 = vertex[ e[0] ] 
        ep2 = vertex[ e[1] ]
        tu = camvtk.Tube( p1=(ep1.x,ep1.y,ep1.z), p2=(ep2.x,ep2.y,ep2.z), radius=0.051, color=camvtk.green )
        myscreen.addActor(tu)
        mid = 0.5*(ep1 + ep2)
        t = camvtk.Text3D(color=camvtk.green, center=(mid.x+0.1,mid.y+0.1,mid.z), text=str(ne), scale=0.2, camera=myscreen.camera)
        myscreen.addActor(t)
        ne=ne+1
        
    # number the faces
    face = [ [2,3,6,7] , 
             [0,3,4,7] , 
             [0,1,4,5] ,
             [1,2,5,6] ,
             [0,1,2,3] ,
             [4,5,6,7] ,
            ]
    nf=0
    for f in face:
        mid = ocl.Point()
        for v in f:
            mid = mid+vertex[v]
        mid=0.25*mid
        t = camvtk.Text3D(color=camvtk.blue, center=(mid.x,mid.y,mid.z), text=str(nf), scale=0.2, camera=myscreen.camera)
        myscreen.addActor(t)
        nf=nf+1
    myscreen.render() 
    print("All done.")
    myscreen.iren.Start() 

if __name__ == "__main__":

    main()
