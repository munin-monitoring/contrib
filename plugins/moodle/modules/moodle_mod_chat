#!/usr/bin/php
<?php
/**
 * Moodle module chat
 * Munin plugin to count users in chat sessions
 *
 * It's required to define a container entry for this plugin in your
 * /etc/munin/plugin-conf.d/moodle.conf
 *
 * @example Example entry for configuration:
 * [moodle*]
 * env.type mysql
 * env.db moodle
 * env.user mysql_user
 * env.pass mysql_pass
 * env.host localhost
 * env.port 3306
 * env.table_prefix mdl_
 *
 * @author Arnaud Trouvé <ak4t0sh@free.fr>
 * @version 1.0 2014
 *
 */
$dbh = null;
$db = getenv('db');
$type = getenv('type');
$host = getenv('host');
$user = getenv('user');
$pass = getenv('pass');
$table_prefix = getenv('table_prefix');
$port = getenv('port');
if (!$port)
    $port = 3306;
$graph_period = time() - 5*60;


if (count($argv) === 2 && $argv[1] === 'config') {
    echo "graph_title Moodle Chat Users\n";
    echo "graph_args --base 1000 --lower-limit 0\n";
    echo "graph_vlabel users\n";
    echo "graph_category Moodle\n";
    echo "graph_scale no\n";
    echo "graph_info Displays the number of users connected and posting message in chat sessions\n";
    echo "chat_users_connected.label users connected\n";
    echo "chat_users_connected.min 0\n";
    echo "chat_users_active.label users posting messages\n";
    echo "chat_users_active.min 0\n";
    exit(0);
}

try {
    $dsn = $type . ':host=' . $host . ';port=' . $port . ';dbname=' . $db;
    $dbh = new PDO($dsn, $user, $pass);
} catch (Exception $e) {
    echo "Connection failed\n";
    exit(1);
}

//Connected
$nb=0;
if (($stmt = $dbh->query("SELECT COUNT(DISTINCT userid) FROM {$table_prefix}chat_users WHERE lastping > $graph_period")) != false) {
    $nb = $stmt->fetchColumn();
}
echo "chat_users_connected.value $nb\n";

//Active
$nb=0;
if (($stmt = $dbh->query("SELECT COUNT(DISTINCT userid) FROM {$table_prefix}chat_users WHERE lastmessageping > $graph_period")) != false) {
    $nb = $stmt->fetchColumn();
}
echo "chat_users_active.value $nb\n";
