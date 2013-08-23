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
      }",
  "compute_sin": "\
      kernel void compute_sin(global float4 *vbo, int width, int height, int seq) {\
          int i = get_global_id(0);\
          float a, b, w, h;\
          float4 point;\
          w = (float) width;\
          h = (float) height;\
          a = h / 4;\
          b = w / 2;\
          point.x = i;\
          point.y = b + a * sin(3.1412 * 2 * ( ( (float)(i + seq) ) / w) );\
          point.z = i + 1;\
          point.w = b + a * sin(3.1412 * 2 * ( ( (float)(i + 1 + seq) ) / w) );\
          vbo[i] = point;\
      }",

  "imgSquare": "\
    constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE |\
                                    CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;\
    kernel void square(read_only image2d_t imgIn, write_only image2d_t imgOut) {\
        int x = get_global_id(0);\
        int y = get_global_id(1);\
        int2 coord = (int2)(x, y);\
        uint4 pixel = read_imageui(imgIn, sampler, coord);\
        uint4 pixelSqr = (uint4)(pixel.x * pixel.x, 0, 0, 0);\
        write_imageui(imgOut, coord, pixelSqr);\
     }",

  "copyTexture": "\
    constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE |\
                                    CLK_ADDRESS_CLAMP | CLK_FILTER_NEAREST;\
    kernel void copyTex(read_only image2d_t texIn, write_only image2d_t texOut) {\
        int x = get_global_id(0);\
        int y = get_global_id(1);\
        int2 coord = (int2)(x, y);\
        float4 pixel = read_imagef(texIn, sampler, coord);\
        write_imagef(texOut, coord, pixel);\
     }"
};
