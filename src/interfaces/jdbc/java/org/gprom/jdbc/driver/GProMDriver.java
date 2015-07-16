package org.gprom.jdbc.driver;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.net.URL;
import java.sql.Connection;
import java.sql.Driver;
import java.sql.DriverManager;
import java.sql.DriverPropertyInfo;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.sql.SQLFeatureNotSupportedException;
import java.sql.Statement;
import java.util.Properties;

import org.apache.log4j.Logger;
import org.gprom.jdbc.backends.BackendInfo;
import org.gprom.jdbc.driver.GProMJDBCUtil.BackendType;
import org.gprom.jdbc.jna.GProMJavaInterface.ConnectionParam;
import org.gprom.jdbc.jna.GProMWrapper;
import org.gprom.jdbc.jna.GProMMetadataLookupPlugin;
import org.gprom.jdbc.metadata_lookup.oracle.OracleMetadataLookup;
import org.gprom.jdbc.utility.LoggerUtil;
import org.gprom.jdbc.utility.PropertyWrapper;

/**
 * GProMDriver extends the SQL driver for adding a perm assistance
 * 
 * @author lorpretzel
 * 
 */
public class GProMDriver implements Driver {
	/** logger */
	private static Logger log = Logger.getLogger(GProMDriver.class);
	
	public static TestOutput tst = new TestOutput();
	
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
				? Boolean.parseBoolean(info.getProperty(GProMDriverProperties.LOAD_DRIVER)) 
				: false;
		/** should we use a Java JDBC based metadata lookup plugin */		
		boolean useJDBCMetadataLookup = info.containsKey(GProMDriverProperties.JDBC_METADATA_LOOKUP) 
				? Boolean.parseBoolean(info.getProperty(GProMDriverProperties.JDBC_METADATA_LOOKUP)) 
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
			if (useJDBCMetadataLookup) {
				w.setupPlugins(backendConnection, getMetadataLookup(backendConnection, backend));
			}
			else {
				w.setupPlugins();
			}
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
	
	private GProMMetadataLookupPlugin getMetadataLookup (Connection con, BackendType backend) throws SQLException {
		switch (backend)
		{
		case HSQL:
			break;
		case Oracle:
			return new OracleMetadataLookup(con).getPlugin();
		case Postgres:
			break;
		default:
			break;
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
	
	public java.util.logging.Logger getParentLogger()
			throws SQLFeatureNotSupportedException {
		// TODO Auto-generated method stub
		return null;
	}
	public void provenance(String s, Statement st, GProMConnection con)
			throws Exception {

		if (!s.toLowerCase().contains("provenance")) {

			if (s.toLowerCase().contains("update")
					|| s.toLowerCase().contains("delete")) {
				try {
					String l = s.replace("update", "select");
				} catch (Exception e) {
					String l = s.replace("delete", "select");
				}
			}

			String x = s.replace(";", "");
			ResultSet rs = st.executeQuery("PROVENANCE OF (" + x + ");");
			ResultSetMetaData rsmd = rs.getMetaData();

			int columnCount = rsmd.getColumnCount();

			// The column count starts from 1
			for (int i = 1; i < columnCount + 1; i++) {

				String name = rsmd.getColumnName(i);
				if (name.contains("_result_tid")
						|| name.toLowerCase().contains("prov")) {
				} else {
					System.out.println("Here------>>>>>>" + name);
				}
			}

			ResultSet rs1 = st.executeQuery(s);
			printResult(rs1,s);
			String x1 = s.replace(";", "");
			rs1 = st.executeQuery("PROVENANCE OF (" + x1 + ");");
			printResult(rs1,s);

			// test error
			try {
				rs = st.executeQuery("PROVENANCE OF (" + x + ");");
			} catch (Exception e) {
				System.out.printf("%s", e);
			}

			rs = st.executeQuery("PROVENANCE OF (" + x + ");");
			printResult(rs);

			log.error("statement shutdown");
			/*con.close();*/
		} else {
			ResultSet rs = st.executeQuery(s);
			ResultSetMetaData rsmd = rs.getMetaData();

			int columnCount = rsmd.getColumnCount();

			// The column count starts from 1
			for (int i = 1; i < columnCount + 1; i++) {

				String name = rsmd.getColumnName(i);
				if (name.contains("_result_tid")
						|| name.toLowerCase().contains("prov")) {
				} else {
					System.out.println("Here------>>>>>>" + name);
				}
			}

			rs = st.executeQuery(s);

			ResultSetMetaData rsmd1 = rs.getMetaData();

			int columnCount1 = rsmd.getColumnCount();

			// The column count starts from 1
			for (int i = 1; i < columnCount + 1; i++) {
				String name = rsmd.getColumnName(i);
				if (name.contains("_result_tid")
						|| name.toLowerCase().contains("prov")) {
				} else {
					System.out.println("Here------>>>>>>" + name);
				}
			}

			printResult(rs);

			// test error
			try {
				rs = st.executeQuery(s);
			} catch (Exception e) {
				System.out.printf("%s", e);
			}

			rs = st.executeQuery(s);
			printResult(rs);

			log.error("statement shutdown");
			/*con.close();*/
		}
	}

	private static void printResult(ResultSet rs) throws SQLException
			 {
		try (PrintWriter out = new PrintWriter(new BufferedWriter(
				new FileWriter("myfile.txt", true)))) {
			tst.Test(s);
			System.out
					.println("-------------------------------------------------------------------------------");
		
			tst.Test("-------------------------------------------------------------------------------");
			tst.Test("\n");
			for (int i = 1; i <= rs.getMetaData().getColumnCount(); i++)
				System.out.print(rs.getMetaData().getColumnLabel(i) + "\t|");
			for (int i = 1; i <= rs.getMetaData().getColumnCount(); i++)
				//out.print(rs.getMetaData().getColumnLabel(i) + "\t|");
			tst.Test(rs.getMetaData().getColumnLabel(i) + "\t|");
			
			System.out.println();
			tst.Test("\n");
			System.out
					.println("-------------------------------------------------------------------------------");
			tst.Test("-------------------------------------------------------------------------------");
			tst.Test("\n");
			while (rs.next()) {
				for (int i = 1; i <= rs.getMetaData().getColumnCount(); i++)
					System.out.print(rs.getString(i) + "\t|");
				for (int i = 1; i <= rs.getMetaData().getColumnCount(); i++)
				
					tst.Test(rs.getString(i) + "\t|");
				tst.Test("\n");
				System.out.println();
			}
			System.out.println();
			tst.Test("\n");

			System.out
					.println("-------------------------------------------------------------------------------");
			tst.Test("-------------------------------------------------------------------------------");
			System.out.println();
			tst.Test("\n");
			System.out.println();
			tst.Test("\n");
		
			
		
			
		} catch (IOException e) {
			// exception handling left as an exercise for the reader
		}
	}
}
