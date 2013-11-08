#!/usr/bin/perl -w
#
# Munin plugin for CSF: ConfigServer Firewall
# This plugin graphs your csf.allow and csf.deny data
#
#---------------------
# Usage:
#   Copy CSF_.pl to /usr/share/munin/plugins
#   chmod +x /usr/share/munin/plugins/CSF_.pl
# 	Create a symbolic link to CSF_<allow|deny>_<reseller|country|service|cluster>
#		ln -s /usr/share/munin/plugins/CSF_ /etc/munin/plugins/CSF_allow_reseller
#
#	Valid combinations:
#		CSF_allow_reseller
#		CSF_deny_reseller
#		CSF_deny_cluster
#		CSF_deny_country
#		CSF_deny_service
#	Note: any other combinations can result in data loss/corruption!
#---------------------
# Log
# Revision 0.1  2013/10/28  F. Katzenberger
# - First version tested reading csf.deny and countying IP addresses w/ CIDR support
# Revision 0.2  2013/10/31  F. Katzenberger
# - Added/developed regex for detecting resellers, countries, and services
# Revision 0.3  2013/11/02 F. Katzenberger
# - developed the param structure for the munin config call
# Revision 0.4  2013/11/03  F. Katzenberger
# - touch ups for alpha testing
# Revision 0.5  2013/11/03  F. Katzenberger
# - adjusted the service lookup regex to include port scans
# - added cluster member deny graph support
# - fixed issue where login was not displayed for SMTP failed logins
#
my $DisplayVer=0.4;
#---------------------
# Known Issues
# 1. when a country or service no longer has an entry in csf.deny, it will be removed by munin
#    from the graph regardless of any datapoints still on the graph.
# 2. distributed attacks are not yet detected and categorized into the existing services
# 3. distributed attacks are not yet graphed by account names
# 4. the SMTP service label is cut short, regex is not showing the forth word.  need to consider 
#    a search and replace form SMTP AUTH to smtpauth which is the name for distributed attacks against SMTP
# 5. add a graph that shows deny by distributed on accounts.
# 6. no ipv6 support yet
# 7. does not yet track other lfd features such as excessive resources and ignore or temporary

use strict;
use warnings;
use Data::Dumper;

# determine what is to be reported on and hwo to display it from the basefile name
my ($report,$type) = ($0 =~ m#_(\b[A-Z]|[a-z]*)_(\b[A-Z]|[a-z]*)#);

# load up the allow/deny file
my $filename = "/etc/csf/csf.".$report;
# my $filename = "csf.".$report.".txt";

# display preferences based on the report and type being passed into this script
my %param = (
	allow => {
		label => 'Reported As: ',
		title => "IP Allowed",
		vtitle => 'addresses',
		graph_args => '--base 1000 -l 0',
		warning => '',
		critical => '',
		info_tag => "",
	},
	deny => {
		label => 'Reported As: ',
		title => "IP Denied",
		vtitle => 'addresses',
		graph_args => '--base 1000 -l 0',
		warning => '',
		critical => '',
		info_tag => "",
	},
	reseller => {
		lookfor => '\bReseller\s(\b\w*)(\s)',
		draw1 => 'AREA',
		draw2 => 'STACK',
	},
	country => {
		lookfor => '\s\((\w*)\/((\w*)|(\w*\s\w*))\/',
		draw1 => 'AREA',
		draw2 => 'STACK',
	},
	service => {
		lookfor => ':\s\W(\b\w*\s\w*|\b\w*)\W\s(\w*\s\w*\s\w*\slogin|\w*\s\w*\slogin|\w*)',
		draw1 => 'AREA',
		draw2 => 'STACK',
	},
	cluster => {
		lookfor => ':\sCluster\smember\s((((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])\.){3}(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])))',
		draw1 => 'AREA',
		draw2 => 'STACK',
	},
	distributed => {
		lookfor => ':\s\W(\b\w*\s\w*|\b\w*)\W\s(\w*\s\w*\s\w*\slogin|\w*\s\w*\slogin|\w*)',
		draw1 => 'AREA',
		draw2 => 'STACK',
	},
);

#Munin Config Options
if ($ARGV[0] and $ARGV[0] eq "config"){
	print "graph_title $param{$report}->{title} by $type\n";
	print "graph_vtitle $param{$report}->{vtitle}\n";
	print "graph_args $param{$report}->{graph_args}\n";
	print "graph_scale yes\n";
	print "graph_category configserver\n";
	print "graph_info This graph shows how many IP addresses in $filename are $report and filtered by $type\n";

	open FILE, $filename;
	
	my @list;
	
	# determine all possible line items of data to be displayed
	my $index = 0;
	while (my $line = <FILE>) {
		next if $line =~ m/^\s*(?:#.*)?$/;
		my ($part1,$part2) = ($line =~ m#$param{$type}->{lookfor}#);
		if ($type eq "cluster" and defined $part1) {
			$part2 = "Member ".$part1;
			$part1 =~ s/\./_/g;
			$part1 .= "_cluster_".$report;
		}
		if ($type eq "reseller" and defined $part1) {
			$part2 = $part1;
		}
		if ($type eq "service" and defined $part1 and $part1 eq "Port Scan") {
			$part1 = "port_scan";
			$part2 = "Port Scan Detected";
		}
		if (defined $part1 and defined $part2) {
			$list[$index] = $part1.",".$part2;
			$index += 1;
		} 		
	}
		
	close(FILE);
	
	# reduce those line items for data labels to unique values
	$index = 0;
	my %comb;
	@comb{@list} = ();
	my @uniq = sort keys %comb;
	
	# generate the print statements for each unique label names and how to draw the data
	for my $item (@uniq){
		my ($again1,$again2) = split(',',$item);
		print $again1.".label ".$again2."\n";
		if ($index == 0) {
			print $again1.".draw $param{$type}->{draw1}\n";	
		} else {
			print $again1.".draw $param{$type}->{draw2}\n";	
		}
		$index += 1;
	}
	exit 0;
}


open FILE, $filename;

my @list;

# determine all possible line items of data to find values for	
my $index = 0;
while (my $line = <FILE>) {
	next if $line =~ m/^\s*(?:#.*)?$/;
	my ($part1,$part2) = ($line =~ m#$param{$type}->{lookfor}#);
	if (defined $part1) {
		$list[$index] = $part1;
		$index += 1;
	} 		
}
close(FILE);

# reduce those line items for data labels to unique values
my %comb;
@comb{@list} = ();
my @uniq = sort keys %comb;

# for each unique value, 
for my $item (@uniq){
	open FILE, $filename;
	my $value = 0;
	my $index = 0;
	while (my $line = <FILE>) {
		next if $line =~ m/^\s*(?:#.*)?$/;
		
		# find the instances in the file and parse out the IP address
		my ($ipaddress) = ($line =~ m#^\s*(((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])\.){3}(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9]))#);
		my ($cidr)      = ($line =~ m#\/(3[0-2]|[1-2][0-9]|[0-9])\s#);
		my ($again1,$again2)    = ($line =~ m#$param{$type}->{lookfor}#);

		if (defined $again1 and $again1 eq $item) {
			if (defined $cidr) {
				$value += 2**(32 - $cidr);
			} else {
				$value += 1;
			}
		}
	}
	# create the print statement for the actual values
	if ($item eq "Port Scan") {
		$item = "port_scan";
	}
	if ($type eq "cluster") {
		$item =~ s/\./_/g;
		$item .= "_cluster_".$report;
	}
	print $item.".value ".$value."\n";
	close(FILE);
}




