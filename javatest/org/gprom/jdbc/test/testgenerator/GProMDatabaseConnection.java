package org.gprom.jdbc.test.testgenerator;

import java.sql.Connection;
import java.sql.SQLException;

import org.dbunit.database.AbstractDatabaseConnection;

public class GProMDatabaseConnection extends AbstractDatabaseConnection {
	
	public void close() throws SQLException {
			
	}

	public Connection getConnection() throws SQLException {
		try {
			return ConnectionManager.getInstance().getConnection();
		} 
		catch (Exception e) {
			throw new SQLException (e.toString());
		}
	}

	public String getSchema() {
		return null;
	}



}
