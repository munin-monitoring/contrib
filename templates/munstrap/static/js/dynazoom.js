function refreshZoom(query, form, image, divOverlay) {
  //INIT
  var qs = new Querystring(query);
  init();
  
  var scale = refreshImg();
  var start_epoch = (+qs.get("rst_start_epoch", form.start_epoch.value));
  var stop_epoch = (+qs.get("rst_stop_epoch", form.stop_epoch.value));
  var initial_left;
  var initial_top;

  //form.btnMaj.onclick = majDates;
  form.plugin_name.onblur = refreshImg;
  form.start_iso8601.onblur = majDates;
  form.stop_iso8601.onblur = majDates;
  form.start_epoch.onblur = refreshImg;
  form.stop_epoch.onblur = refreshImg;
  form.lower_limit.onblur = refreshImg;
  form.upper_limit.onblur = refreshImg;
  form.size_x.onblur = refreshImg;
  form.size_y.onblur = refreshImg;
  form.btnReset.onclick = reset;
  
  // Sets the onClick handler
  image.onclick = click;
  var clickCounter = 0;

  //FUNCTIONS
  function init(){
    form.plugin_name.value = qs.get("plugin_name", "localdomain/localhost.localdomain/if_eth0");
    form.start_epoch.value = qs.get("start_epoch", "1236561663");
    form.stop_epoch.value = qs.get("stop_epoch", "1237561663");
    form.lower_limit.value = qs.get("lower_limit", "");
    form.upper_limit.value = qs.get("upper_limit", "");
    form.size_x.value = qs.get("size_x", "");
    form.size_y.value = qs.get("size_y", "");

    updateStartStop();
  }
  
  function reset(event){
    init();
    
    //Can be not the initial ones in case of manual refresh
    form.start_epoch.value = start_epoch;
    form.stop_epoch.value = stop_epoch;
    updateStartStop();
    
    //Redraw
    scale = refreshImg();
    
    //Reset gui
    clickCounter = 0;
    initial_left = 0;
    initial_top = 0;
    
    image.onmousemove = undefined;
    form.start_iso8601.disabled = false;
    form.stop_iso8601.disabled = false;
    form.start_epoch.disabled = false;
    form.stop_epoch.disabled = false;
  }

  function refreshImg(event) {
    image.src = qs.get("cgiurl_graph", "/munin-cgi/munin-cgi-graph") + "/"
      + form.plugin_name.value 
      + "-pinpoint=" + parseInt(form.start_epoch.value) + "," + parseInt(form.stop_epoch.value)
      + ".png"
      + "?" 
      + "&lower_limit=" + form.lower_limit.value
      + "&upper_limit=" + form.upper_limit.value
      + "&size_x=" + form.size_x.value
      + "&size_y=" + form.size_y.value
    ;

    return ((+form.stop_epoch.value) - (+form.start_epoch.value)) / (+form.size_x.value);
  }

  function updateStartStop() {
    form.start_iso8601.value = new Date(form.start_epoch.value * 1000).formatDate(Date.DATE_ISO8601);
    form.stop_iso8601.value = new Date(form.stop_epoch.value * 1000).formatDate(Date.DATE_ISO8601);
  }

  function divMouseMove(event) {
    var delta_x;
    var size_x;

    // Handling the borders (X1>X2 ou X1<X2)
    var posX = getCoordinatesOnImage(event)[0];
    var current_width = posX - initial_left;
    if (current_width < 0) {
      delta_x = posX - 63; // the Y Axis is 63px from the left border
      size_x = - current_width;
    } else {
      delta_x = initial_left - 63; // the Y Axis is 63px from the left border
      size_x = current_width;
    }
    
    // Compute the epochs UNIX (only for horizontal)
    form.start_epoch.value = start_epoch + scale * delta_x;
    form.stop_epoch.value = start_epoch + scale * ( delta_x + size_x );
    
    // update !
    updateStartStop();
  }

  function startZoom(event) {
    var pos = getCoordinatesOnImage(event);
    initial_left = pos[0];
    initial_top = pos[1];
    
    // Fix the handles
    form.start_iso8601.disabled = true;
    form.stop_iso8601.disabled = true;
    form.start_epoch.disabled = true;
    form.stop_epoch.disabled = true;
    image.onmousemove = divMouseMove;
  }

  function endZoom(event) {
    image.onmousemove = undefined;
    form.start_iso8601.disabled = false;
    form.stop_iso8601.disabled = false;
    form.start_epoch.disabled = false;
    form.stop_epoch.disabled = false;
  }

  function fillDate(date, default_date) {
    return date + default_date.substring(date.length, default_date.length);
  }

  function majDates(event) {
    var default_date = "2009-01-01T00:00:00+0100";

    var start_manual = fillDate(form.start_iso8601.value, default_date);
    var stop_manual = fillDate(form.stop_iso8601.value, default_date);
    
    var dateRegex = /(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2}).(\d{4})/;
    
    if (dateRegex.test(start_manual)) {
      var date_parsed = new Date(start_manual.replace(dateRegex, "$2 $3, $1 $4:$5:$6"));
      form.start_epoch.value = date_parsed.getTime() / 1000;
    }

    if (dateRegex.test(stop_manual)) {
      var date_parsed = new Date(stop_manual.replace(dateRegex, "$2 $3, $1 $4:$5:$6"));
      form.stop_epoch.value = date_parsed.getTime() / 1000;
    }

    //form.submit();
    refreshImg();
  }

  function click(event) {
    switch ((clickCounter++) % 2) {
      case 0: 
        startZoom(event);
        break;
      case 1: 
        endZoom(event);
        break;			
    }
  }
  
  //Coordinates on image
  function findPosition(oElement){
    if(typeof( oElement.offsetParent ) != "undefined"){
      for(var posX = 0, posY = 0; oElement; oElement = oElement.offsetParent){
        posX += oElement.offsetLeft;
        posY += oElement.offsetTop;
      }
      return [ posX, posY ];
    }
    else{
      return [ oElement.x, oElement.y ];
    }
  }
  function getCoordinatesOnImage(event){
    var posX = 0;
    var posY = 0;
    var imgPos;
    imgPos = findPosition(image);
    if (!event) var event = window.event;
    if (event.pageX || event.pageY){
      posX = event.pageX;
      posY = event.pageY;
    }
    else if (event.clientX || event.clientY){
        posX = event.clientX + document.body.scrollLeft
          + document.documentElement.scrollLeft;
        posY = event.clientY + document.body.scrollTop
          + document.documentElement.scrollTop;
      }
    posX = posX - imgPos[0];
    posY = posY - imgPos[1];
    return [ posX, posY ];
  }
};
