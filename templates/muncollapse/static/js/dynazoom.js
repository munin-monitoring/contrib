/* MunCollapse Template DynaZoom JavaScript File
*
* Notes:
* 
*  - No Internet Explorer support (uses "URLSearchParams")
*  - This is not even really a fork of the upstream version any more
*  - Drops upstream requirement of "QueryString" include (URL.URLSearchParams)
*  - Drops upstream requirement of "FormatDate" include (Date.toISOString())
*/

URLSearchParams.prototype.getDefault = function ( name, value ) {
    // Overload URLSearchParams to allow "default" values.
    return ( this.get( name ) === null ) ? value : this.get( name );
};

function refreshZoom( query, form, image ) {
    //INIT
    var qs = new URLSearchParams( query.split( "\?" )[ 1 ] );
    
    init();

    refreshImg();
    
    var start_epoch = parseInt( qs.getDefault( "rst_start_epoch", form.start_epoch.value ), 10 );
    var stop_epoch  = parseInt( qs.getDefault( "rst_stop_epoch", form.stop_epoch.value ), 10 );

    var highLighter      = document.getElementById( "image-overlay" );
    var gutterOffsetLeft = 66;
    var highlightStartX  = 0;
    var clickCounter     = 0;
    var relativeStartX   = 0;
    var graph_shown_width;
    var epoch_shown_start;
    var epoch_shown_stop;
    var eachPixelEpoch;

    form.plugin_name.onblur   = refreshImg;
    form.start_iso8601.onblur = majDates;
    form.stop_iso8601.onblur  = majDates;
    form.start_epoch.onblur   = function() { refreshImg(); updateStartStop(); };
    form.stop_epoch.onblur    = function() { refreshImg(); updateStartStop(); };
    form.lower_limit.onblur   = refreshImg;
    form.upper_limit.onblur   = refreshImg;
    form.size_x.onblur        = refreshImg;
    form.size_y.onblur        = refreshImg;
    form.btnReset.onclick     = reset;
    form.btnShowDay.onclick   = function() { showPeriod( 1 ); };
    form.btnShowWeek.onclick  = function() { showPeriod( 2 ); };
    form.btnShowMonth.onclick = function() { showPeriod( 3 ); };
    form.btnShowYear.onclick  = function() { showPeriod( 4 ); };
    form.onsubmit             = function() { document.activeElement.blur(); refreshImg(); return false; };

    // Sets the onClick handler
    image.onclick = click;

    //FUNCTIONS
    function init() {
        form.plugin_name.value = qs.getDefault( "plugin_name", "localdomain/localhost.localdomain/if_eth0" );
        form.start_epoch.value = qs.getDefault( "start_epoch", "1236561663" );
        form.stop_epoch.value  = qs.getDefault( "stop_epoch",  "1237561663" );
        form.lower_limit.value = qs.getDefault( "lower_limit", "" );
        form.upper_limit.value = qs.getDefault( "upper_limit", "" );
        form.size_x.value      = qs.getDefault( "size_x",      "" );
        form.size_y.value      = qs.getDefault( "size_y",      "" );

        updateStartStop();
    }

    function reset( event ) {
        init();

        //Can be not the initial ones in case of manual refresh
        form.start_epoch.value = start_epoch;
        form.stop_epoch.value  = stop_epoch;
        updateStartStop();

        //Redraw
        scale = refreshImg();

        //Reset gui
        clickCounter                = 0;
        image.onmousemove           = undefined;
        form.start_iso8601.disabled = false;
        form.stop_iso8601.disabled  = false;
        form.start_epoch.disabled   = false;
        form.stop_epoch.disabled    = false;
        highLighter.style.left      = "0px";
        highLighter.style.width     = "2px";
        highLighter.style.display   = "none";

        document.activeElement.blur();
        return false;
    }

    function refreshImg(event) {
        image.src = qs.getDefault( "cgiurl_graph", "/munin-cgi/munin-cgi-graph" ) + "/" +
            form.plugin_name.value +
            "-pinpoint=" + parseInt( form.start_epoch.value, 10 ) + "," + parseInt( form.stop_epoch.value, 10 ) +
            ".png" + "?" +
            "&lower_limit=" + form.lower_limit.value +
            "&upper_limit=" + form.upper_limit.value +
            "&size_x=" + form.size_x.value +
            "&size_y=" + form.size_y.value;
    }

    function updateStartStop() {
        form.start_iso8601.value = new Date( form.start_epoch.value * 1000 ).toISOString();
        form.stop_iso8601.value  = new Date( form.stop_epoch.value * 1000 ).toISOString();
    }

    function majDates( event ) {
        var lowLimit    = new Date(),
            topLimit    = new Date(),
            date_parsed = null;

        lowLimit.setFullYear( lowLimit.getFullYear() - 1 );

        date_parsed = new Date( Date.parse( form.start_iso8601.value ) || lowLimit.getTime() );
        form.start_epoch.value   = Math.floor( date_parsed.getTime() / 1000 );

        date_parsed = new Date( Date.parse( form.stop_iso8601.value) || topLimit.getTime() );
        form.stop_epoch.value   = Math.floor( date_parsed.getTime() / 1000 );

        updateStartStop();

        refreshImg();
    }

    function click( event ) {
        var relativeClickX = getClickLocation( event ),
            thisEpoch      = null;

        switch ( ( clickCounter++ ) % 3 ) {
            case 0: // First click of the displayed graph
                graph_shown_width = parseInt( form.size_x.value, 10) ;
                epoch_shown_start = parseInt( form.start_epoch.value,10 );
                epoch_shown_stop  = parseInt( form.stop_epoch.value, 10 );
                eachPixelEpoch    = ( ( epoch_shown_stop - epoch_shown_start ) / graph_shown_width );
                relativeStartX    = ( relativeClickX < 0 ? 0 : relativeClickX );

                form.start_iso8601.disabled = true;
                form.stop_iso8601.disabled  = true;
                form.start_epoch.disabled   = true;
                form.stop_epoch.disabled    = true;
                
                highlightStartX           = event.pageX;
                highLighter.style.left    = ( relativeStartX + gutterOffsetLeft + image.offsetLeft ) + "px";
                highLighter.style.display = "block";

                form.start_epoch.value = offsetEpoch( relativeClickX );
                updateStartStop();

                image.onmousemove = divMouseMove;

                break;
            case 1: // Second (end) click of the displayed graph
                thisEpoch = offsetEpoch( relativeClickX );

                image.onmousemove           = undefined;
                form.start_iso8601.disabled = false;
                form.stop_iso8601.disabled  = false;
                form.start_epoch.disabled   = false;
                form.stop_epoch.disabled    = false;

                // For negative values, assume we want a new start point and the old end point.
                // If it's not, set it.
                if ( thisEpoch > form.start_epoch.value ) {
                    form.stop_epoch.value = thisEpoch;
                } else {
                    form.stop_epoch.value   = Math.floor( epoch_shown_stop );
                    highLighter.style.width = ( graph_shown_width - relativeStartX ) + "px";
                }
                updateStartStop();

                break;
            case 2: // Nevermind or Do It.
                thisEpoch = offsetEpoch( relativeClickX );

                if ( thisEpoch >= form.start_epoch.value && thisEpoch <= form.stop_epoch.value ) {
                    refreshImg();
                } else {
                    form.start_epoch.value = epoch_shown_start;
                    form.stop_epoch.value  = epoch_shown_stop;
                    updateStartStop();
                }

                highLighter.style.left    = "0px";
                highLighter.style.width   = "2px";
                highLighter.style.display = "none";
        }
    }

    function divMouseMove( event ) {
        var diff           = event.pageX - highlightStartX,
            maxDiff        = graph_shown_width - relativeStartX,
            relativeClickX = getClickLocation( event );

        form.stop_epoch.value = offsetEpoch( relativeClickX );
        updateStartStop();

        highLighter.style.width = ( ( diff < 2 ) ? 2 : ( diff > maxDiff ? maxDiff : diff ) ) + "px";
    }

    function offsetEpoch( clickX ) {
        if ( clickX < 0 ) { return Math.floor( epoch_shown_start ); }

        if ( clickX > graph_shown_width ) { return Math.floor( epoch_shown_stop ); }

        return Math.floor( epoch_shown_start + ( clickX * eachPixelEpoch ) );
    }

    function getClickLocation( event ) {
        return ( event.pageX - image.getBoundingClientRect().x - gutterOffsetLeft );
    }

    function showPeriod( period ) {
        var now  = new Date(),
            past = new Date();

        switch (period) {
            case 1:
                past.setDate( past.getDate() - 1 ); break;
            case 2:
                past.setDate( past.getDate() - 7 ); break;
            case 3:
                past.setMonth( past.getMonth() - 1 ); break;
            case 4:
                past.setFullYear( past.getFullYear() - 1 ); break;
        }

        form.start_epoch.value = Math.floor( past.getTime() / 1000 );
        form.stop_epoch.value  = Math.floor( now.getTime() / 1000 );

        updateStartStop();
        refreshImg();
        document.activeElement.blur();
        return false;
    }
}
