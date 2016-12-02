#!/usr/bin/perl

use strict;
use NetSNMP::OID;
use NetSNMP::ASN (':all');
use NetSNMP::agent (':all');
use IO::Socket;

my %cache = ();         # Cache
my @cache_oids = ();    # Keys, sorted
my $cache_updated = 0;
my $oidbase = ".1.3.6.1.4.1.123456.100.1.1";
my $delimiter = ' = ';

# initialize
my %Munin;
$Munin{PORT} = '4949';
$Munin{HOST} = 'localhost';

# See munin plugins dir for more plugins
our @munin_plugins= qw ( load swap users uptime vmstat df );

# Update cache
sub update_stats {
    return if time() - $cache_updated < 30;
    %cache = ();

    foreach my $plugin (@munin_plugins) {
        $plugin =~ s/^.*\///;
        my $DATA  = munin_fetch($plugin);
        foreach my $line (@$DATA) {
            # Extract name and value
            next if $line !~ m/^(.*)\s=\s(.*)$/xms;
            my ($name,$value) = ($1,$2);
            # Compute OID
            my $oid = "$oidbase";
            foreach my $char (split //, $name) {
                $oid .= ".";
                $oid .= ord($char);
            }
            # Put in the cache
            $cache{$oid} = $value;
        }
    }
    @cache_oids = sort { new NetSNMP::OID($a) <=> new NetSNMP::OID($b) } (keys %cache);
    $cache_updated = time();
}

# Handle request
sub handle_stats {
    my ($handler, $registration_info, $request_info, $requests) = @_;
    update_stats;       # Maybe we should do this in a thread...
    for (my $request = $requests; $request; $request = $request->next()) {
        $SNMP::use_numeric = 1;
        my $oid = $request->getOID();
        my $noid=SNMP::translateObj($oid);
        if ($request_info->getMode() == MODE_GET) {
            # For a GET request, we just check the cache
            if (exists $cache{$noid}) {
                $request->setValue(ASN_OCTET_STR, "$cache{$noid}");
            }
        } 
        elsif ($request_info->getMode() == MODE_GETNEXT) {
            # For a GETNEXT, we need to find a best match. This is the
            # first match strictly superior to the requested OID.
            my $bestoid = undef;
            foreach my $currentoid (@cache_oids) {
                $currentoid = new NetSNMP::OID($currentoid);
                next if $currentoid <= $oid;
                $bestoid = $currentoid;
                last;
            }
            if (defined $bestoid) {
                $SNMP::use_numeric = 1;
                my $noid=SNMP::translateObj($bestoid);
                $request->setOID($bestoid);
                $request->setValue(ASN_OCTET_STR, "$cache{$noid}");
            }
        }
    }
}

my $agent = new NetSNMP::agent(
    'Name' => "munin",
    'AgentX' => 1);

# Register MIB
$agent->register("munin-stats", $oidbase,
    \&handle_stats) or die "registration of handler failed!\n";

# Main loop
$SIG{'INT'} = \&shutdown;
$SIG{'QUIT'} = \&shutdown;
my $running = 1;
while ($running) {
    $agent->agent_check_and_process(1);
}
$agent->shutdown();

sub shutdown {
    # Shutdown requested
    $running = 0;
}

#muninwalk
my @collect;
sub munin_fetch {
    my $SOCKET = IO::Socket::INET -> new (  PeerAddr => $Munin{HOST},
        PeerPort => $Munin{PORT},
        Proto   => 'tcp',
        Timeout => '10',
        Type    => SOCK_STREAM,
    ) or die "Cannot create socket - $@\n";
    my $tmp = <$SOCKET>;
    while (my $fetch = shift(@_)) {
        my $obj = '';
        if ($fetch =~ /^(\w+)(\.)(\w+)$/ ) {
            ($fetch,$obj) = ($1,$3);
        }
        $SOCKET->print("fetch $fetch\n");
        select($SOCKET);
        select(STDOUT);
        while (<$SOCKET> ) {
            chomp;
            $_ =~ s/(.value)//;
            if ($_ =~ /^(.*)(\s)(.+)$/) {
                if (($1 eq '# Unknown') or ($1 eq '# Bad')) {
                    push (@collect,$fetch.$delimiter.'NULL');
                } else {
                    if (!$obj or ($1 eq $obj)) {
                        push (@collect,$fetch.".".$1.$delimiter.$3);
                    }
                }
            } else {
                last;
            }
        }
    }
    $SOCKET->print("quit\n");
    $SOCKET->close();
    return \@collect if @collect;
}

=head1 NAME

munin2snmp - SNMP Agent to query munin-node over snmp

=head1 REQUIREMENTS

Net::SNMP and IO::Socket perl modules, munin-node with some plugins

=head2 Example configuration

/etc/snmp/snmpd.conf

  master       agentx
  agentAddress udp:127.0.0.1:161
  rocommunity  public 127.0.0.1

On a newer system it is enough to define "master" option only

MUNIN-MIB should be installed on the client,
it goes to /usr/local/share/snmp/mibs or /usr/share/munin/mibs
or another place where snmpd expects to find the MIB files.

See also http://www.net-snmp.org/wiki/index.php/FAQ:MIBs_03


=head2 Usage

After setting up snmpd, start the agent:

  ./munin2snmp.pl

Now one can query the agent

  snmpwalk -v 2c -mMUNIN-MIB  -c public localhost .1.3.6.1.4.1.123456.100.1.1

where "1.3.6.1.4.1.123456.100.1.1" is example OID selected as the base
tree for the agent.

You might need to change the host, port, oidbase and munin_plugins you want to use.

The defaults:

  $Munin{PORT} = '4949';
  $Munin{HOST} = 'localhost'
  $oidbase = ".1.3.6.1.4.1.123456.100.1.1"
  @munin_plugins = qw ( load swap users uptime vmstat df );

=head1 ACKNOWLEDGEMENTS

Heavily inspired by
Vincent Bernat: https://github.com/vincentbernat/extend-netsnmp
and Masahito Zembutsu: https://github.com/zembutsu/muninwalk

=head1 AUTHOR

Alex Mestiashvili <mailatgoogl@gmail.com>
 
=head1 LICENSE

GPLv2


