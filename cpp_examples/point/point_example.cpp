
#include <string>
#include <iostream>
#include <cmath>

#include <opencamlib/point.h>

int main() {
    ocl::Point p;
    p.x = 1.2;
    p.y = sqrt(p.x);
    std::cout << p << "\n";
    return 0;
}
