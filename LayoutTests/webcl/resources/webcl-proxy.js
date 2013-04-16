
(function () {

    "use strict";

    var console = window.console || { log : function () {} };
    var nativeWebCL = webcl;

    if (!nativeWebCL) { return; }

    window.WebCLProxy = (function () {

        var API = {
            getWCLPlatform : function () {
                var nativePlatforms = nativeWebCL.getPlatforms();

                if (nativePlatforms) {
                    return new WCLPlatform(nativePlatforms);
                }

                return null;
            },

            createWCLContext : function (properties) {
                /* var nativePlatforms;
                var platform;
                var platformGroup;
                var device;
                var hint;
                */
                var deviceGroup;
                var contextDic;
                var contextGroup;
                var nativeContext;
                var nativeProperties;
                var platformGroup;
                var i;

                if (!properties) {
                    throw new Error("Properties is required in createWCLContext");
                }

                var platform = properties.platform;
                var device = properties.device;

                if (!platform) {
                    throw new Error("WCLPlatform is required in createWCLContext");
                }

                if (!device) {
                    throw new Error("WCLDevice is required in createWCLContext");
                }

                platformGroup = platform.getNativePlatforms();
                deviceGroup = device.getDeviceGroup();
                contextGroup = [];

                for (i in deviceGroup) {
                    nativeProperties = {};
                    nativeProperties.platform = deviceGroup[i].nativePlatform;
                    nativeProperties.devices = deviceGroup[i].nativeDevices;
                    nativeProperties.hint = properties.hint;

                    nativeContext = nativeWebCL.createContext(nativeProperties);

                    if (!(nativeContext instanceof WebCLContext)) {
                        throw (new Error("Could not create a native context"));
                    }

                    contextDic = {};
                    contextDic.nativePlatform = deviceGroup[i].nativePlatform;
                    contextDic.nativeDevices = deviceGroup[i].nativeDevices;
                    contextDic.nativeContext = nativeContext;
                    contextGroup.push(contextDic);
                }

                return new WCLContext(contextGroup);
            }
        };

        var p;
        for (p in nativeWebCL) {
            if (typeof nativeWebCL[p] !== 'function') {
                API[p] = nativeWebCL[p];
            }
        }

        return API;
    }());

    function WCLPlatform(nativePlatforms) {
        return {
            id : "WCLPlatform",

            getNativePlatforms : function () {
                return nativePlatforms;
            },

            getWCLInfo: function (name) {
                var nativeInfos = [], i;
                for (i in nativePlatforms) {
                    var nativePlatInfo = nativePlatforms[i].getInfo(name);

                    if (nativePlatInfo) {
                        nativeInfos.push(nativePlatInfo);
                    }

                }
                return nativeInfos;
            },

            getWCLDevice : function (deviceType) {
                var deviceDic;
                var deviceGroup;
                var nativeDevices;
                var nativePlatform;
                var types;
                var i;
                var j;
                var k;

                //FIXME: Use DEVICE_TYPE_ALL when it works fine
                deviceType = deviceType || nativeWebCL.DEVICE_TYPE_ALL;

                types = [nativeWebCL.DEVICE_TYPE_CPU, nativeWebCL.DEVICE_TYPE_GPU];
                deviceGroup = [];

                for (i in nativePlatforms) {
                    nativePlatform = nativePlatforms[i];
                    deviceDic = {};
                    deviceDic.nativePlatform = nativePlatform;
                    deviceDic.nativeDevices = [];

                    for (j in types) {
                        deviceType = types[j];
                        nativeDevices = nativePlatform.getDevices(deviceType);
                        for (k in nativeDevices) {
                            // used for DEVICE_TYPE_ALL
                            deviceDic.nativeDevices.push(nativeDevices[k]);
                        }
                    }

                    deviceGroup.push(deviceDic);
                }

                if (deviceGroup) {
                    return new WCLDevice(deviceGroup);
                }
                return null;
            }
        };
    }

    /* TODO:
    * @param {nativeDevices} [{platform: 'native platform', devices: 'platform devices'}, ...]
    */
    function WCLDevice(deviceGroup) {
        return {
            id : "WCLDevice",

            getDeviceGroup : function () {
                return deviceGroup;
            },

            getWCLInfo : function (info) {
                var nativeDevice;
                var nativeDevices;
                var nativeDevicesInfo;
                var deviceInfo;
                var i;
                var j;

                // TODO: Improve this function.
                nativeDevicesInfo = [];
                for (i in deviceGroup) {
                    nativeDevices = deviceGroup[i].nativeDevices;
                    for (j in nativeDevices) {
                        nativeDevice = nativeDevices[j];
                        deviceInfo = nativeDevice.getInfo(info);
                        if (deviceInfo) {
                            nativeDevicesInfo.push(deviceInfo);
                        }
                    }

                }
                return nativeDevicesInfo;
            }
        };
    }

    function WCLContext(contextGroup) {
        return {
            id : "WCLContext",

            getContextGroup : function () {
                return contextGroup;
            },

            createWCLBuffer : function (memFlags, sizeInBytes, hostPtr) {
                var nativeContext;
                var nativeBuffer;
                var nativeDevices;
                var bufferDic;
                var bufferGroup;
                var i;
                var j;

                bufferGroup = [];
                for (i in contextGroup) {
                    nativeContext = contextGroup[i].nativeContext;
                    nativeDevices = contextGroup[i].nativeDevices;

                    // create a WCLBuffer for each device associate to a context
                    for (j in nativeDevices) {
                        nativeBuffer = nativeContext.createBuffer(memFlags,
                                                                   sizeInBytes);
                        bufferDic = {};
                        bufferDic.nativePlatform = contextGroup[i].nativePlatform;
                        bufferDic.nativeDevices = [contextGroup[i].nativeDevices[j]];
                        bufferDic.nativeBuffer = nativeBuffer;
                        bufferGroup.push(bufferDic);
                    }
                }

                return new WCLBuffer(bufferGroup);
            },

            createWCLCommandQueue : function (wclDevice, properties) {
                var deviceGroup;
                var nativeDevice;
                var nativeDevices;
                var nativeContext;
                var queueDic;
                var queueGroup;
                var nativeCommandQueue;
                var i;
                var j;

                deviceGroup = wclDevice.getDeviceGroup();
                queueGroup = [];

                /* this loop can be seen as an iteration over each platform and
                 * the command queue should be created for all contexts whose
                 * devices points to the same platform as the contexts.
                 */
                for (i in deviceGroup) {
                    nativeContext = contextGroup[i].nativeContext;
                    nativeDevices = contextGroup[i].nativeDevices;

                    for (j in nativeDevices) {
                        queueDic = {};
                        nativeDevice = nativeDevices[j];
                        nativeCommandQueue = nativeContext.createCommandQueue(nativeDevice);

                        if (!(nativeCommandQueue instanceof WebCLCommandQueue)) {
                            throw (new Error("Could not create a native command queue"));
                        }
                        queueDic.device = nativeDevice;
                        queueDic.nativeCommandQueue = nativeCommandQueue;
                        queueGroup.push(queueDic);
                    }
                }

                return new WCLCommandQueue(queueGroup);
            },

            createWCLImage : function (memFlags, descriptor, hostPtr) {
                //TODO
                return "";
            },

            createWCLProgram : function (source) {
                var programDic;
                var programGroup;
                var nativeContext;
                var nativeProgram;
                var i;

                if (!source) {
                    throw (new Error("source param could not be empty."));
                }

                programGroup = [];
                for (i in contextGroup) {
                    programDic = {};
                    nativeContext = contextGroup[i].nativeContext;
                    nativeProgram = nativeContext.createProgram(source);

                    if (!(nativeProgram instanceof WebCLProgram)) {
                        throw (new Error("Could not create a native program"));
                    }

                    programDic.nativeProgram = nativeProgram;
                    programDic.nativeDevices = contextGroup[i].nativeDevices;
                    programGroup.push(programDic);
                }

                return new WCLProgram(programGroup);
            }

        };
    }

    /**
     * param {nativeProgram} {program: "WebCLProgram", device: "WebCLDevice"}
     */
    function WCLProgram(programGroup) {
        return {
            id : "WCLProgram",

            getNativePrograms : function () {
                return programGroup;
            },

            build : function () {
                var program;
                var devices;
                var i;

                try {
                    for (i in programGroup) {
                        program = programGroup[i].nativeProgram;
                        devices = programGroup[i].nativeDevices;
                        program.build(devices);
                    }
                } catch (e) {
                    var msg = program.getBuildInfo(devices, nativeWebCL.PROGRAM_BUILD_LOG);
                    throw (new Error(msg));
                }
            },

            createWCLKernel : function (kernelName) {
                var nativeDevice;
                var nativeDevices;
                var nativeKernel;
                var nativeProgram;
                var kernelDic;
                var kernelGroup;
                var i;
                var j;

                kernelGroup = [];
                for (i in programGroup) {
                    nativeProgram = programGroup[i].nativeProgram;
                    nativeDevices = programGroup[i].nativeDevices;

                    for (j in nativeDevices) {
                        kernelDic = {};
                        nativeDevice = nativeDevices[j];
                        nativeKernel = nativeProgram.createKernel(kernelName);

                        if (!(nativeKernel instanceof WebCLKernel)) {
                            throw (new Error("Could not create a native kernel"));
                        }

                        kernelDic.nativeKernel = nativeKernel;
                        kernelDic.nativeDevices = [nativeDevice];
                        kernelGroup.push(kernelDic);
                    }
                }

                return new WCLKernel(kernelGroup);
            }

        };
    }

    function WCLBuffer(bufferGroup) {
        return {
            id : "WCLBuffer",

            getBufferGroup : function () {
                return bufferGroup;
            }
        };
    }

    function WCLKernel(kernelGroup) {

        return {
            id : "WCLKernel",

            getKernelGroup : function () {
                return kernelGroup;
            },

            getWCLInfo : function (info) {
                var nativeKernel;
                var nativeDevicesInfo = [];
                var i;

                for (i in kernelGroup) {
                    nativeKernel = kernelGroup[i].nativeKernel;
                    nativeDevicesInfo.push(nativeKernel.getInfo(info));
                }

                return nativeDevicesInfo;
            },

            getWorkGroupInfo: function (wclDevice, info) {
                var i;
                var j;
                var nativeKernel;
                var nativeDevice;
                var nativeDevices;
                var res;
                var groupInfo;

                if (!(wclDevice.id === "WCLDevice")) {
                    throw new Error("WCLDevice expected");
                }

                //FIXME: Info must be a WebCLEnum KERNEL_WORK_GROUP_SIZE, KERNEL_COMPILE_WORK_GROUP_SIZE,
                //KERNEL_LOCAL_MEM_SIZE, KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, KERNEL_PRIVATE_MEM_SIZE
                //A method to validate param info will be created in utils file
                if (!info) {
                    throw new Error("WebCLEnum expected");
                }

                groupInfo = [];
                for (i in kernelGroup) {
                    nativeKernel = kernelGroup[i].nativeKernel;
                    nativeDevices = kernelGroup[i].nativeDevices;
                    for (j in nativeDevices) {
                        nativeDevice = nativeDevices[j];
                        res = nativeKernel.getWorkGroupInfo(nativeDevice, info);
                        groupInfo.push(res);
                    }
                }

                return groupInfo;
            },

            release: function () {
                var i;
                for (i in kernelGroup) {
                    kernelGroup[i].release();
                }

            },

            /*
            setWCLKernelArgs : function (index, wclBuffer) {
                var wclBuffers, nativeKernel, auxArg, i;
                var wclKernelGroup = this.getKernelGroup();

                for (i in wclKernelGroup) {

                    nativeKernel = wclKernelGroup[i].nativeKernel;
                    wclBuffers = wclBuffer.getBufferGroup();
                    auxArg = wclBuffers[i].nativeBuffer;

                    try {
                        nativeKernel.setArg(index, auxArg);
                    } catch (e) {
                        throw e;
                    }
                }
            }
*/

            /**
             * @param {index} Integer
             * @param {arg} WCLBuffer or any
             * @param {type} WebCL.ENUM, required when arg is not a WCLBuffer
             */
            setWCLKernelArgs : function (index, arg, type) {
                var i, wclBuffers = [], wclKernelGroup, nativeKernel;
                var argType = (arg.id && arg.id === "WCLBuffer") ?  "buffer" : "scalar";

                wclKernelGroup = this.getKernelGroup();

                for (i in wclKernelGroup) {

                    nativeKernel = wclKernelGroup[i].nativeKernel;

                    try {
                        if (argType === "buffer") {
                            wclBuffers = arg.getBufferGroup();
                            nativeKernel.setArg(index, wclBuffers[i].nativeBuffer);
                        } else {
                            nativeKernel.setArg(index, arg, type);
                        }
                    } catch (e) {
                        throw e;
                    }
                }
            }
        };
    }

    function WCLCommandQueue(queueGroup) {
        var nativeQueues;
        return {
            id : "WCLCommandQueue",

            getNativeCommandQueue : function () {
                return nativeQueues;
            },

            enqueueReadBuffer : function (args) {
                var nativeCommandQueue;
                var nativeBuffer;
                var bufferGroup;
                var bufferDic;
                var lastBufferDic;
                var block;
                var offset;
                var numBytes;
                var data;
                var lastData;
                var eventsList;
                var readEvent;
                var msg;
                var i;
                var j;

                try {
                    if (!(args.buffer.id === "WCLBuffer")) {
                        throw new Error("No WebCLBuffer in the arguments.");
                    }

                    if (!args.blockingRead) {
                        throw new Error("No block in the arguments.");
                    }

                    if (!args.bufferOffset && args.bufferOffset !== 0) {
                        throw new Error("No offset in the arguments.");
                    }

                    if (!args.numBytes) {
                        throw new Error("No numBytes in the arguments.");
                    }

                    if (!args.data) {
                        throw new Error("No numBytes in the arguments.");
                    }

                    // FIXME: use this when the WCEvent is working
                    /*if (args["eventWaitList"] instanceof Array
                            && !(args["eventWaitList"][0] === "WCLEvent"))
                        throw new Error("No eventWaitList in the arguments.");

                    if (!(args['event'].id === "WCLEvent"))
                        throw new Error("No event in the arguments.");*/
                } catch (e) {
                    throw e;
                }

                bufferGroup = args.buffer.getBufferGroup();
                block = args.blockingRead;
                offset = args.bufferOffset;
                numBytes = args.numBytes;
                data = args.data;
                //eventsList = args["eventWaitList"];
                //readEvent = args["event"];

                lastData = null;
                for (i in queueGroup) {
                    nativeCommandQueue = queueGroup[i].nativeCommandQueue;
                    bufferDic = bufferGroup[i];
                    nativeBuffer = bufferDic.nativeBuffer;
                    nativeCommandQueue.enqueueReadBuffer(nativeBuffer, block,
                                                        offset, numBytes, data);
                    // check if the results are the same
                    if (lastData !== null) {
                        for (j = 0; j < data.length; j++) {
                            if (data[j] !== lastData[j]) {
                                msg = "(Platform " + bufferDic.nativePlatform + " ";
                                msg += "Device " + bufferDic.nativeDevices + " ";
                                msg += "Value " + data[j] + ") != ";
                                msg += "(Platform " + lastBufferDic.nativePlatform + " ";
                                msg += "Device " + lastBufferDic.nativeDevices + " ";
                                msg += "Value " + lastData[j] + ")";
                                throw (new Error(msg));
                            }
                        }
                    } else {
                        lastData = new Int32Array(data.length);
                    }

                    // copy data
                    for (j in data) {
                        lastData[j] = data[j];
                    }
                    lastBufferDic = bufferDic;
                }
            },

            enqueueWriteBuffer : function (args) {

                var nativeCommandQueue;
                var nativeBuffer;
                var bufferGroup;
                var bufferDic;
                var block;
                var offset;
                var numBytes;
                var data;
                var i;
                var eventsList;
                var readEvent;

                try {
                    if (!args.buffer || !(args.buffer.id === "WCLBuffer")) {
                        throw new Error("No WebCLBuffer in the arguments.");
                    }

                    if (args.blockingWrite === undefined) {
                        throw new Error("No block in the arguments.");
                    }

                    if (!args.bufferOffset && args.bufferOffset !== 0) {
                        throw new Error("No offset in the arguments.");
                    }

                    if (!args.numBytes) {
                        throw new Error("No numBytes in the arguments.");
                    }

                    //FIXME: Is really necessary check data ???
                    /*
                    if (!args["data"]) {
                        throw new Error("No numBytes in the arguments.");
                    }*/

                    // FIXME: use this when the WCEvent is working
                    /*if (args["eventWaitList"] instanceof Array
                            && !(args["eventWaitList"][0] === "WCLEvent"))
                        throw new Error("No eventWaitList in the arguments.");

                    if (!(args['event'].id === "WCLEvent"))
                        throw new Error("No event in the arguments.");*/
                } catch (e) {
                    throw e;
                }

                bufferGroup = args.buffer.getBufferGroup();
                block = args.blockkingWrite;
                offset = args.bufferOffset;
                numBytes = args.numBytes;
                data = args.data;
                //eventsList = args["eventWaitList"];
                //writeEvent = args["event"];

                if (data) {
                    for (i in queueGroup) {
                        bufferDic = bufferGroup[i];
                        nativeBuffer = bufferDic.nativeBuffer;
                        nativeCommandQueue = queueGroup[i].nativeCommandQueue;
                        nativeCommandQueue.enqueueWriteBuffer(nativeBuffer, block,
                                                            offset, numBytes, data);
                    }
                }
            },

            enqueueCopyBuffer : function (args) {
                var nativeCommandQueue;
                var srcBufferDic;
                var dstBufferDic;
                var srcNativeBuffer;
                var dstNativeBuffer;
                var srcOffset;
                var dstOffset;
                var numBytes;
                var i;
                var eventsList;
                var copyEvent;

                console.log("enqueueCopyBuffer", args);

                try {
                    if (!(args.srcBuffer.id === "WCLBuffer")) {
                        throw new Error("No srcBuffer in the arguments.");
                    }

                    if (!(args.dstBuffer.id === "WCLBuffer")) {
                        throw new Error("No dstsBuffer in the arguments.");
                    }

                    if (!args.srcOffset && args.srcOffset !== 0) {
                        throw new Error("No srcOffset in the arguments.");
                    }

                    if (!args.dstOffset && args.dstOffset !== 0) {
                        throw new Error("No dstOffset in the arguments.");
                    }

                    if (!args.numBytes) {
                        throw new Error("No numBytes in the arguments.");
                    }

                    // FIXME: use this when the WCEvent is working
                    /*if (args["eventWaitList"] instanceof Array
                            && !(args["eventWaitList"][0] === "WCLEvent"))
                        throw new Error("No eventWaitList in the arguments.");

                    if (!(args['event'].id === "WCLEvent"))
                        throw new Error("No event in the arguments.");*/
                } catch (e) {
                    throw e;
                }

                srcBufferDic = args.srcBuffer.getBufferGroup();
                dstBufferDic = args.dstBuffer.getBufferGroup();
                srcOffset = args.srcOffset;
                dstOffset = args.dstOffset;
                numBytes = args.numBytes;
                //eventsList = args["eventWaitList"];
                //copyEvent = args["event"];

                for (i in queueGroup) {
                    srcNativeBuffer = srcBufferDic[i].nativeBuffer;
                    dstNativeBuffer = dstBufferDic[i].nativeBuffer;
                    nativeCommandQueue = queueGroup[i].nativeCommandQueue;
                    nativeCommandQueue.enqueueCopyBuffer(srcNativeBuffer,
                                                    dstNativeBuffer, srcOffset,
                                                    dstOffset, numBytes);
                }
            },

            enqueueNDRangeKernel : function (args) {
                var nativeCommandQueue;
                var nativeKernel;
                var nativeKernels;
                var offset;
                var globalSize;
                var localSize;
                var eventList;
                var rangeEvent;
                var wclKernel;
                var kernelGroup;
                var i;
                var j;

                try {
                    if (args.kernel.id !== "WCLKernel") {
                        throw new Error("No kernel in the arguments.");
                    }

                    if (!(args.globalWorkOffset instanceof Int32Array)) {
                        throw new Error("No offset in the arguments.");
                    }

                    if (!(args.globalWorkSize instanceof Int32Array)) {
                        throw new Error("No globalWorkSize in the arguments.");
                    }

                    if (!(args.localWorkSize instanceof Int32Array)) {
                        throw new Error("No globalWorkSize in the arguments.");
                    }

                    // FIXME: use this when the WCEvent is working
                    /*if (args["eventWaitList"] instanceof Array
                            && !(args["eventWaitList"][0] === "WCLEvent"))
                        throw new Error("No eventWaitList in the arguments.");

                    if (!(args['event'].id === "WCLEvent"))
                        throw new Error("No event in the arguments.");*/
                } catch (e) {
                    throw e;
                }

                wclKernel = args.kernel;
                kernelGroup = wclKernel.getKernelGroup();
                offset = args.globalWorkOffset;
                globalSize = args.globalWorkSize;
                localSize = args.localWorkSize;
                //eventsList = args["eventWaitList"];
                //rangeEvent = args["event"];

                for (i in kernelGroup) {
                    nativeKernel = kernelGroup[i].nativeKernel;
                    for (j in queueGroup) {
                        nativeCommandQueue = queueGroup[j].nativeCommandQueue;
                        nativeCommandQueue.enqueueNDRangeKernel(nativeKernel, offset,
                                globalSize, localSize);
                    }
                }
            },

            finish : function () {
                var nativeCommandQueue;
                var i;

                for (i in queueGroup) {
                    nativeCommandQueue = queueGroup[i].nativeCommandQueue;
                    nativeCommandQueue.finish();
                }
            }
        };
    }

    function WCLSampler() {
        return {
            id : "WCLSampler"
        };
    }

    function WCLEvent() {
        return {
            id : "WCLEvent"
        };
    }

}());

