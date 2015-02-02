package org.gprom.jdbc.jdbc;

/**
 * 
 * @author alex
 *
 */
public interface JNINativeInterface {
	
	
//Native Methods
	/**
	 * Initialize the PERM module. You have to make sure that all the callback methods are implemented
	 * by the class initializing the PERM module
	 * @return true if the initializing process was successfully, false otherwise
	 * @throws NoSuchMethodException
	 */
	public boolean initializeMiddleware() throws NoSuchMethodException; 
	
	/**
	 * Native methods which sends the query to the midperm application and
	 * returns the transformed query for further execution
	 * dataType: 1:HSQLDB 2:POSTGRES ...
	 * @param sql the query to transform
	 * @param int dbType the type of the attached database
	 * @return the transformed query
	 */
	public String executeQueryJNI(String sql,int dbType);
	
	
	
	
	
}
