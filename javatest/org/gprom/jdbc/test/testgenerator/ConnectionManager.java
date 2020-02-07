package org.gprom.jdbc.test.testgenerator;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Properties;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.postgresql.util.PSQLException;
import org.gprom.jdbc.driver.GProMConnection;
import org.gprom.jdbc.driver.GProMDriver;
import org.gprom.jdbc.driver.GProMDriverProperties;
import org.gprom.jdbc.test.testgenerator.ConnectionOptions.BackendOptionKeys;
import org.gprom.jdbc.utility.LoggerUtil;

public class ConnectionManager {

	static Logger log = LogManager.getLogger(ConnectionManager.class);
	
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
	
	private Connection con = null;
	private GProMConnection gCon = null;

	
	private ConnectionManager () throws Exception {
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
	
	
	private void createConnection () throws Exception {
		Class.forName("org.gprom.jdbc.driver.GProMDriver");
		loadBackendDriver();
		
		try {
			Properties props = new Properties();
			props.setProperty("user", ConnectionOptions.getInstance().getUser());
			props.setProperty("password", ConnectionOptions.getInstance().getPassword());
			props.setProperty(GProMDriverProperties.JDBC_METADATA_LOOKUP_NAME, "true");
			log.debug("trying to create new connection: " + props);
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
					props.setProperty(GProMDriverProperties.JDBC_METADATA_LOOKUP_NAME, "true");
					log.debug("retry to create new connection: " + props);
					con = DriverManager.getConnection(constructURL(), props);
				}
				catch (Exception e2) {
					con = null;
				}
			}
			e.getMessage();
		}
		
		gCon = (GProMConnection) con;
		gCon.getW().setLogLevel(2);
		OptionsManager.getInstance().resetOptions (con);
	}
	
	/**
	 * @throws Exception 
	 * 
	 */
	private void loadBackendDriver() throws Exception {
		String backend = ConnectionOptions.getInstance().getBackend();
		
		if (backend.equals("Oracle"))
			Class.forName("oracle.jdbc.OracleDriver");
		//TODO load other drivers
	}

	public String constructURL () throws Exception {
		if (ConnectionOptions.getInstance().getBackend().equals("Oracle"))
		{
			String host =ConnectionOptions.getInstance().getHost();
			String sid = ConnectionOptions.getInstance().getBackendOption(BackendOptionKeys.OracleSID);
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
	    	ResultSet rs = null;
	
	    	try {
	    		st = con.createStatement();
	    		switch(gCon.getBackend()) {
	    		case HSQL:
	    			break;
	    		case Oracle:
	    		{
	    			String parser = gCon.getW().getStringOption("plugin.parser");
	    			if (parser.equals("oracle"))
	    				rs = st.executeQuery("SELECT 1 from dual;");
	    			else if (parser.equals("dl"))
	    				rs = st.executeQuery("Q(X) :- dual(X).");
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
	    			try { 
	    				if (rs != null)
	    					rs.close();
	    			} catch (Exception rsE) {
	    				LoggerUtil.logException(rsE, log);
	    			}
	
	    			if (st != null)
	    				st.close();
	    		}
	    		catch (SQLException e1) {
	    			LoggerUtil.logException(e1, log);
	    			e1.printStackTrace();
	    		}
	    		finally {
	
	    		}
	    		return false;
	    	}
	    	return true;
    }
}
