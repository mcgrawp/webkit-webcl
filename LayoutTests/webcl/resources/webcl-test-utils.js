
//Define the number of iterations that the stress tests will run
var STRESS_ITERS = 100000;

var WebCLTestUtils = (function () {

    /**
     * Generate random number from 0 to {maxNumber}
     * 
     */
    var getRandomNumber = function (maxNumber){

        try{

            maxNumber = parseFloat(maxNumber);
            if(isNaN(parseInt(maxNumber))){
                throw new Error ('getRandomNumber::maxNumber is not a number');
            }
            
            if(maxNumber <= 0) {
                throw new Error('getRandomNumber::maxNumber must be a positive integer');
            }

            var x = Math.floor((Math.random() * maxNumber) + 1);
            return x;
            
        } catch(e){
            throw e;
        }
    };


    /**
     * Generate random one dimension arrays 
     * 
     */
    var getRandomVector = function( size, maxNumber){

        try {

            var ret = new Array(size);
            for(var i=0; i< size; i++) {
                ret[i] = this.getRandomNumber(maxNumber);
            }

            return ret;
        } catch(e) {
            throw e;
        }
    }; 

    return {
        none: false,
        getRandomVector: getRandomVector,
        getRandomNumber: getRandomNumber
    };

}());
