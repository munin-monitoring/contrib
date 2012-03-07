package org.vafer.jmx;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.TreeMap;
import java.util.regex.Pattern;

import javax.management.ObjectName;

public final class Enums {

    private TreeMap<String, LinkedHashMap<Integer, Pattern>> sections = new TreeMap<String, LinkedHashMap<Integer, Pattern>>();
    
    public boolean load(String filePath) throws IOException {
        BufferedReader input = null;
        LinkedHashMap<Integer, Pattern> section = new LinkedHashMap<Integer, Pattern>();
        try {
            input = new BufferedReader(new InputStreamReader(new FileInputStream(filePath)));
            String line;
            int linenr = 0;
            while((line = input.readLine()) != null) {
                linenr += 1;
                line = line.trim();
                if (line.startsWith("#")) {
                    continue;
                }
                if (line.startsWith("[") && line.endsWith("]")) {
                    // new section
                    String id = line.substring(1, line.length() - 1);
                    section = new LinkedHashMap<Integer, Pattern>();
                    sections.put(id, section);
                } else {
                    String[] pair = line.split("=");
                    if (pair.length == 2) {
                        Integer number = Integer.parseInt(pair[0].trim());
                        Pattern pattern = Pattern.compile(pair[1].trim());
                        if (section.put(number, pattern) != null) {
                            System.err.println("Line " + linenr + ": previous definitions of " + number);
                        }
                    }
                }
            }
        } finally {
            if (input != null) {
                input.close();      
            }
        }
        return false;
    }

    public static String id(ObjectName beanName, String attributeName) {
        StringBuilder sb = new StringBuilder();
        sb.append(beanName.getDomain());
        sb.append('.');
        sb.append(beanName.getKeyProperty("type"));
        sb.append(':');
        sb.append(attributeName);
        return sb.toString();        
    }
    
    public Number resolve(String id, String value) {
        LinkedHashMap<Integer, Pattern> section = sections.get(id);
        if (section == null) {
            return null;
        }
        for(Map.Entry<Integer, Pattern> entry : section.entrySet()) {
            if (entry.getValue().matcher(value).matches()) {
                return entry.getKey();
            }
        }        
        return null;
    }
}
