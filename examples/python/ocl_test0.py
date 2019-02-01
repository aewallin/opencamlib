import ocl

if __name__ == "__main__":
    p = ocl.Point()
    p.x=7
    p.y=8
    p.z=-3

    print(p)
    q = ocl.Point(1,2,3)
    print(q)
    r = p + q
    s = ocl.Point(66,66,66)
    s += r
    print(r)
    print(s)
    print("2*s=", (s*2))
    print("s.norm=", s.norm())
    print("normalizing")
    s.normalize()
    print("s.norm=", s.norm())
    print("s dot r", s.dot(r))
    print("s cross r", s.cross(r))
    t = ocl.Triangle(p,q,r)
    print(t)
    s= ocl.STLSurf()
    print(s)
    s.addTriangle(t)
    s.addTriangle(t)
    print(s)
    print("end.")
    
