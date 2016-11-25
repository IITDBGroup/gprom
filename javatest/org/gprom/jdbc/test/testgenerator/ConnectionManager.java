package org.gprom.jdbc.test.testgenerator;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Properties;

import org.apache.log4j.Logger;
import org.dbunit.database.IDatabaseConnection;
import org.postgresql.util.PSQLException;
import org.gprom.jdbc.driver.GProMConnection;
import org.gprom.jdbc.driver.GProMDriver;
import org.gprom.jdbc.driver.GProMDriverProperties;
import org.gprom.jdbc.utility.LoggerUtil;

public class ConnectionManager {

	static Logger log = Logger.getLogger(ConnectionManager.class);
	
	static {
		try {
			instance = new ConnectionManager ();
		} 
		catch (Exception e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	private static ConnectionManager instance;
	
	private Connection con;
	private GProMConnection gCon;
	private IDatabaseConnection iCon;
	
	private ConnectionManager () throws Exception {
		createConnection ();
	}
	
	public static ConnectionManager getInstance () {
		return instance;
	}
	
	public Connection getConnection () throws Exception {
		if (con == null || !testConnection()){
			createConnection ();
		}
		return con;
	}
	
	public GProMConnection getGProMConnection () throws Exception {
		if (con == null) { // || !testConnection()){
			createConnection ();
		}
		return gCon;
	}
	
	
	public IDatabaseConnection getIDatabaseConnection () throws Exception {
		if (iCon == null || !testConnection()) {
			createConnection ();
		}
		return iCon;
	}
	
	private void createConnection () throws Exception {
		Class.forName("org.gprom.jdbc.driver.GProMDriver");
		loadBackendDriver();
		
		try {
			Properties props = new Properties();
			props.setProperty("user", ConnectionOptions.getInstance().getUser());
			props.setProperty("password", ConnectionOptions.getInstance().getPassword());
			props.setProperty(GProMDriverProperties.JDBC_METADATA_LOOKUP, "true");
			con = DriverManager.getConnection(constructURL(), props);
		}
		catch (PSQLException e) {
			con = null;
			while (con == null) {
				try {
					Thread.sleep(1000);
				} catch (InterruptedException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
				try {
					Properties props = new Properties();
					props.setProperty("user", ConnectionOptions.getInstance().getUser());
					props.setProperty("password", ConnectionOptions.getInstance().getPassword());
					props.setProperty(GProMDriverProperties.JDBC_METADATA_LOOKUP, "true");
					con = DriverManager.getConnection(constructURL(), props);
				}
				catch (Exception e2) {
					con = null;
				}
			}
			e.getMessage();
		}
		
		gCon = (GProMConnection) con;
		iCon = new GProMDatabaseConnection();
		gCon.getW().setLogLevel(1);
		OptionsManager.getInstance().resetOptions (con);
	}
	
	/**
	 * @throws ClassNotFoundException 
	 * @throws IOException 
	 * @throws FileNotFoundException 
	 * 
	 */
	private void loadBackendDriver() throws ClassNotFoundException, FileNotFoundException, IOException {
		String backend = ConnectionOptions.getInstance().getBackend();
		
		if (backend.equals("Oracle"))
			Class.forName("oracle.jdbc.OracleDriver");
	}

	public String constructURL () throws Exception {
		if (ConnectionOptions.getInstance().getBackend().equals("Oracle"))
		{
			String host =ConnectionOptions.getInstance().getHost();
			String sid = ConnectionOptions.getInstance().getBackendOption("SID");
			int port = ConnectionOptions.getInstance().getPort();
			String user = ConnectionOptions.getInstance().getUser();
			String passwd = ConnectionOptions.getInstance().getPassword();
			
			log.debug("");
			String url = "jdbc:gprom:oracle:thin:"  + user + "/" + passwd + 
					"@(DESCRIPTION=(ADDRESS_LIST=(ADDRESS=(PROTOCOL=TCP)(HOST=" + host + ")(PORT=" + port + ")))(CONNECT_DATA=(SID=" + sid +")))";

			
			return url;
		}
		if (ConnectionOptions.getInstance().getBackend().equals("SQLite"))
		{
			String host =ConnectionOptions.getInstance().getHost();
			String db = ConnectionOptions.getInstance().getDbName();
			
			log.debug("");
			String url = "jdbc:gprom:sqlite:";

			
			return url;			
		}
		else
			throw new Exception("backend not supported: " + ConnectionOptions.getInstance().getBackend());
	}
	
    private boolean testConnection () {
    	Statement st = null;
    	
    	try {
    		st = con.createStatement();
    		switch(gCon.getBackend()) {
			case HSQL:
				break;
			case Oracle:
			{
				String parser = gCon.getW().getStringOption("plugin.parser");
				if (parser.equals("oracle"))
					st.execute("SELECT 1 from dual;");
				else if (parser.equals("dl"))
					st.execute("Q(X) :- dual(X).");
			}
				break;
			case Postgres:
				break;
			default:
				break;
    		
    		}
    	}
    	catch (Exception e){
    		try {
    			if (st != null)
    				st.close();
    		}
			catch (SQLException e1) {
//				LoggerUtil.logException(e1, log);
				e1.printStackTrace();
			}
    		finally {
    			
    		}
    		return false;
    	}
    	return true;
    }
}
