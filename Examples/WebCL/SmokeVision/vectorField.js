function VectorField(dim, viscosity, dt, boundaries) {
    var i, j, k;

    this.field = new Float32Array(getFlatSize(dim) * 3);

    this.dim = dim;
    this.viscosity = viscosity;
    this.dt = dt;
    this.slip = true;
    this.vorticityScale = 3.0;

    for (i = 0; i < this.dim + 2; i++) {
        for (j = 0; j < this.dim + 2; j++) {
            for (k = 0; k < this.dim + 2; k++) {
                this.field[vindex(i, j, k, 0, dim)] = 0.0;
                this.field[vindex(i, j, k, 1, dim)] = 0.0;
                this.field[vindex(i, j, k, 2, dim)] = 0.0;
            }
        }
    }
}

VectorField.prototype.setTimestep = function (value) {
    this.dt = value;
};

VectorField.prototype.setViscosity = function (value) {
    this.viscosity = value;
};

VectorField.prototype.reset = function () {
    var bufSize = 4 * numCells;
    var i, j, k;

    for (i = 0; i < this.dim + 2; i++) {
        for (j = 0; j < this.dim + 2; j++) {
            for (k = 0; k < this.dim + 2; k++) {
                this.field[vindex(i, j, k, 0, dim)] = 0.0;
                this.field[vindex(i, j, k, 1, dim)] = 0.0;
                this.field[vindex(i, j, k, 2, dim)] = 0.0;
            }
        }
    }

    clQueue.enqueueWriteBuffer(vectorBuffer, false, 0, bufSize * 3, this.getField(), []);
};

VectorField.prototype.getField = function () {
    return this.field;
};

VectorField.prototype.draw = function () {

};

VectorField.prototype.step = function (source) {
    this.addField(source);
    this.vorticityConfinement();
    this.diffusion();
    this.projection();
    this.advection();
    this.projection();
};

VectorField.prototype.addField = function (source) {

    vectorAddKernel.setArg2(0, vectorBuffer);
    vectorAddKernel.setArg2(1, vectorSourceBuffer);
    vectorAddKernel.setArg2(2, new Uint32Array([this.dim]));
    vectorAddKernel.setArg2(3, new Float32Array([this.dt]));

    try {
        var globalWS = new Int32Array(3);
        globalWS[0] = globalWS[1] = globalWS[2] = localThreads;

        var start = Date.now();
        clQueue.enqueueNDRangeKernel(vectorAddKernel, null, globalWS, null);
        clQueue.finish();
        clTime += Date.now() - start;
    } catch (e) {
        console.innerHTML = e;
        console.log("vectorField.addField", [e]);
    }
};

VectorField.prototype.diffusion = function () {
    var globalWS = new Int32Array(3);
    globalWS[0] = globalWS[1] = globalWS[2] = localThreads;

    try {
        vectorCopyKernel.setArg2(0, vectorBuffer);
        vectorCopyKernel.setArg2(1, vectorTempBuffer);
        vectorCopyKernel.setArg2(2, new Uint32Array([this.dim]));

        var start = Date.now();
        clQueue.enqueueNDRangeKernel(vectorCopyKernel, null, globalWS, null);
        clTime += Date.now() - start;

        vectorDiffusionKernel.setArg2(0, vectorBuffer);
        vectorDiffusionKernel.setArg2(1, vectorTempBuffer);
        vectorDiffusionKernel.setArg2(2, new Uint32Array([this.dim]));
        vectorDiffusionKernel.setArg2(3, new Float32Array([this.dt]));
        vectorDiffusionKernel.setArg2(4, new Float32Array([this.viscosity]));

        start = Date.now();
        clQueue.enqueueNDRangeKernel(vectorDiffusionKernel, null, globalWS, null);
        clTime += Date.now() - start;
    } catch (e) {
        console.error("vectorField.diffusion", [e]);
    }

    this.setBoundaryVelocities();
};

VectorField.prototype.projection = function () {
    var globalWS = new Int32Array(3);
    var i;
    globalWS[0] = globalWS[1] = globalWS[2] = localThreads;

    try {
        vectorInitFieldKernel.setArg2(0, scalarTempBuffer);
        vectorInitFieldKernel.setArg2(1, new Uint32Array([this.dim]));

        var start = Date.now();
        clQueue.enqueueNDRangeKernel(vectorInitFieldKernel, null, globalWS, null);
        clTime += Date.now() - start;

        vectorInitFieldKernel.setArg2(0, scalarSecondTempBuffer);
        vectorInitFieldKernel.setArg2(1, new Uint32Array([this.dim]));

        start = Date.now();
        clQueue.enqueueNDRangeKernel(vectorInitFieldKernel, null, globalWS, null);
        clTime += Date.now() - start;

        vectorProjectionFirst.setArg2(0, vectorBuffer);
        vectorProjectionFirst.setArg2(1, scalarTempBuffer);
        vectorProjectionFirst.setArg2(2, scalarSecondTempBuffer);
        vectorProjectionFirst.setArg2(3, new Uint32Array([this.dim]));
        vectorProjectionFirst.setArg2(4, new Float32Array([1 / this.dim]));

        start = Date.now();
        clQueue.enqueueNDRangeKernel(vectorProjectionFirst, null, globalWS, null);
        clTime += Date.now() - start;

        this.setScalarFieldDensities(scalarTempBuffer);
        this.setScalarFieldDensities(scalarSecondTempBuffer);

        for (i = 0; i < 20; i++) {
            vectorProjectionSecond.setArg2(0, vectorBuffer);
            vectorProjectionSecond.setArg2(1, scalarTempBuffer);
            vectorProjectionSecond.setArg2(2, scalarSecondTempBuffer);
            vectorProjectionSecond.setArg2(3, new Uint32Array([this.dim]));
            vectorProjectionSecond.setArg2(4, new Float32Array([1 / this.dim]));

            start = Date.now();
            clQueue.enqueueNDRangeKernel(vectorProjectionSecond, null, globalWS, null);
            clTime += Date.now() - start;

            this.setScalarFieldDensities(scalarTempBuffer);
        }

        vectorProjectionThird.setArg2(0, vectorBuffer);
        vectorProjectionThird.setArg2(1, scalarTempBuffer);
        vectorProjectionThird.setArg2(2, scalarSecondTempBuffer);
        vectorProjectionThird.setArg2(3, new Uint32Array([this.dim]));
        vectorProjectionThird.setArg2(4, new Float32Array([1 / this.dim]));

        start = Date.now();
        clQueue.enqueueNDRangeKernel(vectorProjectionThird, null, globalWS, null);
        clTime += Date.now() - start;

        this.setBoundaryVelocities();
    } catch (e) {
        console.error("vectorField.projection", [e]);
    }
};

