


    sub get_fax_stats {

        my $astman = Asterisk::AMI->new(PeerAddr => "$host",
                                        PeerPort => "$port",
                                        Username => "$username",
                                        Secret   => "$secret"
        );

        croak "Unable to connect to asterisk" unless ( $astman );
        my $actionid = $astman->send_action({ Action => 'Command',
                                              Command => 'fax show stats',
                                              $amiparams{ timeout } });
        my $response = $astman->get_response( $actionid );
        my $arrayref = $response->{CMD};
        my @array    = @$arrayref;
        my $null     = qq{};
        my ( %faxstats, $section);
        foreach my $line ( @array ) {
            next if ( ( ! $line ) || ( $line =~ /-----------/ ) );
            my ( $key, $value ) = split( ':', $line );
            $section = $key if ( $value eq $null );
            $key =~ s/\s+$//g;
            $value =~ s/^\s+//g;
            #$faxstats{ "$key" } = $value if ( $value ne $null );
            $faxstats{ "$section" }{ "$key" } = $value if ( $value ne $null );
        };
        $astman->disconnect;

        return( %faxstats );

    };

