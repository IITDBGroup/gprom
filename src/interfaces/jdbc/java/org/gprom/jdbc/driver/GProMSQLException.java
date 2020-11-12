/**
 * 
 */
package org.gprom.jdbc.driver;

import java.sql.SQLException;

/**
 * @author lord_pretzel
 *
 */
public class GProMSQLException extends SQLException {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	public GProMSQLException (String mes) {
		super(mes);
	}
	
	public GProMSQLException () {
		super();
	}	
	
	public GProMSQLException (Exception e) {
		super(e);
	}	  
}
