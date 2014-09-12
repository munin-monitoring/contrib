#!/usr/bin/php
<?php
/**
 * Moodle Users Online
 * Munin plugin to count online users
 *
 * It's required to define a container entry for this plugin in your
 * /etc/munin/plugin-conf.d/munin-node (or a separate and dedicated file).
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
$db = getenv('db');
$type = getenv('type');
$host = getenv('host');
$user = getenv('user');
$pass = getenv('pass');
$port = getenv('port');
if (!$port)
    $port = 3306;