#!/usr/bin/perl

#
# can-install.pl - Can we install this set of Munin plugins?
#

    use Carp;
    use strict;

    # Step(1) - Is Asterisk installed?
    my $system;
    my $asterisk = `$system which asterisk`;
    chomp( $asterisk );
    print "no - Cannot find program 'asterisk' \n" if !$asterisk;
    exit( 0 ) if !$asterisk;

    # Step(2) - Are we running the correct version of Asterisk?
    my $command       = 'core show version';
    my $string        = `$asterisk -rx \"$command\"`;
    my @string        = split( / /, "$string" );
    my $version       = $string[ 1 ];
    my @vals          = split( '\.', "$version" );
    my $short_version = $vals[ 0 ] . '.' . $vals[ 1 ];
    print "no - Running wrong version of Asterisk. Need 1.4\n" if $short_version ne '1.4';
    exit( 0 ) if $short_version ne '1.4';

    # Are the Digium FFA modules installed?
    my $command = 'module show like res_fax_digium.so';
    my $string  = `$asterisk -rx \"$command\"`;
    my @string  = split( /\n/, "$string" );
    my @vals    = split( / /, $string[ 2 ] );
    my $module  = $vals[ 0 ];
    print "no - Digium FFA module not installed" if ! $module;
    exit( 0 ) if ! $module;

    # Step(4) - Is Asterisk::AMI installed?
    eval "use Asterisk::AMI";
    print "PERL module Asterisk::AMI not found. Exiting...\n" if $@;
    exit( 0 ) if $@;

    print "yes\n";
