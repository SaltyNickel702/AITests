#define CL_HPP_TARGET_OPENCL_VERSION 300
#define CL_HPP_MINIMUM_OPENCL_VERSION 120
#include <CL/opencl.hpp>
#include <functional>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>


#define CHECK(err) if(err != CL_SUCCESS) { \
    std::cerr << "OpenCL error: " << err << "\n"; exit(1); }


using namespace std;

void test () {
    cl_uint platformCount;
    clGetPlatformIDs(0, nullptr, &platformCount);
    cout << "Found " << platformCount << " OpenCL platforms." << endl;
}

struct Kernel {
    Kernel (string fileName, function<void()> onRun) {
        execute = onRun;
    }
    Kernel (string fileName) : Kernel(fileName,[](){}) {}
    function<void()> execute;    


    private:
        char* kernelSource; //source code
};


std::string loadFile(const char* path)
{
    std::ifstream file(path);
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}
void SimpleKernel () {
    constexpr int N = 16;

    // -----------------------------
    // CPU data
    // -----------------------------
    std::vector<float> data(N);
    for (int i = 0; i < N; i++)
        data[i] = float(i);

    // -----------------------------
    // Platform + device
    // -----------------------------
    cl_uint platformCount;
    clGetPlatformIDs(0, nullptr, &platformCount);
    std::vector<cl_platform_id> platforms(platformCount);
    clGetPlatformIDs(platformCount, platforms.data(), nullptr);

    cl_platform_id platform = platforms[0];

    cl_uint deviceCount;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, nullptr, &deviceCount);
    std::vector<cl_device_id> devices(deviceCount);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, deviceCount, devices.data(), nullptr);

    cl_device_id device = devices[0];

    // -----------------------------
    // Context + queue
    // -----------------------------
    cl_int err;
    cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    CHECK(err);

    cl_command_queue queue =
        clCreateCommandQueue(context, device, 0, &err);
    CHECK(err);

    // -----------------------------
    // Buffer (CPU → GPU)
    // -----------------------------
    cl_mem buffer = clCreateBuffer(
        context,
        CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        sizeof(float) * N,
        data.data(),
        &err
    );
    CHECK(err);

    // -----------------------------
    // Program + kernel
    // -----------------------------
    std::string source = loadFile("double.cl");
    const char* src = source.c_str();
    size_t len = source.size();

    cl_program program = clCreateProgramWithSource(
        context, 1, &src, &len, &err);
    CHECK(err);

    err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS)
    {
        size_t logSize;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                              0, nullptr, &logSize);
        std::string log(logSize, '\0');
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG,
                              logSize, log.data(), nullptr);
        std::cerr << log << "\n";
        exit(1);
    }

    cl_kernel kernel = clCreateKernel(program, "multiply_by_two", &err);
    CHECK(err);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer);

    // -----------------------------
    // Launch kernel
    // -----------------------------
    size_t globalSize = N; // one thread per element

    err = clEnqueueNDRangeKernel(
        queue,
        kernel,
        1,
        nullptr,
        &globalSize,
        nullptr,   // let OpenCL choose local size
        0, nullptr, nullptr
    );
    CHECK(err);

    clFinish(queue);

    // -----------------------------
    // GPU → CPU
    // -----------------------------
    clEnqueueReadBuffer(
        queue,
        buffer,
        CL_TRUE,
        0,
        sizeof(float) * N,
        data.data(),
        0, nullptr, nullptr
    );

    // -----------------------------
    // Result
    // -----------------------------
    for (float v : data)
        std::cout << v << " ";
    std::cout << "\n";

    // -----------------------------
    // Cleanup
    // -----------------------------
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseMemObject(buffer);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
}

int main () {
    SimpleKernel();
    return 0;
}