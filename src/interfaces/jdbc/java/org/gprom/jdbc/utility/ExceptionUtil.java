/**
 * 
 */
package org.gprom.jdbc.utility;

import java.util.Formatter;

/**
 * @author lord_pretzel
 *
 */
public class ExceptionUtil {

	public static Exception formatMessageException (String format, Object ...args) {
		StringBuilder mes = new StringBuilder();
		Formatter f = new Formatter(mes);
		
		f.format(format, args);
		
		f.close();
		
		return new Exception(mes.toString());
	}
	
}
