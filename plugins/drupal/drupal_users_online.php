#!/usr/bin/php
<?php
/**
 * Drupal Users Online
 * Munin plugin to count online users
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
  echo "graph_title Drupal Online Users\n";
  echo "graph_args --base 1000 --lower-limit 0\n";
  echo "graph_vlabel Online Users Count / ${graph_period}\n";
  echo "graph_category cms\n";
  echo "graph_scale no\n";
  echo "graph_info Displays the online users, members and anonymous, in your Drupal site\n";

  echo "online_users.label online users\n";
  echo "online_members.label online members\n";
  echo "online_anonymous.label online anonymous\n";

  echo "online_users.min 0\n";
  echo "online_users.type GAUGE\n";

  echo "online_members.min 0\n";
  echo "online_members.type GAUGE\n";

  echo "online_anonymous.min 0\n";
  echo "online_anonymous.type GAUGE\n";
  exit(0);
}

// Connect to database
$dbh = new mysqli($host, $user, $pass, $db, $port);
if (mysqli_connect_errno()) {
  echo "Connecction failed: ".mysqli_connect_error(). PHP_EOL;
  exit(1);
}

// Print out the actual values
$users_online_all = (int) get_all_online_users($dbh);
$users_members = (int) get_online_registered_users($dbh);
$users_anonymous = (int) get_online_anonymous_users($dbh);
echo "online_users.value $users_online_all\n";
echo "online_members.value $users_members\n";
echo "online_anonymous.value $users_anonymous\n";

$dbh->close();


/**
 * Get count for all online users
 * @return integer $count total online users
 */
function get_all_online_users(&$dbh = NULL, $active_interval = 900) {

  $table_prefix = getenv('table_prefix');

  $sql = "SELECT COUNT(DISTINCT(uid)) AS count FROM {$table_prefix}sessions
    WHERE timestamp >= (UNIX_TIMESTAMP(now()) - ?)";
  $stmt = $dbh->prepare($sql);
  $stmt->bind_param("i", $active_interval);
  $stmt->execute();

  $count = 0;
  $stmt->bind_result($count);

  if ($stmt->fetch() === TRUE)
    return (int) $count;

  return 0;

}


/**
 * Get count for online users which are registered members
 * @return integer $count total registered users online
 */
function get_online_registered_users(&$dbh = NULL, $active_interval = 900) {

  $table_prefix = getenv('table_prefix');

  $sql = "SELECT COUNT(DISTINCT(uid)) AS count FROM {$table_prefix}sessions WHERE uid != 0
    AND timestamp >= (UNIX_TIMESTAMP(now()) - ?)";
  $stmt = $dbh->prepare($sql);

  $stmt->bind_param("i", $active_interval);
  $stmt->execute();

  $count = 0;
  $stmt->bind_result($count);

  if ($stmt->fetch() === TRUE)
    return (int) $count;

  return 0;

}


/**
 * Get count for anonymous users
 * @param object $dbh database object
 * @param integer $active_interval seconds of anonymous user activity to consider in results (defaults to 900)
 * @return integer $count total anonymous users online
 */
function get_online_anonymous_users(&$dbh = NULL, $active_interval = 900) {

  $table_prefix = getenv('table_prefix');

  /**
   * Returns anonymous users count who have been active for at least one hour
   */
  $sql = "SELECT COUNT(DISTINCT(hostname)) AS count FROM
    {$table_prefix}sessions WHERE uid = 0 AND timestamp >= (UNIX_TIMESTAMP(now()) - ?)";

  /**
   * If interested in all anonymous users count
   *
   * SELECT COUNT(DISTINCT(hostname)) AS count FROM sessions WHERE uid = 0
   */

  $stmt = $dbh->prepare($sql);

  $stmt->bind_param("i", $active_interval);
  $stmt->execute();

  $count = 0;
  $stmt->bind_result($count);

  if ($stmt->fetch() === TRUE)
    return (int) $count;

  return 0;

}
