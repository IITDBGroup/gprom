/**
 * 
 */
package org.gprom.jdbc.backends;

import java.net.URL;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.log4j.Logger;



/**
 * @author lord_pretzel
 *
 */
public class OracleBackendInfo implements BackendInfo {

	static Logger log = Logger.getLogger(OracleBackendInfo.class);
	
	public static OracleBackendInfo inst = new OracleBackendInfo();
	
	/* (non-Javadoc)
	 * @see org.gprom.jdbc.backends.BackendInfo#getHost(java.net.URL)
	 */
	@Override
	public String getHost(String url) {
		String result;
		Matcher m = Pattern.compile("\\(HOST=[^)]+\\)").matcher(url);
		if (!m.find())
			return "";
		result = m.group().replace("(HOST=", "").replace(")", "");
		log.debug("HOST: " + result);
		
		return result;
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.backends.BackendInfo#getPort(java.net.String)
	 */
	@Override
	public String getPort(String url) {
		String result;
		Matcher m = Pattern.compile("\\(PORT=[^)]+\\)").matcher(url);
		if (!m.find())
			return "";
		result = m.group().replace("(PORT=", "").replace(")", "");
		log.debug("PORT: " + result);
		
		return result;
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.backends.BackendInfo#getUser(java.net.String)
	 */
	@Override
	public String getUser(String url) {
		String result;
		Matcher m = Pattern.compile("jdbc:gprom:oracle:thin:[^/]+/").matcher(url);
		if (!m.find())
			return "";
		result = m.group().replace("jdbc:gprom:oracle:thin:", "").replace("/", "");
		log.debug("USER: " + result);
		
		return result;
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.backends.BackendInfo#getPassword(java.net.String)
	 */
	@Override
	public String getPassword(String url) {
		String result;
		Matcher m = Pattern.compile("jdbc:gprom:oracle:thin:[^/]+/[^@]+@").matcher(url);
		if (!m.find())
			return "";
		result = m.group().replaceFirst("jdbc:gprom:oracle:thin:[^/]+/", "").replace("@", "");
		log.debug("PW: " + result);
		
		return result;
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.backends.BackendInfo#getDatabase(java.net.String)
	 */
	@Override
	public String getDatabase(String url) {
		String result;
		Matcher m = Pattern.compile("\\(SID=[^)]+\\)").matcher(url);
		if (!m.find())
			return "";
		result = m.group().replace("(SID=", "").replace(")", "");
		log.debug("SID: " + result);
		
		return result;
	}

}
