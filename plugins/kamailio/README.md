# Kamailio Munin Plugin
Munin plugins for Kamailio. It monitors:
## Number of transactions, user and contact numbers.
![kamailio_transaction](http://desmond.imageshack.us/Himg820/scaled.php?server=820&filename=kamailiotransactionsuse.png&res=landing "kamailio_transaction")

##  Usage of shared memory (total, used and real used).
![kamailio_shared_memory](http://desmond.imageshack.us/Himg837/scaled.php?server=837&filename=kamailiomysqlsharedmemo.png&res=landing "kamailio_shared_memory")

##  Memory usage by Kamailio, Freeswitch and RTPproxy.
![kamailio_memory](http://desmond.imageshack.us/Himg851/scaled.php?server=851&filename=kamailiomemoryweek.png&res=landing "kamailio_memory")

it requires MySQL [statistics] (http://siremis.asipto.com/install-charts-panel) table created in Kamailio database.

## Configuration
edit /etc/munin/plugin-conf.d/munin-node and add:

      [kamailio*]
      user root
      group root
      env.mysql [optional-override-of-mysqladmin-path]
      env.mysqlauth -u[User] -p[Password]
      env.kamailiodb [kamailio data base]

