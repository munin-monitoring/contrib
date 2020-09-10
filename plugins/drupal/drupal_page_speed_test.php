#!/usr/bin/php
<?php
/**
 * Drupal Page Speed Test
 * Munin plugin to time a page speed return from the server
 *
 * It's required to define a container entry for this plugin in your
 * /etc/munin/plugin-conf.d/munin-node (or a separate and dedicated file).
 *
 * @example Example entry for configuration:
 * [drupal*]
 * env.url_login http://www.example.com
 *
 * // Username and password should be set using key=value where key is the form input name and value is its value
 * env.url_login_user signInName=admin
 * env.url_login_pass password=admin
 *
 * // You may specify other key=value fields which may be required when submitting the form's POST action to perform the login
 * env.url_login_post_fields target=http://www.example.com
 *
 * // Each page to test should be specified in the format of <label>=<url> where label is alphanumeric with no spaces and url is the page URL to test
 * // You may specify multiple pages to time their page load by separating them with commas ,
 * env.urls label_page1=http://www.example.com/page1.php,label_page2=http://www.example.com/page2.php
 * env.cookie_file /tmp/cookie.txt
 *
 * @author Liran Tal <liran.tal@hp.com>
 * @version 1.1 2013
 *
 */


/**
 * Environment variabels are set by munin's configuration
 * @see /etc/munin/plugin-conf.d/munin-node
 */
$graph_period = getenv('graph_period');

$settings['url_login'] = getenv('url_login');
$settings['url_login_user'] = getenv('url_login_user');
$settings['url_login_pass'] = getenv('url_login_pass');
$settings['url_login_post_fields'] = getenv('url_login_post_fields');
$settings['urls'] = getenv('urls');

$urls_tmp = explode(',', $settings['urls']);
$settings['test_pages'] = array();
foreach ($urls_tmp as $url) {
  $url_info = explode('=', $url);
  if (!empty($url_info[0]) && !empty($url_info[1]))
    $settings['test_pages'][$url_info[0]] = $url_info[1];
}

$cookie = getenv('cookie_file');
if (empty($cookie))
  $cookie = '/tmp/cookie.txt';

$debug = FALSE;


if(count($argv) == 2 && $argv[1] == 'autoconf') {
  echo "yes\n";
  exit(0);
}

if (count($argv) === 2 && $argv[1] === 'config') {
  echo "graph_title Drupal Page Speed Test\n";
  echo "graph_args --base 1000 --lower-limit 0\n";
  echo "graph_vlabel Page Load Time / ${graph_period}\n";
  echo "graph_category cms\n";
  echo "graph_scale no\n";
  echo "graph_info Displays the time it took for several pages to load\n";

  foreach ($settings['test_pages'] as $label => $value) {
    echo "$label.label $label\n";
    echo "$label.type GAUGE\n";
    echo "$label.min 0\n";
  }

  if (!empty($settings['url_login'])) {
    echo "login.label login\n";
    echo "login.type GAUGE\n";
    echo "login.min 0\n";
  }

  exit(0);
}


// General curl settings
$ch = curl_init();
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);

curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, false);
curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
curl_setopt($ch, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.1.6) Gecko/20070725 Firefox/2.0.0.6");
curl_setopt($ch, CURLOPT_TIMEOUT, 60);
curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 60);

curl_setopt($ch, CURLOPT_COOKIEFILE, $cookie);
curl_setopt($ch, CURLOPT_COOKIEJAR, $cookie);

if (!empty($settings['url_login'])) {
  $time_login = perform_login();
  echo "login.value $time_login\n";
}

foreach ($settings['test_pages'] as $label => $value) {
  $page_time = perform_get_page($value);
  echo "$label.value $page_time\n";
}

curl_close($ch);




function perform_get_page($url) {

  global $ch;
  global $settings;
  global $debug;

  $options['url'] = $url;

  if ($debug)
    print 'Getting: ' . $options['url'] . PHP_EOL;

  curl_setopt($ch, CURLOPT_URL, $options['url']);

  $start = microtime(true);
  $result = curl_exec($ch);
  $end = microtime(true);

  if ($debug)
    print $result;

  $time = $end - $start;

  if ($debug)
    print $time . PHP_EOL;

  return $time;

}


function perform_login() {

  global $ch;
  global $debug;
  global $settings;

  // define post fields
  $user_info = explode('=', $settings['url_login_user']);
  $pass_info = explode('=', $settings['url_login_pass']);

  $tmp = explode(',', $settings['url_login_post_fields']);
  $post_fields_extra = array();
  foreach ($tmp as $tmp_post_field) {
    $post_field_info = explode('=', $tmp_post_field);
    if (!empty($post_field_info[0]) && !empty($post_field_info[1]))
      $post_fields_extra[$post_field_info[0]] = $post_field_info[1];
  }

  // Add the username and password info
  $post_fields_extra[$user_info[0]] = $user_info[1];
  $post_fields_extra[$pass_info[0]] = $pass_info[1];

  $return_url = "";

  $post_fields = $post_fields_extra;

  $options['url'] = $settings['url_login'];
  $options['post_fields'] = http_build_query($post_fields);

  if ($debug)
    print 'Getting: ' . $options['url'] . PHP_EOL;

  // if post fields provided, set the post fields data
  if (isset($options['post_fields']))
  {
    curl_setopt($ch, CURLOPT_POSTFIELDS, $options['post_fields']);
    curl_setopt($ch, CURLOPT_POST, TRUE);
  }

  curl_setopt($ch, CURLOPT_URL, $options['url']);

  $start = microtime(true);
  $result = curl_exec($ch);
  $end = microtime(true);

  if ($debug)
    print $result;

  $time = $end - $start;

  if ($debug)
    print $time . PHP_EOL;

  return $time;

}
