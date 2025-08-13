# Prosody plugins

For now, we have 3 plugins for Prosody:

- `prosody_openmetrics`: For Prosody version >= `0.12`, using `mod_http_openmetrics`
- `prosody_0.12_`: For Prosody version = `0.12`, using `mod_admin_telnet`
- `prosody_`: For Prosody versions < `0.12`, using `mod_admin_telnet`

## prosody_openmetrics (for Prosody version >= 0.12)

OpenMetrics plugin to monitor a [Prosody](https://prosody.im) server.

This plugin provides graph `c2s`, `s2s`, `presence`, `http_file_share`, `active_users`, `client_features`, `client_identities`, `cpu`, `memory`, `muc`, `message_encryption_status`.

### Install

- Install Python3 package `prometheus-client`
- Enable [`mod_http_openmetrics`](https://prosody.im/doc/modules/mod_http_openmetrics) Prosody module
- Enable Prosody modules required for some monitoring modules
- Link this plugin in your munin plugins directory

```bash
ln -s /usr/share/munin/plugins/prosody_openmetrics /etc/munin/plugins/prosody_openmetrics
```

After the installation you need to restart your munin-node service.

### Configuration

Some modules need specific Prosody modules to be activated:

- `active_users`: [`mod_measure_active_users`](https://modules.prosody.im/mod_measure_active_users.html) Prosody module
- `cpu`: [`mod_measure_cpu`](https://modules.prosody.im/mod_measure_cpu.html) Prosody module
- `client_features`: [`measure_client_features`](https://modules.prosody.im/mod_measure_client_features.html) Prosody module
- `client_identities`: [`measure_client_identities`](https://modules.prosody.im/mod_measure_client_identities.html) Prosody module
- `memory`: [`mod_measure_memory`](https://modules.prosody.im/mod_measure_memory.html) Prosody module
- `message_encryption_status`: [`mod_measure_message_e2ee`](https://modules.prosody.im/mod_measure_message_e2ee.html) Prosody module
- `presence`: [`mod_measure_client_presence`](https://modules.prosody.im/mod_measure_client_presence.html) Prosody module
- `http_file_share`: [`mod_http_file_share`](https://prosody.im/doc/modules/mod_http_file_share) Prosody module

You need to create a file named `prosody_openmetrics` placed in the directory `/etc/munin/plugin-conf.d/` with the following config:

```
[prosody_openmetrics]
env.metrics_url http://localhost:5280/metrics
env.modules c2s,s2s,presence,http_file_share,active_users,client_features,client_identities,cpu,memory,muc,message_encryption_status
env.hosts example.com,file.example.com
```

## prosody_0.12_ (for Prosody version = 0.12)

See `prosody_` instructions below.

This plugin is the same as `prosody_` but with new Prosody `0.12` output format.

## prosody_ (for Prosody version < 0.12)

`prosody_` is a plugin for the monitoring software `munin <http://http://munin-monitoring.org/>`_ to monitor a `Prosody <http://prosody.im>`_ xmpp server (version < `0.12`).

This wildcard plugin provided at the moment only the `c2s`, `s2s`, `presence`, `uptime` and `users` suffixes.

### Install

It is very simple to install the plugin.

```
chmod 755 prosody_
ln -s /usr/share/munin/plugins/prosody_ /etc/munin/plugins/prosody_c2s
ln -s /usr/share/munin/plugins/prosody_ /etc/munin/plugins/prosody_s2s
ln -s /usr/share/munin/plugins/prosody_ /etc/munin/plugins/prosody_presence
ln -s /usr/share/munin/plugins/prosody_ /etc/munin/plugins/prosody_uptime
ln -s /usr/share/munin/plugins/prosody_ /etc/munin/plugins/prosody_users
```

After the installation you need to restart your munin-node.

### Configuration

When you want to change the default host (localhost) and port (5582) than you can change this in the **/etc/munin/plugin-conf.d/munin-node** config file like this:

```
[prosody_*]
env.host example.com
env.port 5582
```

If you want to get the number of registered users, add the following lines to **/etc/munin/plugin-conf.d/munin-node**:

```
[prosody_users]
user prosody
group prosody
```
