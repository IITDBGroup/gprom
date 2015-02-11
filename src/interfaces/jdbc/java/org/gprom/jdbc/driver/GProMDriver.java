package org.gprom.jdbc.driver;

import java.io.IOException;
import java.net.URL;
import java.sql.Connection;
import java.sql.Driver;
import java.sql.DriverManager;
import java.sql.DriverPropertyInfo;
import java.sql.SQLException;
import java.sql.SQLFeatureNotSupportedException;
import java.util.Properties;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;
import org.gprom.jdbc.backends.BackendInfo;
import org.gprom.jdbc.driver.GProMJDBCUtil.BackendType;
import org.gprom.jdbc.jna.GProMJavaInterface.ConnectionParam;
import org.gprom.jdbc.jna.GProMWrapper;
import org.gprom.jdbc.utility.LoggerUtil;
import org.gprom.jdbc.utility.PropertyWrapper;

/**
 * GProMDriver extends the SQL driver for adding a perm assistance
 * 
 * @author Alex
 * 
 * @see http://de.cute-solutions.de/2009/03/jdbc-treiber-mit-filterfunktion-und-logging
 *      -im-eigenbau-implementierung-der-driver-klasse-25/
 */
public class GProMDriver implements Driver {
	/** logger */
	private static Logger log = Logger.getLogger(GProMDriver.class);
	
	
	protected Driver driver;
	private GProMWrapper w;
	
	/*
	private static final int MAJOR_VERSION = 0;
	private static final int MINOR_VERSION = 8;
	private static final String VERSION_FLAG = "b";
	private static final int CURRENT_VERSION = 1;
	 */
	public GProMDriver() {
		w = GProMWrapper.inst;
	}

	public boolean acceptsURL(String url) throws SQLException {
		if (url == null)
	        return false;
	    if (!url.startsWith("jdbc:gprom:"))
	        return false;
	    return true;

	}

	public GProMConnectionInterface connect (String url, Properties info)
			throws SQLException {
		// if the given URL could no be handled by the driver return null.
		if (!acceptsURL(url))
			return null;

		/** should we load the backend driver or not */
		boolean loadBackendDriver = info.containsKey(GProMDriverProperties.LOAD_DRIVER) 
				? (Boolean) info.get("GProM.LoadBackEnd") 
				: false;  
		
		/*
		 * Load the driver to connect to the database and create a new
		 * GProMConnection.
		 */
		try {
			w.init();
			
			BackendType backend = GProMJDBCUtil.inst.getBackendTypeFromURL(url);
			String driverClass = GProMJDBCUtil.inst.getDriverClass(backend);
			PropertyWrapper backendOpts = GProMJDBCUtil.inst.getOptionsForBackend(backend);
			Connection backendConnection;
			String backendURL = GProMJDBCUtil.inst.stripGProMPrefix(url);
			
			if (driverClass == null)
				throw new Exception("did not find driver for: " +  backend);
			
			// look if a suitable driver is in the DriverMapping if automatic loading is active
			if (loadBackendDriver && driverClass != null) {
				// load the driver from the classpath in the DriverMapping
				// property
				Class.forName(driverClass);				
			}
			
			// init a new GProMConnection from the driver loaded before and
			// return it
			driver = DriverManager.getDriver(backendURL);
			if (driver == null)
				throw new Exception("did not find class for driver: " +  driver);
				
			// create a jdbc connection to the backend.
			log.info("trying to connect to: " + backendURL);
			backendConnection = driver.connect(backendURL, info);
			if (backendConnection == null)
				throw new Exception("was unable to create connection: " + backendURL);
			
			// extract backend connection parameters from JDBC connection
			extractConnectionParameters(url, backendOpts, backend);
			
			// setup GProM C libraries options and plugins
			w.setupOptions(backendOpts);
			w.setupPlugins();
			w.setLogLevel(4);
			
			return new GProMConnection(backendConnection,
					backendOpts, backend, w);
		} catch (Exception ex) {
			log.error("Error loading the driver and getting a connection.");
			LoggerUtil.logException(ex, log);
			System.exit(-1);
		}
		return null;
	}
	
	/**
	 * @param backendConnection
	 * @param backendOpts
	 */
	private void extractConnectionParameters(String url,
			PropertyWrapper opts, BackendType backend) {
//		return;
		
		BackendInfo i = GProMJDBCUtil.inst.getBackendInfo(backend);
		GProMWrapper.inst.setConnectionOption(opts, ConnectionParam.Database, i.getDatabase(url));
		GProMWrapper.inst.setConnectionOption(opts, ConnectionParam.Host, i.getHost(url));
		GProMWrapper.inst.setConnectionOption(opts, ConnectionParam.User, i.getUser(url));
		GProMWrapper.inst.setConnectionOption(opts, ConnectionParam.Password, i.getPassword(url));
		GProMWrapper.inst.setConnectionOption(opts, ConnectionParam.Port, i.getPort(url));
	}

	public DriverPropertyInfo[] getPropertyInfo(String url, Properties info)
			throws SQLException {
		if (driver == null)
			return null;
		return driver.getPropertyInfo(url, info);
	}

	/**
	 * Simply through these calls to the backend driver.
	 * 
	 * @return whatever the backend driver tells us
	 */
	public int getMajorVersion() {
		if (driver == null)
			return 0;
		return driver.getMajorVersion();
	}

	public int getMinorVersion() {
		if (driver == null)
			return 0;
		return driver.getMinorVersion();
	}

	public boolean jdbcCompliant() {
		if (driver == null)
			return false;
		return driver.jdbcCompliant();
	}

	static {
		try {
			// Register this with the DriverManager
			DriverManager.registerDriver(new GProMDriver());
		} catch (SQLException e) {
			e.printStackTrace();
			System.exit(-1);
		}
	}

	/* (non-Javadoc)
	 * @see java.sql.Driver#getParentLogger()
	 */
	@Override
	public java.util.logging.Logger getParentLogger()
			throws SQLFeatureNotSupportedException {
		// TODO Auto-generated method stub
		return null;
	}

}
