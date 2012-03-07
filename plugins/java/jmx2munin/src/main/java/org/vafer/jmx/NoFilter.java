package org.vafer.jmx;

import javax.management.ObjectName;

public final class NoFilter implements Filter {

    public boolean include(ObjectName bean, String attribute) {
        return true;
    }
}
