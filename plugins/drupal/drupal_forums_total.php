#!/usr/bin/php
<?php
/**
 * Drupal Forums and Comments
 * Munin plugin to count total forums and comments
 *
 * It's required to define a container entry for this plugin in your
 * /etc/munin/plugin-conf.d/munin-node (or a separate and dedicated file).
 *
 * @example Example entry for configuration:
 * [drupal*]
 * env.db drupal
 * env.user mysql_user
 * env.pass mysql_pass
 * env.host localhost
 * env.port 3306
 * env.table_prefix drupal_
 *
 * @author Liran Tal <liran.tal@hp.com>
 * @version 1.0 2013
 *
 */


/**
 * Environment variabels are set by munin's configuration
 * @see /etc/munin/plugin-conf.d/munin-node
 */
$db = getenv('db');
$host = getenv('host');
$user = getenv('user');
$pass = getenv('pass');
$port = getenv('port');
if (!$port)
  $port = 3306;

$graph_period = getenv('graph_period');

if(count($argv) == 2 && $argv[1] == 'autoconf') {
  echo "yes\n";
  exit(0);
}

if (count($argv) === 2 && $argv[1] === 'config') {
  echo "graph_title Drupal Total Forums\n";
  echo "graph_args --base 1000 --lower-limit 0\n";
  echo "graph_vlabel Total Forums and comments Count / ${graph_period}\n";
  echo "graph_category cms\n";
  echo "graph_scale no\n";
  echo "graph_info Displays the sum of forums and comments made in your Drupal site\n";

  echo "forums_total.label total forums\n";
  echo "forums_total.type GAUGE\n";

  echo "comments_total.label total comments\n";
  echo "comments_total.type GAUGE\n";

  echo "forums_total.min 0\n";
  echo "comments_total.min 0\n";

  exit(0);
}

// Connect to database
$dbh = new mysqli($host, $user, $pass, $db, $port);
if (mysqli_connect_errno()) {
  echo "Connecction failed: ".mysqli_connect_error(). PHP_EOL;
  exit(1);
}

// Print out the actual values
$forums_total = (int) get_forums_total($dbh);
$comments_total = (int) get_comments_total($dbh);
echo "forums_total.value $forums_total\n";
echo "comments_total.value $comments_total\n";

$dbh->close();


/**
 * Get count for all forums
 * @return integer $count total forums content type nodes created
 */
function get_forums_total(&$dbh = NULL) {

  $table_prefix = getenv('table_prefix');

  $sql = "SELECT COUNT(nid) AS count FROM {$table_prefix}node WHERE type = 'forum'";
  $result = $dbh->query($sql);
  $row = $result->fetch_assoc();

  return (int) $row['count'];

}

/**
 * Get count for all comments made through-out the site for a 'forum' node content type
 * @return integer $count all comments
 */
function get_comments_total(&$dbh = NULL) {

  $table_prefix = getenv('table_prefix');

  $sql = "SELECT COUNT(c.cid) AS count FROM {$table_prefix}comments c JOIN {$table_prefix}node n ON c.nid = n.nid WHERE n.type = 'forum'";
  $result = $dbh->query($sql);
  $row = $result->fetch_assoc();

  return (int) $row['count'];

}
