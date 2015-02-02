package org.gprom.jdbc.metadata_lookup.hsqldb;

import java.sql.Connection;

import org.gprom.jdbc.jdbc.JDBCCallbackInterface;
import org.gprom.jdbc.jdbc.JNINativeInterface;
import org.gprom.jdbc.jdbc.NativeException;

import org.apache.log4j.Logger;

import org.gprom.jdbc.container.DBCatalogTopContainer;

import org.gprom.jdbc.structure.FormPGAttributeClass;
import org.gprom.jdbc.structure.FormPGOperator;
import org.gprom.jdbc.structure.FormPGType;
import org.gprom.jdbc.structure.PGClass;
import org.gprom.jdbc.structure.RewriteRuleClass;


/**
 * JNI class which is implemented as singleton. It has all the native and all
 * the callback methods
 * if you change the name of the class or the package structure has changed. 
 * you have to rewrite the jdbc_JNIInterface.h and jdbc_JNIInterface.c
 * 
 * @author alex
 * 
 */
public class HSQLDBInterface implements JDBCCallbackInterface  {

	// Variable
	private static Logger log = Logger.getLogger(HSQLDBInterface.class);

	private static HSQLDBInterface jniInterface = null;

	private DBCatalogTopContainer catalogObjectHolder;

	private HSQLDBCatalogLookup lookUp;

	/**
	 * Default constructor which may not be used outside this class
	 */
	private HSQLDBInterface() {

	}

	/**
	 * Gets a valid instance of the JNIInterface class and initializes the
	 * various OID containers
	 * 
	 * @param con
	 *            The JDBC connection object
	 * @return a JNIInterface instance
	 */
	public static HSQLDBInterface getInstance(Connection con) {
		if (jniInterface == null) {
			jniInterface = new HSQLDBInterface();
			jniInterface.lookUp = new HSQLDBCatalogLookup(con);
			jniInterface.catalogObjectHolder = new DBCatalogTopContainer();
			jniInterface.catalogObjectHolder.createOtherContainer(jniInterface.catalogObjectHolder);
			
		}
		return jniInterface;
	}

	// Native Methods
//	public native boolean initializeMiddleware() throws NoSuchMethodException;
//	public native String executeQueryJNI(String sql,int dbType);

	
	// Callback Methods
	/**
	 * Sends a debug message to log4j
	 * 
	 * @param debug
	 *            the debug message
	 */
	public void jniDebugMessage(String debug) {
		log.debug(debug);
	}

	/**
	 * Sends an error message to log4j
	 * 
	 * @param error
	 *            the error message
	 */
	public void jniErrorMessage(String error) {
		log.error(error);
	}

	/**
	 * Sends an info message to log4j
	 * 
	 * @param info
	 *            the info message
	 */
	public void jniInfoMessage(String info) {
		log.info(info);
	}

	public int checkRelationName(String relation) {
		return lookUp.checkRelationByName(relation,catalogObjectHolder);
	}

	public int getUserAttribute(int relationID) {
		return lookUp.getNumbersOfAttributes(relationID, catalogObjectHolder);
	}

	public String getRelationNameBy(int oid) {
		return catalogObjectHolder.getRelationHolder().getRelationByOID(oid);
	}

	public int getRelationNameSpaceID(String namespace) {
		return catalogObjectHolder.getOIDByName(namespace.toUpperCase());
	}

	public PGClass createPGClass(int relationId) {
		return new PGClass(relationId, lookUp, catalogObjectHolder);
	}

	public FormPGAttributeClass createFormPGAttribute(int oid, int index) {
		return new FormPGAttributeClass(oid, lookUp, catalogObjectHolder, index);
	}

	public int getRelationTypeID(int typeId) {
		return lookUp.getRelationByType(typeId);
	}

	public int getOperatorTypeID(String opername, int leftType, int rightType) {
		if (opername.equals("~~") || opername.contains("LIKE")) {
			return catalogObjectHolder.putObjectInMap("LIKE");
		} else {
			return catalogObjectHolder.putObjectInMap(opername);
		}

	}

	public FormPGOperator createFormPGOperator(int oid, String opername, int ltypeId,
			int rtypeId, char oprkind) {
		return new FormPGOperator(oid, ltypeId, rtypeId, oprkind,
				catalogObjectHolder);
	}

	public int getIDforType(String typename) {
		return catalogObjectHolder.getTypeHolder().getIDforType(typename);
	}

	public int checkFunction(String funcName) {
		// Check first if its an aggregate function
		if (catalogObjectHolder.checkForAggregateFunction(funcName)) {
			return 3;
		}
		catalogObjectHolder.putObjectInMap(funcName.toUpperCase());
		return lookUp.checkFunctionType(funcName);
	}

	public String getFunctionName(int oid) {
		// For transsql we should define a function
		if (oid == 3778) {
			return "'reconstructTransToSQL'";
		}
		return catalogObjectHolder.getObjectByOID(oid);
	}

	public boolean checkKeyword(String identifier) {
		if (lookUp.checkForReservedFunctionName(identifier,catalogObjectHolder)) {
			return true;
		}
		return false;
	}

	public int getArrayTypeByID(int oid) {
		return 0;
	}

	public int getFunctionID(String funcName) {
		return catalogObjectHolder.getOIDByName(funcName.toUpperCase());
	}

	public int checkForSchema(String schemaName) {
		return lookUp.checkForSchema(schemaName,catalogObjectHolder);
	}

	public void throwsNativeException(String errorMessage) {
		log.error(errorMessage, new NativeException(errorMessage));
	}

	public String getNamespaceName(int namespaceId) {
		return catalogObjectHolder.getObjectByOID(namespaceId);
	}

	public int getNamespaceID(int relationId) {
		return lookUp.getNamespaceID(relationId, catalogObjectHolder);
	}

	public FormPGOperator createEqualityOperator(int leftType, int rightType) {
		return new FormPGOperator(leftType, rightType, catalogObjectHolder.getTypeHolder());
	}

	public String getRelationRelkind(int relId) {
		String relation = catalogObjectHolder.getObjectByOID(relId);
		return lookUp.checkRelationkind(relation);
	}

	public FormPGType createFormPGType(int typeId) {
		return new FormPGType(typeId,lookUp, catalogObjectHolder);
	}

	public RewriteRuleClass createRewriteRule(int viewId) {
		return new RewriteRuleClass(viewId, lookUp,catalogObjectHolder);
	}

	// Loads the JNI library
	static {
		// System.load("/Users/alex/workspace/JDBCInterface/lib/libJNITestC.dylib");
		// System.load("/Users/alex/workspace/JDBCInterface/lib/libJdbcInterface.jnilib");
		System.loadLibrary("jni_test");
	}

}

// -Djava.library.path=/Users/alex/workspace/JDBCInterface/lib/