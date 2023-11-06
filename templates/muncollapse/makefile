# MunCollapsible
#
# Makefile for the MunCollapsible template.

# HTTP Fetch program to use.  Called as $(HTTP_FETCH) <output_file> <input_url>
#
# Swap the comments to use curl instead.
HTTP_FETCH = wget -nv -O
#HTTP_FETCH = curl -s -w "%{http_code} - %{filename_effective} - %{size_download} bytes\n" -o


# The versions of the external libraries to use.
BOOTSTRAP_VERSION = 4.4.1
JQUERY_VERSION    = 3.4.1.slim
TYPEAHEAD_VERSION = latest

FAVICON_FILES  := android-chrome-144x144.png android-chrome-192x192.png android-chrome-36x36.png android-chrome-48x48.png android-chrome-72x72.png android-chrome-96x96.png apple-touch-icon-114x114.png apple-touch-icon-120x120.png apple-touch-icon-144x144.png apple-touch-icon-152x152.png apple-touch-icon-180x180.png apple-touch-icon-57x57.png apple-touch-icon-60x60.png apple-touch-icon-72x72.png apple-touch-icon-76x76.png apple-touch-icon-precomposed.png apple-touch-icon.png browserconfig.xml favicon-16x16.png favicon-194x194.png favicon-32x32.png favicon-96x96.png favicon.ico manifest.json mstile-144x144.png mstile-150x150.png mstile-310x150.png mstile-310x310.png mstile-70x70.png
FAVICON_BASEURL = https://raw.githubusercontent.com/munin-monitoring/munin/master/web/static/img/favicons/

LOGO_URL = https://raw.githubusercontent.com/munin-monitoring/munin/master/web/static/img/logo-h.png

BOOTSTRAP_CSS_URL = https://stackpath.bootstrapcdn.com/bootstrap/$(BOOTSTRAP_VERSION)/css/bootstrap.min.css
BOOTSTRAP_JS_URL  = https://stackpath.bootstrapcdn.com/bootstrap/$(BOOTSTRAP_VERSION)/js/bootstrap.bundle.min.js
JQUERY_JS_URL     = https://code.jquery.com/jquery-$(JQUERY_VERSION).min.js
TYPEAHEAD_JS_URL  = https://twitter.github.io/typeahead.js/releases/$(TYPEAHEAD_VERSION)/typeahead.bundle.js
LAZYSIZES_URL     = http://afarkas.github.io/lazysizes/lazysizes.min.js

CWD := $(abspath $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST))))))


main:
	@echo $(CWD)
	@echo Downloading External Libraries...

	$(HTTP_FETCH) $(CWD)/static/css/bootstrap.min.css $(BOOTSTRAP_CSS_URL)
	$(HTTP_FETCH) $(CWD)/static/js/bootstrap.min.js $(BOOTSTRAP_JS_URL)
	$(HTTP_FETCH) $(CWD)/static/js/jquery.min.js $(JQUERY_JS_URL)
	$(HTTP_FETCH) $(CWD)/static/js/typeahead.bundle.min.js $(TYPEAHEAD_JS_URL)
	$(HTTP_FETCH) $(CWD)/static/js/lazysizes.min.js $(LAZYSIZES_URL)

	@echo Downloading Logo...
	@mkdir -p $(CWD)/static/img
	$(HTTP_FETCH) $(CWD)/static/img/logo-munin.png $(LOGO_URL)

	@echo Downloading Favicon Files...

	@mkdir -p $(CWD)/static/img/favicon
	@for file in ${FAVICON_FILES}; do \
		eval $(HTTP_FETCH) $(CWD)/static/img/favicon/$${file} $(FAVICON_BASEURL)$${file}; \
	done



	