/*
 * Sanitize all tab links
 */
$("ul#tabs>li>a").each(function (index) {
    var eid = $(this).attr('href').replace(/[^#\w]/gi, '_');
    $(this).attr('href', eid);
});

/*
 * Sanitize all tab ids
 */
$("div#munin_nodeview_tab>div").each(function (index) {
    var eid = $(this).attr('id').replace(/[^\w]/gi, '_');
    $(this).attr('id', eid);
});

/*
 * Update the URL with selected tab and active selected tab on page refresh
 */
(function () {
    'use strict';

    $('a[data-toggle="tab"]').on('shown.bs.tab', function (e) {
        var id = $(this).parents('[role="tablist"]').attr('id');
        var key = 'lastTag';
        if (id) {
            key += ':' + id;
        }

        localStorage.setItem(key, $(e.target).attr('href'));
        location.hash = $(e.target).attr('href');
    });

    $('[role="tablist"]').each(function (idx, elem) {
        var id = $(elem).attr('id');
        var key = 'lastTag';
        if (id) {
            key += ':' + id;
        }

        var lastTab = localStorage.getItem(key);
        if (!lastTab) {
            lastTab = location.hash;
        }
        if (lastTab) {
            $('[href="' + lastTab + '"]').tab('show');
        }
    });
})();