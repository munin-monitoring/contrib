package org.vafer.jmx;

import javax.management.ObjectName;

public interface Output {

    public void output(ObjectName beanName, String attributeName, Object value);

}