// TEST WEBCLPROXY

(function (command) {

    "use strict";

    if (command === 'RUN') {

        var dummyKernel = "kernel void myDummyKernel(global int *in, global int *out) { int i = get_global_id(0); out[i] = in[i];}";

        var enumWGI = [webcl.KERNEL_WORK_GROUP_SIZE, webcl.KERNEL_COMPILE_WORK_GROUP_SIZE, webcl.KERNEL_LOCAL_MEM_SIZE,
                       webcl.KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, webcl.KERNEL_PRIVATE_MEM_SIZE];
        console.log("Initialization done");

        var cl = WebCLProxy;
        console.log(cl);

        var plat = cl.getWCLPlatform();
        console.log("WCLPlatform: ", plat);

        // Context creation

        plat.version = plat.getWCLInfo(cl.PLATFORM_VERSION);
        plat.dev = plat.getWCLDevice();
        console.log("WCLDevice: ", plat.dev);

        //Get devices
        console.log("Native devices: ", plat.dev.getDeviceGroup());

        // Get devices info (DEVICE_TYPE)
        plat.dev.info = plat.dev.getWCLInfo(cl.DEVICE_TYPE);
        console.log("WCLDevice Info: ", plat.dev.info);

        var ctxProperties = {platform: plat, device: plat.dev, hint: null};
        console.log(ctxProperties);

        plat.ctx = cl.createWCLContext(ctxProperties);
        console.log("WCLContext:", plat.ctx);

        var queue = plat.ctx.createWCLCommandQueue(plat.dev, null);
        console.log("WCLQueue: ", queue);

        console.log("Created context for platform", plat, ": ", plat.version);

        // Initializing Data
        var DATA_SIZE = 10;
        var data = new Int32Array(DATA_SIZE);
        var result = new Int32Array(DATA_SIZE);
        var i;
        for (i = 0; i < DATA_SIZE; i++) {
            data[i] = i;
        }

        /* Old way to set kernel args
         * --------------------------
        var bufferIn = plat.ctx.createWCLBuffer(webcl.MEM_WRITE_ONLY,
                                            Int32Array.BYTES_PER_ELEMENT * DATA_SIZE);
        var bufferOut = plat.ctx.createWCLBuffer(webcl.MEM_WRITE_ONLY,
                                            Int32Array.BYTES_PER_ELEMENT * DATA_SIZE);
        console.log("WCLBuffer: ", bufferIn);

        var args = {"buffer": bufferIn,
                    "blockingWrite": true,
                    "bufferOffset": 0,
                    "numBytes": Int32Array.BYTES_PER_ELEMENT * DATA_SIZE,
                    "data": data};
        queue.enqueueWriteBuffer(args);
        console.log("WCLCommandQueue: write buffer");
        */

        /* Testing enqueueCopyBuffer */
        var bufferIn = plat.ctx.createWCLBuffer(webcl.MEM_WRITE_ONLY,
                Int32Array.BYTES_PER_ELEMENT * DATA_SIZE);

        var bufferCopy = plat.ctx.createWCLBuffer(webcl.MEM_WRITE_ONLY,
                Int32Array.BYTES_PER_ELEMENT * DATA_SIZE);

        var args = {"srcBuffer": bufferIn,
                "dstBuffer": bufferCopy,
                "srcOffset": 0,
                "dstOffset": 0,
                "numBytes": Int32Array.BYTES_PER_ELEMENT * DATA_SIZE};
        queue.enqueueCopyBuffer(args);
        console.log("WCLCommandQueue: copy buffer");

        // Kernel creation
        try {
            plat.ctx.prog = plat.ctx.createWCLProgram(dummyKernel);
            console.log("WCLProgram: ", plat.ctx.prog);

            plat.ctx.prog.build();
            plat.ctx.kernel = plat.ctx.prog.createWCLKernel("myDummyKernel");
            console.log("WCLKernel: ", plat.ctx.kernel);
/*
            plat.ctx.kernel.setWCLKernelArgs(0, bufferIn);
            plat.ctx.kernel.setWCLKernelArgs(1, bufferOut);
            console.log("Set Args: ok");
*/

            var wclBufferIn = new WebCLTestUtils.WCLBufferArg("bufferIn",
                    Int32Array.BYTES_PER_ELEMENT * DATA_SIZE, data);

            var wclBufferOut = new WebCLTestUtils.WCLBufferArg("bufferOut",
                    Int32Array.BYTES_PER_ELEMENT * DATA_SIZE, result);

            WebCLTestUtils.setWCLKernelArgs([wclBufferIn, wclBufferOut],
                    plat.ctx, queue, plat.ctx.kernel);

            var globalWorkOffset = new Int32Array(1);
            var globalWorkSize = new Int32Array(1);
            var localWorkSize = new Int32Array(1);
            globalWorkOffset[0] = 0;
            globalWorkSize[0] = 10;
            localWorkSize[0] = 2;

            var kernelArgs = {"kernel": plat.ctx.kernel,
                    "globalWorkOffset": globalWorkOffset,
                    "globalWorkSize": globalWorkSize,
                    "localWorkSize": localWorkSize};
            queue.enqueueNDRangeKernel(kernelArgs);
            queue.finish();
            console.log("WCLCommandQueue: run and finish");

            /* Old way
             * --------------------------
            args = {"buffer": bufferOut,
                    "blockingRead": true,
                    "bufferOffset": 0,
                    "numBytes": Int32Array.BYTES_PER_ELEMENT * DATA_SIZE,
                    "data": result};
            queue.enqueueReadBuffer(args);
            */
            WebCLTestUtils.readWCLBuffers([wclBufferOut], queue);

            console.log("WCLCommandQueue: read buffer");
            console.log("result: ", result);

            console.log("Kernel Work Group Info:");

            var k;
            for (k = 0; k < enumWGI.length; k++) {
                console.log("WorkGroupInfo: ", plat.ctx.kernel.getWorkGroupInfo(plat.dev, enumWGI[k]));
            }
        } catch (e) {
            console.log("Kernel creation failed for platform", plat, ": ", e);
        }
    }
}('RUN'));
