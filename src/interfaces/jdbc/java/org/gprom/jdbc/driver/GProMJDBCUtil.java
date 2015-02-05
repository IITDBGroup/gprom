/**
 * 
 */
package org.gprom.jdbc.driver;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.Properties;

import org.apache.log4j.Logger;
import org.gprom.jdbc.utility.LoggerUtil;

/**
 * @author lord_pretzel
 *
 */
public interface GProMJDBCUtil {

	/** supported backend types */
	public enum BackendType {
		Oracle,
		HSQL
	}
	
	// interface
	public BackendType getBackendTypeFromURL (URL jdbcURL) throws GProMSQLException; 
	public String getDriverClass (URL jdbcURL) throws GProMSQLException;
	public String getDriverClass (BackendType backend) throws GProMSQLException;
	public URL stripGProMPrefix (URL url) throws MalformedURLException;
	
	// singleton
	public static GProMJDBCUtil inst = new GProMJDBCUtil () {

		private Properties driverProps = null;
		Logger log = Logger.getLogger(GProMJDBCUtil.class); 
		
		@Override
		public BackendType getBackendTypeFromURL(URL jdbcURL) throws GProMSQLException {
			String backStr;
			String prefix;
			try {
				prefix = stripGProMPrefix(jdbcURL).toString().split(":")[0];
			}
			catch (MalformedURLException e) {
				LoggerUtil.logException(e, log);
				throw new GProMSQLException(e);
			}
			loadProps();
			
			backStr = driverProps.getProperty("urlPrefix." + prefix);
			
			return BackendType.valueOf(backStr);
		}

		@Override
		public String getDriverClass(URL jdbcURL) throws GProMSQLException {
			return getDriverClass(getBackendTypeFromURL(jdbcURL));
		}

		@Override
		public String getDriverClass(BackendType backend) {
			loadProps();
			
			String driverName = driverProps.getProperty("driverClass." + backend.toString());
			
			return driverName;
		}
			
		private void loadProps () {
			if (driverProps != null)
				return;
			try {
				driverProps = new Properties();
				driverProps.load(this.getClass().getResourceAsStream(
						"GProMDriver.properties"));
			} catch (IOException ex) {
				log.error("Error loading the GProMDriver.properties");
				log.error(ex.getMessage());
			}
		}

		@Override
		public URL stripGProMPrefix(URL url) throws MalformedURLException {
			String u = url.toString();
			u = u.replace("jdbc:gprom:", "jdbc:");
			
			return new URL(u);
		}
	};
	
}
