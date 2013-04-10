/**
 * WebCLTest module.
 */
WebCLTest = (function() {
"use restrict";

    var cl = null;

    var platforms = null;

    var platform = null;

    var devices = null;

    var device = null;

    var context = null;

    var queue = null;

    var program = null;

    var getCL = function() {
        return cl;
    };

    var getPlatforms = function () {
        return platforms;
    };

    var getPlatform = function(platformId) {
        return platforms[platformId];
    };

    var getDevice = function(deviceId) {
        return devices[deviceId];
    };

    var getDevices = function() {
        return devices;
    };

    var getContext = function() {
        return context;
    };

    var getQueue = function() {
        return queue;
    };

    var getProgram = function() {
        return program;
    };

    /**
     * Initialize common CL resources.
     * @param {string} deviceType The device type(gpu, cpu, etc).
     */
    var initCL = function(platformId, deviceId, deviceType) {

        if (typeof(webcl) === "undefined") {
            throw ("Failed to init CL support ...");
        }

        cl = webcl;
        if (cl === null) {
            throw ("Failed to init CL support ...");
        }

        platforms = cl.getPlatforms();
        if (platforms.length === 0) {
            throw ("Failed to get platforms ... ");
        }

        if (platformId === null || platformId === "" || typeof(platformId) === "undefined") {
            platform = platforms[platformId];
        } else {
            platform = platforms[0];
        }

        devices = platform.getDevices(deviceType === "gpu" ? cl.DEVICE_TYPE_GPU : cl.DEVICE_TYPE_CPU);
        if (devices.length === 0) {
            throw ("Failed to get devices list ... ");
        }

        if (deviceId === null || deviceId === "" || typeof(deviceId) === "undefined") {
            device = devices[deviceId];
        } else {
            device = devices[0];
        }

    };

    var initCLContext = function(deviceType) {
        var contextProperties = {platform: platform, devices: devices, deviceType: cl.DEVICE_TYPE_ALL, shareGroup: 0, hint: null};
        context = cl.createContext(contextProperties);
        if (context === null || typeof(context) === "undefined") {
            throw("Invalid CL context ...");
        }
    };

    var initCLProgram = function(kernelSource) {
        program = getContext().createProgram(kernelSource);
        if (program === null || typeof(program) === "undefined") {
            throw("Invalid CL program ...");
        }
    };

    var initCommandQueue = function(deviceId) {
        var _device = getDevice(deviceId);
        queue = context.createCommandQueue(_device, null);
        if (queue === null || typeof(queue) === "undefined") {
            throw("Invalid CL queue ...");
        }
    };

    var clBuildProgram = function(deviceId) {
        var _device = getDevice(deviceId);
        program.build(_device);
    };

    var setupCLSimpleProgram = function(deviceType, kernelSource) {
        initCL(0, 0, deviceType);
        initCLContext(deviceType);
        initCLProgram(kernelSource);
        initCommandQueue(0);
        clBuildProgram(0);
    };

    return {
        getCL: getCL,
        getPlatform: getPlatform,
        getDevice: getDevice,
        getContext: getContext,
        getQueue: getQueue,
        getProgram: getProgram,
        initCL: initCL,
        initCLContext: initCLContext,
        initCLProgram: initCLProgram,
        initCommandQueue: initCommandQueue,
        clBuildProgram: clBuildProgram,
        setupCLSimpleProgram: setupCLSimpleProgram,
        none: false
    };

}());
