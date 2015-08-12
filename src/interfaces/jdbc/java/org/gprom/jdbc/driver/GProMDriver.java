package org.gprom.jdbc.driver;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
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
import java.util.logging.ConsoleHandler;
import java.util.logging.FileHandler;
import java.util.logging.SimpleFormatter;

import java.io.IOException;
import java.util.logging.FileHandler;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.logging.SimpleFormatter;



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
	//private static Logger log = Logger.getLogger(GProMDriver.class);
	private final static Logger logger = Logger.getLogger(GProMDriver.class.getName());
	 private static FileHandler fh = null;
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
		//	log.info("trying to connect to: " + backendURL);
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
			//log.error("Error loading the driver and getting a connection.");
			//LoggerUtil.logException(ex, log);
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
		FileReader fr = new FileReader("java.txt"); 
BufferedReader br = new BufferedReader(fr); 
String s1111; 
while((s1111 = br.readLine()) != null) { 
System.out.println(s1111); 
if(s1111.equals("on")){ init();
			 logger.log(Level.INFO, "message 1");
			 logger.log(Level.SEVERE, "message 2");
			 logger.log(Level.FINE, "message 3");}else{System.out.println("off");
} }
fr.close(); 

			
		
			
			
			
		if (!s.toLowerCase().contains("provenance")) {

			if (s.toLowerCase().contains("update")
					|| s.toLowerCase().contains("delete")) {
				try {
					String l = s.replace("update", "select CURRENT_TIMESTAMP, ");
				} catch (Exception e) {
					String l = s.replace("delete", "select CURRENT_TIMESTAMP, ");
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
			//String x1 = s.replace(";", "");
			
			
		String x1ss = s.replace (";","");
		//String x1 = x1ss.replace("select","select CURRENT_TIMESTAMP(6),");            // WORKS!! 
		String x1 = x1ss.replace("select","select ORA_ROWSCN,");  //Currently testing how to get SCN.. the below mentioned functions have so far not worked.
			 /*
			  * Things tried for SCN :
			  * 1) DBMS_FLASHBACK.GET_SYSTEM_CHANGE_NUMBER 
			  * 2) SCN_BASE     //for oracle 9 or below
			  * 3) ORA_ROWSCN  //Psuedo row
			  * 4) SCN_TO_TIMESTAMP(ORA_ROWSCN)
			  * */
			  
			
			
			rs1 = st.executeQuery("PROVENANCE OF (" + x1 + ");");
			printResult(rs1,s);

			// test error
			try {
				rs = st.executeQuery("PROVENANCE OF (" + x + ");");
			} catch (Exception e) {
				System.out.printf("%s", e);
			}

			rs = st.executeQuery("PROVENANCE OF (" + x + ");");
			printResult(rs, s);

			//log.error("statement shutdown");
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

			printResult(rs,s);

			// test error
			try {
				rs = st.executeQuery(s);
			} catch (Exception e) {
				System.out.printf("%s", e);
			}

			rs = st.executeQuery(s);
			printResult(rs,s);

			//log.error("statement shutdown");
			/*con.close();*/
		}
		
		
	}

	private static void printResult(ResultSet rs, String s) throws SQLException
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
	
	public static void init(){
		 try {
			 fh=new FileHandler("logging.log", false);
			 
			 Logger logg = Logger.getLogger("");
			 fh.setFormatter(new SimpleFormatter());
			 logg.addHandler(fh);
			 logg.setLevel(Level.CONFIG);}catch (Exception e) {
				 e.printStackTrace();
				 }} 
}
