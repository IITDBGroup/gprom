package org.gprom.jdbc.test.testgenerator;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.Statement;

import org.dbunit.database.IDatabaseConnection;
import org.postgresql.util.PSQLException;

public class ConnectionManager {

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
	
	public IDatabaseConnection getIDatabaseConnection () throws Exception {
		if (iCon == null || !testConnection()) {
			createConnection ();
		}
		return iCon;
	}
	
	private void createConnection () throws Exception {
		Class.forName("org.postgresql.Driver");
		
		try {
			con = DriverManager.getConnection("jdbc:postgresql://127.0.0.1:5432/" + ConnectionOptions.getInstance().getDbName() 
					, ConnectionOptions.getInstance().getUser(), ConnectionOptions.getInstance().getPassword());
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
					con = DriverManager.getConnection("jdbc:postgresql://127.0.0.1:5432/" + ConnectionOptions.getInstance().getDbName() 
							, ConnectionOptions.getInstance().getUser(), ConnectionOptions.getInstance().getPassword());
				}
				catch (Exception e2) {
					con = null;
				}
			}
			e.getMessage();
		}
		
		iCon = new GProMDatabaseConnection ();
//		config = iCon.getConfig();
//		config.setProperty(DatabaseConfig.PROPERTY_DATATYPE_FACTORY, new OracleDataTypeFactory());

		
		OptionsManager.getInstance().resetOptions (con);
	}
	
    private boolean testConnection () {
    	Statement st;
    	
    	try {
    		st = con.createStatement();
    		st.execute("SELECT current_timestamp;");
    	}
    	catch (Exception e){
    		return false;
    	}
    	return true;
    }
}
