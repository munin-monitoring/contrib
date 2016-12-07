/*
 * Sanitize all tab links
 */
$( "ul#tabs>li>a" ).each(function( index ) {
    var eid = element_clear($( this ).attr('href')).replace(/[^#\w]/gi,'_');
    $( this ).attr('href', eid);
});

/*
 * Sanitize all tab ids
 */
$( "div#munin_nodeview_tab>div" ).each(function( index ) {
    var eid = element_clear($( this ).attr('id')).replace(/[^\w]/gi,'_');
    $( this ).attr('id', eid);
});

/*
 * Update the URL with selected tab and active selected tab on page refresh
 */
$(document).ready(function() {
    if(location.hash) {
        $('a[href="' + location.hash + '"]').tab('show');
    }
    $(document.body).on("click", "a[data-toggle=tab]", function(event) {
        location.hash = this.getAttribute("href");
    });
});
$(window).on('popstate', function() {
    var anchor = location.hash || $("a[data-toggle=tab]").first().attr("href");
    $('a[href="' + location.hash + '"]').tab('show');
});