VectorField.prototype.advection = function () {
    var globalWS = new Int32Array(3);
    globalWS[0] = globalWS[1] = globalWS[2] = localThreads;

    try {
        vectorCopyKernel.setArg2(0, vectorBuffer);
        vectorCopyKernel.setArg2(1, vectorTempBuffer);
        vectorCopyKernel.setArg2(2, new Uint32Array([this.dim]));

        var start = Date.now();
        clQueue.enqueueNDRangeKernel(vectorCopyKernel, null, globalWS, null);
        clTime += Date.now() - start;

        vectorAdvectionKernel.setArg2(0, vectorBuffer);
        vectorAdvectionKernel.setArg2(1, vectorTempBuffer);
        vectorAdvectionKernel.setArg2(2, new Uint32Array([this.dim]));
        vectorAdvectionKernel.setArg2(3, new Float32Array([this.dt]));

        start = Date.now();
        clQueue.enqueueNDRangeKernel(vectorAdvectionKernel, null, globalWS, null);
        clTime += Date.now() - start;
    } catch (e) {
        console.error("vectorField.advection", [e]);
    }

    this.setBoundaryVelocities();
};


VectorField.prototype.vorticityConfinement = function () {
    var globalWS = new Int32Array(3);
    globalWS[0] = globalWS[1] = globalWS[2] = localThreads;

    try {
        vectorCopyKernel.setArg2(0, vectorBuffer);
        vectorCopyKernel.setArg2(1, vectorTempBuffer);
        vectorCopyKernel.setArg2(2, new Uint32Array([this.dim]));

        var start = Date.now();
        clQueue.enqueueNDRangeKernel(vectorCopyKernel, null, globalWS, null);
        clTime += Date.now() - start;

        vectorVorticityFirstKernel.setArg2(0, vectorBuffer);
        vectorVorticityFirstKernel.setArg2(1, vectorTempBuffer);
        vectorVorticityFirstKernel.setArg2(2, scalarTempBuffer);
        vectorVorticityFirstKernel.setArg2(3, new Uint32Array([this.dim]));
        vectorVorticityFirstKernel.setArg2(4, new Float32Array([this.dt * this.vorticityScale]));

        start = Date.now();
        clQueue.enqueueNDRangeKernel(vectorVorticityFirstKernel, null, globalWS, null);
        clTime += Date.now() - start;

        vectorVorticitySecondKernel.setArg2(0, vectorBuffer);
        vectorVorticitySecondKernel.setArg2(1, vectorTempBuffer);
        vectorVorticitySecondKernel.setArg2(2, scalarTempBuffer);
        vectorVorticitySecondKernel.setArg2(3, new Uint32Array([this.dim]));
        vectorVorticitySecondKernel.setArg2(4, new Float32Array([this.dt * this.vorticityScale]));

        start = Date.now();
        clQueue.enqueueNDRangeKernel(vectorVorticitySecondKernel, null, globalWS, null);
        clTime += Date.now() - start;
    } catch (e) {
        console.error("vectorField.vorticityConfinement", [e]);
    }
};

VectorField.prototype.setBoundaryVelocities = function () {
    var globalWS = new Int32Array(3);
    globalWS[0] = globalWS[1] = globalWS[2] = localThreads;

    try {
        vectorBoundariesKernel.setArg2(0, vectorBuffer);
        vectorBoundariesKernel.setArg2(1, new Uint32Array([this.dim]));

        var start = Date.now();
        clQueue.enqueueNDRangeKernel(vectorBoundariesKernel, null, globalWS, null);
        clTime += Date.now() - start;
    } catch (e) {
        console.error("vectorField.setBoundaryVelocities", [e]);
    }
};

VectorField.prototype.setScalarFieldDensities = function (field) {
    var globalWS = new Int32Array(3);
    globalWS[0] = globalWS[1] = globalWS[2] = localThreads;

    try {
        scalarBoundariesKernel.setArg2(0, field);
        scalarBoundariesKernel.setArg2(1, new Uint32Array([this.dim]));

        var start = Date.now();
        clQueue.enqueueNDRangeKernel(scalarBoundariesKernel, null, globalWS, null);
        clTime += Date.now() - start;
    } catch (e) {
        console.error("vectorField.setScalarFieldDensities", [e]);
    }
};
