# WebCL for WebKit

This project aims to implement the [WebCL](http://www.khronos.org/webcl/) support in [WebKit](http://www.webkit.org/) web browser engine.

## Build Instructions

### OSX

_Note: [Here](http://www.webkit.org/building/tools.html) you will find how to install the WebKit developer tools._

The code is being synchronised with WebKit periodically. For this reason, the branch might be changed before the build. Our actual stable branch is **master_r152423**.

After change the branch, you can run the script: `<src-dir>/Tools/Scripts/build-webkit`

We have some WebCL examples in `<src-dir>/Examples/WebCL/`. To run the _Hello_ example, use: `<src-dir>/Tools/Scripts/run-safari <src-dir>/Examples/WebCL/Hello/index.html` 

### Linux (EFL port)

_Note: The build steps below were tested using Ubuntu 12.04, 13.04 and NVIDIA graphic cards._

Manually install OpenCL headers package.

    sudo apt-get install opencl-headers

Follow WebKit-EFL build steps described [here](http://trac.webkit.org/wiki/EFLWebKit).

After the build process has been finished, run some of our WebCL example (`<src_dir>/Examples/WebCL`) using _MiniBrowser_:

    <src_dir>/WebKitBuild/Release/bin/MiniBrowser <src_dir>/Examples/WebCL/Hello/index.html

## Supported Configurations

**Mac**

  - OSX: 10.8.3
  - XCode: 4.6.1 (4H512)

**Linux (EFL port)**

 - Ubuntu: 12.04 and 13.04
 - OpenCL Headers: 1.1-2010

## Links
- [WebCL](http://www.khronos.org/webcl/)
- [OpenCL](http://www.khronos.org/opencl/)
- [WebGL](http://www.khronos.org/registry/webgl/specs/latest/)
- [Demo Videos](http://www.youtube.com/user/SamsungSISA)
