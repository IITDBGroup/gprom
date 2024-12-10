/**
 * 
 */
package org.gprom.jdbc.driver;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.Properties;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.gprom.jdbc.backends.BackendInfo;
import org.gprom.jdbc.backends.OracleBackendInfo;
import org.gprom.jdbc.backends.PostgresBackendInfo;
import org.gprom.jdbc.backends.SQLiteBackendInfo;
import org.gprom.jdbc.backends.DuckDBBackendInfo;
import org.gprom.jdbc.utility.LoggerUtil;
import org.gprom.jdbc.utility.PropertyWrapper;

/**
 * @author lord_pretzel
 *
 */
public interface GProMJDBCUtil {

	/** supported backend types */
	public enum BackendType {
		Oracle,
		HSQL,
		Postgres,
		Hive,
		SQLite,
		DuckDB
	}
	
	// interface
	public BackendType getBackendTypeFromURL (URL jdbcURL) throws GProMSQLException;
	public BackendType getBackendTypeFromURL (String jdbcURL) throws GProMSQLException;
	public String getDriverClass (URL jdbcURL) throws GProMSQLException;
	public String getDriverClass (BackendType backend) throws GProMSQLException;
	public URL stripGProMPrefix (URL url) throws MalformedURLException;
	public String stripGProMPrefix (String url) throws MalformedURLException;
	public PropertyWrapper getOptionsForBackend (BackendType type);
	public BackendInfo getBackendInfo (BackendType type);
	
	// singleton
	public static GProMJDBCUtil inst = new GProMJDBCUtil () {

		private PropertyWrapper driverProps = null;
		Logger log = LogManager.getLogger(GProMJDBCUtil.class); 
		
		@Override
		public BackendType getBackendTypeFromURL(String jdbcURL) throws GProMSQLException {
			String backStr;
			String prefix;

			loadProps();
			try {
				prefix = stripGProMPrefix(jdbcURL).split(":")[1];
				backStr = driverProps.getProperty("urlPrefix." + prefix).trim();
				log.debug("prefix: <" + prefix + "> backstr: <" + backStr +">");
				return BackendType.valueOf(backStr);
			}
			catch (MalformedURLException e) {
				throw new GProMSQLException(e);
			}
		}

		@Override
		public BackendType getBackendTypeFromURL(URL jdbcURL) throws GProMSQLException {		
			return getBackendTypeFromURL(jdbcURL.toString());
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
				driverProps = new PropertyWrapper();
				driverProps.load(this.getClass().getClassLoader().getResourceAsStream("GProMDriver.properties"));
//						//						getResourceAsStream(
//						"lib/GProMDriver.properties"));
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
		
		public String stripGProMPrefix (String url) throws MalformedURLException {		
			return url.replace("jdbc:gprom:", "jdbc:");
		}
		
		public PropertyWrapper getOptionsForBackend (BackendType type) {
			loadProps();
			return driverProps.getAllProps("gpromOptions." + type).trimProperties();
		}
		
		public BackendInfo getBackendInfo (BackendType type) {
			switch(type) {
			case HSQL:
				return null;
			case Oracle:
				return OracleBackendInfo.inst;
			case Postgres:
				return PostgresBackendInfo.inst;
			case SQLite:
				return SQLiteBackendInfo.inst;
			case DuckDB:
				return DuckDBBackendInfo.inst;
			default:
				return null;
			}
		}
	};
	
}
