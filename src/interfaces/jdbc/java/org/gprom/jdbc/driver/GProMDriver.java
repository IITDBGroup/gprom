package org.gprom.jdbc.driver;

import java.io.BufferedReader;
import java.io.FileReader;
import java.lang.reflect.InvocationTargetException;
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
import java.util.logging.FileHandler;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.logging.SimpleFormatter;

import org.gprom.jdbc.backends.BackendInfo;
import org.gprom.jdbc.driver.GProMJDBCUtil.BackendType;
import org.gprom.jdbc.instrumentation.IInstrumentationLogger;
import org.gprom.jdbc.instrumentation.Javaonoff;
import org.gprom.jdbc.jna.GProMJavaInterface.ConnectionParam;
import org.gprom.jdbc.jna.GProMMetadataLookupPlugin;
import org.gprom.jdbc.jna.GProMWrapper;
import org.gprom.jdbc.metadata_lookup.oracle.OracleMetadataLookup;
import org.gprom.jdbc.utility.PropertyWrapper;

/**
 * GProMDriver extends the SQL driver for adding a perm assistance
 * 
 * @author lorpretzel
 * 
 */
public class GProMDriver implements Driver {
	static boolean check = checkJDBC.checkJDBC();

	/** logger */
	// private static Logger log = Logger.getLogger(GProMDriver.class);
	private final static Logger logger = Logger.getLogger(GProMDriver.class
			.getName());
	private static FileHandler fh = null;
	

	protected Driver driver;
	private GProMWrapper w;
	private IInstrumentationLogger queryAndProvLogger = null;

	/*
	 * private static final int MAJOR_VERSION = 0; private static final int
	 * MINOR_VERSION = 8; private static final String VERSION_FLAG = "b";
	 * private static final int CURRENT_VERSION = 1;
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

	public GProMConnectionInterface connect(String url, Properties info)
			throws SQLException {
		// if the given URL could no be handled by the driver return null.
		if (!acceptsURL(url))
			return null;

		/** should we load the backend driver or not */
		boolean loadBackendDriver = info
				.containsKey(GProMDriverProperties.LOAD_DRIVER) ? Boolean
				.parseBoolean(info
						.getProperty(GProMDriverProperties.LOAD_DRIVER))
				: false;
		/** should we use a Java JDBC based metadata lookup plugin */
		boolean useJDBCMetadataLookup = info
				.containsKey(GProMDriverProperties.JDBC_METADATA_LOOKUP) ? Boolean
				.parseBoolean(info
						.getProperty(GProMDriverProperties.JDBC_METADATA_LOOKUP))
				: false;

		boolean useProvAndQueryLogger = info
				.containsKey(GProMDriverProperties.USE_PROV_AND_QUERY_LOGGER) ? Boolean
				.parseBoolean(info
						.getProperty(GProMDriverProperties.USE_PROV_AND_QUERY_LOGGER))
				: false;

