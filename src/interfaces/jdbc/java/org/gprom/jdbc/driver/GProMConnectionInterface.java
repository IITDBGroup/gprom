package org.gprom.jdbc.driver;

import java.sql.Connection;
import java.sql.SQLException;


public interface GProMConnectionInterface extends Connection{

	public GProMStatement createGProMStatement() throws SQLException;
}
