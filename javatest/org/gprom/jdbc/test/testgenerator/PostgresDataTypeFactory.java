/**
 * 
 */
package org.gprom.jdbc.test.testgenerator;

import java.sql.Types;

import org.dbunit.dataset.datatype.DataType;
import org.dbunit.dataset.datatype.DataTypeException;
import org.dbunit.dataset.datatype.DefaultDataTypeFactory;


/**
 *
 * Part of Project PermTester
 * @author Boris Glavic
 *
 */
public class PostgresDataTypeFactory extends DefaultDataTypeFactory {

	public static final DataType POSTGRES_ARRAY = new PostgresArrayDataType ();
	
	public DataType createDataType(int sqlType, String sqlTypeName) throws DataTypeException {
		System.out.println("createDataType(sqlType=" + sqlType + ", sqlTypeName=" + sqlTypeName + ") - start");

        // Map Oracle DATE to TIMESTAMP
		if (sqlType == Types.DATE) {
			return DataType.TIMESTAMP;
		}

		// TIMESTAMP
		if (sqlTypeName.startsWith("TIMESTAMP")) {
			return DataType.TIMESTAMP;
		}

		// ARRAY
		if ("ARRAY".equals(sqlTypeName)) {
			return POSTGRES_ARRAY;
		}

		return super.createDataType(sqlType, sqlTypeName);
	}
	
	
}
