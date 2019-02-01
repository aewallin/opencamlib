import ocl as cam
import camvtk
import time
import vtk
import math
import datetime

# 2018.08: Epos not wrapped

def drawellipse(myscreen, ellcenter, a_axis, b_axis):
    resolution=50
    for n in range(0,resolution):
        angle1= (float(n)/float(resolution))*2*math.pi
        angle2= (float(n+1)/float(resolution))*2*math.pi
        x=ellcenter.x + a_axis*math.cos(angle1)
        y=ellcenter.y + b_axis*math.sin(angle1)
        z=ellcenter.z
        x2=ellcenter.x + a_axis*math.cos(angle2)
        y2=ellcenter.y + b_axis*math.sin(angle2)
        myscreen.addActor( camvtk.Line(p1=(x,y,z),p2=(x2,y2,z), color=camvtk.grey)  )

def main(ycoord=0.970, filename="test"):
    myscreen = camvtk.VTKScreen()
    
    myscreen.camera.SetPosition(2, 5, 5)
    myscreen.camera.SetFocalPoint(1.38,1, 0)
    
    #ycoord = 1.1
    
    a=cam.Point(3,ycoord,-2)
    b=cam.Point(-1,ycoord,3)    
    
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)));
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)));
    #c=cam.Point(0,0,0.3)
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    #t = cam.Triangle(a,b,c)
    
    cutter = cam.BullCutter(1,0.2,20)

    print(cutter   )
    xar = camvtk.Arrow(color=camvtk.red, rotXYZ=(0,0,0))
    myscreen.addActor(xar)
    yar = camvtk.Arrow(color=camvtk.green, rotXYZ=(0,0,90))
    myscreen.addActor(yar)
    zar = camvtk.Arrow(color=camvtk.blue, rotXYZ=(0,-90,0))
    myscreen.addActor(zar)
    
    cl = cam.Point(2.1748, 1, 0)
    radius1=1
    radius2=0.25
    
    tor = camvtk.Toroid(r1=radius1, r2=radius2, center=(cl.x, cl.y, cl.z),rotXYZ=(0,0,0))
    #tor.SetWireframe()
    #myscreen.addActor(tor)
    
    cyl = camvtk.Cylinder(center=(cl.x,cl.y,cl.z) , radius=radius1, height=2, color=(0,1,1),
                    rotXYZ=(90,0,0), resolution=50 )
    #myscreen.addActor(cyl)
    
    cl_line = camvtk.Line( p1=(cl.x,cl.y,-100),p2=(cl.x,cl.y,+100), color=camvtk.red )
    myscreen.addActor(cl_line)

    tube = camvtk.Tube(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z),color=(1,1,0))
    tube.SetOpacity(0.2)
    myscreen.addActor(tube)
    
    # cylindrical-cutter circle at z=0 plane
    #cir= camvtk.Circle(radius=radius1, center=(cl.x,cl.y,cl.z), color=camvtk.yellow)
    #myscreen.addActor(cir)
    
    #clp = camvtk.Point(center=(cl.x,cl.y,cl.z))
    #myscreen.addActor(clp)
    
    # short axis of ellipse = radius2
    # long axis of ellipse = radius2/sin(theta)
    # where theta is the slope of the line
    dx = b.x - a.x
    dz = b.z - a.z
    #print "dx=", dx
    #print "dz=", dz
    theta = math.atan(dz/dx)  ## dx==0 is special case!! (i.e. vertical lines)
    print("theta=",theta)
    a_axis = abs( radius2/math.sin(theta) )
    print("a=", a_axis)
    # ellipse
    #a=2
    b_axis=radius2
    print("b= ", b_axis)
    
    
    # slice the tube with a plane at z=0 and find the ellipse center
    # line is from Point a to b:
    # a + t*(b-a)
    # find t so that z-component is zero:
    # a.z + t( b.z -a.z) = 0
    # t= a.z / (b.z - a.z)
    # so point
    tparam = -a.z / (b.z - a.z)  # NOTE horizontal lines are a special case!!
    ellcenter = a + tparam*(b-a)
    print("ellcenter (z=0?) =", ellcenter)
    # center of the 
    # ecen_tmp=cam.Point(ellcenter,a.y,0)
    
    #drawellipse(myscreen, ellcenter, a_axis, b_axis)

    
    
    oe = cam.Ellipse(ellcenter, a_axis, b_axis, radius1)
    
    #oe2 = cam.Ellipse(ellcenter, a_axis, b_axis, 0.05) # to locate text on the outside of the ellipse

        
    nmax=20
    #delta=0.05
    #td = 1

    
    
    t = camvtk.Text()
    t.SetPos( (myscreen.width-450, myscreen.height-30) )

    
    myscreen.addActor( t)
    t2 = camvtk.Text()
    ytext = "Y: %3.3f" % (ycoord)
    t2.SetText(ytext)
    t2.SetPos( (50, myscreen.height-150) )   
    myscreen.addActor( t2)
        

    
    #w2if = vtk.vtkWindowToImageFilter()
    #w2if.SetInput(myscreen.renWin)
    #lwr = vtk.vtkPNGWriter()
    #lwr.SetInput( w2if.GetOutput() )
  
    epos = cam.Epos()
    epos.setS(0,1)
    #epos1.setS(0,1)

    t.SetText("OpenCAMLib 10.03-beta, " + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))


    #p5 = oe.ePoint(epos5)
    #pt = oe2.oePoint(epos5)
    #print "before= ", epos5.s, " , ", epos5.t
    nsteps = cam.Ellipse.solver(oe,  cl)
    epos = oe.epos1
    cce = oe.ePoint(epos)
    cle = oe.oePoint(epos)
    #epos2 = cam.Epos()
    #epos.s = epos.s
    #epos.t = epos.t
    #print nsteps
    print("solution1 at: ", epos.s , " , ", epos.t )
    #print "solution2 at: ", epos2.s , " , ", epos2.t 
    print(" cl =", cl)
    print(" cle=", cle)
    
    xoffset = cl.x - cle.x
    print("xoffset= ", xoffset)
    # we slide xoffset along the x-axis from ellcenter 
    # to find the correct z-plane
    # line is: a + t*(b-a)
    # find t so that x-component is ellcenter.x + xoffset
    # a.x + t(b.x-a.x) = ellcenter.x + xoffset
    # t= (ellcenter.x + xoffset - a.x) / (b.x - a.x)
    tparam2 = (ellcenter.x + xoffset - a.x) / (b.x - a.x)
    slide = tparam2*(b-a)
    print("sliding z-delta: ", slide.z)
    elc2 = a + tparam2*(b-a)
    print("ellcenter2=", elc2)
    #convlist.append(nsteps)
    fe = cam.Ellipse(elc2, a_axis, b_axis, radius1)
    fecen = camvtk.Sphere(center=(elc2.x,elc2.y,elc2.z), radius=0.01, color=camvtk.pink)
    myscreen.addActor(fecen)
    fccp = fe.ePoint(epos)
    fclp = fe.oePoint(epos)
    print("solver cl=", fclp, " == ", cl, " ??")
    
    fcir= camvtk.Circle(radius=radius1, center=(cl.x,cl.y,elc2.z), color=camvtk.yellow)
    myscreen.addActor(fcir)
    
    fccpoint = camvtk.Sphere(center=(fccp.x,fccp.y,fccp.z), radius=0.01, color=camvtk.green)
    myscreen.addActor(fccpoint)
    
    fclpoint = camvtk.Sphere(center=(fclp.x,fclp.y,fclp.z), radius=0.01, color=camvtk.blue)
    myscreen.addActor(fclpoint)
    
    # line from ellipse center to fcc
    myscreen.addActor(camvtk.Line( p1=(elc2.x,elc2.y,elc2.z),p2=(fccp.x,fccp.y,fccp.z), color=camvtk.cyan ))
    # the offset normal
    myscreen.addActor(camvtk.Line( p1=(fclp.x,fclp.y,fclp.z),p2=(fccp.x,fccp.y,fccp.z), color=camvtk.yellow ))
    
    drawellipse(myscreen, elc2, a_axis, b_axis)
    #convtext = "%i" % (nsteps)
    #print (pt.x, pt.y, pt.z)
    #center=(pt.x, pt.y, pt.z)    
    #tst = camvtk.Text3D( color=(1,1,1), center=(pt.x, pt.y, 0)  , 
    #text=convtext, scale=0.02)
    #tst.SetCamera(myscreen.camera)
    #myscreen.addActor(tst)
            
    colmax=11
    colmin=4
    nsteps = nsteps - colmin
    colmax = colmax - colmin
    convcolor=( float(nsteps*nsteps)/(colmax), float((colmax-nsteps))/colmax, 0 )
    #esphere = camvtk.Sphere(center=(p5.x,p5.y,0), radius=0.01, color=convcolor)
    end_sphere = camvtk.Sphere(center=(cce.x,cce.y,0), radius=0.01, color=camvtk.green)
    cl_sphere = camvtk.Sphere(center=(cle.x,cle.y,0), radius=0.01, color=camvtk.pink)
    cl_sphere.SetOpacity(0.4)
    
    clcir= camvtk.Circle(radius=radius1, center=(cle.x,cle.y,cle.z), color=camvtk.pink)
    myscreen.addActor(clcir)
    
    #myscreen.addActor(esphere)
    myscreen.addActor(end_sphere)
    myscreen.addActor(cl_sphere)
    #myscreen.render()
  
    print("done.")
    myscreen.render()
    lwr.SetFileName(filename)
    #lwr.Write()
    #raw_input("Press Enter to terminate")         
    #time.sleep(0.5)
    myscreen.iren.Start()


if __name__ == "__main__":
    main()
    #myscreen.iren.Start()
    #raw_input("Press Enter to terminate")    

    
