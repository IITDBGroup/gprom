/**
 * 
 */
package org.gprom.jdbc.test.testgenerator;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Types;

import org.dbunit.dataset.datatype.AbstractDataType;
import org.dbunit.dataset.datatype.TypeCastException;


/**
 *
 * Part of Project PermTester
 * @author Boris Glavic
 *
 */
public class PostgresArrayDataType extends AbstractDataType {

	/**
	 * @param name
	 * @param sqlType
	 * @param classType
	 * @param isNumber
	 */
	public PostgresArrayDataType () {
		super("ARRAY", Types.ARRAY, String.class, false);
	}

	/* (non-Javadoc)
	 * @see org.dbunit.dataset.datatype.DataType#typeCast(java.lang.Object)
	 */
	@Override
	public Object typeCast (Object value) throws TypeCastException {
		 if (value instanceof String) {
			 String stringValue = (String)value;
			 return stringValue;
		 }
		 return value;
	}
	
	@Override
    public Object getSqlValue(int column, ResultSet resultSet)
	    throws SQLException, TypeCastException {
		String value;
	    	
	    System.out.println("getSqlValue(column=" + column + ", resultSet=" + resultSet + ") - start");
		
		 value = resultSet.getString(column);
		if (value == null || resultSet.wasNull())
		{
		    return null;
		}
		return value;
	}
	

}
