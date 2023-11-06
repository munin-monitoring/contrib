#!/usr/bin/php
<?php
/**
 * Drupal Files Total
 * Munin plugin to count total files uploads
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
  echo "graph_title Drupal Total Files\n";
  echo "graph_args --base 1000 --lower-limit 0\n";
  echo "graph_vlabel Files Count / ${graph_period}\n";
  echo "graph_category cms\n";
  echo "graph_scale no\n";
  echo "graph_info Displays the online files uploaded to your Drupal site\n";

  echo "files_total.label total files\n";
  echo "files_total.type GAUGE\n";
  echo "files_total.min 0\n";

  exit(0);
}

// Connect to database
$dbh = new mysqli($host, $user, $pass, $db, $port);
if (mysqli_connect_errno()) {
  echo "Connecction failed: ".mysqli_connect_error(). PHP_EOL;
  exit(1);
}

// Print out the actual values
$total_files = (int) get_all_files($dbh);
echo "files_total.value $total_files\n";

$dbh->close();


/**
 * Get count for all online users
 * @return integer $count total online users
 */
function get_all_files(&$dbh = NULL) {

  $table_prefix = getenv('table_prefix');

  $sql = "SELECT COUNT(fid) AS count FROM {$table_prefix}files";
  $result = $dbh->query($sql);
  $row = $result->fetch_assoc();

  return (int) $row['count'];

}

