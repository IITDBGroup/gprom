package org.gprom.jdbc.driver;

import java.sql.Connection;
import java.sql.Driver;
import java.sql.DriverManager;
import java.sql.DriverPropertyInfo;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.SQLFeatureNotSupportedException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Properties;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.gprom.jdbc.backends.BackendInfo;
import org.gprom.jdbc.driver.GProMJDBCUtil.BackendType;
import org.gprom.jdbc.jna.GProMJavaInterface.ConnectionParam;
import org.gprom.jdbc.jna.GProMMetadataLookupPlugin;
import org.gprom.jdbc.jna.GProMNativeLibraryLoader;
import org.gprom.jdbc.jna.GProMWrapper;
import org.gprom.jdbc.metadata_lookup.oracle.OracleMetadataLookup;
import org.gprom.jdbc.metadata_lookup.postgres.PostgresMetadataLookup;
import org.gprom.jdbc.metadata_lookup.sqlite.SQLiteMetadataLookup;
import org.gprom.jdbc.metadata_lookup.duckdb.DuckDBMetadataLookup;
import org.gprom.jdbc.utility.LoggerUtil;
import org.gprom.jdbc.utility.PropertyWrapper;

/**
 * GProMDriver wraps a database backends JDBC driver using libgprom to add provenance support.
 * 
 * @author lordpretzel
 * 
 */
public class GProMDriver implements Driver {
	/** logger */
	private static Logger log = LogManager.getLogger(GProMDriver.class);

	protected Driver driver;
	private GProMWrapper w = null;
	
	private static final int MAJOR_VERSION = 1;
	private static final int MINOR_VERSION = 0;
	
	public GProMDriver() {
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
		boolean loadBackendDriver;
		boolean useJDBCMetadataLookup;
		
		// if the given URL could no be handled by the driver return null.
		if (!acceptsURL(url))
			return null;
		log.info("creating GProM connection for {}\nwith options:\n{}", url, info);
		
		// get values of properties
		try {
			/** should we load the backend driver or not */
			loadBackendDriver = GProMDriverProperties.getBoolOptionOrDefault(
					GProMDriverProperties.LOAD_DRIVER_NAME, info); 
					
			/** should we use a Java JDBC based metadata lookup plugin */		
			useJDBCMetadataLookup = GProMDriverProperties.getBoolOptionOrDefault(
					GProMDriverProperties.JDBC_METADATA_LOOKUP_NAME, info);
		} catch (Exception e) {
			throw new SQLException(e);
		}
		
		/*
		 * load native library if we haven't done this
		 */
		try {
			GProMNativeLibraryLoader.inst.loadLibrary();
			w = GProMWrapper.inst;
		} catch (Exception e) {
			throw new SQLException(e);
		}
		
		/*
		 * Load the driver to connect to the database and create a new
		 * GProMConnection.
		 */
		try {
			log.info("initializeof libgprom ...");
			w.init();
			log.info("initialization of libgprom finished.");
			BackendType backend = GProMJDBCUtil.inst.getBackendTypeFromURL(url);
			String driverClass = GProMJDBCUtil.inst.getDriverClass(backend);
			PropertyWrapper backendOpts = GProMJDBCUtil.inst.getOptionsForBackend(backend);
			Connection backendConnection;
			String backendURL = GProMJDBCUtil.inst.stripGProMPrefix(url);
			log.debug("got backend information and properties {}", backendOpts);
			
			if (driverClass == null)
				throw new Exception("did not find driver for: " +  backend);
			
			// look if a suitable driver is in the DriverMapping if automatic loading is active
			if (loadBackendDriver && driverClass != null) {
				// load the driver from the classpath in the DriverMapping
				// property
				Class.forName(driverClass);
				log.info("loaded backend driver {}", driverClass);
			}
			
			// init a new GProMConnection from the driver loaded before and
			// return it
			driver = DriverManager.getDriver(backendURL);
			if (driver == null)
				throw new Exception("did not find class for driver: " +  driver);
			log.info("driver class is {}", driver.getClass());
			
			// create a jdbc connection to the backend.
			log.info("trying to connect to: " + backendURL);
			backendConnection = driver.connect(backendURL, info);
			if (backendConnection == null)
				throw new Exception("was unable to create connection: " + backendURL);
			
			log.info("created connection object for backend DB: " + backendURL);
			
			// extract backend connection parameters from JDBC connection
			extractConnectionParameters(backendURL, backendOpts, backend, info);
			
			// setup GProM C libraries options and plugins
			w.setupOptions(backendOpts);
			if (useJDBCMetadataLookup) {
				w.setupPlugins(backendConnection, getMetadataLookup(backendConnection, backend));
			}
			else {
				w.setupPlugins();
			}
			w.setLogLevel(4);
			log.debug("have setup options: " + w.toString());
			
			return new GProMConnection(backendConnection,
					backendOpts, backend, w);
		} catch (Throwable ex) {
			log.error("Error loading the driver and getting a connection.");
			LoggerUtil.logException(ex, log);
//			LogManager.shutdown(); //TODO this should be dealt with be users of the driver
//			System.exit(2);
		}
		return null;
	}
	
