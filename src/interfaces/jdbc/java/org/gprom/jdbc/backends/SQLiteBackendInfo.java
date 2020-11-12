/**
 * 
 */
package org.gprom.jdbc.backends;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

/**
 * @author lord_pretzel
 *
 */
public class SQLiteBackendInfo implements BackendInfo {

	static Logger log = LogManager.getLogger(SQLiteBackendInfo.class);

	public static SQLiteBackendInfo inst = new SQLiteBackendInfo();
	
	/* (non-Javadoc)
	 * @see org.gprom.jdbc.backends.BackendInfo#getHost(java.lang.String)
	 */
	@Override
	public String getHost(String url) throws Exception {
		return null;
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.backends.BackendInfo#getPort(java.lang.String)
	 */
	@Override
	public String getPort(String url) throws Exception {
		return null;
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.backends.BackendInfo#getUser(java.lang.String)
	 */
	@Override
	public String getUser(String url) throws Exception {
		return null;
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.backends.BackendInfo#getPassword(java.lang.String)
	 */
	@Override
	public String getPassword(String url) throws Exception {
		return null;
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.backends.BackendInfo#getDatabase(java.lang.String)
	 */
	@Override
	public String getDatabase(String url) throws Exception {
		String result;
//		Matcher m = Pattern.compile("\\(HOST=[^)]+\\)").matcher(url);
//		if (!m.find())
//			return "";
		result = url.replace("jdbc:gprom:sqlite:", "");
		log.debug("DB: " + result);
		return result;
	}

}
