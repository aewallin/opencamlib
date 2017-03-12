# python stl file tools
# originally from Julian Todd / FreeSteel
# public domain code.

import re
import struct
import math
import sys

###########################################################################        
def TriangleNormal(x0, y0, z0, x1, y1, z1, x2, y2, z2):
    # calculate facet normal
    v01 = (x1 - x0, y1 - y0, z1 - z0)
    v02 = (x2 - x0, y2 - y0, z2 - z0)
    n = ( v01[1] * v02[2] - v01[2] * v02[1], 
          v01[2] * v02[0] - v01[0] * v02[2], 
          v01[0] * v02[1] - v01[1] * v02[0])
    ln = math.sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2])
    if ln > 0.0:
        return (n[0] / ln, n[1] / ln, n[2] / ln)
        


###########################################################################        
class reader:
    def __init__(self, fn = None):
        self.fn = fn
        if self.fn:
            fl = open(self.fn, "r")
            self.isascii = self.IsAscii(fl)
            fl.close()

        self.little_endian = (struct.unpack("<f", struct.pack("@f", 140919.00))[0] == 140919.00)
        
#        print "computer is little endian: ", self.little_endian
#        print "file is ascii: ", self.isascii
        
        self.nfacets = 0
        self.ndegenerate = 0
        
        self.mr = MeasureBoundingBox()
            
            
    def IsAscii(self, fdata):
        l = fdata.readline(1024)
        isascii = l[:5] == "solid" and (len(l) == 5 or (re.search("[^A-Za-z0-9\,\.\/\;\:\'\"\+\-\s\r\n]", l[6:]) == None)) # some files say 'solid' but are binary files, we try to find a non alphanumerical character in the rest to the first line
        fdata.seek(0)
        return isascii

    def ReadVertex(self, l):
        l = l.replace(",", ".") # Catia writes ASCII STL with , as decimal point
        if re.search("facet", l) or re.search("outer", l) or re.search("endloop", l) or re.search("endfacet", l):
            return
            
        vertex = re.search("vertex\s*([\d\-+\.EeDd]+)\s*([\d\-+\.EeDd]+)\s*([\d\-+\.EeDd]+)", l)
        
        if vertex:
            return (float(vertex.group(1)), float(vertex.group(2)), float(vertex.group(3)))



    def BinaryReadFacets(self, fl, fs = None):
        # 80 bytes of header
        hdr = fl.read(80)
        
        # 4 bytes for number of facets
        self.nfacets = struct.unpack("<i", fl.read(4))[0]

        nfacets = 0
        # we don't loop over self.nfacets because then we can recover any broken headers that show the wrong number of facets
        while True:
            try:
                #50 byte records with normals and vertices per facet
                fl.read(12) # override normal
                
                xyz = struct.unpack("<9f", fl.read(36)) # little endian
                if TriangleNormal(xyz[0], xyz[1], xyz[2], xyz[3], xyz[4], xyz[5], xyz[6], xyz[7], xyz[8]) == None:
                    self.ndegenerate = self.ndegenerate + 1
                if (fs):
                    fs.PushTriangle(xyz[0], xyz[1], xyz[2], xyz[3], xyz[4], xyz[5], xyz[6], xyz[7], xyz[8])
                    
                self.mr.PushTriangle(xyz[0], xyz[1], xyz[2], xyz[3], xyz[4], xyz[5], xyz[6], xyz[7], xyz[8])
                
                fl.read(2) # padding
                nfacets += 1
            
            except struct.error, e:
                break
        
        if self.nfacets != nfacets:
            sys.stderr.write("Number of facets according to header: %d, number of facets read: %d\n" % (self.nfacets, nfacets))
            self.nfacets = nfacets
                
            
    def AsciiReadFacets(self, fl, fs = None):
        lines = fl.readlines()
        xyz = []
        for l in lines:
            tpl = self.ReadVertex(l)
            if tpl:
                xyz.append(tpl[0])
                xyz.append(tpl[1])
                xyz.append(tpl[2])
        
            if len(xyz) == 9:
                if not TriangleNormal(xyz[0], xyz[1], xyz[2], xyz[3], xyz[4], xyz[5], xyz[6], xyz[7], xyz[8]):
                    self.ndegenerate += 1
                if (fs):
                    fs.PushTriangle(xyz[0], xyz[1], xyz[2], xyz[3], xyz[4], xyz[5], xyz[6], xyz[7], xyz[8])

                self.nfacets += 1
                self.mr.PushTriangle(xyz[0], xyz[1], xyz[2], xyz[3], xyz[4], xyz[5], xyz[6], xyz[7], xyz[8])
                xyz = []

        

