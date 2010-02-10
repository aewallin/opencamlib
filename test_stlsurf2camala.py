import camala as cam
import pyvtk
import time



if __name__ == "__main__":
	s= cam.STLSurf()
	print s.str()
	myscreen = pyvtk.VTKScreen()
	stl = pyvtk.STLSurf("sphere.stl")
	print "STL surface read"
	myscreen.addActor(stl)
	stl.SetWireframe()
	
	polydata = stl.src.GetOutput()
	pyvtk.vtkPolyData2CamalaSTL(polydata, s)

	print s.str()

		

	myscreen.iren.Start()
	

	#raw_input("Press Enter to terminate") 
	
