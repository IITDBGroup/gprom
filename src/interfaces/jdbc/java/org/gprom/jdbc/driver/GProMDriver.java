package org.gprom.jdbc.driver;

import java.io.IOException;
import java.sql.Connection;
import java.sql.Driver;
import java.sql.DriverManager;
import java.sql.DriverPropertyInfo;
import java.sql.SQLException;
import java.sql.SQLFeatureNotSupportedException;
import java.util.Properties;

import org.apache.log4j.Logger;

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

	/*
	private static final int MAJOR_VERSION = 0;
	private static final int MINOR_VERSION = 8;
	private static final String VERSION_FLAG = "b";
	private static final int CURRENT_VERSION = 1;
	 */
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
		// if the given URL could no be handled by the driver return null.
		if (!acceptsURL(url))
			return null;

		/** should we load the backend driver or not */
		boolean loadBackendDriver = info.containsKey(GProMDriverProperties.LOAD_DRIVER) 
				? (Boolean) info.get("GProM.LoadBackEnd") 
				: false;  
		
		// remove the :perm indicator from the given URL
		String URL = url;
		URL = URL.replaceAll(":gprom", "");

		/*
		 * Load the mapping properties from location inside the jar file.
		 */
		Properties properties = new Properties();
		try {
			properties.load(this.getClass().getResourceAsStream(
					"GProMDriver.properties"));
		} catch (IOException ex) {
			log.error("Error loading the GProMDriver.properties");
			log.error(ex.getMessage());
			return null;
		}

		/*
		 * Load the driver to connect to the database and create a new
		 * GProMConnection.
		 */
		try {
			// look if a suitable driver is in the DriverMapping if automatic loading is active
			if (loadBackendDriver && properties.containsKey(selectDatabaseDriver(URL))) {
				// load the driver from the classpath in the DriverMapping
				// property
				Class.forName(properties
								.getProperty(selectDatabaseDriver(URL)));				
			}
			// init a new GProMConnection from the driver loaded before and
			// return it
			driver = DriverManager.getDriver(URL);
			return new GProMConnection(driver.connect(URL, info),
					getGProMConfForDB());
		} catch (Exception ex) {
			ex.printStackTrace();
			log.error("Error loading the driver and getting a connection");
			log.error(ex.getMessage());
			System.exit(-1);
		}
		return null;
	}
	
	protected Properties getGProMConfForDB (){
		return null;
	}

	/**
	 * Extracts the String which indicates the driver from the URL
	 * 
	 * @param url the gprom URL
	 * @return String which indicates the database type
	 */
	protected String selectDatabaseDriver(String url) {
		String dbase[] = url.split(":");
		return dbase[1];
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