	private GProMMetadataLookupPlugin getMetadataLookup (Connection con, BackendType backend) throws SQLException {
		switch (backend)
		{
			case HSQL:
				break;
			case Oracle:
				return new OracleMetadataLookup(con).getPlugin();
			case Postgres:
				return new PostgresMetadataLookup(con).getPlugin();
			case SQLite:
				return new SQLiteMetadataLookup(con).getPlugin();
			case DuckDB:
				return new DuckDBMetadataLookup(con).getPlugin();
			default:
				throw new SQLException("no JDBC metadata lookup for Backend " + backend.toString());
		}
		return null;
	}
	
	/**
	 * @param backendConnection
	 * @param backendOpts
	 * @throws Exception 
	 */
	private void extractConnectionParameters(String url,
			PropertyWrapper opts, BackendType backend, Properties userOpts) throws Exception {
		BackendInfo i = GProMJDBCUtil.inst.getBackendInfo(backend);
		String db = i.getDatabase(url);
		String host = i.getHost(url);
		String user = i.getUser(url);
		String passwd = i.getPassword(url);
		String port = i.getPort(url);
		
		// try to user from info
		if (user == null) {
			if (userOpts.getProperty("user") != null)
				user = userOpts.getProperty("user");
		}
		// try to get passwd from info
		if (passwd == null) {
			if (userOpts.getProperty("password") != null)
				passwd = userOpts.getProperty("password");	
		}
		
		if (db != null)
			GProMWrapper.inst.setConnectionOption(opts, ConnectionParam.Database, db);
		if (host != null)
			GProMWrapper.inst.setConnectionOption(opts, ConnectionParam.Host, host);
		if (user != null)
			GProMWrapper.inst.setConnectionOption(opts, ConnectionParam.User, user);
		if (passwd != null)
			GProMWrapper.inst.setConnectionOption(opts, ConnectionParam.Password, passwd);
		if (port != null)
			GProMWrapper.inst.setConnectionOption(opts, ConnectionParam.Port, port);
	}

	public DriverPropertyInfo[] getPropertyInfo(String url, Properties info)
			throws SQLException {

		return constructPropertyInfo(url, info);
	}

	private DriverPropertyInfo[] constructPropertyInfo(String url, Properties info) throws SQLException {
		List<DriverPropertyInfo> infos = new ArrayList<DriverPropertyInfo> (); 
		
		if (driver != null) {
			infos.addAll(Arrays.asList(driver.getPropertyInfo(url, info)));
		}
		infos.addAll(Arrays.asList(GProMDriverProperties.getInfos(info)));		
		
		return infos.toArray(new DriverPropertyInfo[0]);
	}
	
	
	
	/**
	 * Simply through these calls to the backend driver.
	 * 
	 * @return whatever the backend driver tells us
	 */
	public int getMajorVersion() {
//		if (driver == null)
			return MAJOR_VERSION;
//		return driver.getMajorVersion();
	}

	public int getMinorVersion() {
//		if (driver == null)
			return MINOR_VERSION;
//		return driver.getMinorVersion();
	}

	public boolean jdbcCompliant() {
//		if (driver == null)
			return false;
//		return driver.jdbcCompliant();
	}

	static {
		try {
			// Register this with the DriverManager
			DriverManager.registerDriver(new GProMDriver());
		} catch (SQLException e) {
			e.printStackTrace();
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
