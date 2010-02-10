import camala as cam

def printP(p):
	print "p=(",p.x, p.y, p.z, ")"

if __name__ == "__main__":
	p = cam.Point()
	p.x=7
	p.y=8
	p.z=-3
	#q = cam.Point(1,2,3)
	#p=q
	#print p
	#print str(p)
	##print p.str()
	print p.str()
	print p.id
	q = cam.Point(1,2,3)
	print q.str()
	r = p + q
	s = cam.Point(66,66,66)
	s += r
	print r.str()
	print r.id
	print s.str()
	print s.id
	print "2*s=", (s*2).str()
	print "s.norm=", s.norm()
	print "normalizing"
	s.normalize()
	print "s.norm=", s.norm()
	print "s dot r", s.dot(r)
	print "s cross r", s.cross(r).str()
	t = cam.Triangle(p,q,r)
	print t.str()
	#a = t.p[1]
	#print a
	s= cam.STLSurf()
	print s
	print s.str()
	s.addTriangle(t)
	s.addTriangle(t)
	print s.str()
	print "end."
	
