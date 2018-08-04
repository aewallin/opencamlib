import ocl as cam
import camvtk
import time
import vtk
import math
import datetime

# 2018.08: Epos not wrapped

def drawellipse(myscreen, ellcenter, a_axis, b_axis):
    resolution=50
    for n in xrange(0,resolution):
        angle1= (float(n)/float(resolution))*2*math.pi
        angle2= (float(n+1)/float(resolution))*2*math.pi
        x=ellcenter.x + a_axis*math.cos(angle1)
        y=ellcenter.y + b_axis*math.sin(angle1)
        z=ellcenter.z
        x2=ellcenter.x + a_axis*math.cos(angle2)
        y2=ellcenter.y + b_axis*math.sin(angle2)
        myscreen.addActor( camvtk.Line(p1=(x,y,z),p2=(x2,y2,z), color=camvtk.grey)  )

def calcEcenter(oe,a,b,cl,sln):
    pos = cam.Epos()
    if sln == 1:
        pos = oe.epos1
    if sln == 2:
        pos = oe.epos2
    
    cce = oe.ePoint(pos)
    cle = oe.oePoint(pos)
    print "solution at: ", pos
    print "  cce=", cce
    print "  cle=", cle
    
    xoffset = cl.x - cle.x
    print " xoffset= ", xoffset
    # we slide xoffset along the x-axis from ellcenter 
    # to find the correct z-plane
    # line is: a + t*(b-a)
    # find t so that x-component is ellcenter.x + xoffset
    # a.x + t(b.x-a.x) = ellcenter.x + xoffset
    # t= (ellcenter.x + xoffset - a.x) / (b.x - a.x)
    tparam = (oe.center.x + xoffset - a.x) / (b.x - a.x)
    return a + tparam*(b-a)
    
    

