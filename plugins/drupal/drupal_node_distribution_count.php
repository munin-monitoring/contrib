#!/usr/bin/php
<?php
/**
 * Drupal Node Distribution Count
 * Munin plugin to count node content type usage across a Drupal site
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

$host = $host.':'.$port;

// Connect to database
mysql_connect($host, $user, $pass) or die ("Could not connect to database: ".
  mysql_error());
mysql_select_db($db) or die ("Could not select database: ".mysql_error());

// Get all node types (required for configuration information too)
$node_types = get_all_node_types_count();

if (count($argv) === 2 && $argv[1] === 'config') {
  echo "graph_title Drupal Node Distribution Count\n";
  echo "graph_args --base 1000 --lower-limit 0\n";
  echo "graph_vlabel Node Distribution Count / ${graph_period}\n";
  echo "graph_category cms\n";
  echo "graph_scale nol\n";
  echo "graph_info Displays the nodes content type distribution count in your Drupal site\n";

  foreach ($node_types as $node_type => $node_count) {
    echo "$node_type.label $node_type\n";
    echo "$node_type.type GAUGE\n";
    echo "$node_type.draw LINE2\n";
    echo "$node_type.min 0\n";
  }

  exit(0);
}


// Print out the actual values for each node type and it's count
//$node_count_all = (int) get_all_node_count();
//echo "nodes_total.value $node_count_all\n";
foreach ($node_types as $node_type => $node_count) {
  echo "$node_type.value $node_count\n";
}


/**
 * Get count for all nodes
 * @return integer $count total nodes created
 */
function get_all_node_count() {

  $table_prefix = getenv('table_prefix');
  $node_count = 0;

  // Get all node types and their count
  $result = mysql_query("SELECT COUNT(nid) AS count FROM {$table_prefix}node");
  $row = mysql_fetch_array($result, MYSQL_ASSOC);
  $node_count = (int) $row['count'];

  mysql_free_result($result);
  return $node_count;

}


/**
 * Get all node types and their count
 * @return array $node_types associative array of node types and their count
 */
function get_all_node_types_count() {

  $node_types = array();
  $table_prefix = getenv('table_prefix');

  // Get all node types and their count
  $result = mysql_query("SELECT COUNT(nid) AS count, type FROM
    {$table_prefix}node GROUP BY type");
  while ($row = mysql_fetch_array($result, MYSQL_ASSOC)) {
    if (empty($row['type']))
      continue;
    $node_types[$row['type']] = $row['count'];
  }

  mysql_free_result($result);
  return $node_types;

}

