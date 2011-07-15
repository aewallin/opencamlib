
#include <string>
#include <iostream>
#include <cmath>

#include <opencamlib/point.hpp>
#include <opencamlib/numeric.hpp>

int main() {
    std::cout << ocl::revision() << "\n";
    ocl::Point p1;
    p1.x = 1.2;
    p1.y = sqrt(p1.x);
    p1.z = p1.x + p1.y;
    std::cout << "p1=" << p1 << "\n";
    ocl::Point p2(1.23,4.56,3.219);
    std::cout << "p2=" << p2 << "\n";
    
    std::cout << "addition:\n";
    std::cout << "  " << p1 << " + " << p2 << " = " << p1+p2 << "\n";
    std::cout << "subtraction:\n";
    std::cout << "  " << p1 << " - " << p2 << " = " << p1-p2 << "\n";
    std::cout << "dot product:\n";
    std::cout << "  " << p1 << " dot " << p2 << " = " << p1.dot(p2) << "\n";
    std::cout << "cross product:\n";
    std::cout << "  " << p1 << " cross " << p2 << " = " << p1.cross(p2) << "\n";
    std::cout << "scalar multiplication:\n";
    std::cout << "  " << p1 << " * 0.1  = " << 0.1*p1 << "\n";
    std::cout << "norm:\n";
    std::cout << "  norm( " << p1 << " )  = " << p1.norm() << "\n";
    std::cout << "normalize:\n";
    ocl::Point p3 = p1;
    p3.normalize();
    std::cout << "  " << p1 << ".normalize()  = " << p3 << " norm=" << p3.norm() << "\n";
    
    return 0;
}
