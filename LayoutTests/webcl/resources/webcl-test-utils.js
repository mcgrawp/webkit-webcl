var WebCLTestUtils = (function () {

    /**
     * Represents a buffer parameter to setWCLKernelArgs
     *
     * @param {name} Some string that represents the param name
     * @param {size} Param size in bytes
     * @param {data} The data that object stores
     */
    var WCLBufferArg = function (name, size, data) {
        if (!name) {
            throw new Error("{name} is required");
        }

        if (!size) {
            throw new Error("{size} is required");
        }

        this.name = name;
        this.size = size;
        this.data = data;
        this.wclBuffer = null;

        return this;
    };

    /**
     * Represents a scalar parameter to setWCLKernelArgs
     *
     * @param {name} Some string that represents the param name
     * @param {type} Param size type. webcl.CHAR, webcl.UINT, etc
     * @param {data} The data that object stores
     */
    var WCLScalarArg = function (name, type, data) {

        if (!name) {
            throw new Error("{name} is required");
        }

        if (!type) {
            throw new Error("{type} is required");
        }

        this.name = name;
        this.type = type;
        this.data = data;

        return this;
    };

    var readWCLBuffers = function (arrWCLBufferArgs, wclCommandQueue) {
        var i, args;

        for (i in arrWCLBufferArgs) {
            args = {
                "buffer": arrWCLBufferArgs[i].wclBuffer,
                "blockingRead": true,
                "bufferOffset": 0,
                "numBytes": arrWCLBufferArgs[i].size,
                "data": arrWCLBufferArgs[i].data
            };

            try {
                wclCommandQueue.enqueueReadBuffer(args);
            } catch (e) {
                throw new Error("readWCLBuffers", e);
            }
        }
    };

    /**
     * This method helps user create and set arguments to run webcl kernel
     *
     * Ex.
     *
     * var args = [
     *  new WCLBufferArg ('input', 1024, new Uint32Array(data)),
     *  new WCLScalarArg ('param', webcl.INT, 5)
     * ];
     *
     * @param {arrArgs} List of WCLScalaArg or WCLBufferArg objects
     * @param {wclContext} A valid WCLContext
     * @param {wclCommandQueue} A valid WCLCommandQueue
     * @param {wclkernel} A valid WCLkernel
     */
    var setWCLKernelArgs = function (arrArgs, wclContext, wclCommandQueue, wclKernel) {
        var i, arg, typeArg, type, wclBuffers = [];

        if (wclCommandQueue.id !== "WCLCommandQueue") {
            throw new Error("A valid WCLCommandQueue is expected");
        }

        if (wclContext.id !== "WCLContext") {
            throw new Error("A valid WCLContext is expected");
        }

        if (wclKernel.id !== "WCLKernel") {
            throw new Error("A valid WCLKernel is expected");
        }

        if (!(arrArgs instanceof Array) || arrArgs.length === 0) {
            throw new Error("{arrArgs} be an array with one or more items");
        }

        for (i in arrArgs) {
            arg = type = typeArg = undefined;

            if (arrArgs[i] instanceof WCLBufferArg) {
                typeArg = "buffer";
            }

            if (arrArgs[i] instanceof WCLScalarArg) {
                typeArg = "scalar";
            }

            if (!typeArg) {
                throw new Error("arrArgs item must be a WCLBufferArg or WCLScalarArg");
            }

            if (typeArg === "buffer") {
                console.log("context: ", wclContext);
                arrArgs[i].wclBuffer = wclContext.createWCLBuffer(webcl.MEM_READ_WRITE, arrArgs[i].size);
                arg = arrArgs[i].wclBuffer;
                wclCommandQueue.enqueueWriteBuffer({"buffer": arg, "blockingWrite": false,
                    "bufferOffset": 0, "numBytes": arrArgs[i].size, "data": arrArgs[i].data});
            }

            if (typeArg === "scalar") {
                arg = arrArgs[i].data;
                type = arrArgs[i].type;
            }

            wclKernel.setWCLKernelArgs(i, arg, type);
        }
    };

    /**
     * Generate random number from 0 to {maxNumber}
     *
     */
    var getRandomNumber = function (maxNumber) {

        try {

            maxNumber = parseFloat(maxNumber);
            if (isNaN(parseInt(maxNumber, 10))) {
                throw new Error('getRandomNumber::maxNumber is not a number');
            }

            if (maxNumber <= 0) {
                throw new Error('getRandomNumber::maxNumber must be a positive integer');
            }

            var x = Math.floor((Math.random() * maxNumber) + 1);
            return x;

        } catch (e) {
            throw e;
        }
    };


    /**
     * Generate random one dimension arrays
     *
     */
    var getRandomVector = function (size, maxNumber) {
        var i;

        try {

            var ret = [];
            for (i = 0; i < size; i++) {
                ret[i] = this.getRandomNumber(maxNumber);
            }

            return ret;
        } catch (e) {
            throw e;
        }
    };

    /**
     *  Receives a string and returns a mixed string with upper and lower case
     *
     *  @param {str} String
     */
    var mixedString = function (str) {
        var i;
        var newChars = [];
        var c;
        var m;

        for (i = 0; i < str.length; ++i) {
            c = str.substr(i, 1);
            m = (Math.random() > 0.5) ? c.toLowerCase() : c.toUpperCase();
            newChars.push(m);
        }

        return newChars.join("");
    };

    return {
        none: false,
        getRandomVector: getRandomVector,
        getRandomNumber: getRandomNumber,
        WCLBufferArg: WCLBufferArg,
        WCLScalarArg: WCLScalarArg,
        setWCLKernelArgs: setWCLKernelArgs,
        readWCLBuffers: readWCLBuffers,
        mixedString: mixedString
    };

}());
