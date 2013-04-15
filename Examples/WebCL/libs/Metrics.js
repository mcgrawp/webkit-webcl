/**
 *  API to calculate and summarize execution time
 *  
 *  @author Alexandre Rocha <alerock@gmail.com>
 *  @license LGPL
 */
Metrics = (function(){

    /* Private fields */
    DIV_METRICS = "conteinerMetrics";
    DIV_FPS = "divFPS";
    result = null;
    interval = null;
    
    /* Public API */
    
    API = {
            
            average : null,
            duration : null,
            iterations : null,
            lastPeriod: 0,
            lastIteration: 0,
            
            /**
             *  If is defined a DIV with id = DIV_METRICS, this function
             *  shows Average, Duration and Iterations value
             */
            showMetrics : function(){
              
                var elm, avg, duration;
                var _this = this;
                var delta = this.duration - this.lastPeriod;
                var iteration = this.iterations - this.lastIteration;
                this.lastPeriod =  this.duration;
                this.lastIteration = this.iterations;
                
                if(document.getElementById(DIV_METRICS) != undefined) {
                    
                    setTimeout(function(){
                        elm = document.getElementById(DIV_METRICS);
                        
                        elm.innerHTML = '';
                        
                        avg = document.createElement('div');
                        avg.innerHTML = 'Average (ms): ' + _this.average;
                        
                        duration = document.createElement('div');
                        duration.innerHTML = 'Duration (ms): ' + _this.duration;
                        
                        iterations = document.createElement('div');
                        iterations.innerHTML = 'Iterations: ' + _this.iterations;

                        fps = document.createElement('div');
                        //TODO: calculate fps
                        fps.innerHTML = 'FPS: ' +  (_this.iterations / _this.duration / 1000);

                        elm.appendChild(avg);
                        elm.appendChild(duration);
                        elm.appendChild(iterations);
                        //elm.appendChild(fps);
                        
                    }, 1);
                }
                
            },
            
            /**
             * Reset last calculation and set new Iterations number
             * 
             * @param it Number of iterations to calculate and show preview data
             */
            init: function(/* number of iterations to calculate metrics */ it) {
              result = new Array(); 
              this.average = this.duration = this.iterations = 0;
              interval = it;
              this.showMetrics();
            },
            
            /**
             * Run all methods to generate metrics
             */
            calculate : function(){
              this.average = this.getAverage();
              this.duration = this.getDuration();
              this.iterations = this.getIterations();
              
            },
            
            /**
             * This function is called by program in each iteration
             * to save the execution time
             * 
             * It checks if the number of iterations to recalculate
             * metrics is reached . If true, the values (average, duration 
             * and iterations) are refreshed 
             * 
             * @param {Number} res Time in milisecs
             */
            addDuration : function(/* ms */ res) {
                
                result.push(res);
                
                if( ! (this.getIterations() % interval)) {
                    this.calculate();
                    this.showMetrics();
                }
            },
            
            /**
             * Shows current average
             * 
             * @returns {Number}
             */
            getAverage : function(){
                return (this.getDuration() / result.length);
            },
            
            /**
             * Shows current duration
             * 
             * @returns {Number}
             */
            getDuration : function(){
                var sum = 0;
                
                for(var i = 0; i < result.length; i++){
                    sum += result[i];
                }
                
                return sum;
            },
            
            /**
             * Shows current iterations number
             * 
             * @returns {Number}
             */
            getIterations : function(){
                return result.length;
            }
        };
        
        return API;
})();
