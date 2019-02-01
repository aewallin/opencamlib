import ocl

print ocl.version()

p0 = ocl.CLPoint(0,0,0)
p1 = ocl.CLPoint(1,2,3)
p2 = ocl.CLPoint(1.1,2.2,3.3)
clp=[]
clp.append(p0)
clp.append(p1)
clp.append(p2)

f = ocl.LineCLFilter()
f.setTolerance(0.01)
for p in clp:
    f.addCLPoint(p)

f.run()
p2 = f.getCLPoints()
for p in p2:
    print(p)
