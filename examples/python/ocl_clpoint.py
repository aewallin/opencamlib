import ocl

# print(ocl.version())

p = ocl.Point(1,2,3)
print p
cc = ocl.CCPoint(4,5,6)
print cc
cl = ocl.CLPoint()
print cl
cl2 = ocl.CLPoint(7,8,9)
print cl2
cl3 = ocl.CLPoint(10,11,12,cc)
print cl3
cc.x=77
print cl3
