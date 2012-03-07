package org.vafer.jmx;

import java.io.IOException;
import java.util.Collection;

import javax.management.AttributeNotFoundException;
import javax.management.InstanceNotFoundException;
import javax.management.IntrospectionException;
import javax.management.MBeanAttributeInfo;
import javax.management.MBeanException;
import javax.management.MBeanInfo;
import javax.management.MBeanServerConnection;
import javax.management.MalformedObjectNameException;
import javax.management.ObjectInstance;
import javax.management.ObjectName;
import javax.management.ReflectionException;
import javax.management.remote.JMXConnector;
import javax.management.remote.JMXConnectorFactory;
import javax.management.remote.JMXServiceURL;

public final class Query {
    
    public void run(String url, String expression, Filter filter, Output output) throws IOException, MalformedObjectNameException, InstanceNotFoundException, ReflectionException, IntrospectionException, AttributeNotFoundException, MBeanException {
        JMXConnector connector = JMXConnectorFactory.connect(new JMXServiceURL(url));
        MBeanServerConnection connection = connector.getMBeanServerConnection();
        final Collection<ObjectInstance> mbeans = connection.queryMBeans(new ObjectName(expression), null);

        for(ObjectInstance mbean : mbeans) {
            final ObjectName mbeanName = mbean.getObjectName();
            final MBeanInfo mbeanInfo = connection.getMBeanInfo(mbeanName);
            final MBeanAttributeInfo[] attributes = mbeanInfo.getAttributes();
            for (final MBeanAttributeInfo attribute : attributes) {
                if (attribute.isReadable()) {
                    if (filter.include(mbeanName, attribute.getName())) {
                        final String attributeName = attribute.getName();
                        try {
                            output.output(
                                    mbean.getObjectName(),
                                    attributeName,
                                    connection.getAttribute(mbeanName, attributeName)
                                    );
                        } catch(Exception e) {
                            // System.err.println("Failed to read " + mbeanName + "." + attributeName);
                        }                        
                    }
                }
            }

        }
        connector.close();
    }
}
