<?php
/**
 * Part of Munin PHP OPcache plugin - Refer to php_opcache for installation instructions.
 */

if (function_exists('opcache_get_status')) 
{
	$data = opcache_get_status(false);
	$output = array(
		'mem_used.value' => $data['memory_usage']['used_memory'],
		'mem_free.value' => $data['memory_usage']['free_memory'],
		'mem_wasted.value' => $data['memory_usage']['wasted_memory'],
	);
}
else
{
	// OPCache not installed :(
	$output = array(
		'mem_used.value' => 0,
		'mem_free.value' => 0,
	);
}

header('Content-Type: text/plain');
foreach ($output as $key => $value)
{
	echo $key, ' ', $value, "\n";
}
?>
