package org.gprom.jdbc.jna;

import java.sql.SQLException;

/**
 * 
 * @author alex
 *
 */
public class NativeGProMLibException extends SQLException {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	public NativeGProMLibException(){
		super();
	}
	
	public NativeGProMLibException(String s){
		super(s);
	}
}
