/**
 *  A kind of wrapper to console
 *
 *  It creates a window object called Logger
 *
 *  It allows developer change log level in runtime using
 *  Logger.setLogLevel(level) method.
 *
 *  Levels defined
 *      LOGGER.NONE => Shows only console.error
 *      LOGGER.LOG => Shows console.error, console.warn and console.log
 *      LOGGER.VERBOSE => Shows all
 *
 * @module Logger
 */
window.LOGGER = (function () {

    var NONE    = 0; //error
    var LOG     = 1; //log e warn
    var VERBOSE = 2; //info, group e time

    /**
     * compatibility mode overwriting console
     * API methods
     *
     */
    if (!window.console) {
        window.console = {};
        window.console.time = function () {};
        window.console.timeEnd = function () {};
        window.console.group = function () {};
        window.console.groupEnd = function () {};
        window.console.log = function () {};
        window.console.info = function () {};
        window.console.warn = function () {};
    }

    /* Set private vars */
    var log      = console.log;
    var time     = console.time;
    var timeEnd  = console.timeEnd;
    var group    = console.group;
    var groupEnd = console.groupEnd;
    var warn     = console.warn;
    var info     = console.info;

    /**
     * Change log level in runtime
     *
     * @param {logLevel} NONE, LOG or VERBOSE
     */
    this.setLogLevel = function (logLevel) {

        switch (logLevel) {
        case 0:
            console.time = console.timeEnd = console.log = console.group =
                console.groupEnd = console.info = function () {};
            break;

        case 1:
            console.time = console.timeEnd = console.group = console.groupEnd
                = console.info = function () {};
            console.log = log;
            console.warn = warn;
            break;

        case 2:
            console.time = time;
            console.timeEnd = timeEnd;
            console.group = group;
            console.groupEnd = groupEnd;
            console.log = log;
            console.warn = warn;
            console.info = info;
            break;
        }
    };

    return this;
}());
