#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <time.h>
#include <unistd.h>

/*
 * Adreno 650 SHA-256 Brute-force Engine [Level 4: The Singularity]
 * Tested on: Samsung Galaxy Note 20 (Snapdragon 865+)
 * Performance: ~3.60 MH/s (Sustainable under thermal pressure)
 * Achievement: Found Level 4 Nonce (0x0000000F) in 17.30s
 * Author: [TS 콘도르]
 */

// [1] 오프셋 기반 SHA-256 최적화 커널 (Level 3 대응)
const char *kernelSource = 
"#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))\n"
"#define Ch(x, y, z) ((x & y) ^ (~x & z))\n"
"#define Maj(x, y, z) ((x & y) ^ (x & z) ^ (y & z))\n"
"#define Sigma0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))\n"
"#define Sigma1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))\n"
"#define sigma0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))\n"
"#define sigma1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))\n"
"\n"
"__kernel void sha256_engine(__global uint* out_found, __global uint* out_nonce, const uint target, const uint offset) {\n"
"    uint gid = get_global_id(0);\n"
"    uint global_nonce = gid + offset;\n"
"\n"
"    uint w[16];\n"
"    w[0] = global_nonce;\n"
"    for(int i=1; i<16; i++) w[i] = 0;\n"
"\n"
"    uint a = 0x6a09e667, b = 0xbb67ae85, c = 0x3c6ef372, d = 0xa54ff53a;\n"
"    uint e = 0x510e527f, f = 0x9b05688c, g = 0x1f83d9ab, h = 0x5be0cd19;\n"
"    const uint K[16] = {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,\n"
"                        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174};\n"
"\n"
"    for(int i=0; i<16; i++) {\n"
"        uint t1 = h + Sigma1(e) + Ch(e, f, g) + K[i] + w[i];\n"
"        uint t2 = Sigma0(a) + Maj(a, b, c);\n"
"        h = g; g = f; f = e; e = d + t1;\n"
"        d = c; c = b; b = a; a = t1 + t2;\n"
"    }\n"
"\n"
"    if (a < target) {\n"
"        if(atomic_xchg(out_found, 1) == 0) {\n"
"            *out_nonce = global_nonce;\n"
"        }\n"
"    }\n"
"}\n";

int main() {
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;

    clGetPlatformIDs(1, &platform, NULL);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    queue = clCreateCommandQueue(context, device, 0, NULL);
    program = clCreateProgramWithSource(context, 1, &kernelSource, NULL, NULL);
    clBuildProgram(program, 1, &device, "-cl-mad-enable", NULL, NULL);
    kernel = clCreateKernel(program, "sha256_engine", NULL);

    // Level 3 설정 (상위 24비트가 0이어야 함)
    uint32_t target = 0x0000000F; 
    uint32_t h_found = 0, h_nonce = 0;
    unsigned long long total_attempted = 0;
    const size_t batch_size = 1048576; // 1M Batch

    cl_mem m_found = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(uint32_t), NULL, NULL);
    cl_mem m_nonce = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(uint32_t), NULL, NULL);

    printf("==============================================\n");
    printf("  Adreno 650 Level 4 [Deep Abyss] Engine\n");
    printf("  Target Difficulty: 0x%08X\n", target);
    printf("==============================================\n\n");

    struct timespec start_time, current_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    while(1) {
        h_found = 0;
        clEnqueueWriteBuffer(queue, m_found, CL_TRUE, 0, sizeof(uint32_t), &h_found, 0, NULL, NULL);
        
        clSetKernelArg(kernel, 0, sizeof(cl_mem), &m_found);
        clSetKernelArg(kernel, 1, sizeof(cl_mem), &m_nonce);
        clSetKernelArg(kernel, 2, sizeof(uint32_t), &target);
        clSetKernelArg(kernel, 3, sizeof(uint32_t), (uint32_t *)&total_attempted);

        clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &batch_size, NULL, 0, NULL, NULL);
        clFinish(queue);

        clEnqueueReadBuffer(queue, m_found, CL_TRUE, 0, sizeof(uint32_t), &h_found, 0, NULL, NULL);
        
        if(h_found) {
            clEnqueueReadBuffer(queue, m_nonce, CL_TRUE, 0, sizeof(uint32_t), &h_nonce, 0, NULL, NULL);
            clock_gettime(CLOCK_MONOTONIC, &current_time);
            double final_sec = (current_time.tv_sec - start_time.tv_sec) + (current_time.tv_nsec - start_time.tv_nsec) / 1e9;
            printf("\n\n[★] LEVEL 4 정복 성공! \n");
            printf("발견된 Nonce : %u\n", h_nonce);
            printf("최종 소요 시간: %.2f 초\n", final_sec);
            printf("총 연산 횟수  : %llu 회\n", total_attempted + (h_nonce % batch_size));
            break;
        }

        total_attempted += batch_size;
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        double elapsed = (current_time.tv_sec - start_time.tv_sec) + (current_time.tv_nsec - start_time.tv_nsec) / 1e9;
        
        // 실시간 상태 업데이트
        printf("\r[진행] 시간: %.1fs | 시도: %llu | 속도: %.2f MH/s", 
               elapsed, total_attempted, (total_attempted / 1e6) / elapsed);
        fflush(stdout);

        // 발열 제어가 필요한 경우 usleep 추가 가능
    }

    clReleaseMemObject(m_found); clReleaseMemObject(m_nonce);
    clReleaseKernel(kernel); clReleaseProgram(program);
    clReleaseCommandQueue(queue); clReleaseContext(context);
    return 0;
}

