import ocl as cam
import camvtk
import time



if __name__ == "__main__":
	s= cam.STLSurf()
	print s.str()
	myscreen = camvtk.VTKScreen()
	stl = camvtk.STLSurf("demo.stl")
	print "STL surface read"
	myscreen.addActor(stl)
	stl.SetWireframe()
	
	polydata = stl.src.GetOutput()
	camvtk.vtkPolyData2OCLSTL(polydata, s)

	print s.str()

		

	myscreen.iren.Start()
	

	#raw_input("Press Enter to terminate") 
	
