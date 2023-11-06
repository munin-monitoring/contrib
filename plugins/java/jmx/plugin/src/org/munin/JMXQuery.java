package org.munin;

import java.io.IOException;
import java.text.NumberFormat;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

import javax.management.Attribute;
import javax.management.AttributeList;
import javax.management.InstanceNotFoundException;
import javax.management.IntrospectionException;
import javax.management.MBeanAttributeInfo;
import javax.management.MBeanInfo;
import javax.management.MBeanServerConnection;
import javax.management.ObjectName;
import javax.management.ReflectionException;
import javax.management.openmbean.CompositeDataSupport;
import javax.management.remote.JMXConnector;
import javax.management.remote.JMXConnectorFactory;
import javax.management.remote.JMXServiceURL;

public class JMXQuery {
	private static final String USERNAME_KEY = "username";
	private static final String PASSWORD_KEY = "password";
	public static final String USAGE = "Usage of program is:\njava -cp jmxquery.jar org.munin.JMXQuery --url=<URL> [--user=<username> --pass=<password>] [--conf=<config file> [config]]\n, where <URL> is a JMX URL, for example: service:jmx:rmi:///jndi/rmi://HOST:PORT/jmxrmi\nWhen invoked with the config file (see examples folder) - operates as Munin plugin with the provided configuration\nWithout options just fetches all JMX attributes using provided URL";
	private String url;
	private String username;
	private String password;
	private JMXConnector connector;
	private MBeanServerConnection connection;
	private Configuration config;

	public JMXQuery(String url) {
		this(url, null, null);
	}

	public JMXQuery(String url, String username, String password) {
		this.url = url;
		this.username = username;
		this.password = password;
	}

	private void connect() throws IOException {
		Map<String, Object> environment = null;
		if ((username != null) && (password != null)) {
			environment = new HashMap();

			environment.put("jmx.remote.credentials", new String[] { username,
					password });
			environment.put("username", username);
			environment.put("password", password);
		}

		JMXServiceURL jmxUrl = new JMXServiceURL(url);
		connector = JMXConnectorFactory.connect(jmxUrl, environment);
		connection = connector.getMBeanServerConnection();
	}

	private void list() throws IOException, InstanceNotFoundException,
			IntrospectionException, ReflectionException {
		if (config == null) {
			listAll();
		} else {
			listConfig();
		}
	}

	private void listConfig() {
		for (Configuration.FieldProperties field : config.getFields()) {
			try {
				Object value = connection.getAttribute(
						field.getJmxObjectName(), field.getJmxAttributeName());
				output(field.getFieldname(), value, field.getJmxAttributeKey());
			} catch (Exception e) {
				System.err.println("Fail to output " + field);
				e.printStackTrace();
			}
		}
	}

	private void output(String name, Object attr, String key) {
		if ((attr instanceof CompositeDataSupport)) {
			CompositeDataSupport cds = (CompositeDataSupport) attr;
			if (key == null) {
				throw new IllegalArgumentException(
						"Key is null for composed data " + name);
			}
			System.out.println(name + ".value " + format(cds.get(key)));
		} else {
			System.out.println(name + ".value " + format(attr));
		}
	}

	private void output(String name, Object attr) {
		CompositeDataSupport cds;
		Iterator it;
		if ((attr instanceof CompositeDataSupport)) {
			cds = (CompositeDataSupport) attr;
			for (it = cds.getCompositeType().keySet().iterator(); it.hasNext();) {
				String key = it.next().toString();
				System.out.println(name + "." + key + ".value "
						+ format(cds.get(key)));
			}
		} else {
			System.out.println(name + ".value " + format(attr));
		}
	}

