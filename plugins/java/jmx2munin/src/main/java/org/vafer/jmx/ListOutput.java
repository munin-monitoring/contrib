package org.vafer.jmx;

import java.util.HashSet;
import java.util.Set;

import javax.management.ObjectName;

public final class ListOutput implements Output {

    private final Set<String> seen = new HashSet<String>();

    public void output(ObjectName beanName, String attributeName, Object value) {
        Value.flatten(beanName, attributeName, value, new Value.Listener() {
            public void value(ObjectName beanName, String attributeName, String value) {
                final String id = Enums.id(beanName, attributeName);
                if (!seen.contains(id)) {
                    System.out.println("[" + id + "]");
                    seen.add(id);
                }
            }
            public void value(ObjectName beanName, String attributeName, Number value) {
            }
        });
    }

}
