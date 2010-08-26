
std::vector<Triangle>* Triangle::zslice(const double zcut) const {
    std::vector<Triangle>* tlist = new std::vector<Triangle>();
    if ( (zcut < this->bb.minpt.z) || ((zcut > this->bb.maxpt.z)) ) {
        //std::cout << "zcut outside bbox, nothing to do.\n";
        tlist->push_back(*this);
        return tlist;
    } else {
        // find out how many vertices are below zcut
        std::vector<Point> below;
        std::vector<Point> above;
        for (int m=0;m<3;++m) {
            if ( p[m].z <= zcut )
                below.push_back(p[m]);
            else
                above.push_back(p[m]);
        }
        //std::cout << below.size() << " vertices below zcut\n";
        assert( (below.size() == 1) || (below.size() == 2) );
        
        if ( below.size() == 2 ) {
            assert( above.size() == 1 );
            // find two new intersection points 
            // edge is p1 + t*(p2-p1) = zcut
            // so t = zcut-p1 / (p2-p1)
            double t1 = (zcut - above[0].z) / (below[0].z - above[0].z); // div by zero?!
            double t2 = (zcut - above[0].z) / (below[1].z - above[0].z);
            Point p1 = above[0];
            Point p2 = above[0] + t1*(below[0] - above[0]);
            Point p3 = above[0] + t2*(below[1] - above[0]);
            tlist->push_back( Triangle( p1 , p2 , p3) );
            return tlist;
            
        } else if ( below.size() == 1 ) {
            assert( above.size() == 2 );
            // find intersection points and add two new triangles
            // t = (zcut -p1) / (p2-p1)
            double t1 = (zcut - above[0].z) / (below[0].z - above[0].z); 
            double t2 = (zcut - above[1].z) / (below[0].z - above[1].z);
            Point p1 = above[0] + t1*(below[0]-above[0]); 
            Point p2 = above[1] + t2*(below[0]-above[1]);
            Point p3 = above[0];
            Point p4 = above[1];
            Triangle tri1 = Triangle(p1,p2,p3);
            tlist->push_back( tri1 );
            Triangle tri2;
            // which of the below points to select for the other tri
            // either p1 or p2
            double tmp1, tmp2;
            xy_line_line_intersection( p3, p1, tmp1, p4, p2, tmp2 ); 
            std::cout << " p3-p1 tmp1=" << tmp1 << "\n"; 
            if ( (tmp1 > 0.0) && (tmp1 < 1.0) ) // p4-p2 intesects, so choose p1
                tri2 = Triangle(p1,p3,p4);
            else
                tri2 = Triangle(p2,p3,p4);
            tlist->push_back( tri2 );
            return tlist;
        } else {
            assert(0);
        }
        
    }
    
    return tlist;
}



boost::python::list Triangle::getZslice(const double zcut) const {
    boost::python::list py_tlist;
    std::vector<Triangle>* tlist;
    tlist = this->zslice(zcut);
    BOOST_FOREACH(Triangle t, *tlist) {
        py_tlist.append(t);
    }
    return py_tlist;
}

