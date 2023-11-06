NFS plugins for FreeBSD
------------------------

The NFS plugins for Munin depend on the /proc filesystem in Linux for
statistics which does not exist in FreeBSD by default. While one can
add the linproc package to their installation to emulate this
functionality, I felt it would be better to write plugins that made use
of native tools.

I'm leveraging the nfsstat command to create these plugins. I'm sure
there is a better primary data source in FreeBSD to gather this output
from but for my purposes, using nfsstat is adequate and acceptable.
Unfortunately this means if the output of the nfsstat command changes
due to updates, these plugins will likely report inaccurate data.
These are the days of our lives, as they say. Someone with appropriate
levels of FreeBSD knowledge can probably improve these plugins
trivially.

I'm specifically targeting FreeNAS/NAS4Free, but the code should work
on any FreeBSD system with minimal package additions. Installing munin
on FreeNAS is not supported in general, but it's easy enough to do and
I wanted it monitored, ergo we are here.
