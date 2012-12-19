-------- JMX plugin for Munin ---------

Java JMX Munin plugin enables you to monitor JMX attributes in Munin.
As soon as JMX embedded in Java 5, any Java process may expose parameters to be monitored using JMX interface,
look http://java.sun.com/j2se/1.5.0/docs/guide/management/agent.html and http://java.sun.com/jmx for details
In Java version < 5 it is still possible to expose JMX interface using third party libraries

To see what can be monitored by JMX, run <JDK>/bin/jconsole.exe and connect to 
the host/port you setup in your Java process.

Some examples are:
* standard Java JMX implementation exposes memory, threads, OS, garbage collector parameters
* Tomcat exposes multiple parameters - requests, processing time, threads, etc..
* spring framework allows to expose Java beans parameters to JMX
* your application may expose any attributes for JMX by declaration or explicitly.
* can monitor localhost or remote processes

-------- Installation ---------

Pre-requsisites are:
- installed munin-node
- Java version 5 JRE

1) Files from "plugin" folder must be copied to /usr/share/munin/plugins (or another - where your munin plugins located)
2) Make sure that jmx_ executable : chmod a+x /usr/share/munin/plugins/jmx_
3) Copy configuration files that you want to use, from "examples" folder, into /usr/share/munin/plugins folder
4) create links from the /etc/munin/plugins folder to the /usr/share/munin/plugins/jmx_
The name of the link must follow wildcard pattern:
jmx_<configname>,
where configname is the name of the configuration (config filename without extension), for example:
ln -s /usr/share/munin/plugins/jmx_ /etc/munin/plugins/jmx_process_memory
5) optionally specify the environment variable for JMX URL. The default URL corresponds to localhost:1616.
If you have different port listening by JMX or different hostname to monitor, specify jmxurl parameter
in  /etc/munin/plugin-conf.d/munin-node:

[jmx_*]
env.jmxurl service:jmx:rmi:///jndi/rmi://localhost:1616/jmxrmi

-------- Check Installation ---------

To check that all installed properly, try invoke plugins from command line, using links like:

root@re:/etc/munin/plugins# ./jmx_java_process_memory config
graph_category Java
...
root@re:/etc/munin/plugins# ./jmx_java_process_memory
java_memory_nonheap_committed.value 35291136
...

If you have configured environment for jmxurl, do not forget to export it before!

-------- Configuration Files ---------

Folder "examples" contains configuration files for Java and Tomcat monitoring examples.
The format of configuration file is a superset of Munin plugin "config" command output
(http://munin.projects.linpro.no/wiki/protocol-config)
It has the following additions:

<fieldname>.jmxObjectName JMX object name, e.g. java.lang:type=Memory
<fieldname>.jmxAttributeName JMX attribute name, e.g. NonHeapMemoryUsage
<fieldname>.jmxAttributeKey If attribute is a composed data (structure), the name of the field in structure, e.g. max

% separates comments in file



