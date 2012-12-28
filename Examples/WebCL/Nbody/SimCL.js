/*
 * Copyright (C) 2011 Samsung Electronics Corporation. All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided the following conditions
 * are met:
 * 
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY SAMSUNG ELECTRONICS CORPORATION AND ITS
 * CONTRIBUTORS "AS IS", AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SAMSUNG
 * ELECTRONICS CORPORATION OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES(INCLUDING
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS, OR BUSINESS INTERRUPTION), HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING
 * NEGLIGENCE OR OTHERWISE ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// local OpenCL info
//
var platforms;                           // array of OpenCL platform ids
var platform;                            // OpenCL platform id
var devices;                             // array of OpenCL device ids
var device;                              // OpenCL device id
var context;                                // OpenCL context
var queue;                                  // OpenCL command queue
var program;                                // OpenCL program
var kernel;                                 // OpenCL kernel

var curPosBuffer;                           // OpenCL buffer created from GL VBO
var curVelBuffer;                           // OpenCL buffer created from GL VBO
var nxtPosBuffer;                           // OpenCL buffer
var nxtVelBuffer;                           // OpenCL buffer

var bufferSize = null;
var cl = null;

var globalWorkSize = new Int32Array(1);
var localWorkSize = new Int32Array(1);
var workGroupSize = 0;

function getKernel(id) {
  var kernelScript = document.getElementById(id);
  if (kernelScript === null || kernelScript.type !== "x-kernel")
    return null;
  return kernelScript.firstChild.textContent;
}


function InitCL() {

  try {
    // Create CL buffers from GL VBOs
    // (Initial load of positions is via gl.bufferData)
    //
    curPosBuffer = context.createFromGLBuffer(cl.MEM_READ_WRITE, userData.curPosVBO);
    if (curPosBuffer === null) {
      console.error("Failed to allocate device memory");
      return null;
    }

    curVelBuffer = context.createFromGLBuffer(cl.MEM_READ_WRITE, userData.curVelVBO);
    if (curVelBuffer === null) {
      console.error("Failed to allocate device memory");
      return null;
    }

    bufferSize = NBODY * POS_ATTRIB_SIZE * Float32Array.BYTES_PER_ELEMENT;

    // Create CL working buffers (will be copied to current buffers after computation)
    //
    nxtPosBuffer = context.createBuffer(cl.MEM_READ_WRITE, bufferSize);
    if (nxtPosBuffer === null) {
      console.error("Failed to allocate device memory");
      return null;
    }

    nxtVelBuffer = context.createBuffer(cl.MEM_READ_WRITE, bufferSize);
    if (nxtVelBuffer === null) {
      console.error("Failed to allocate device memory");
      return null;
    }

    globalWorkSize[0] = NBODY;
    localWorkSize[0] = Math.min(workGroupSize, NBODY);

    // Initial load of velocity data
    //
    //queue.enqueueAcquireGLObjects(curVelBuffer, null);
    queue.enqueueAcquireGLObjects(curVelBuffer);

    queue.enqueueWriteBuffer(curVelBuffer, true, 0, bufferSize, userData.curVel);

    //queue.enqueueReleaseGLObjects(curVelBuffer, null);
    queue.enqueueReleaseGLObjects(curVelBuffer);

    queue.finish(GetNullResults, 0);
  }
  catch (e)
  {
    console.error("Nbody Demo Failed ; Message: "+ e.message);
  }
  return cl;
}

function GetNullResults(userData)
{
}

function SimulateCL(cl) {
  try {
    if (cl === null)
      return;
    if (userData.isGLCLshared) {
      queue.enqueueAcquireGLObjects(curPosBuffer);
      queue.enqueueAcquireGLObjects(curVelBuffer);
      //queue.enqueueAcquireGLObjects(curPosBuffer, null);
      //queue.enqueueAcquireGLObjects(curVelBuffer, null);
    }
    var localMemSize = localWorkSize[0] * POS_ATTRIB_SIZE * Float32Array.BYTES_PER_ELEMENT;
    kernel.setArg(0, curPosBuffer);
    kernel.setArg(1, curVelBuffer);
    kernel.setArg( 2, NBODY, cl.LONG);
    kernel.setArg(3, DT, cl.FLOAT_KERNEL_ARG);
    kernel.setArg(4, EPSSQR, cl.LONG);
    kernel.setArg(5, localMemSize);  // __local: val (ignored) and size
    kernel.setArg(6, nxtPosBuffer);
    kernel.setArg(7, nxtVelBuffer);
    queue.enqueueNDRangeKernel(kernel, null, globalWorkSize, localWorkSize );

    queue.finish();
    queue.enqueueCopyBuffer(nxtPosBuffer, curPosBuffer, 0, 0, bufferSize);
    queue.enqueueCopyBuffer(nxtVelBuffer, curVelBuffer, 0, 0, bufferSize);

    if (userData.isGLCLshared) {
      //queue.enqueueReleaseGLObjects(curPosBuffer, null);
      //queue.enqueueReleaseGLObjects(curVelBuffer, null);
      queue.enqueueReleaseGLObjects(curPosBuffer);
      queue.enqueueReleaseGLObjects(curVelBuffer);
    }

    if (!userData.isGLCLshared || userData.drawMode === JS_DRAW_MODE) {
      queue.enqueueReadBuffer(curPosBuffer, true, 0, bufferSize, userData.curPos);
      queue.enqueueReadBuffer(curVelBuffer, true, 0, bufferSize, userData.curVel);
    }
  }
  catch (e)
  {
    console.error("Nbody Demo Failed ; Message: "+ e.message);
  }
}

function GetWorkGroupSize() {
  try {
    if (typeof(WebCL) === "undefined") {
      console.error("WebCL is yet to be defined");
      return null;
    }

    cl = new WebCL();

    if (cl === null) {
      console.error("Failed to create WebCL context");
      return;
    }

    // Select a compute device
    //
    platforms = cl.getPlatforms();

    if (platforms.length === 0) {
      console.error("No platforms available");
      return;
    }
    platform = platforms[0];

    // Select a compute device
    //
    devices = platform.getDevices(cl.DEVICE_TYPE_GPU);
    if (devices.length === 0) {
      console.error("No devices available");
      //return;
    }
    device = devices[0];

    // Create a compute context
    //
    context = cl.createContext({platform:platform,devices:devices,deviceType:cl.DEVICE_TYPE_GPU, shareGroup:1, hint:null});
    //context = cl.createSharedContext(cl.DEVICE_TYPE_GPU, null, null);

    // Create a command queue
    //
    queue = context.createCommandQueue(devices, null);

    // Create the compute program from the source buffer
    //
    var kernelSource = getKernel("nbody_kernel");
    if (kernelSource === null) {
      console.error("No kernel named: " + "nbody_kernel");
      return;
    }

    program = context.createProgram(kernelSource);

    // Build the program executable
    //
    program.build(devices);

    // Create the compute kernel in the program we wish to run
    //
    kernel = program.createKernel("nbody_kernel");

    // Get the maximum work group size for executing the kernel on the device
    //
    workGroupSize = kernel.getWorkGroupInfo(device, cl.KERNEL_WORK_GROUP_SIZE);
  }
  catch (e)
  {
    console.error("Nbody Demo Failed ; Message: "+ e.message);
  }

  return workGroupSize;
}
