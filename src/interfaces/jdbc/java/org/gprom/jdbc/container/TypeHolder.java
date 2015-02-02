package org.gprom.jdbc.container;

import java.util.HashMap;

import org.apache.log4j.Logger;


/**
 * Creates a mapping between the postgres datatype system and the needed SQL types.
 * @author alex
 *
 */
public class TypeHolder extends DBCatalogTopContainer{

	private static Logger log = Logger.getLogger(TypeHolder.class);
	private DBCatalogTopContainer topContainer;
	private HashMap<String,Integer>	postgresTypeToOID;
	private HashMap<Integer, String> postgresNameToOID;
	private HashMap<String, Integer> mappingByString;
	private HashMap<Integer, Integer> typeLengthByOID;
	
	//DB DATA TYPE
	private static String INTEGER_TYPE = "INTEGER";
	private static String DOUBLE_TYPE = "DOUBLE";
	private static String VARCHAR_TYPE = "VARCHAR";
	private static String CHAR_TYPE = "CHAR";
	private static String LONGVARCHAR_TYPE = "LONGVARCHAR";
	private static String DECIMAL_TYPE = "DECIMAL";
	private static String NUMERIC_TYPE = "NUMERIC";
	private static String BOOLEAN_TYPE = "BOOLEAN";
	private static String SMALLINT_TYPE = "SMALLINT";
	private static String BIGINT_TYPE ="BIGINT";
	private static String STRING_TYPE = "TEXT";
	//NOT USED
//	private static String VARCHAR_IGNORECASE_TYPE = "VARCHAR_IGNORECASE";
//	private static String DATE_TYPE = "DATE";
//	private static String TIME_TYPE = "TIME";
//	private static String TIMESTAMP_TYPE = "TIMESTAMP";
//	private static String REAL_TYPE = "REAL";
//	private static String BINARY_TYPE = "BINARY";
//	private static String TINYINT_TYPE = "TINYINT";
//	private static String VARBINARY_TYPE = "VARBINARY";
//	private static String LONGVARBINARY_TYPE = "LONGVARBINARY";
//	private static String OBJECT_TYPE = "OBJECT";
	
	//Define the postgres type for the sqlType array
	private static String[] postgresTypeForSQL = {
									"int8",
									"numeric",
									"varchar",
									"char",
									"varchar",
									"numeric",
									"numeric",
									"bool",
									"int8",
									"int8",
									"text"
	};
	
	//Define the needed sqlTypes
	private static String[] sqlType ={
							INTEGER_TYPE,
							DOUBLE_TYPE,
							VARCHAR_TYPE,
							CHAR_TYPE,
							LONGVARCHAR_TYPE,
							DECIMAL_TYPE,
							NUMERIC_TYPE,
							BOOLEAN_TYPE,
							SMALLINT_TYPE,
							BIGINT_TYPE,
							STRING_TYPE
	};
	
	//Define the oid of the postgres type
	public int[] typeOID = {
					20,
					1700,
					25,
					1043,
					16,
					18,
					25,
					1560,
					1562
	};
	
	//Define the length of the postgres type
	public int[] typeLength = {
					8,
					-1,
					-1,
					-1,
					1,
					1,
					-1,
					-1,
					-1
	};
	
	//Define the postgres type
	public String[] postgresType = {
					"int8",
					"numeric",
					"text",
					"varchar",
					"bool",
					"char",
					"text",
					"bit",
					"varbit"
	};
		
	public TypeHolder(DBCatalogTopContainer dbc){
		topContainer = dbc;
		postgresTypeToOID = new HashMap<String, Integer>();
		postgresNameToOID = new HashMap<Integer, String>();
		typeLengthByOID = new HashMap<Integer, Integer>();
		mappingByString = new HashMap<String, Integer>();
		createPostgresMapping();
		createMapping();
	}
	
	
	/**
	 * Creates for the defined postgres types a mapping to an OID
	 * also puts the length for an OID in a map
	 */
	private void createPostgresMapping(){
		for(int i = 0; i < postgresType.length; i++){
			postgresTypeToOID.put(postgresType[i], typeOID[i]);
			postgresNameToOID.put(typeOID[i], postgresType[i]);
			typeLengthByOID.put(typeOID[i], typeLength[i]);
		}
	}
	/**
	 * Creates the mapping between the type of the postgres system 
	 * and the db
	 */
	private void createMapping(){
		for(int i  = 0; i < sqlType.length; i++){
			mappingByString.put(sqlType[i],postgresTypeToOID.get(postgresTypeForSQL[i]));
			topContainer.putObjectInMapByOid(postgresTypeToOID.get(postgresTypeForSQL[i]),sqlType[i]);
		}
	}

	/**
	 * Gets the OID for a SQL type, if not know return a string type
	 * @param jdbcDataType
	 * @return the postgres oid for this type
	 */
	public int getTypeID(String jdbcDataType){
		if(mappingByString.get(jdbcDataType)== null){
			return 25;
		}else{
			return mappingByString.get(jdbcDataType);
		}
	}

	/**
	 * Gets the length for a type
	 * @param typeOID
	 * @return the length for the typeOID
	 */
	public int getTypeLength(int typeOID){
		return typeLengthByOID.get(typeOID);
	}

	/**
	 * Gets the postgres type id for a typename. if the type name is an aggregation
	 * return the right target type id
	 * @param typename
	 * @return an id for the typename
	 */
	public int getIDforType(String typename) {
		if (postgresTypeToOID.containsKey(typename)){
			return postgresTypeToOID.get(typename);
		}else{
			return 705;
		}
	}
	
	/**
	 * Ges the postgres type name for an id
	 * @param typeId
	 * @return the postgres type name
	 */
	public String getTypeNameForID(int typeId){
		return postgresNameToOID.get(typeId);
	}
	/**
	 * Check if the argument type id is a text type
	 * @param argType
	 * @return true if is a text type, false if its a supported numeric type
	 */
	public boolean isText(int argType){
		if(argType != 20 && argType != 1700){
			return true;
		}else{
			return false;
		}
	}
}
