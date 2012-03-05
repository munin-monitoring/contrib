package org.vafer.jmx.munin;

import java.text.NumberFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Hashtable;
import java.util.Locale;

import javax.management.ObjectName;

import org.vafer.jmx.Enums;
import org.vafer.jmx.Output;
import org.vafer.jmx.Value;

public final class MuninOutput implements Output {

    private final Enums enums;

    public MuninOutput(Enums enums) {
        this.enums = enums;
    }

    public static String attributeName(ObjectName bean, String attribute) {
        StringBuilder sb = new StringBuilder();
        sb.append(fieldname(beanString(bean)));
        sb.append('_');
        sb.append(fieldname(attribute));
        return sb.toString().toLowerCase(Locale.US);        
    }
    
    private static String fieldname(String s) {
        return s.replaceAll("[^A-Za-z0-9]", "_");
    }

    private static String beanString(ObjectName beanName) {
        StringBuilder sb = new StringBuilder();
        sb.append(beanName.getDomain());

        Hashtable<String, String> properties = beanName.getKeyPropertyList();

        String keyspace = "keyspace";
        if (properties.containsKey(keyspace)) {
            sb.append('.');
            sb.append(properties.get(keyspace));
            properties.remove(keyspace);
        }

        String type = "type";
        if (properties.containsKey(type)) {
            sb.append('.');
            sb.append(properties.get(type));
            properties.remove(type);
        }

        ArrayList<String> keys = new ArrayList(properties.keySet());
        Collections.sort(keys);
        
        for(String key : keys) {
            sb.append('.');
            sb.append(properties.get(key));
        }
        
        return sb.toString();
        // return beanName.getCanonicalName();
    }
    
    public void output(ObjectName beanName, String attributeName, Object value) {
        Value.flatten(beanName, attributeName, value, new Value.Listener() {
            public void value(ObjectName beanName, String attributeName, String value) {
                final Number v = enums.resolve(Enums.id(beanName, attributeName), value);
                if (v != null) {
                    value(beanName, attributeName, v);
                } else {
                    value(beanName, attributeName, Double.NaN);                    
                }
            }
            public void value(ObjectName beanName, String attributeName, Number value) {
                final String v;

                if (Double.isNaN(value.doubleValue())) {
                    v = "U";
                } else {
                    final NumberFormat f = NumberFormat.getInstance();
                    f.setMaximumFractionDigits(2);
                    f.setGroupingUsed(false);
                    v = f.format(value);            
                }
                
                System.out.println(attributeName(beanName, attributeName) + ".value " + v);
            }
        });        
   }
}