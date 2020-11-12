/**
 * 
 */
package org.gprom.jdbc.backends;

import java.util.Properties;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

/**
 * @author lord_pretzel
 *
 */
public class PostgresBackendInfo implements BackendInfo { // Hacky fix?

	static Logger log = LogManager.getLogger(PostgresBackendInfo.class);
	
	public static PostgresBackendInfo inst = new PostgresBackendInfo();

	/**
	 * 
	    jdbc:postgresql:/ = 1
	    jdbc:postgresql:database = 2
	    jdbc:postgresql://host:[port]/[database] = 3 4 5
	 */
	private static Pattern postgresURLPatternDBOnly = Pattern.compile("jdbc:postgresql:([a-zA-Z_][a-zA-Z_0-9]+)$");
	private static Pattern postgresURLPattern = Pattern.compile("jdbc:postgresql://([^/:]+)(:(\\d{4}))?/([a-zA-Z_][a-zA-Z_0-9]+)?$");

	
	/* (non-Javadoc)
	 * @see org.gprom.jdbc.backends.BackendInfo#getHost(java.lang.String)
	 */
	@Override
	public String getHost(String url) throws Exception {
		Properties p = extractUrlParts(url);
		
		return p.getProperty("host");
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.backends.BackendInfo#getPort(java.lang.String)
	 */
	@Override
	public String getPort(String url) throws Exception {
		Properties p = extractUrlParts(url);
		
		return p.getProperty("port");
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.backends.BackendInfo#getUser(java.lang.String)
	 */
	@Override
	public String getUser(String url) {
		return null;
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.backends.BackendInfo#getPassword(java.lang.String)
	 */
	@Override
	public String getPassword(String url) {
		return null;
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.backends.BackendInfo#getDatabase(java.lang.String)
	 */
	@Override
	public String getDatabase(String url) throws Exception {
		Properties p = extractUrlParts(url);
		
		return p.getProperty("db");
	}

	
	private Properties extractUrlParts (String url) throws Exception {
		Properties result = new Properties();
		Matcher dbUrl = postgresURLPatternDBOnly.matcher(url); 
		
		result.setProperty("host", "127.0.0.1");
		result.setProperty("db", "postgres");
		result.setProperty("port", "5432");
		
		if (url.equals("jdbc:postgresql:/")) {
			return result;
		}
		else if (dbUrl.matches()) {
			result.setProperty("db", dbUrl.group(1));
		}
		else {
			Matcher fullUrl = postgresURLPattern.matcher(url);
			if (!fullUrl.matches())
				throw new Exception("not a valid postgres URL");
			String host = fullUrl.group(1);
			String db = fullUrl.group(4);
			String port = fullUrl.group(3);
			
			if (host != null)
				result.setProperty("host", host);
			if (db != null)
				result.setProperty("db", db);
			if (port != null)
				result.setProperty("port", port);
		}
		
		return result;
	}

}
