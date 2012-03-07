package org.vafer.jmx;

import javax.management.ObjectName;

public interface Filter {

    public boolean include(ObjectName bean, String attribute);

}
