import camala as cam
import pyvtk
import time


if __name__ == "__main__":
	p = cam.Point()
	p.x=7
	p.y=8
	p.z=-3
	print p.str()
	print p.id
	q = cam.Point(1,2,3)
	r = p + q
	t = cam.Triangle(p,q,r)
	print t.str()
	s= cam.STLSurf()
	print s
	print s.str()
	s.addTriangle(t)
	s.addTriangle(t)
	print s.str()
	print "end."
	
	myscreen = pyvtk.VTKScreen()
	print "screen created"
	stl = pyvtk.STLSurf("sphere.stl")
	print "STL surface read"
	myscreen.addActor(stl)
	
	#for n in range(1,2):
	#	myscreen.render()
	#	myscreen.camera.Azimuth( 1 )
	#	time.sleep(0.01)
	#print "done animating"
	#a = myscreen.iren.GetInteractorStyle()
	#a.SetCurrentStyleToTrackballCamera()
	
	#print a
	b = stl.src.GetOutput()
	print b
	print "Verts:",b.GetNumberOfVerts()
	print "Cells:",b.GetNumberOfCells()
	print "Lines:",b.GetNumberOfLines()
	print "Polys:",b.GetNumberOfPolys()
	print "Strips:",b.GetNumberOfStrips()
	c = b.GetCell(0)
	print c
	print "Points:",c.GetNumberOfPoints()
	print "Edges:",c.GetNumberOfEdges()
	print "Faces:",c.GetNumberOfFaces()
	ps = c.GetPoints()
	print ps
	n=ps.GetNumberOfPoints()
	print "Nr of Points:",n
	for id in range(0,n):
		print id,"=",
		print ps.GetPoint(id)
		myscreen.addActor( pyvtk.Sphere(radius=0.5,center=ps.GetPoint(id)) )
	#p1=ps.GetPoint(1)
	
	myscreen.iren.Start()

	#raw_input("Press Enter to terminate") 
	