################################################################################        
class writer:
    def __init__(self, fn, write_ascii = False):
        self.fn = fn
        self.ascii = write_ascii
        self.scale = 1.0
        
    def write(self, fc):
        self.fl = open(self.fn, "w")
        self.WriteHeader(self.fl, fc.nfacets)
        for t in xrange(fc.nfacets):
            x0, y0, z0, x1, y1, z1, x2, y2, z2 = fc.GetFacet(t)
            self.WriteFacet(x0, y0, z0, x1, y1, z1, x2, y2, z2)
            
        self.WriteFooter(self.fl)
        self.fl.flush()                
        self.fl.close()
        
    def WriteHeader(self, fl, nfacets):
        if self.ascii:
            fl.write("solid\n")
        else:
            str = "Stereolithography                                                               "
            assert(len(str) == 80)
            fl.write(str)
            fl.write(struct.pack("<i", nfacets))
            
    def WriteFacet(self, x0, y0, z0, x1, y1, z1, x2, y2, z2, skip_degenerated = True):
        if self.scale != 1.0:
            x0 *= self.scale
            y0 *= self.scale
            z0 *= self.scale
            x1 *= self.scale
            y1 *= self.scale
            z1 *= self.scale
            x2 *= self.scale
            y2 *= self.scale
            z2 *= self.scale
        # calculate facet normal
        n = TriangleNormal(x0, y0, z0, x1, y1, z1, x2, y2, z2)
        if n == None:
            if skip_degenerated: return
            n = (0.0, 0.0, 0.0)
            
        if self.ascii:
            self.fl.write("facet normal %f %f %f\n" % n)            
            self.fl.write("outer loop\n vertex %f %f %f\n vertex %f %f %f\n vertex %f %f %f\nendloop\nendfacet\n" % 
                          (x0, y0, z0, x1, y1, z1, x2, y2, z2))
        else:
            self.fl.write(struct.pack("<12f2c", n[0], n[1], n[2], x0, y0, z0, x1, y1, z1, x2, y2, z2, " ", " "))
        
    def WriteFooter(self, fl):
        if self.ascii:
            fl.write("endsolid\n")

    def PushTriangle(self, x0, y0, z0, x1, y1, z1, x2, y2, z2):
        self.WriteFacet(x0, y0, z0, x1, y1, z1, x2, y2, z2) 

################################################################################              
class MeasureBoundingBox:
    def __init__(self):
        self.xlo = None
        self.xhi = None
        self.ylo = None
        self.yhi = None
        self.zlo = None
        self.zhi = None
        
    def PushTriangle(self, x0, y0, z0, x1, y1, z1, x2, y2, z2):
        for v in [(x0, y0, z0), (x1, y1, z1), (x2, y2, z2)]:
            if self.xlo is None or v[0] < self.xlo:
                self.xlo = v[0]
            if self.ylo is None or v[1] < self.ylo:
                self.ylo = v[1]
            if self.zlo is None or v[2] < self.zlo:
                self.zlo = v[2]
            if self.xhi is None or v[0] > self.xhi:
                self.xhi = v[0]
            if self.yhi is None or v[1] > self.yhi:
                self.yhi = v[1]
            if self.zhi is None or v[2] > self.zhi:
                self.zhi = v[2]

            

###########################################################################        
class converter(reader):
    def __init__(self, fin = None):
        reader.__init__(self, fin)
        
        # read to find number of facets, but substract degenerated facets
        self.wr = None
        
    def convert(self, fout, freadfrom = None):
        if self.fn:
            rmod =  self.isascii and "r" or "rb"        
            fl = open(self.fn, rmod)
            if self.isascii:
                self.AsciiReadFacets(fl)
            else:
                self.BinaryReadFacets(fl)
            fl.close()
            
        elif freadfrom:
            if self.isascii:
                self.AsciiReadFacets(freadfrom)
            else:
                self.BinaryReadFacets(freadfrom)
            freadfrom.seek(0) # rewind to start
            
        self.wr = writer(fout, not self.isascii)
        wmod = self.isascii and "wb" or "w"
        self.fpout = open(fout, wmod)
        self.wr.fl = self.fpout
        self.wr.WriteHeader(self.fpout, self.nfacets - self.ndegenerate)
    
        self.ndegenerate = 0    
        if self.fn:
            rmod =  self.isascii and "r" or "rb"        
            fl = open(self.fn, rmod)
            if self.isascii:
                self.AsciiReadFacets(fl, self)
            else:
                self.BinaryReadFacets(fl, self)
            fl.close()
            
        elif freadfrom:
            if self.isascii:
                self.AsciiReadFacets(freadfrom, self)
            else:
                self.BinaryReadFacets(freadfrom, self)

        self.wr.WriteFooter(self.fpout)
        self.fpout.close()

    
    def PushTriangle(self, x0, y0, z0, x1, y1, z1, x2, y2, z2):
        if self.wr != None:
            self.wr.WriteFacet(x0, y0, z0, x1, y1, z1, x2, y2, z2) 



###########################################################################
# use all the options flag.  could have -tk which causes it to import using tk, -in, -out
# design all the settings so it works as pipe, or as files.
# stltools --in=file.stl --out=fil1.stl -b/-a if --out missing then piping
# stltools --tk does the following.
# stltools --in=file.stl --stats prints bounding box etc.
if __name__ == '__main__':
    import tkFileDialog
    fin = tkFileDialog.askopenfilename(
                defaultextension = '*.stl',
                filetypes = [('Stereolithography','*.stl'),('all files','*.*')],
                title = "Open STL")
    a = converter(fin)

    t = a.isascii and "Save as STL (Binary format)" or "Save as STL (ASCII format)"
    fout = tkFileDialog.asksaveasfilename(
                defaultextension = '*.stl',
                filetypes = [('Stereolithography','*.stl'),('all files','*.*')],
                title = t)
                
    a.convert(fout)



# Example STL ascii file:
#
# solid
# ...
# facet normal 0.00 0.00 1.00
#    outer loop
#      vertex  2.00  2.00  0.00
#      vertex -1.00  1.00  0.00
#      vertex  0.00 -1.00  0.00
#    endloop
#  endfacet
# ...
# endsolid