def main(ycoord=1.2, filename="test", theta=60, fi=45):
    myscreen = camvtk.VTKScreen()
    focal = cam.Point(2.17, 1, 0)
    r = 14
    theta = (float(theta)/360)*2*math.pi
    
    
    campos = cam.Point( r*math.sin(theta)*math.cos(fi), r*math.sin(theta)*math.sin(fi), r*math.cos(theta) ) 
    myscreen.camera.SetPosition(campos.x, campos.y, campos.z)
    myscreen.camera.SetFocalPoint(focal.x,focal.y, focal.z)
    
    #ycoord = 1.1
    
    # the two points that define the edge
    a=cam.Point( 3 , ycoord , 2.999999)
    b=cam.Point( -1, ycoord , 3)    
    
    myscreen.addActor(camvtk.Point(center=(a.x,a.y,a.z), color=(1,0,1)));
    myscreen.addActor(camvtk.Point(center=(b.x,b.y,b.z), color=(1,0,1)));
    #c=cam.Point(0,0,0.3)
    myscreen.addActor( camvtk.Line(p1=(a.x,a.y,a.z),p2=(b.x,b.y,b.z)) )
    #t = cam.Triangle(a,b,c)
    
    cutter_length = 2
    cutter = cam.BullCutter(1,0.2, cutter_length)

    print cutter   
    xar = camvtk.Arrow(color=camvtk.red, rotXYZ=(0,0,0))
    myscreen.addActor(xar)
    yar = camvtk.Arrow(color=camvtk.green, rotXYZ=(0,0,90))
    myscreen.addActor(yar)
    zar = camvtk.Arrow(color=camvtk.blue, rotXYZ=(0,-90,0))
    myscreen.addActor(zar)
    
    cl = cam.Point(2.1748, 1, 0)
    radius1=1
    radius2=0.25
    
    
    #tor.SetWireframe()
    #myscreen.addActor(tor)
    
    cyl = camvtk.Cylinder(center=(cl.x,cl.y,cl.z) , radius=radius1, height=2, color=(0,1,1),
                    rotXYZ=(90,0,0), resolution=50 )
    #myscreen.addActor(cyl)
    
    cl_line = camvtk.Line( p1=(cl.x,cl.y,-100),p2=(cl.x,cl.y,+100), color=camvtk.red )
    myscreen.addActor(cl_line)
    
    cl_tube = camvtk.Tube(p1=(cl.x,cl.y,-100),p2=(cl.x,cl.y,+100),radius=radius1, color=camvtk.green)
    cl_tube.SetOpacity(0.1)
    myscreen.addActor(cl_tube)
    
    a_inf = a + (-100*(b-a))
    b_inf = a + (+100*(b-a))

    tube = camvtk.Tube(p1=(a_inf.x,a_inf.y,a_inf.z),p2=(b_inf.x,b_inf.y,b_inf.z),radius=0.05*radius2, color=camvtk.red)
    tube.SetOpacity(0.3)
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
    print "theta=",theta
    a_axis = abs( radius2/math.sin(theta) )
    print "a=", a_axis
    # ellipse
    #a=2
    b_axis=radius2
    print "b= ", b_axis
    
    
    # slice the tube with a plane at z=0 and find the ellipse center
    # line is from Point a to b:
    # a + t*(b-a)
    # find t so that z-component is zero:
    # a.z + t( b.z -a.z) = 0
    # t= a.z / (b.z - a.z)
    # so point
    tparam = -a.z / (b.z - a.z)  # NOTE horizontal lines are a special case!!
    ellcenter = a + tparam*(b-a)
    print "ellcenter (z=0?) =", ellcenter
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
    t.SetText("OpenCAMLib " + datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
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

    

    #p5 = oe.ePoint(epos5)
    #pt = oe2.oePoint(epos5)
    #print "before= ", epos5.s, " , ", epos5.t
    
    # RUN THE SOLVER!
    nsteps = cam.Ellipse.solver(oe,  cl)
    
    print "solver done. back to python:"
    print "1st (s,t) solution=", oe.epos1
    print "2st (s,t) solution=", oe.epos2
    
    elc1 = calcEcenter(oe,a,b, cl,1)
    elc2 = calcEcenter(oe,a,b, cl,2)
    print "elc1=", elc1
    print "elc2=", elc2
    #exit()
    
    #elc2 = elc2
    #epos = oe.epos2
    
    fe1 = cam.Ellipse(elc1, a_axis, b_axis, radius1)
    fe2 = cam.Ellipse(elc2, a_axis, b_axis, radius1)
    
    # draw ellipse-centers
    myscreen.addActor( camvtk.Sphere(center=(elc1.x,elc1.y,elc1.z), radius=0.01, color=camvtk.lgreen) )
    myscreen.addActor( camvtk.Sphere(center=(elc2.x,elc2.y,elc2.z), radius=0.01, color=camvtk.pink) )
    
    # cc-points on the ellipse
    ccp1 = fe1.ePoint(oe.epos1)
    ccp2 = fe2.ePoint(oe.epos2)
    myscreen.addActor( camvtk.Sphere(center=(ccp1.x,ccp1.y,ccp1.z), radius=0.01, color=camvtk.lgreen) )
    myscreen.addActor( camvtk.Sphere(center=(ccp2.x,ccp2.y,ccp2.z), radius=0.01, color=camvtk.pink) )
    
    cl1 = fe1.oePoint(oe.epos1)
    cl2 = fe2.oePoint(oe.epos2)
    
    # circles
    myscreen.addActor( camvtk.Circle(radius=radius1, center=(cl1.x,cl1.y,cl1.z), color=camvtk.green) )
    myscreen.addActor( camvtk.Circle(radius=radius1, center=(cl2.x,cl2.y,cl2.z), color=camvtk.pink) )
    
    # torus
    tor = camvtk.Toroid(r1=radius1, r2=radius2, center=(cl1.x, cl1.y, cl1.z),rotXYZ=(0,0,0), color=camvtk.green)
    tor.SetOpacity(0.4)
    myscreen.addActor( tor)
    tor = camvtk.Toroid(r1=radius1, r2=radius2, center=(cl2.x, cl2.y, cl2.z),rotXYZ=(0,0,0), color=camvtk.pink)
    tor.SetOpacity(0.4)
    myscreen.addActor( tor)
    
    # line: ellipse-center to cc-point
    myscreen.addActor(camvtk.Line( p1=(elc1.x,elc1.y,elc1.z),p2=(ccp1.x,ccp1.y,ccp1.z), color=camvtk.cyan ))
    myscreen.addActor(camvtk.Line( p1=(elc2.x,elc2.y,elc2.z),p2=(ccp2.x,ccp2.y,ccp2.z), color=camvtk.cyan ))
    
    # line: cc-point to cl-point
    myscreen.addActor(camvtk.Line( p1=(cl1.x,cl1.y,cl1.z),p2=(ccp1.x,ccp1.y,ccp1.z), color=camvtk.yellow ))
    myscreen.addActor(camvtk.Line( p1=(cl2.x,cl2.y,cl2.z),p2=(ccp2.x,ccp2.y,ccp2.z), color=camvtk.yellow ))    
    
    # true cl
    #clt = cc1. 
    
    #fclpoint = camvtk.Sphere(center=(fclp.x,fclp.y,fclp.z), radius=0.01, color=camvtk.blue)
    #myscreen.addActor(fclpoint)
    
    # line from ellipse center to fcc
    # the offset normal
    #myscreen.addActor(camvtk.Line( p1=(fclp.x,fclp.y,fclp.z),p2=(fccp.x,fccp.y,fccp.z), color=camvtk.yellow ))
    
    drawellipse(myscreen, elc1, a_axis, b_axis)
    drawellipse(myscreen, elc2, a_axis, b_axis)

    #convtext = "%i" % (nsteps)
    #print (pt.x, pt.y, pt.z)
    #center=(pt.x, pt.y, pt.z)    
    #tst = camvtk.Text3D( color=(1,1,1), center=(pt.x, pt.y, 0)  , 
    #text=convtext, scale=0.02)
    #tst.SetCamera(myscreen.camera)
    #myscreen.addActor(tst)
            
    #colmax=11
    #colmin=4
    #nsteps = nsteps - colmin
    #colmax = colmax - colmin
    #convcolor=( float(nsteps*nsteps)/(colmax), float((colmax-nsteps))/colmax, 0 )
    #esphere = camvtk.Sphere(center=(p5.x,p5.y,0), radius=0.01, color=convcolor)
    #cce = oe.ePoint(epos)
    #cle = oe.oePoint(epos)
    #end_sphere = camvtk.Sphere(center=(cce.x,cce.y,0), radius=0.01, color=camvtk.green)
    #cl_sphere = camvtk.Sphere(center=(cle.x,cle.y,0), radius=0.01, color=camvtk.pink)
    #cl_sphere.SetOpacity(0.4)
    
    #clcir= camvtk.Circle(radius=radius1, center=(cle.x,cle.y,cle.z), color=camvtk.pink)
    #myscreen.addActor(clcir)
    
    #myscreen.addActor(esphere)
    #myscreen.addActor(end_sphere)
    #myscreen.addActor(cl_sphere)
    #myscreen.render()
  
    print "done."
    myscreen.render()
    lwr.SetFileName(filename)
    
    #raw_input("Press Enter to terminate")         
    time.sleep(0.5)
    #lwr.Write()
    myscreen.iren.Start()


if __name__ == "__main__":
    main()
    #myscreen.iren.Start()
    #raw_input("Press Enter to terminate")    

    
