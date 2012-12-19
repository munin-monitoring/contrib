These [munin](http://munin-monitoring.org) plugins allow you to graph
various things happening in a [Freenet](http://freenetproject.org/)
node. To run them you need perl 5.10.0 or later and
[Net::FCP::Tiny](http://search.cpan.org/dist/Net-FCP-Tiny/) from the
CPAN.

They all talk to `localhost:9481` unless you specify otherwise with
the `FREENET_HOST` and `FREENET_PORT` environmental variables in the
`munin-node` configuration file, e.g.:

    [freenet_*]
    env.FREENET_HOST freenet.example.net
    env.FREENET_PORT 1337

These plugins are maintained
[on GitHub](http://github.com/avar/munin-plugin-freenet) by
[Ævar Arnfjörð Bjarmason](http://github.com/avar/munin-plugin-freenet),
if you need something else graphed hack it up and submit a pull
request.

This software is available under the terms of the
[DWTFYWWI](http://github.com/avar/DWTFYWWI/blob/master/DWTFYWWI)
license.
