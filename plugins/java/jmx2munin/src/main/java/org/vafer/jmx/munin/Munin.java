package org.vafer.jmx.munin;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

import org.vafer.jmx.Enums;
import org.vafer.jmx.Filter;
import org.vafer.jmx.ListOutput;
import org.vafer.jmx.NoFilter;
import org.vafer.jmx.Query;

import com.beust.jcommander.JCommander;
import com.beust.jcommander.Parameter;

public final class Munin {

    @Parameter(description = "")
    private List<String> args = new ArrayList<String>();
    
    @Parameter(names = "-url", description = "jmx url", required = true)
    private String url;

    @Parameter(names = "-query", description = "query expression", required = true)
    private String query;

    @Parameter(names = "-enums", description = "file string to enum config")
    private String enumsPath;

    @Parameter(names = "-attribute", description = "attributes to return")
    private List<String> attributes = new ArrayList<String>();

    private void run() throws Exception {
        final Filter filter;
        if (attributes == null || attributes.isEmpty()) {
            filter = new NoFilter();
        } else {
            filter = new MuninAttributesFilter(attributes);
        }

        final Enums enums = new Enums();
        if (enumsPath != null) {
            enums.load(enumsPath);
        }
        
        final String cmd = args.toString().toLowerCase(Locale.US);
        if ("[list]".equals(cmd)) {
            new Query().run(url, query, filter, new ListOutput());
        } else {
            new Query().run(url, query, filter, new MuninOutput(enums));
        }
    }

    public static void main(String[] args) throws Exception {
        Munin m = new Munin();
        
        JCommander cli = new JCommander(m);
        try {
            cli.parse(args);            
        } catch(Exception e) {
            cli.usage();
            System.exit(1);
        }

        m.run();
    }
}