	private void listAll() throws IOException, InstanceNotFoundException,
			IntrospectionException, ReflectionException {
		Set<ObjectName> mbeans = connection.queryNames(null, null);
		for (ObjectName name : mbeans) {
			MBeanInfo info = connection.getMBeanInfo(name);
			MBeanAttributeInfo[] attrs = info.getAttributes();
			String[] attrNames = new String[attrs.length];
			for (int i = 0; i < attrs.length; i++) {
				attrNames[i] = attrs[i].getName();
			}
			try {
				AttributeList attributes = connection.getAttributes(name,
						attrNames);
				for (Attribute attribute : attributes.asList()) {
					output(name.getCanonicalName() + "%" + attribute.getName(),
							attribute.getValue());
				}
			} catch (Exception e) {
				System.err.println("error getting " + name + ":"
						+ e.getMessage());
			}
		}
	}

	private String format(Object value) {
		if (value == null)
			return null;
		if ((value instanceof String))
			return (String) value;
		if ((value instanceof Number)) {
			NumberFormat f = NumberFormat.getInstance();
			f.setMaximumFractionDigits(2);
			f.setGroupingUsed(false);
			return f.format(value);
		}
		if ((value instanceof Object[])) {
			return Integer.toString(Arrays.asList((Object[]) value).size());
		}
		return value.toString();
	}

	private void disconnect() throws IOException {
		connector.close();
	}

	public static void main(String[] args) {
		int arglen = args.length;
		if (arglen < 1) {
			System.err
					.println("Usage of program is:\njava -cp jmxquery.jar org.munin.JMXQuery --url=<URL> [--user=<username> --pass=<password>] [--conf=<config file> [config]]\n, where <URL> is a JMX URL, for example: service:jmx:rmi:///jndi/rmi://HOST:PORT/jmxrmi\nWhen invoked with the config file (see examples folder) - operates as Munin plugin with the provided configuration\nWithout options just fetches all JMX attributes using provided URL");
			System.exit(1);
		}

		String url = null;
		String user = null;
		String pass = null;
		String config_file = null;
		boolean toconfig = false;
		for (int i = 0; i < arglen; i++) {
			if (args[i].startsWith("--url=")) {
				url = args[i].substring(6);
			} else if (args[i].startsWith("--user=")) {
				user = args[i].substring(7);
			} else if (args[i].startsWith("--pass=")) {
				pass = args[i].substring(7);
			} else if (args[i].startsWith("--conf=")) {
				config_file = args[i].substring(7);
			} else if (args[i].equals("config")) {
				toconfig = true;
			}
		}

		if ((url == null) || ((user != null) && (pass == null))
				|| ((user == null) && (pass != null))
				|| ((config_file == null) && (toconfig))) {
			System.err
					.println("Usage of program is:\njava -cp jmxquery.jar org.munin.JMXQuery --url=<URL> [--user=<username> --pass=<password>] [--conf=<config file> [config]]\n, where <URL> is a JMX URL, for example: service:jmx:rmi:///jndi/rmi://HOST:PORT/jmxrmi\nWhen invoked with the config file (see examples folder) - operates as Munin plugin with the provided configuration\nWithout options just fetches all JMX attributes using provided URL");
			System.exit(1);
		}

		if (toconfig) {
			try {
				Configuration.parse(config_file).report(System.out);
			} catch (Exception e) {
				System.err.println(e.getMessage() + " reading " + config_file);
				System.exit(1);
			}
		} else {
			JMXQuery query = new JMXQuery(url, user, pass);
			try {
				query.connect();
				if (config_file != null) {
					query.setConfig(Configuration.parse(config_file));
				}
				query.list();
			} catch (Exception ex) {
				System.err.println(ex.getMessage() + " querying " + url);
				ex.printStackTrace();
				System.exit(1);
			} finally {
				try {
					query.disconnect();
				} catch (IOException e) {
					System.err.println(e.getMessage() + " closing " + url);
				}
			}
		}
	}

	private void setConfig(Configuration configuration) {
		config = configuration;
	}

	public Configuration getConfig() {
		return config;
	}
}
