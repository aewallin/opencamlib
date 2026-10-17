#include "stlsurf.hpp"
#include "stlreader.hpp"
#include "check.hpp"
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <limits>
#include <stdexcept>

static void u32(std::string& data, std::uint32_t value) {
    for (int i = 0; i < 4; ++i) data += static_cast<char>(value >> (8*i));
}
static std::string binary(const std::string& header) {
    std::string data = header;
    data.resize(80, '\0');
    u32(data, 1);
    for (float value : {0.f,0.f,1.f, 0.f,0.f,0.f, 1.f,0.f,0.f, 0.f,1.f,0.f}) {
        std::uint32_t bits;
        std::memcpy(&bits, &value, 4);
        u32(data, bits);
    }
    data.append(2, '\0');
    return data;
}
static void read(const std::string& data, ocl::STLSurf& surface) {
    const char* filename = "stlreader-fixture.stl";
    { std::ofstream file(filename, std::ios::binary); file.write(data.data(), data.size()); }
    try { ocl::STLReader reader(L"stlreader-fixture.stl", surface); }
    catch (...) { std::remove(filename); throw; }
    std::remove(filename);
}
static void invalid(const std::string& data) {
    ocl::STLSurf surface;
    surface.addTriangle(ocl::Triangle());
    const auto original = surface.tris.front();
    bool caught = false;
    try { read(data, surface); } catch (const std::runtime_error&) { caught = true; }
    CHECK(caught);
    CHECK(surface.size() == 1);
    CHECK(surface.tris.front().p[0] == original.p[0]);
}
int main() {
    const std::string facet = "facet normal 0 0 1\nouter loop\nvertex 0 0 0\n"
                              "vertex 1 0 0\nvertex 0 1 0\nendloop\nendfacet\n";
    for (const auto& data : {binary("binary"), binary("solid binary"),
                            "solid example\n" + facet + "endsolid example\n",
                            "solid " + std::string(10000, 'x') + "\n" + facet + "endsolid\n"}) {
        ocl::STLSurf surface;
        read(data, surface);
        CHECK(surface.size() == 1);
        const auto& t = surface.tris.front();
        CHECK(t.p[0] == ocl::Point(0,0,0));
        CHECK(t.p[1] == ocl::Point(1,0,0));
        CHECK(t.p[2] == ocl::Point(0,1,0));
        read(data, surface);
        CHECK(surface.size() == 2);
    }
    ocl::STLSurf multiple;
    read("solid a\n" + facet + "endsolid a\nsolid b\n" + facet + "endsolid b\n", multiple);
    CHECK(multiple.size() == 2);
    invalid(""); invalid("solid"); invalid(binary("solid").substr(0, 100));
    invalid(binary("binary").substr(0, 133));
    invalid("solid test\n" + facet); // Missing terminator.
    invalid("solid test\n" + facet + "facet normal 0 0 1\nouter loop\nvertex 0 0 0\nendsolid\n");
    invalid("solid test\n" + facet + "garbage\nendsolid\n");
    invalid("solid test\nfacet normal 0 0 1\nouter loop\nvertex 0 0 0\nvertex 1 0 0\nendloop\nendfacet\nendsolid\n");
    std::string count_overflow(80, '\0'); u32(count_overflow, 0xffffffffu);
    invalid(count_overflow);
    for (const auto& third : {ocl::Point(0,0,0), ocl::Point(2,0,0),
                             ocl::Point(std::numeric_limits<double>::quiet_NaN(),1,0)}) {
        bool caught = false;
        try {
            ocl::STLSurf surface;
            surface.addTriangle(ocl::Triangle({0,0,0}, {1,0,0}, third));
        } catch (const std::invalid_argument&) { caught = true; }
        CHECK(caught);
    }
    ocl::STLSurf unicode_surface;
#ifdef _WIN32
    { std::ofstream file(L"stlreader-\u00e9.stl", std::ios::binary); file << binary("binary"); }
#else
    { std::ofstream file(u8"stlreader-\u00e9.stl", std::ios::binary); file << binary("binary"); }
#endif
    ocl::STLReader unicode_reader(L"stlreader-\u00e9.stl", unicode_surface);
    CHECK(unicode_surface.size() == 1);
#ifdef _WIN32
    _wremove(L"stlreader-\u00e9.stl");
#else
    std::remove(u8"stlreader-\u00e9.stl");
#endif
    ocl::STLSurf empty;
    read("solid empty\nendsolid empty\n", empty);
    CHECK(empty.size() == 0);
    std::string nonfinite = binary("binary");
    nonfinite.replace(96, 4, "\0\0\xc0\x7f", 4);
    invalid(nonfinite);
    ocl::STLSurf precise;
    read("solid precise\nfacet normal 0 0 1\nouter loop\n"
         "vertex 0.123456789012345 0 0\nvertex 1 0 0\nvertex 0 1 0\n"
         "endloop\nendfacet\nendsolid precise\n", precise);
    CHECK(precise.tris.front().p[0].x == 0.123456789012345);
    bool missing = false;
    try { ocl::STLReader reader(L"missing-reader-test-directory/missing.stl", multiple); }
    catch (const std::runtime_error&) { missing = true; }
    CHECK(missing);
}
