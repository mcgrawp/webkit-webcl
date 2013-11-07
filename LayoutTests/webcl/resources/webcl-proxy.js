/**
 *   WebCLProxy:
 *
 *            -------------
 *           |             |
 *           | WebCL Tests |
 *           |             |
 *            -------------
 *                  |
 *            -------------
 *           |             |
 *           | WebCLProxy  |
 *           |             |
 *            -------------
 *                  |
 *            -------------
 *           |             |
 *           |    WebCL    |
 *           |             |
 *            -------------
 */

(function () {

    "use strict";

    /* Only errors log */
    if (window.LOGGER) {
        window.LOGGER.setLogLevel(0);
    }

    console.time("Init");

    var nativeWebCL = webcl;

    if (!nativeWebCL) {
        throw new Error("No WebCL detected.");
    }

    /**
     * @module WebCLProxy
     *
     * It is a framework helpful to run a single kernel in all devices
     * available for each platform.
     */
    window.WebCLProxy = (function () {

        var API = {

            /**
             * Returns WCLPlatform instance.
             *
             * @name getWCLPlatform
             * @function
             * @returns {WCLPlatform} Instance of WCLPlatform.
             */
            getWCLPlatform : function () {

                console.group("WebCLProxy::getWCLPlatform");
                console.time("getWCLPlatform");

                var nativePlatforms = nativeWebCL.getPlatforms();

                console.info("nativePlatfforms", nativePlatforms);
                console.timeEnd("getWCLPlatform");
                console.groupEnd();

                if (nativePlatforms) {
                    return new WCLPlatform(nativePlatforms);
                }

                return null;
            },

            /**
             * Returns all WebCL supported extensions
             */
            getSupportedExtensions : function () {
                return nativeWebCL.getSupportedExtensions();
            },

            /**
             * Returns extension by name
             *
             * @param {name} Extension's name
             */
            getExtension : function (name) {
                return nativeWebCL.getExtension(name);
            }
        };

        /**
         * "Import" all constants from webcl
         */
        var constants;
        for (constants in nativeWebCL) {
            if (typeof nativeWebCL[constants] !== 'function') {
                API[constants] = nativeWebCL[constants];
            }
        }

        console.info("WebCLProxy", API);
        console.timeEnd("Init");

        return API;
    }());

    /**
     * Check if {wclObject} is a instance of {type}(WCL class)
     *
     * @name isWCLType
     * @function
     * @param {wclObject} The object to be tested.
     * @param {type} String that represents the type that the object should be.
     * @returns {boolean} true if {wclObject} is instance of {type}
     *                    otherwise returns false.
     */
    function isWCLType(wclObject, type) {
        return wclObject && wclObject.id && wclObject.id  === type;
    }

    /**
     * Represents a list of native platforms available from WebCL module.
     *
     * @name WCLPlatform
     * @class
     */

    /**
     * Creates a new WCLPlatform instance.
     *
     * @name WCLPlatform
     * @constructor
     * @param {nativePlatforms} List of native platforms taken from WebCL.
     */
    function WCLPlatform(nativePlatforms) {

        return {
            id : "WCLPlatform",

            getNativePlatforms : function () {
                return nativePlatforms;
            },

            /**
             * TODO: Function description.
             *
             * @name getWCLInfo
             * @function
             * @param
             * @returns
             */
            getWCLInfo: function (name) {
                var nativeInfos = [], i;

                console.group("WCLPlatform::getWCLInfo");
                console.time("WCLPlatform::getWCLInfo");

                for (i in nativePlatforms) {
                    var nativePlatInfo = nativePlatforms[i].getInfo(name);

                    if (nativePlatInfo) {
                        nativeInfos.push(nativePlatInfo);
                    }

                }

                console.info("nativeInfo: ", nativeInfos);
                console.timeEnd("WCLPlatform::getWCLInfo");
                console.groupEnd();

                return nativeInfos;
            },

            /**
             * TODO: Function description.
             *
             * @name getWCLDevice
             * @function
             * @param
             * @returns
             */
            getWCLDevice : function (deviceType) {
                var deviceDic;
                var deviceGroup;
                var nativeDevices;
                var nativePlatform;
                var types;
                var i;
                var j;

                console.group("WCLPlatform::getWCLDevice");
                console.time("WCLPlatform::getWCLDevice");

                //FIXME: Use DEVICE_TYPE_ALL when it has been working
                deviceType = deviceType || nativeWebCL.DEVICE_TYPE_ALL;

                console.info("deviceType", deviceType);

                switch (deviceType) {
                case nativeWebCL.DEVICE_TYPE_CPU:
                    types = [deviceType];
                    break;

                case nativeWebCL.DEVICE_TYPE_GPU:
                    types = [deviceType];
                    break;

                case nativeWebCL.DEVICE_TYPE_DEFAULT:
                    types = [deviceType];
                    break;

                default:
                    types = [nativeWebCL.DEVICE_TYPE_CPU, nativeWebCL.DEVICE_TYPE_GPU];
                }

                deviceGroup = [];

                console.group("nativePlatforms");

                for (i in nativePlatforms) {
                    nativePlatform = nativePlatforms[i];
                    deviceDic = {};
                    deviceDic.nativePlatform = nativePlatform;
                    deviceDic.nativeDevices = [];

                    console.info("deviceDic", deviceDic);

                    // remove this "for" when DEVICE_TYPE_ALL has been working
                    for (j in types) {
                        console.info("types", types[j]);

                        deviceType = types[j];
                        nativeDevices = nativePlatform.getDevices(deviceType);

                        console.info("nativeDevice", nativeDevices);
                        deviceDic.nativeDevices.push(nativeDevices[0]);
                    }

                    deviceGroup.push(deviceDic);
                }

                console.groupEnd();

                console.timeEnd("WCLPlatform::getWCLDevice");
                console.groupEnd();

                if (deviceGroup) {
                    return new WCLDevice(deviceGroup);
                }

                return null;
            },

            /**
             * TODO: Function description.
             *
             * @name createWCLContext
             * @function
             * @param
             * @returns
             */
            createWCLContext : function (wclDevice, interop) {
                var deviceGroup;
                var contextDic;
                var contextGroup;
                var nativeExtension;
                var nativeContext;
                var nativeProperties;
                var i;
                var ex;

                console.group("WCLPlatform::createWCLContext");
                console.time("WCLPlatform::createWCLContext");

                if (!wclDevice) {
                    wclDevice = this.getWCLDevice();
                }

                if (interop) {
                    nativeExtension = nativeWebCL.getExtension("KHR_GL_SHARING");
                    if (!nativeExtension) {
                        throw new Error("GL Extension is not supported");
                    }
                }

                console.info("wcDevice: ", wclDevice);

                if (!isWCLType(wclDevice, "WCLDevice")) {
                    ex = new Error("WCLDevice is required in createWCLContext");
                    console.error("WCLPlatform::createWCLContext", ex);
                    throw ex;
                }

                deviceGroup = wclDevice.getDeviceGroup();
                contextGroup = [];

                console.group("deviceGroup");

                for (i in deviceGroup) {
                    nativeProperties = {};
                    nativeProperties.platform = deviceGroup[i].nativePlatform;
                    nativeProperties.devices = deviceGroup[i].nativeDevices;
                    nativeProperties.deviceType = nativeWebCL.DEVICE_TYPE_DEFAULT;

                    console.info("nativeProperties: ", nativeProperties);

                    try {
                        if (!interop) {
                            nativeContext = nativeWebCL.createContext(nativeProperties);
                        } else {
                                nativeContext = nativeExtension.createContext(nativeProperties);
                        }
                    } catch(e) {
                        throw new Error("Create Context Exception; Message: ", e.message);
                    }

                    console.info("nativeContext: ", nativeContext);

                    if (!(nativeContext instanceof WebCLContext)) {
                        throw (new Error("Could not create a native context"));
                    }

                    contextDic = {};
                    contextDic.nativePlatform = deviceGroup[i].nativePlatform;
                    contextDic.nativeDevices = deviceGroup[i].nativeDevices;
                    contextDic.nativeContext = nativeContext;
                    contextGroup.push(contextDic);
                }

                console.groupEnd();
                console.timeEnd("WCLPlatform::createWCLContext");
                console.groupEnd();

                return new WCLContext(contextGroup);
            }
        };
    }

    /**
     * Represents a list of devices available for each native platform from WebCL module.
     *
     * @name WCLDevice
     * @class
     */

    /**
     * Creates a new WCLDevice instance.
     *
     * @name WCLDevice
     * @constructor
     * @param {nativeDevices} List of all native platforms and their devices
     *        in a dictionary format. For example:
     *        [{nativePlatform: 'native platform', nativeDevices: 'platform devices'}, ...]
     */
    function WCLDevice(deviceGroup) {
        return {
            id : "WCLDevice",

            /**
             * TODO: Function description.
             *
             * @name getDeviceGroup
             * @function
             * @param
             * @returns
             */
            getDeviceGroup : function () {
                return deviceGroup;
            },

            /**
             * TODO: Function description.
             *
             * @name getWCLInfo
             * @function
             * @param
             * @returns
             */
            getWCLInfo : function (info) {
                var nativeDevice;
                var nativeDevices;
                var nativeDevicesInfo;
                var deviceInfo;
                var i;
                var j;

                console.group("WCLDevice::getWCLInfo");
                console.time("WCLDevice::getWCLInfo");

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

                console.info("nativeDevicesInfo: ", nativeDevicesInfo);
                console.timeEnd("WCLDevice::getWCLInfo");
                console.groupEnd();

                return nativeDevicesInfo;
            }
        };
    }

    /**
     * Represents a list of native contexts. For each native platform and its devices
     * is created a native context to form that list.
     *
     * @name WCLContext
     * @class
     */

    /**
     * Creates a new WCLContext instance.
     *
     * @name WCLContext
     * @constructor
     * @param {contextGroup} List of native contexts with their platforms
     *                       and their devices in a dictionary format.
              For example:
     *        [{nativeContext: 'native context', nativePlatform: 'context platform',
     *          nativeDevices: 'platform devices'}, ...]
     */
    function WCLContext(contextGroup) {
        return {
            id : "WCLContext",

            /**
             * TODO: Function description.
             *
             * @name getContextGroup
             * @function
             * @param
             * @returns
             */
            getContextGroup : function () {
                return contextGroup;
            },

            /**
             * TODO: Function description.
             *
             * @name createWCLBuffer
             * @function
             * @param
             * @returns
             */
            createWCLBuffer : function (memFlags, sizeInBytes, hostPtr) {
                var nativeContext;
                var nativeBuffer;
                var nativeDevices;
                var bufferDic;
                var bufferGroup;
                var i;
                var j;

                console.group("WCLContext::createWCLBuffer");
                console.time("WCLContext::createWCLBuffer");

                console.group("contextGroup");

                bufferGroup = [];
                for (i in contextGroup) {
                    nativeContext = contextGroup[i].nativeContext;
                    nativeDevices = contextGroup[i].nativeDevices;

                    // create a WCLBuffer for each device associate to a context
                    for (j in nativeDevices) {
                        nativeBuffer = nativeContext.createBuffer(memFlags,
                                                                   sizeInBytes);

                        console.info("memFlags: ", memFlags, "sizeInBytes: ", sizeInBytes);
                        console.log("Created nativeBuffer: ", nativeBuffer);

                        bufferDic = {};
                        bufferDic.nativePlatform = contextGroup[i].nativePlatform;
                        bufferDic.nativeDevices = [contextGroup[i].nativeDevices[j]];
                        bufferDic.nativeBuffer = nativeBuffer;
                        bufferGroup.push(bufferDic);
                    }

                    console.info("bufferDic: ", bufferDic);
                }

                console.groupEnd();
                console.info("bufferGroup: ", bufferDic);
                console.timeEnd("WCLContext::createWCLBuffer");
                console.groupEnd();

                return new WCLBuffer(bufferGroup, "buffer");
            },

            /**
             * TODO: Function description.
             *
             * @name createWCLCommandQueue
             * @function
             * @param
             * @returns
             */
            createWCLCommandQueue : function () {
                var nativeDevice;
                var nativeDevices;
                var nativeContext;
                var queueDic;
                var queueGroup;
                var nativeCommandQueue;
                var i;
                var j;
                var ex;

                queueGroup = [];

                console.group("WCLContext::createWCLCommandQueue");
                console.time("WCLContext::createWCLCommandQueue");

                console.group("contextGroup");

                /* this loop could be seen as an iteration over each platform and
                 * the command queue should be created for all contexts whose
                 * devices points to the same platform as the contexts.
                 */
                for (i in contextGroup) {

                    nativeContext = contextGroup[i].nativeContext;
                    nativeDevices = contextGroup[i].nativeDevices;

                    console.info("nativeContext: ", nativeContext);

                    console.group("nativeDevices");

                    for (j in nativeDevices) {
                        queueDic = {};
                        nativeDevice = nativeDevices[j];

                        console.info("nativeDevice: ", nativeDevices[j]);

                        nativeCommandQueue = nativeContext.createCommandQueue(nativeDevice);

                        console.log("Created nativeCommandoQueue: ", nativeCommandQueue);

                        if (!(nativeCommandQueue instanceof WebCLCommandQueue)) {
                            ex =  new Error("Could not create a native command queue");
                            console.error("createCommandQueue", ex);
                            throw ex;
                        }
                        queueDic.device = nativeDevice;
                        queueDic.nativeCommandQueue = nativeCommandQueue;
                        queueGroup.push(queueDic);
                    }

                    console.groupEnd();
                }

                console.groupEnd();
                console.timeEnd("WCLContext::createWCLCommandQueue");
                console.groupEnd();

                return new WCLCommandQueue(queueGroup);
            },

            /**
             * TODO: Function description.
             *
             * @name createWCLImage
             * @function
             * @param
             * @returns
             */
            createWCLImage : function (memFlags, descriptor, hostPtr) {
                //TODO
                return "";
            },

            /**
             * TODO: Function description.
             *
             * @name createWCLProgram
             * @function
             * @param
             * @returns
             */
            createWCLProgram : function (source) {
                var programDic;
                var programGroup;
                var nativeContext;
                var nativeProgram;
                var i;

                console.group("WCLContext::createWCLCProgram");
                console.time("WCLContext::createWCLCProgram");

                console.info("source: ", source);

                if (!source) {
                    throw (new Error("source param could not be empty."));
                }

                console.group("contextGroup");

                programGroup = [];
                for (i in contextGroup) {
                    programDic = {};
                    nativeContext = contextGroup[i].nativeContext;
                    console.info("nativeContext: ", nativeContext);

                    nativeProgram = nativeContext.createProgram(source);

                    if (!(nativeProgram instanceof WebCLProgram)) {
                        throw (new Error("Could not create a native program"));
                    }

                    console.log("Created nativeProgram: ", nativeProgram);

                    programDic.nativeProgram = nativeProgram;
                    programDic.nativeDevices = contextGroup[i].nativeDevices;
                    programGroup.push(programDic);
                }

                console.groupEnd();

                console.timeEnd("WCLContext::createWCLCProgram");
                console.groupEnd();

                return new WCLProgram(programGroup);
            },

            createWCLUserEvent : function () {
                var eventGroup = [];
                var eventDic;
                var nativeEvent;
                var nativeContext;
                var nativeDevice;
                var i;
                var j;

                console.group("WCLContext::createUserEvent");
                console.time("WCLContext::createUserEvent");
                console.group("contextGroup");

                for (i in contextGroup) {
                    eventDic = {};

                    nativeContext = contextGroup[i].nativeContext;
                    console.info("nativeContext", nativeContext);

                    if (contextGroup[i].nativeDevices &&
                            (contextGroup[i].nativeDevices instanceof Array)) {

                        for (j in contextGroup[i].nativeDevices) {
                            nativeEvent = nativeContext.createUserEvent();
                            console.log("Created WebCLEvent", nativeEvent);

                            eventDic.nativeContext = nativeContext;
                            eventDic.nativeEvent = nativeEvent;
                            eventDic.nativeDevice = contextGroup[i].nativeDevices[j];
                            eventGroup.push(eventDic);
                        }
                    }
                }

                console.groupEnd();
                console.timeEnd("WCLContext::createUserEvent");
                console.groupEnd();

                return new WCLEvent(eventGroup);
            },

            createFromGLBuffer : function(memFlags, glBuffer) {
                var nativeContext;
                var nativeBufferCLGL;
                var nativeDevices;
                var bufferDic;
                var bufferGroup;
                var i;
                var j;

                console.group("WCLContext::createWCLBuffer");
                console.time("WCLContext::createWCLBuffer");

                console.group("contextGroup");

                bufferGroup = [];
                for (i in contextGroup) {
                    nativeContext = contextGroup[i].nativeContext;
                    nativeDevices = contextGroup[i].nativeDevices;

                    // create a WCLBuffer for each device associate to a context
                    for (j in nativeDevices) {
                        nativeBufferCLGL = nativeContext.createFromGLBuffer(memFlags,
                                                                     glBuffer);

                        console.info("memFlags: ", memFlags, "glBuffer: ", glBuffer);
                        console.log("Created nativeBufferCLGL: ", nativeBufferCLGL);

                        bufferDic = {};
                        bufferDic.nativePlatform = contextGroup[i].nativePlatform;
                        bufferDic.nativeDevices = [contextGroup[i].nativeDevices[j]];
                        bufferDic.nativeBuffer = nativeBufferCLGL;
                        bufferGroup.push(bufferDic);
                    }

                    console.info("bufferDic: ", bufferDic);
                }

                console.groupEnd();
                console.info("bufferGroup: ", bufferDic);
                console.timeEnd("WCLContext::createWCLBuffer");
                console.groupEnd();

                return new WCLBuffer(bufferGroup, "bufferCLGL");
            }

        };
    }

    /**
     * Represents a list of programs.
     * There is a native program for each native context.
     *
     * @name WCLProgram
     * @class
     */

    /*
     * Creates a new WCLProgram instance.
     *
     * @name WCLProgram
     * @constructor
     * @param {programGroup} List of native programs and their devices
     *        in a dictionary format. For example:
     *        [{nativeProgram: 'native program', nativeDevices: 'program devices'}, ...]
     */
    function WCLProgram(programGroup) {
        return {
            id : "WCLProgram",

            /**
             * TODO: Function description.
             *
             * @name getNativePrograms
             * @function
             * @param
             * @returns
             */
            getNativePrograms : function () {
                return programGroup;
            },

            /**
             * TODO: Function description.
             *
             * @name build
             * @function
             * @param
             * @returns
             */
            build : function () {
                var program;
                var devices;
                var i;

                console.group("WCLProgram::build");
                console.time("WCLProgram::build");

                try {

                    console.group("programGroup");

                    for (i in programGroup) {
                        program = programGroup[i].nativeProgram;
                        devices = programGroup[i].nativeDevices;

                        console.info("program: ", program, "devices: ", devices);

                        program.build(devices);
                    }

                    console.groupEnd();

                } catch (e) {
                    var msg = program.getBuildInfo(devices, nativeWebCL.PROGRAM_BUILD_LOG);
                    throw (new Error("Cannot build program: ", msg));
                }

                console.timeEnd("WCLProgram::build");
                console.groupEnd();
            },

            /**
             * TODO: Function description.
             *
             * @name createWCLKernel
             * @function
             * @param
             * @returns
             */
            createWCLKernel : function (kernelName) {
                var nativeDevice;
                var nativeDevices;
                var nativeKernel;
                var nativeProgram;
                var kernelDic;
                var kernelGroup;
                var i;
                var j;

                console.group("WCLProgram::createWCLKernel");
                console.time("WCLProgram::createWCLKernel");
                console.info("kernelName: ", kernelName);

                kernelGroup = [];

                console.group("programGroup");

                for (i in programGroup) {
                    nativeProgram = programGroup[i].nativeProgram;
                    nativeDevices = programGroup[i].nativeDevices;

                    console.info("nativeProgram: ", nativeProgram);
                    console.group("devices");

                    for (j in nativeDevices) {
                        kernelDic = {};
                        nativeDevice = nativeDevices[j];
                        console.info("nativeDevice: ", nativeDevice);

                        nativeKernel = nativeProgram.createKernel(kernelName);

                        if (!(nativeKernel instanceof WebCLKernel)) {
                            throw (new Error("Could not create a native kernel"));
                        }

                        console.log("Created nativeKernel: ", nativeKernel);

                        kernelDic.nativeKernel = nativeKernel;
                        kernelDic.nativeDevices = [nativeDevice];
                        kernelGroup.push(kernelDic);
                    }

                    console.groupEnd();
                }

                console.groupEnd();
                console.info("kernelGroup: ", kernelGroup);
                console.timeEnd("WCLProgram::createWCLKernel");
                console.groupEnd();

                return new WCLKernel(kernelGroup);
            }

        };
    }

    /**
     * WebCLBuffer container
     *
     * @name WCLBuffer
     * @class
     * @param {bufferGroup} List of dictionary which contains a native platform,
     *                      device and native buffer, ex:
     *                          [ {nativePlatform: 'native platform',
     *                              nativeDevice: 'native device',
     *                              nativeBuffer: 'native buffer'}, ...]
     * @param {bufferType} Specify which type is the buffer. This class
     * encapsulates all types of buffer in WebCL.
     */
    function WCLBuffer(bufferGroup, bufferType) {
        return {
            id : "WCLBuffer",

            // Types: buffer, image, bufferCLGL, imageCLGL
            getType : function () {
                if (!bufferType) {
                    return "buffer";
                } else {
                    return bufferType;
                }
            },

            /**
             * @function getBufferGroup
             * @returns
             */
            getBufferGroup : function () {
                return bufferGroup;
            }
        };
    }

    /**
     * Represents a list of kernels created for each native device from
     * WCLDevice instance used to create WCLContext instance.
     *
     * @name WCLKernel
     * @class
     */

    /**
     * Creates a new WCLKernel instance.
     *
     * @name WCLKernel
     * @constructor
     * @param {kernelGroup} List of native kernels and their devices
     *        in a dictionary format. For example:
     *        [{nativeKernel: 'native kernel', nativeDevices: 'kernel devices'}, ...]
     */
    function WCLKernel(kernelGroup) {

        return {
            id : "WCLKernel",

            /**
             * TODO: Function description.
             *
             * @name getKernelGroup
             * @function
             * @param
             * @returns
             */
            getKernelGroup : function () {
                return kernelGroup;
            },

            /**
             * TODO: Function description.
             *
             * @name getWCLInfo
             * @function
             * @param
             * @returns
             */
            getWCLInfo : function (info) {
                var nativeKernel;
                var nativeDevicesInfo = [];
                var i;

                console.group("WCLKernel::getWCLInfo");
                console.time("WCLKernel::getWCLInfo");
                console.group("kernelGroup");

                for (i in kernelGroup) {
                    nativeKernel = kernelGroup[i].nativeKernel;
                    console.log("nativeKernel: ", nativeKernel);

                    nativeDevicesInfo.push(nativeKernel.getInfo(info));
                }

                console.log("nativeDevicesInfo: ", nativeDevicesInfo);
                console.timeEnd("WCLKernel::getWCLInfo");
                console.groupEnd();

                return nativeDevicesInfo;
            },

            /**
             * TODO: Function description.
             *
             * @name getWorkGroupInfo
             * @function
             * @param
             * @returns
             */
            getWorkGroupInfo: function (wclDevice, info) {
                var i;
                var j;
                var nativeKernel;
                var nativeDevice;
                var nativeDevices;
                var res;
                var groupInfo;
                var ex;

                console.group("WCLKernel::getWCLInfo");
                console.time("WCLKernel::getWCLInfo");

                if (!isWCLType(wclDevice, "WCLDevice")) {
                    ex = new Error("WCLDevice expected");
                    console.error("Error: ", ex);
                    throw ex;
                }

                console.info("wclDevice: ", wclDevice);

                //FIXME: Info must be a WebCLEnum KERNEL_WORK_GROUP_SIZE, KERNEL_COMPILE_WORK_GROUP_SIZE,
                //KERNEL_LOCAL_MEM_SIZE, KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, KERNEL_PRIVATE_MEM_SIZE
                //A method to validate param info will be created in utils file
                if (!info) {
                    ex = new Error("WebCLEnum expected");
                    console.error("Error: ", ex);
                    throw ex;
                }

                console.info("info", info);

                groupInfo = [];

                console.group("kernelGroup");

                for (i in kernelGroup) {

                    nativeKernel = kernelGroup[i].nativeKernel;
                    nativeDevices = kernelGroup[i].nativeDevices;

                    console.info("nativeKernel: ", nativeKernel);
                    console.group("nativeDevices");

                    for (j in nativeDevices) {
                        nativeDevice = nativeDevices[j];
                        res = nativeKernel.getWorkGroupInfo(nativeDevice, info);

                        console.info("nativeDevice: ", nativeDevice, "getWorkGroupInfo: ", res);

                        groupInfo.push(res);
                    }

                    console.groupEnd();
                }

                console.groupEnd();

                console.log("groupInfo: ", groupInfo);

                console.timeEnd("WCLKernel::getWCLInfo");
                console.groupEnd();

                return groupInfo;
            },

            /**
             * TODO: Function description.
             *
             * @name release
             * @function
             * @param
             * @returns
             */
            release: function () {
                var i;
                for (i in kernelGroup) {
                    kernelGroup[i].release();
                }

            },

            /**
             * TODO: Function description.
             *
             * @name setWCLKernelArgs
             * @function
             * @param {index} Integer
             * @param {arg} WCLBuffer or any value
             * @param {type} WebCL.ENUM, required when arg is not a WCLBuffer
             */
            setWCLKernelArgs : function (index, arg) {
                var i, wclBuffers = [], wclKernelGroup, nativeKernel;
                var argType = isWCLType(arg, "WCLBuffer") ?  "buffer" : "scalar";

                console.group("WCLKernel::setWCLKernelArgs");
                console.time("WCLKernel::setWCLKernelArgs");

                console.info("index: ", index, "arg: ", arg);

                wclKernelGroup = this.getKernelGroup();

                console.group("kernelGroup");

                for (i in wclKernelGroup) {

                    nativeKernel = wclKernelGroup[i].nativeKernel;

                    console.info("nativeKernel: ", nativeKernel);

                    try {
                        if (argType === "buffer") {
                            wclBuffers = arg.getBufferGroup();
                            nativeKernel.setArg(index, wclBuffers[i].nativeBuffer);
                        } else {
                            nativeKernel.setArg(index, arg);
                        }
                    } catch (e) {
                        console.error("Error setWCLKernelArgs", e);
                        throw e;
                    }

                    console.log("Passed setArg");
                }

                console.groupEnd();
                console.timeEnd("WCLKernel::setWCLKernelArgs");
                console.groupEnd();

            }
        };
    }

    /**
     * Represents a list of commands queue created for each native device from
     * WCLDevice instance used to create WCLContext instance.
     *
     * @name WCLCommandQueue
     * @class
     */

    /**
     * Creates a new WCLCommandQueue instance.
     *
     * @name WCLCommandQueue
     * @constructor
     * @param {kernelGroup} List native command queues and their devices
     *        in a dictionary format. For example:
     *        [{nativeCommandQueue: 'native cmd queue', nativeDevice: 'native device'}, ...]
     */
    function WCLCommandQueue(queueGroup) {

        return {
            id : "WCLCommandQueue",

            /**
             * TODO: Function description.
             *
             * @name getCommandQueueGroup
             * @function
             * @returns {nativeQueues} List of native command queues.
             */
            getCommandQueueGroup : function () {
                return queueGroup;
            },

            /**
             * @function enqueueReadBuffer
             * Get buffer from device to host and compare the results if there
             * are more than one device.
             *
             * @param {dictionary} {"WCLBuffer": wclbuffer object,
             *                      "blockingRead": if the read is blocking,
             *                      "bufferOffset": byte offset,
             *                      "numBytes": number bytes to read,
             *                      "data", vector to store the result}
             */
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
                //var readEvent;
                var msg;
                var i;
                var j;
                var k;
                var ex;
                var nativeEvent;
                var nativeEventList;

                console.group("WCLCommandQueue::enqueueReadBuffer");
                console.time("WCLCommandQueue::enqueueReadBuffer");
                console.info("args: ", args);

                try {
                    if (!isWCLType(args.buffer, "WCLBuffer")) {
                        ex = new Error("No WebCLBuffer in the arguments.");
                        console.error("Invalid argument error", ex);
                        throw ex;
                    }

                    if (!args.blockingRead) {
                        ex = new Error("No block in the arguments.");
                        console.error("Invalid argument error", ex);
                        throw ex;
                    }

                    if (!args.bufferOffset && args.bufferOffset !== 0) {
                        ex = new Error("No offset in the arguments.");
                        console.error("Invalid argument error", ex);
                        throw ex;
                    }

                    if (!args.numBytes) {
                        ex = new Error("No numBytes in the arguments.");
                        console.error("Invalid argument error", ex);
                        throw ex;
                    }

                    if (!args.data) {
                        ex = new Error("No data in the arguments.");
                        console.error("Invalid argument error", ex);
                        throw ex;
                    }

                    if (args.eventWaitList) {

                        if (!args.eventWaitList instanceof Array) {
                            ex = new Error("Arg eventWaitList must be an array");
                        }

                        for (i in args.eventWaitList) {
                            if (!isWCLType(args.eventWaitList[i], "WCLEvent")) {
                                ex = new Error("No WCLevent in the arguments.");
                            }
                        }

                        if (ex) {
                            console.error("Invalid arguments", ex);
                            throw ex;
                        }
                    }

                } catch (e) {
                    throw e;
                }

                bufferGroup = args.buffer.getBufferGroup();
                block = args.blockingRead;
                offset = args.bufferOffset;
                numBytes = args.numBytes;
                data = args.data;
                eventsList = args.eventWaitList;
                //readEvent = args["event"];

                lastData = null;

                console.group("queueGroup");

                for (i in queueGroup) {

                    console.info("queueGroup: ", queueGroup[i]);

                    nativeCommandQueue = queueGroup[i].nativeCommandQueue;
                    bufferDic = bufferGroup[i];
                    nativeBuffer = bufferDic.nativeBuffer;

                    /* empty events list */
                    nativeEventList = nativeEvent = null;

                    //TODO: event isnt working at this time, so this code doesnt break anything.
                    //TODO: Now isnt using listEvents because it breaks webkit.
                    for (k in eventsList) {
                        nativeEventList = [];
                        console.info("wclEvent", eventsList[k].getEventGroup());
                        console.info("wclEventDic", eventsList[k].getEventGroup()[i]);
                        nativeEvent = eventsList[k].getEventGroup()[i].nativeEvent;
                        nativeEventList.push(nativeEvent);
                    }

                    console.info("nativeCommandQueue: ", nativeCommandQueue, "nativeBuffer: ", nativeBuffer);

                    try {
                        nativeCommandQueue.enqueueReadBuffer(nativeBuffer, block,
                                                         offset, numBytes, data);
                    } catch (e) {
                        throw e;
                    }

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

                                ex = new Error(msg);
                                console.error("Diferent results", ex);
                                throw ex;
                            }
                        }
                    } else {
                        lastData = new Int32Array(data.length);
                    }

                    console.log("Passed WCLCommandQueue::enqueueReadBuffer");

                    // copy data
                    for (j in data) {
                        lastData[j] = data[j];
                    }
                    lastBufferDic = bufferDic;
                }

                console.groupEnd();
                console.timeEnd("WCLCommandQueue::enqueueReadBuffer");
                console.groupEnd();
            },

            /**
             * @function enqueueWriteBuffer
             * Write buffer from host to device are more than one device.
             *
             * @param {dictionary} {"WCLBuffer": wclbuffer object,
             *                      "blockingWrite": if the write is blocking,
             *                      "bufferOffset": byte offset,
             *                      "numBytes": number of bytes to read,
             *                      "data", vector to write to device}
             */
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
                var k;
                var eventsList;
                //var writeEvent;
                var ex;
                var nativeEventList;
                var nativeEvent;

                console.group("WCLCommandQueue::enqueueWriteBuffer");
                console.time("WCLCommandQueue::enqueueWriteBuffer");

                console.info("args: ", args);

                try {
                    if (!isWCLType(args.buffer, "WCLBuffer")) {
                        ex = new Error("No WebCLBuffer in the arguments.");
                        console.error("Invalid arguments", ex);
                        throw ex;
                    }

                    if (args.blockingWrite === undefined) {
                        ex = new Error("No block in the arguments.");
                        console.error("Invalid arguments", ex);
                        throw ex;
                    }

                    if (!args.bufferOffset && args.bufferOffset !== 0) {
                        ex = new Error("No offset in the arguments.");
                        console.error("Invalid arguments", ex);
                        throw ex;
                    }

                    if (!args.numBytes) {
                        console.error("Invalid arguments", ex);
                        throw new Error("No numBytes in the arguments.");
                    }

                    if (!args.data) {
                        throw new Error("No numBytes in the arguments.");
                    }

                    if (args.eventWaitList) {

                        if (!args.eventWaitList instanceof Array) {
                            ex = new Error("Arg eventWaitList must be an array");
                        }

                        for (i in args.eventWaitList) {
                            if (!isWCLType(args.eventWaitList[i], "WCLEvent")) {
                                ex = new Error("No WCLevent in the arguments.");
                            }
                        }

                        if (ex) {
                            console.error("Invalid arguments", ex);
                            throw ex;
                        }
                    }

                } catch (e) {
                    throw e;
                }

                bufferGroup = args.buffer.getBufferGroup();
                block = args.blockkingWrite;
                offset = args.bufferOffset;
                numBytes = args.numBytes;
                data = args.data;
                eventsList = args.eventWaitList;
                //writeEvent = args["event"];

                console.group("queueGroup");

                if (data) {
                    for (i in queueGroup) {
                        bufferDic = bufferGroup[i];
                        nativeBuffer = bufferDic.nativeBuffer;
                        nativeCommandQueue = queueGroup[i].nativeCommandQueue;

                        /* empty events list */
                        nativeEventList = nativeEvent = null;

                        //TODO: event isn't working at this time, so this code doesn't break anything.
                        for (k in eventsList) {
                            nativeEventList = [];
                            console.info("wclEvent", eventsList[k].getEventGroup());
                            console.info("wclEventDic", eventsList[k].getEventGroup()[i]);
                            nativeEvent = eventsList[k].getEventGroup()[i].nativeEvent;
                            nativeEventList.push(nativeEvent);
                        }


                        console.info("bufferDic: ", bufferDic, "nativeBuffer: ",
                                nativeBuffer, "nativeCommandQueue: ", nativeCommandQueue);

                        try {
                        //TODO: Uncomment the "nativeEvent" below when WebCLEvent is working.
                            nativeCommandQueue.enqueueWriteBuffer(nativeBuffer, block,
                                        offset, numBytes, data, nativeEventList);
                        } catch (e) {
                            throw e;
                        }

                        console.log("Passed native enqueueWriteBuffer");
                    }
                }

                console.groupEnd();
                console.timeEnd("WCLCommandQueue::enqueueWriteBuffer");
                console.groupEnd();
            },

            /**
             * @function enqueueCopyBuffer
             * Copy the contents of source buffer to destiny buffer.
             *
             * @param {dictionary} {"srcBuffer": wclbuffer object,
             *                      "dstBuffer": wclbuffer object,
             *                      "srcOffset": byte offset for source buffer,
             *                      "dstOffset": byte offset for destiny buffer,
             *                      "numBytes": number bytes to copy}
             */
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
                var k;
                var eventsList;
                var ex;
                var nativeEvent;
                var nativeEventList;

                console.group("WCLCommandQueue::enqueueCopyBuffer");
                console.time("WCLCommandQueue::enqueueCopyBuffer");
                console.info("args: ", args);

                try {
                    if (!isWCLType(args.srcBuffer, "WCLBuffer")) {
                        ex = new Error("No srcBuffer in the arguments.");
                        console.error("Invalid argument", ex);
                        throw ex;
                    }

                    if (!isWCLType(args.dstBuffer, "WCLBuffer")) {
                        ex = new Error("No dstsBuffer in the arguments.");
                        console.error("Invalid argument", ex);
                        throw ex;
                    }

                    if (!args.srcOffset && args.srcOffset !== 0) {
                        ex = new Error("No srcOffset in the arguments.");
                        console.error("Invalid argument", ex);
                        throw ex;
                    }

                    if (!args.dstOffset && args.dstOffset !== 0) {
                        ex = new Error("No dstOffset in the arguments.");
                        console.error("Invalid argument", ex);
                        throw ex;
                    }

                    if (!args.numBytes) {
                        ex = new Error("No numBytes in the arguments.");
                        console.error("Invalid argument", ex);
                        throw ex;
                    }

                    if (args.eventWaitList) {

                        if (!args.eventWaitList instanceof Array) {
                            ex = new Error("Arg eventWaitList must be an array");
                        }

                        for (i in args.eventWaitList) {
                            if (!isWCLType(args.eventWaitList[i], "WCLEvent")) {
                                ex = new Error("No WCLevent in the arguments.");
                            }
                        }

                        if (ex) {
                            console.error("Invalid arguments", ex);
                            throw ex;
                        }
                    }

                } catch (e) {
                    throw e;
                }

                srcBufferDic = args.srcBuffer.getBufferGroup();
                dstBufferDic = args.dstBuffer.getBufferGroup();
                srcOffset = args.srcOffset;
                dstOffset = args.dstOffset;
                numBytes = args.numBytes;
                eventsList = args.eventWaitList;

                console.group("queueGroup");

                for (i in queueGroup) {
                    srcNativeBuffer = srcBufferDic[i].nativeBuffer;
                    dstNativeBuffer = dstBufferDic[i].nativeBuffer;
                    nativeCommandQueue = queueGroup[i].nativeCommandQueue;

                    /* empty events list */
                    nativeEventList = nativeEvent = null;

                    //TODO: event isn't working at this time, so this code doesn't break anything.
                    for (k in eventsList) {
                        nativeEventList = [];
                        console.info("wclEvent", eventsList[k].getEventGroup());
                        console.info("wclEventDic", eventsList[k].getEventGroup()[i]);
                        nativeEvent = eventsList[k].getEventGroup()[i].nativeEvent;
                        nativeEventList.push(nativeEvent);
                    }

                    console.info("srcNativeBuffer: ", srcNativeBuffer, "dstNativeBuffer: ",
                            dstNativeBuffer, "nativeCommandQueue: ", nativeCommandQueue, nativeEvent);

                    try {
                        nativeCommandQueue.enqueueCopyBuffer(srcNativeBuffer,
                                                      dstNativeBuffer, srcOffset,
                                                      dstOffset, numBytes,
                                                      nativeEvent);
                    } catch (e) {
                        throw e;
                    }

                    console.log("Passed enqueueCopyBuffer");
                }

                console.groupEnd();
                console.timeEnd("WCLCommandQueue::enqueueCopyBuffer");
                console.groupEnd();

            },

            /**
             * @function enqueueCopyBuffer
             * Copy the contents of source buffer to destiny buffer.
             *
             * @param {dictionary} {"kernel": wclkernel object,
             *                      "globalWorkOffset": offset to start each id,
             *                      "globalWorkSize": byte offset for source buffer,
             *                      "localWorkSize": byte offset for destiny buffer}
             */
            enqueueNDRangeKernel : function (args) {
                var nativeCommandQueue;
                var nativeKernel;
                var nativeEvent;
                var nativeEventList;
                var offset;
                var globalSize;
                var localSize;
                var eventList;
                var wclKernel;
                var kernelGroup;
                var kernelDic;
                var i;
                var k;
                var ex;

                console.group("WCLCommandQueue::enqueueNDRangeKernel");
                console.time("WCLCommandQueue::enqueueNDRangeKernel");
                console.info("args: ", args);

                try {
                    if (!isWCLType(args.kernel, "WCLKernel")) {
                        ex = new Error("No kernel in the arguments.");
                        console.error("Invalid arguments", ex);
                        throw ex;
                    }

                    if (!(args.globalWorkOffset === null ||
                            args.globalWorkOffset instanceof Int32Array)) {
                        ex = new Error("No offset in the arguments.");
                        console.error("Invalid arguments", ex);
                        throw ex;
                    }

                    if (!(args.globalWorkSize instanceof Int32Array)) {
                        ex = new Error("No globalWorkSize in the arguments.");
                        console.error("Invalid arguments", ex);
                        throw ex;
                    }

                    if (!(args.localWorkSize === null || args.localWorkSize instanceof Int32Array)) {
                        ex = new Error("No globalWorkSize in the arguments.");
                        console.error("Invalid arguments", ex);
                        throw ex;
                    }

                    if (args.eventWaitList) {

                        if (!args.eventWaitList instanceof Array) {
                            ex = new Error("Arg eventWaitList must be an array");
                        }

                        for (i in args.eventWaitList) {
                            if (!isWCLType(args.eventWaitList[i], "WCLEvent")) {
                                ex = new Error("No WCLevent in the arguments.");
                            }
                        }

                        if (ex) {
                            console.error("Invalid arguments", ex);
                            throw ex;
                        }
                    }

                } catch (e) {
                    throw e;
                }

                wclKernel =   args.kernel;
                kernelGroup = wclKernel.getKernelGroup();
                offset =      args.globalWorkOffset;
                globalSize =  args.globalWorkSize;
                localSize =   args.localWorkSize;
                eventList =   args.eventWaitList;

                console.group("queueGroup");

                for (i in queueGroup) {

                    console.info("queueGroup", queueGroup);

                    /* empty events list */
                    nativeEventList = nativeEvent = null;

                    nativeKernel = kernelGroup[i].nativeKernel;
                    console.info("nativeKernel", nativeKernel);

                    //TODO: event isn't working at this time, so this code doesn't break anything.
                    for (k in eventList) {
                        nativeEventList = [];
                        console.info("wclEvent", eventList[k].getEventGroup());
                        console.info("wclEventDic", eventList[k].getEventGroup()[i]);
                        nativeEvent = eventList[k].getEventGroup()[i].nativeEvent;
                        nativeEventList.push(nativeEvent);
                    }

                    nativeCommandQueue = queueGroup[i].nativeCommandQueue;
                    try {
                        nativeCommandQueue.enqueueNDRangeKernel(nativeKernel,
                                    offset, globalSize, localSize, nativeEvent);
                    } catch (e) {
                        throw e;
                    }

                    console.info("nativeCommandQueue: ", nativeCommandQueue);
                    console.log("Passed native enqueueNDRangeKernel");
                }

                console.groupEnd();
                console.timeEnd("WCLCommandQueue::enqueueNDRangeKernel");
                console.groupEnd();
            },

            /**
             * @function enqueueAcquireGLObjects
             *
             * @param {dictionary} {"memObjects": list of WCLBuffers,
             *                      "eventWaitList": list of events,
             *                      "event": variable to keep the AcquireGLObject event}
             */
            enqueueAcquireGLObjects : function (args) {
                var i;
                var k;
                var ex;
                var eventsList;
                var memObjects;
                var bufferDic;
                var bufferGroup;
                var bufferObjects;
                var nativeCommandQueue;
                var nativeEvent;
                var nativeEventList;
                var nativeBuffer;

                console.group("WCLCommandQueue::enqueueAcquireGLObjects");
                console.time("WCLCommandQueue::enqueueAcquireGLObject");
                console.info("args: ", args);

                try {
                    if (args.memObjects) {

                        if (!(args.memObjects instanceof Array)) {
                            ex = new Error("Arg memObjects must be an array");
                        }

                        for (i in args.memObjects) {
                            if (!isWCLType(args.memObjects[i], "WCLBuffer")) {
                                ex = new Error("No WCLBuffer in the arguments");
                            }
                        }
                    }

                    if (args.eventWaitList) {

                        if (!args.eventWaitList instanceof Array) {
                            ex = new Error("Arg eventWaitList must be an array");
                        }

                        for (i in args.eventWaitList) {
                            if (!isWCLType(args.eventWaitList[i], "WCLEvent")) {
                                ex = new Error("No WCLEvent in the arguments.");
                            }
                        }

                        if (ex) {
                            console.error("Invalid arguments", ex);
                            throw ex;
                        }
                    }

                } catch (e) {
                    throw e;
                }

                bufferObjects = args.memObjects;
                eventsList = args.eventWaitList;

                console.group("queueGroup");

                /* empty events list */
                nativeEventList = nativeEvent = null;

                //TODO: event isn't working at this time, so this code doesn't break anything.
                for (k in eventsList) {
                    nativeEventList = [];
                    console.info("wclEvent", eventsList[k].getEventGroup());
                    console.info("wclEventDic", eventsList[k].getEventGroup()[i]);
                    nativeEvent = eventsList[k].getEventGroup()[i].nativeEvent;
                    nativeEventList.push(nativeEvent);
                }

                console.info("memObjects: ", bufferObjects, "nativeCommandQueue: ", nativeCommandQueue);

                // for each device
                for (i in bufferObjects) {
                    nativeCommandQueue = queueGroup[i].nativeCommandQueue;
                    bufferGroup = bufferObjects[i].getBufferGroup();
                    memObjects = [];

                    // get all buffers related to the interoperability
                    for (k in bufferGroup) {
                        nativeBuffer = bufferGroup[k].nativeBuffer;
                        memObjects.push(nativeBuffer);
                    }

                    try {
                        // according to the WD, the 1st parameter must be an array and not an obj
                        nativeCommandQueue.enqueueAcquireGLObjects(memObjects,
                                                                    eventsList, nativeEvent);
                    } catch (e) {
                        throw e;
                    }
                }

                console.log("Passed AcquireGLObject");

                console.groupEnd();
                console.timeEnd("WCLCommandQueue::enqueueAcquireGLObjects");
                console.groupEnd();
            },

            /**
             * @function enqueueReleaseGLObjects
             *
             * @param {dictionary} {"memObjects": list of WCLBuffers,
             *                      "eventWaitList": list of events,
             *                      "event": variable to keep the ReleaseGLObject event}
             */
            enqueueReleaseGLObjects : function (args) {
                var i;
                var k;
                var ex;
                var eventsList;
                var memObjects;
                var bufferDic;
                var bufferGroup;
                var bufferObjects;
                var nativeCommandQueue;
                var nativeEvent;
                var nativeEventList;
                var nativeBuffer;

                console.group("WCLCommandQueue::enqueueReleaseGLObjects");
                console.time("WCLCommandQueue::enqueueReleaseGLObject");
                console.info("args: ", args);

                try {
                    if (args.memObjects) {

                        if (!(args.memObjects instanceof Array)) {
                            ex = new Error("Arg memObjects must be an array");
                        }

                        for (i in args.memObjects) {
                            if (!isWCLType(args.memObjects[i], "WCLBuffer")) {
                                ex = new Error("No WCLBuffer in the arguments");
                            }
                        }
                    }

                    if (args.eventWaitList) {

                        if (!args.eventWaitList instanceof Array) {
                            ex = new Error("Arg eventWaitList must be an array");
                        }

                        for (i in args.eventWaitList) {
                            if (!isWCLType(args.eventWaitList[i], "WCLEvent")) {
                                ex = new Error("No WCLEvent in the arguments.");
                            }
                        }

                        if (ex) {
                            console.error("Invalid arguments", ex);
                            throw ex;
                        }
                    }

                } catch (e) {
                    throw e;
                }

                bufferObjects = args.memObjects;
                eventsList = args.eventWaitList;

                console.group("queueGroup");

                /* empty events list */
                nativeEventList = nativeEvent = null;

                //TODO: event isn't working at this time, so this code doesn't break anything.
                for (k in eventsList) {
                    nativeEventList = [];
                    console.info("wclEvent", eventsList[k].getEventGroup());
                    console.info("wclEventDic", eventsList[k].getEventGroup()[i]);
                    nativeEvent = eventsList[k].getEventGroup()[i].nativeEvent;
                    nativeEventList.push(nativeEvent);
                }

                console.info("memObjects: ", bufferObjects, "nativeCommandQueue: ", nativeCommandQueue);

                for (i in bufferObjects) {
                    nativeCommandQueue = queueGroup[i].nativeCommandQueue;
                    bufferGroup = bufferObjects[i].getBufferGroup();
                    memObjects = [];
                    for (k in bufferGroup) {
                        nativeBuffer = bufferGroup[k].nativeBuffer;
                        memObjects.push(nativeBuffer);
                    }

                    try {
                        // according to the WD, the 1st parameter must be an array and not an obj
                        nativeCommandQueue.enqueueReleaseGLObjects(memObjects,
                                                                    eventsList, nativeEvent);
                    } catch (e) {
                        throw e;
                    }
                }
                console.log("Passed ReleaseGLObject");

                console.groupEnd();
                console.timeEnd("WCLCommandQueue::enqueueReleaseGLObjects");
                console.groupEnd();
            },

            /**
             * @function finish
             * It waits for each command in command queue have been processed
             * and completed.
             */
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

    /**
     * TODO:
     *
     * @name WCLSampler
     * @class
     */
    function WCLSampler() {
        return {
            id : "WCLSampler"
        };
    }

    /**
     * Creates a new WCLEvent instance.
     *
     * @name WCLEvent
     * @constructor
     * @params {eventGroup} A list of eventDic  {nativeContext: WebCLContext,
     *                                           nativeEvent: WebCLEvent,
     *                                           nativeDevice: WebCLDevice}
     */
    function WCLEvent(eventGroup) {
        return {
            id : "WCLEvent",

            //TODO: Implements method
            getWCLInfo : function (info) {
                return "";
            },

            getEventGroup : function () {
                return eventGroup;
            },

            /**
             * @function setCallBack
             *
             * Wrapper to WebCLEvent setCallback method.
             * Used to set a callBack for all events in eventGroup
             *
             * @param {status} - execution status
             * @param {notify} - function to be executed
             * @param optional {userData}
             */
            setCallback : function (status, notify, userData) {
                var nativeEvent;
                var i;

                console.group("WCLEvent::setCallback");
                console.time("WCLEvent::setCallback");

                for (i in eventGroup) {
                    nativeEvent = eventGroup[i].nativeEvent;
                    console.info("nativeEvent", nativeEvent);

                    nativeEvent.setCallback(status, notify, userData);
                }

                console.timeEnd("WCLEvent::setCallback");
                console.groupEnd();
            }
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

        var logger = window.LOGGER;
        logger.setLogLevel(2);

        var cl = WebCLProxy;

        var plat = cl.getWCLPlatform();

        plat.version = plat.getWCLInfo(cl.PLATFORM_VERSION);

        /* To create the context to all available devices
           just call createWCLContext without args.
           Use getWCLDevice to create WCLDevice
           instance with some specific type of devices
           like CPU or GPU type.
        */

        plat.dev = plat.getWCLDevice();

        // Get devices info (DEVICE_TYPE)
        plat.dev.info = plat.dev.getWCLInfo(cl.DEVICE_TYPE);

        plat.ctx = plat.createWCLContext();

        var queue = plat.ctx.createWCLCommandQueue();

        // Initializing Data
        var DATA_SIZE = 10;
        var data = new Int32Array(DATA_SIZE);
        var result = new Int32Array(DATA_SIZE);
        var i;
        for (i = 0; i < DATA_SIZE; i++) {
            data[i] = i;
        }

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

        // Kernel creation
        try {
            plat.ctx.prog = plat.ctx.createWCLProgram(dummyKernel);

            plat.ctx.prog.build();
            plat.ctx.kernel = plat.ctx.prog.createWCLKernel("myDummyKernel");

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

            WebCLTestUtils.readWCLBuffers([wclBufferOut], queue);

            var k;
            var blob;
            for (k = 0; k < enumWGI.length; k++) {
                blob = plat.ctx.kernel.getWorkGroupInfo(plat.dev, enumWGI[k]);
            }
        } catch (e) {
            console.error("Error", e);
        }
    }
}(/*"RUN"*/));
