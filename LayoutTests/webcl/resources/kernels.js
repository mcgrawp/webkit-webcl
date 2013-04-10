var Kernels =
{
 "square": "__kernel void square(\n        __global int* input,\n        __global int* output,\n        const unsigned int count)\n{\n    int i = get_global_id(0);\n    if(i < count)\n        output[i] = input[i] * input[i];\n}\n\n", 
 "kernel_item": "__kernel void kernel_item(\n        __global unsigned int *out)\n{\n        int i = get_global_id(0);\n        out[i] = 15;\n}\n"
}
