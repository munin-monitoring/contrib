# -*- perl -*-

use strict;
use warnings;

use Test::More;
use File::Find ();
use Capture::Tiny ':all';

use vars qw/*name *dir *prune/;
*name  = *File::Find::name;
*dir   = *File::Find::dir;
*prune = *File::Find::prune;
my $num_plugins = 0;

sub wanted {
    my ( $dev, $ino, $mode, $nlink, $uid, $gid, $interpreter );

    ( ( $dev, $ino, $mode, $nlink, $uid, $gid ) = lstat($_) )
        && -f _
        && ( $interpreter = hashbang("$_") )
        && ++$num_plugins
        && process_file( $_, $name, $interpreter );
}

File::Find::find( { wanted => \&wanted }, 'plugins' );

sub hashbang {
    my ($filename) = @_;
    open my $file, '<', $filename;
    my $firstline = <$file>;
    close $file;

    $firstline =~ m{ ^\#!                 # hashbang
                     \s*                  # optional space
                     (?:/usr/bin/env\s+)? # optional /usr/bin/env
                     (?<interpreter>\S+)  # interpreter
               }xm;

    return $+{interpreter};
}

sub process_file {
    my ( $file, $filename, $interpreter ) = @_;
    use v5.10.1;

    if ( $interpreter =~ m{/bin/sh} ) {
        subtest $filename => sub {
            plan tests => 2;
            ok( check_file_with( [ 'sh', '-n', $file ] ), "sh syntax check" );
            ok( check_file_with( [ 'checkbashisms', $file ] ),
                "checkbashisms" );
        };
    }
    elsif ( $interpreter =~ m{/bin/bash} ) {
        ok( check_file_with( [ 'bash', '-n', $file ] ),
            $filename . " bash syntax check" );
    }
    elsif ( $interpreter =~ m{perl} ) {
        ok( check_file_with( [ 'perl', '-cw', $file ] ),
            $filename . " perl syntax check" );
    }
    elsif ( $interpreter =~ m{python} ) {
        ok( check_file_with(
                [ 'pylint', '--errors-only', '--report=no', $file ]
            ),
            $filename . " python syntax check"
        );
    }
    elsif ( $interpreter =~ m{php} ) {
        ok( check_file_with( [ 'php', '-l', $file ] ),
            $filename . " php syntax check" );
    }
    elsif ( $interpreter =~ m{j?ruby} ) {
        ok( check_file_with( [ 'ruby', '-cw', $file ] ),
            $filename . " ruby syntax check" );
    }
    elsif ( $interpreter =~ m{gawk} ) {
        ok( check_file_with(
                [   'gawk', '--source', "'BEGIN { exit(0) } END { exit(0) }'",
                    '--file', $file
                ]
            ),
            $filename . " gawk syntax check"
        );
    }
    else {
        fail( $filename . " unknown interpreter " . $interpreter );
    }
}

sub check_file_with {
    my ($check_command) = @_;
    my ( $stdout, $stderr, $exit ) = capture {
        system( @{$check_command} );
    };
    if ( $exit == 0 ) {
        return 1;
    }
    else {
        diag($stdout);
        diag($stderr);
        return;
    }
}

done_testing($num_plugins);
