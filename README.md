# WebCL for WebKit

This project aims to implement the [WebCL](http://www.khronos.org/webcl/) support in [WebKit](http://www.webkit.org/) web browser engine.

## Build Instructions

### OSX

_Note: [Here](http://www.webkit.org/building/tools.html) you will find how to install the WebKit developer tools._

The code is being synchronised with WebKit periodically. For this reason, the branch might be changed before the build. Our actual stable branch is **master_r144637**.

After change the branch, you can run the script: `<src-dir>/Tools/Scripts/build-webkit`

We have some WebCL examples in `<src-dir>/Examples/WebCL/`. To run the _Hello_ example, use: `<src-dir>/Tools/Scripts/run-safari <src-dir>/Examples/WebCL/Hello/index.html` 

### Linux (EFL port)

_Currently in development._

## Supported Configurations

**Mac**

  - OSX: 10.8.3
  - XCode: 4.6.1 (4H512)

**Linux (EFL port)**

_Currently in development._

## Links
- [WebCL](http://www.khronos.org/webcl/)
- [OpenCL](http://www.khronos.org/opencl/)
- [WebGL](http://www.khronos.org/registry/webgl/specs/latest/)
- [Demo Videos](http://www.youtube.com/user/SamsungSISA)
