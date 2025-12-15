#include <CL/cl.h> // use "vcpkg install opencl" to install OpenCL
#include <functional>
#include <iostream>

using namespace std;

void test () {
    cl_uint platformCount;
    clGetPlatformIDs(0, nullptr, &platformCount);
    cout << "Found " << platformCount << " OpenCL platforms." << endl;
}

struct Kernel {
    Kernel (string fileName, function<*void()> onRun) {
        execute = onRun;
    }
    Kernel (string fileName) : Kernel(fileName,[](){}) {}
    function<*void()> execute;    


    private:
        char* kernelSource;
}

int main () {
    test();
    return 0;
}