/**
 * WebCLTest module.
 */
WebCLTest = (function () {
    "use strict";

    var cl = null;
    var clEvent = null;
    var context = null;
    var device = null;
    var devices = null;
    var deviceType = null;
    var kernel = null;
    var platform = null;
    var platforms = null;
    var program = null;
    var queue = null;

    var getCL = function () {
        return cl;
    };

    var getPlatforms = function () {
        return platforms;
    };

    var getPlatform = function (platformId) {
        return platforms[platformId];
    };

    var getDevice = function (deviceId) {
        return devices[deviceId];
    };

    var getDevices = function () {
        return devices;
    };

    var getDeviceType = function () {
        return deviceType;
    };

    var getKernel = function () {
        return kernel;
    };

    /**
     * Initialize common CL resources.
     */
    var init = function (platformId, deviceId, type) {
        if (webcl === undefined) {
            throw (new Error("Failed to init CL support ..."));
        }

        cl = webcl;
        if (cl === null) {
            throw (new Error("Failed to init CL support ..."));
        }

        platforms = cl.getPlatforms();
        if (platforms.length === 0) {
            throw (new Error("Failed to get platforms ... "));
        }

        if (platformId === null || platformId === "" || platformId === undefined) {
            platform = platforms[0];
        } else {
            platform = platforms[platformId];
        }

        if (type === "gpu") {
            deviceType = cl.DEVICE_TYPE_GPU;
        } else if (type === "cpu") {
            deviceType = cl.DEVICE_TYPE_CPU;
        } else if (type === "all") {
            deviceType = cl.DEVICE_TYPE_ALL;
        } else {
            deviceType = cl.DEVICE_TYPE_DEFAULT;
        }

        devices = platform.getDevices(deviceType);
        if (devices.length === 0) {
            throw (new Error("Failed to get devices list ... "));
        }

        if (deviceId === null || deviceId === "" || deviceId === undefined) {
            device = devices[0];
        } else {
            device = devices[deviceId];
        }
    };

    var createContext = function (contextProperties) {

        if (contextProperties === null || contextProperties === undefined) {
            contextProperties = { "platform": platform,
                                    "devices": devices,
                                    "deviceType": deviceType};
        }


        context = cl.createContext(contextProperties);
        if (!(context instanceof WebCLContext)) {
            throw (new Error("Invalid CL context ..."));
        }

        return context;
    };

    var createProgram = function (kernelSource, kernelName) {
        if (!(context instanceof WebCLContext)) {
            throw (new Error("Invalid CL context ..."));
        }

        if (kernelSource === "" || kernelName === "") {
            throw (new Error("Kernel source or kernel name are empty..."));
        }

        program = context.createProgram(kernelSource);
        if (!(program instanceof WebCLProgram)) {
            throw (new Error("Invalid CL program ..."));
        }

        program.build(devices);
        kernel = program.createKernel(kernelName);
        if (!(kernel instanceof WebCLKernel)) {
            throw (new Error("Invalid kernel instance ..."));
        }

        return program;
    };

    var createCommandQueue = function () {
        if (!(context instanceof WebCLContext)) {
            throw (new Error("Invalid CL context ..."));
        }

        queue = context.createCommandQueue(device, null);
        if (!(queue instanceof WebCLCommandQueue)) {
            throw (new Error("Invalid CL queue ..."));
        }

        return queue;
    };

    var createEvent = function (eventCallback, executionStatus) {
        if (!(context instanceof WebCLContext)) {
            throw (new Error("Invalid CL context ..."));
        }

        clEvent = context.createUserEvent();
        if (!(clEvent instanceof WebCLEvent)) {
            throw (new Error("Fail when trying to create an event..."));
        }

        // executionStatus could be cl.STOPPED, cl.RUNNING, cl.SUBMITTED, cl.QUEUED
        clEvent.setCallback(executionStatus, eventCallback, null);
        return clEvent;
    };

    return {
        createCommandQueue: createCommandQueue,
        createContext: createContext,
        createEvent: createEvent,
        createProgram: createProgram,
        getCL: getCL,
        getDevice: getDevice,
        getDevices: getDevices,
        getDeviceType: getDeviceType,
        getKernel: getKernel,
        getPlatform: getPlatform,
        getPlatforms: getPlatforms,
        init: init
    };

}());
