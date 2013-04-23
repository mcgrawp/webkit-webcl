var Kernels =
{
  "square": "\
     kernel void square(global int* input, global int* output, const unsigned int count) {\
         int i = get_global_id(0);\
         if(i < count)\
            output[i] = input[i] * input[i];\
     }",

  "kernel_item": "\
     kernel void kernel_item(global unsigned int *out) {\
         int i = get_global_id(0);\
         out[i] = 15;\
     }",

  "vector_add": "\
      kernel void vector_add( global  int* vectorIn1, global  int* vectorIn2,  global  int* vectorOut, int uiVectorWidth) {\
          int x = get_global_id(0);\
          if (x >= (uiVectorWidth))\
          {\
              return;\
          }\
          vectorOut[x] = vectorIn1[x] + vectorIn2[x];\
      }",

  "vector_sub": "\
      kernel void vector_sub(global  int* vectorIn1, global  int* vectorIn2, global  int* vectorOut, int uiVectorWidth) {\
          int x = get_global_id(0);\
          if (x >= (uiVectorWidth))\
          {\
              return;\
          }\
          vectorOut[x] = vectorIn1[x] - vectorIn2[x];\
      }",

  "vector_div": "\
      kernel void vector_div(global  int* vectorIn1, global  int* vectorIn2, global  int* vectorOut, int uiVectorWidth) {\
          int x = get_global_id(0);\
          if (x >= (uiVectorWidth))\
          {\
              return;\
          }\
          vectorOut[x] = vectorIn1[x] / vectorIn2[x];\
      }",

  "vector_mult": "\
      kernel void vector_mult(global  int* vectorIn1, global  int* vectorIn2, global  int* vectorOut, int uiVectorWidth) {\
          int x = get_global_id(0);\
          if (x >= (uiVectorWidth))\
          {\
              return;\
          }\
          vectorOut[x] = vectorIn1[x] * vectorIn2[x];\
      }"
};
