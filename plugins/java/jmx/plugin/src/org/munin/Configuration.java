package org.munin;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import javax.management.MalformedObjectNameException;
import javax.management.ObjectName;

public class Configuration {
	private Properties graph_properties = new Properties();
	private Map<String, Configuration.FieldProperties> fieldMap = new HashMap();
	private List<Configuration.FieldProperties> fields = new ArrayList();

	public class FieldProperties extends Properties {
		private static final long serialVersionUID = 1L;
		private ObjectName jmxObjectName;
		private String jmxAttributeName;
		private String jmxAttributeKey;
		private String fieldname;
		private static final String JMXOBJECT = "jmxObjectName";
		private static final String JMXATTRIBUTE = "jmxAttributeName";
		private static final String JMXATTRIBUTEKEY = "jmxAttributeKey";

		public FieldProperties(Configuration paramConfiguration,
				String fieldname) {
			this.fieldname = fieldname;
		}

		public String getJmxAttributeKey() {
			return jmxAttributeKey;
		}

		public String getJmxAttributeName() {
			return jmxAttributeName;
		}

		public ObjectName getJmxObjectName() {
			return jmxObjectName;
		}

		public String toString() {
			return fieldname;
		}

		public void set(String key, String value)
				throws MalformedObjectNameException, NullPointerException {
			if ("jmxObjectName".equals(key)) {
				if (jmxObjectName != null)
					throw new IllegalStateException(
							"jmxObjectName already set for " + this);
				jmxObjectName = new ObjectName(value);
			} else if ("jmxAttributeName".equals(key)) {
				if (jmxAttributeName != null)
					throw new IllegalStateException(
							"jmxAttributeName already set for " + this);
				jmxAttributeName = value;
			} else if ("jmxAttributeKey".equals(key)) {
				if (jmxAttributeKey != null)
					throw new IllegalStateException(
							"jmxAttributeKey already set for " + this);
				jmxAttributeKey = value;
			} else {
				put(key, value);
			}
		}

		public void report(PrintStream out) {
			for (Iterator it = entrySet().iterator(); it.hasNext();) {
				Map.Entry entry = (Map.Entry) it.next();
				out.println(fieldname + '.' + entry.getKey() + " "
						+ entry.getValue());
			}
		}

		public String getFieldname() {
			return fieldname;
		}
	}

	public static Configuration parse(String config_file) throws IOException,
			MalformedObjectNameException, NullPointerException {
		BufferedReader reader = new BufferedReader(new FileReader(config_file));
		Configuration configuration = new Configuration();
		try {
			for (;;) {
				String s = reader.readLine();
				if (s == null)
					break;
				if ((!s.startsWith("%")) && (s.length() > 5)
						&& (!s.startsWith(" "))) {
					configuration.parseString(s);
				}
			}
		} finally {
			reader.close();
		}

		return configuration;
	}

	private void parseString(String s) throws MalformedObjectNameException,
			NullPointerException {
		String[] nameval = s.split(" ", 2);
		if (nameval[0].indexOf('.') > 0) {
			String name = nameval[0];
			String fieldname = name.substring(0, name.lastIndexOf('.'));
			if (!fieldMap.containsKey(fieldname)) {
				Configuration.FieldProperties field = new Configuration.FieldProperties(
						this, fieldname);
				fieldMap.put(fieldname, field);
				fields.add(field);
			}
			Configuration.FieldProperties field = (Configuration.FieldProperties) fieldMap
					.get(fieldname);
			String key = name.substring(name.lastIndexOf('.') + 1);
			field.set(key, nameval[1]);
		} else {
			graph_properties.put(nameval[0], nameval[1]);
		}
	}

	public Properties getGraphProperties() {
		return graph_properties;
	}

	public void report(PrintStream out) {
		for (Iterator it = graph_properties.entrySet().iterator(); it.hasNext();) {
			Map.Entry entry = (Map.Entry) it.next();
			out.println(entry.getKey() + " " + entry.getValue());
		}

		for (Configuration.FieldProperties field : fields) {
			field.report(out);
		}
	}

	public List<Configuration.FieldProperties> getFields() {
		return fields;
	}
}