		if (useProvAndQueryLogger)
		//TODO use option to decide which if any prov and query logger to use		
		{
			String className = info
					.getProperty(GProMDriverProperties.PROV_AND_QUERY_LOGGER_CLASS);
			try {
				Class clazz = Class.forName(className);
				queryAndProvLogger = (IInstrumentationLogger) clazz.getConstructor().newInstance();
			} catch (ClassNotFoundException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (InstantiationException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IllegalAccessException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (IllegalArgumentException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (InvocationTargetException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (NoSuchMethodException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} catch (SecurityException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
		}
		/*
		 * Load the driver to connect to the database and create a new
		 * GProMConnection.
		 */
		try {
			w.init();

			BackendType backend = GProMJDBCUtil.inst.getBackendTypeFromURL(url);
			String driverClass = GProMJDBCUtil.inst.getDriverClass(backend);
			PropertyWrapper backendOpts = GProMJDBCUtil.inst
					.getOptionsForBackend(backend);
			Connection backendConnection;
			String backendURL = GProMJDBCUtil.inst.stripGProMPrefix(url);

			if (driverClass == null)
				throw new Exception("did not find driver for: " + backend);

			// look if a suitable driver is in the DriverMapping if automatic
			// loading is active
			if (loadBackendDriver && driverClass != null) {
				// load the driver from the classpath in the DriverMapping
				// property
				Class.forName(driverClass);
			}

			// init a new GProMConnection from the driver loaded before and
			// return it
			driver = DriverManager.getDriver(backendURL);
			if (driver == null)
				throw new Exception("did not find class for driver: " + driver);

			// create a jdbc connection to the backend.
			// log.info("trying to connect to: " + backendURL);

			backendConnection = driver.connect(backendURL, info);

			if (backendConnection == null)
				throw new Exception("was unable to create connection: "
						+ backendURL);

			// extract backend connection parameters from JDBC connection
			extractConnectionParameters(url, backendOpts, backend);

			// setup GProM C libraries options and plugins
			w.setupOptions(backendOpts);
			if (useJDBCMetadataLookup) {
				w.setupPlugins(backendConnection,
						getMetadataLookup(backendConnection, backend));
			} else {
				w.setupPlugins();
			}
			w.setLogLevel(4);

			return new GProMConnection(backendConnection, backendOpts, backend,
					w, queryAndProvLogger);
		} catch (Exception ex) {
			// log.error("Error loading the driver and getting a connection.");
			// LoggerUtil.logException(ex, log);
			System.exit(-1);
		}
		return null;
	}

	private GProMMetadataLookupPlugin getMetadataLookup(Connection con,
			BackendType backend) throws SQLException {
		switch (backend) {
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
	private void extractConnectionParameters(String url, PropertyWrapper opts,
			BackendType backend) {
		// return;

		BackendInfo i = GProMJDBCUtil.inst.getBackendInfo(backend);
		GProMWrapper.inst.setConnectionOption(opts, ConnectionParam.Database,
				i.getDatabase(url));
		GProMWrapper.inst.setConnectionOption(opts, ConnectionParam.Host,
				i.getHost(url));
		GProMWrapper.inst.setConnectionOption(opts, ConnectionParam.User,
				i.getUser(url));
		GProMWrapper.inst.setConnectionOption(opts, ConnectionParam.Password,
				i.getPassword(url));
		GProMWrapper.inst.setConnectionOption(opts, ConnectionParam.Port,
				i.getPort(url));
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

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.sql.Driver#getParentLogger()
	 */

	public java.util.logging.Logger getParentLogger()
			throws SQLFeatureNotSupportedException {
		// TODO Auto-generated method stub
		return null;
	}

	public void provenance(String s, Statement st, GProMConnection con)
			throws Exception {
		FileReader fr = new FileReader("java.txt");
		BufferedReader br = new BufferedReader(fr);
		String s1111;
		while ((s1111 = br.readLine()) != null) {
			System.out.println(s1111);
			if (s1111.equals("on")) {
				init();
				System.out.println("Java logging on....");
			} else {
				System.out.println("Java logging off....");
			}
		}
		fr.close();

		if (!s.toLowerCase().contains("provenance")) {

			if (s.toLowerCase().contains("update")
					|| s.toLowerCase().contains("delete")) {
				try {
					String l = s
							.replace("update", "select CURRENT_TIMESTAMP, ");
				} catch (Exception e) {
					String l = s
							.replace("delete", "select CURRENT_TIMESTAMP, ");
				}
			}

			String x = s.replace(";", "");

			ResultSet rs = st.executeQuery("PROVENANCE with tuple versions OF(" + x + ");");
			ResultSetMetaData rsmd = rs.getMetaData();

			int columnCount = rsmd.getColumnCount();

			// The column count starts from 1
			for (int i = 1; i < columnCount + 1; i++) {

				String name = rsmd.getColumnName(i);
				if (name.contains("_result_tid")
						|| name.toLowerCase().contains("prov")) {
				} else {
					if (check) {
						System.out.println("Here------>>>>>>" + name);
					}
				}
			}

			ResultSet rs1 = st.executeQuery(s);
			Javaonoff.getInstance().printResult(rs1, s);
			// String x1 = s.replace(";", "");

			String x1ss = s.replace(";", "");
			String x1 = x1ss.replace("select", "select CURRENT_TIMESTAMP(6),"); // WORKS!!
			// String x1 = x1ss.replace("select", "select row_number()");
			/*
			 * Currently testing how to get SCN.. the below mentioned functions
			 * have so far not worked.
			 * 
			 * Things tried for SCN and ROWID : 1)
			 * DBMS_FLASHBACK.GET_SYSTEM_CHANGE_NUMBER 2) SCN_BASE //for oracle
			 * 9 or below 3) ORA_ROWSCN //Psuedo row 4)
			 * SCN_TO_TIMESTAMP(ORA_ROWSCN) 5) ROWID
			 */

			rs1 = st.executeQuery("PROVENANCE OF (" + x1 + ");");
			Javaonoff.getInstance().printResult(rs1, s);

			// test error
			try {
				rs = st.executeQuery("PROVENANCE OF (" + x + ");");
			} catch (Exception e) {
				System.out.printf("%s", e);
			}

			rs = st.executeQuery("PROVENANCE OF (" + x + ");");
			Javaonoff.getInstance().printResult(rs, s);

			// log.error("statement shutdown");
			/* con.close(); */
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
					if (check) {
						System.out.println("Here------>>>>>>" + name);
					}
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
					if (check) {
						System.out.println("Here------>>>>>>" + name);
					} // These are the tuples that will be affected by the run
						// query
				}
			}

			Javaonoff.getInstance().printResult(rs, s);

			// test error
			try {
				rs = st.executeQuery(s);
			} catch (Exception e) {
				System.out.printf("%s", e);
			}

			rs = st.executeQuery(s);
			queryAndProvLogger.printResult(rs, s);

			// log.error("statement shutdown");
			/* con.close(); */
		}
		System.out.println("Done");

	}

/*	private static void printResult(ResultSet rs, String s) throws SQLException {
		
		try (PrintWriter out = new PrintWriter(new BufferedWriter(
				new FileWriter("myfile.txt", true)))) {

			Javaonoff.QueryStoretxt(s);
			j.QueryStorecsv(s);

			if (check) {
				System.out
						.println("-------------------------------------------------------------------------------");
			}
			j.ResultStoretxt("-------------------------------------------------------------------------------");
			j.ResultStorecsv("-------------------------------------------------------------------------------");
			j.ResultStoretxt("\n");
			j.ResultStorecsv("\n");
			for (int i = 1; i <= rs.getMetaData().getColumnCount(); i++)
				if (check) {
					System.out
							.print(rs.getMetaData().getColumnLabel(i) + "\t|");
				}
			for (int i = 1; i <= rs.getMetaData().getColumnCount(); i++)
			// out.print(rs.getMetaData().getColumnLabel(i) + "\t|");
			{
				j.ResultStoretxt(rs.getMetaData().getColumnLabel(i) + "\t|");
				j.ResultStorecsv(rs.getMetaData().getColumnLabel(i) + "\t|");
			}
			System.out.println();
			j.ResultStoretxt("\n");
			j.ResultStorecsv("\n");

			if (check) {
				System.out
						.println("-------------------------------------------------------------------------------");
			}
			j.ResultStoretxt("-------------------------------------------------------------------------------");
			j.ResultStoretxt("\n");
			j.ResultStorecsv("-------------------------------------------------------------------------------");
			j.ResultStorecsv("\n");
			while (rs.next()) {
				for (int i = 1; i <= rs.getMetaData().getColumnCount(); i++)
					if (check) {
						System.out.print(rs.getString(i) + "\t|");
					}
				for (int i = 1; i <= rs.getMetaData().getColumnCount(); i++)

				{
					j.ResultStoretxt(rs.getString(i) + "\t|");
					j.ResultStoretxt("\n");
					j.ResultStorecsv(rs.getString(i) + "\t|");
					j.ResultStorecsv("\n");
				}
				System.out.println();
			}
			System.out.println();
			j.ResultStoretxt("\n");
			j.ResultStorecsv("\n");

			if (check) {
				System.out
						.println("-------------------------------------------------------------------------------");
			}
			j.ResultStoretxt("-------------------------------------------------------------------------------");
			j.ResultStorecsv("-------------------------------------------------------------------------------");
			System.out.println();
			j.ResultStorecsv("\n");
			j.ResultStoretxt("\n");
			System.out.println();
			j.ResultStoretxt("\n");
			j.ResultStorecsv("\n");

		} catch (IOException e) {
			// exception handling left as an exercise for the reader
		}
		
	}*/

	public static void init() {
		try {
			fh = new FileHandler("logging.log", false);

			Logger logg = Logger.getLogger("");
			fh.setFormatter(new SimpleFormatter());
			logg.addHandler(fh);
			logg.setLevel(Level.CONFIG);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
}
