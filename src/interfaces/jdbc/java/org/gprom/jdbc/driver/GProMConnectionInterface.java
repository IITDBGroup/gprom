package org.gprom.jdbc.driver;

import java.sql.Connection;


public interface GProMConnectionInterface extends Connection{

	public GProMStatement createGProMStatement();
}
