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
            run_check(
                {   command     => [ 'sh', '-n', $file ],
                    description => 'sh syntax check'
                }
            );
            run_check(
                {   command     => [ 'checkbashisms', $file ],
                    description => 'checkbashisms'
                }
            );
        };
    }
    elsif ( $interpreter =~ m{/bin/ksh} ) {
        run_check(
            {   command     => [ 'ksh', '-n', $file ],
                description => 'ksh syntax check',
                filename    => $filename
            }
        );
    }
    elsif ( $interpreter =~ m{bash} ) {
        run_check(
            {   command     => [ 'bash', '-n', $file ],
                description => 'bash syntax check',
                filename    => $filename
            }
        );
    }
    elsif ( $interpreter =~ m{perl} ) {
        run_check(
            {   command     => [ 'perl', '-cw', $file ],
                description => 'perl syntax check',
                filename    => $filename
            }
        );
    }
    elsif ( $interpreter =~ m{python} ) {
    SKIP: {
            skip 'need better syntax check for python', 1;
            run_check(
                {   command => [
                        'pylint',        '--rcfile=/dev/null',
                        '--errors-only', '--report=no',
                        $file
                    ],
                    description => 'python syntax check',
                    filename    => $filename
                }
            );
        }
    }
    elsif ( $interpreter =~ m{php} ) {
        run_check(
            {   command     => [ 'php', '-l', $file ],
                description => 'php syntax check',
                filename    => $filename
            }
        );
    }
    elsif ( $interpreter =~ m{j?ruby} ) {
        run_check(
            {   command     => [ 'ruby', '-cw', $file ],
                description => 'ruby syntax check',
                filename    => $filename
            }
        );
    }
    elsif ( $interpreter =~ m{gawk} ) {
        run_check(
            {   command => [
                    'gawk', '--source', 'BEGIN { exit(0) } END { exit(0) }',
                    '--file', $file
                ],
                description => 'gawk syntax check',
                filename    => $filename
            }
        );
    }
    elsif ( $interpreter =~ m{expect} ) {
    SKIP: {
            skip 'no idea how to check expect scripts', 1;
            pass("No pretending everything is ok");
        }
    }
    else {
        fail( $filename . " unknown interpreter " . $interpreter );
    }
}

sub run_check {
    my ($args)        = @_;
    my $check_command = $args->{command};
    my $description   = $args->{description};
    my $filename      = $args->{filename};

    my $message;

    if ($filename) {
        $message = sprintf( '%s: %s', $filename, $description );
    }
    else {
        $message = $description;
    }

    my ( $stdout, $stderr, $exit ) = capture {
        system( @{$check_command} );
    };

    ok( ( $exit == 0 ), $message );

    if ($exit) {
        diag(
            sprintf(
                "\nCommand: %s\n\nSTDOUT:\n\n%s\n\nSTDERR:\n\n%s\n\n",
                join( " ", @{$check_command} ),
                $stdout, $stderr
            )
        );
    }
}

done_testing($num_plugins);
