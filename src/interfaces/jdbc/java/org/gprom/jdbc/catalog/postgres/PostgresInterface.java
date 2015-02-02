package org.gprom.jdbc.catalog.postgres;


import java.sql.Connection;

import org.gprom.jdbc.jdbc.JDBCCallbackInterface;
import org.gprom.jdbc.jdbc.JNINativeInterface;
import org.gprom.jdbc.jdbc.NativeException;


import org.apache.log4j.Logger;

import org.gprom.jdbc.structure.FormPGAttributeClass;
import org.gprom.jdbc.structure.FormPGOperator;
import org.gprom.jdbc.structure.FormPGType;
import org.gprom.jdbc.structure.PGClass;
import org.gprom.jdbc.structure.RewriteRuleClass;
import org.gprom.jdbc.container.PostgresMaps;

public class PostgresInterface implements JNINativeInterface, JDBCCallbackInterface{

	// Variable
	private static Logger log = Logger.getLogger(PostgresInterface.class);
	private static PostgresInterface postgresInterface;
	
	private PostgresMaps postgresMaps;

	private PostgresCatalogLookup lookUp;
	
	private PostgresInterface(){
		
	}
	
	public static PostgresInterface getInstance(Connection con) {
		if (postgresInterface == null) {
			postgresInterface = new PostgresInterface();
			postgresInterface.lookUp = new PostgresCatalogLookup(con);
			postgresInterface.postgresMaps = new PostgresMaps();
			try {
				postgresInterface.initializeMiddleware();
			} catch (NoSuchMethodException e) {
				log.error("Something went wrong while initializing the PERM Module");
				log.error(e.getMessage());
				System.exit(-1);
			}
		}
		return postgresInterface;
	}
	
	// Native Methods
//	public native boolean initializeMiddleware() throws NoSuchMethodException;
//	public native String executeQueryJNI(String sql,int dbType);
	
	// Callback Methods
	public int checkForSchema(String schemaName) {
		return lookUp.checkForSchemaByName(schemaName, postgresMaps);
	}
	public int checkFunction(String funcName) {
		return lookUp.checkFunction(funcName, postgresMaps);
	}
	public boolean checkKeyword(String identifier) {
		return lookUp.checkForReservedKeywords(identifier);
	}
	public int checkRelationName(String relation) {
		return lookUp.checkRelationByName(relation, postgresMaps);
	}
	public FormPGOperator createEqualityOperator(int leftType, int rightType) {
		return lookUp.createEqualityOperator(leftType, rightType, postgresMaps);
	}

	public FormPGAttributeClass createFormPGAttribute(int oid, int index) {
		return lookUp.createFormPGAttribute(oid, index, postgresMaps);
	}
	public FormPGOperator createFormPGOperator(int oid, String opername,
			int ltypeId, int rtypeId, char oprkind) {
		return lookUp.createFormPGOperatorByID(oid, postgresMaps);
	}
	public FormPGType createFormPGType(int typeId) {
		return lookUp.createFormPGType(typeId);
	}
	public PGClass createPGClass(int oid) {
		return lookUp.createPGClass(oid, postgresMaps);
	}
	public RewriteRuleClass createRewriteRule(int viewName) {
		return lookUp.createRewriteRule(viewName);
	}
	public int getArrayTypeByID(int oid) {
		return lookUp.getArrayTypeId(oid, postgresMaps);
	}
	public int getFunctionID(String funcName) {
		return postgresMaps.getFunctionId(funcName);
	}
	public String getFunctionName(int oid) {
		return lookUp.getFunctionNameByID(oid, postgresMaps);
	}
	public int getIDforType(String typename) {
		return lookUp.getIDForTypeName(typename,postgresMaps);
	}
	public int getNamespaceID(int relationId) {
		return lookUp.getNamespaceIDbyRelationId(relationId);
	}
	public String getNamespaceName(int namespaceId) {
		return postgresMaps.getSchemaNameByID(namespaceId);
	}
	public int getOperatorTypeID(String opername, int leftType, int rightType) {
		return lookUp.getOperatorId(opername, leftType, rightType, postgresMaps);
	}
	public String getRelationNameBy(int oid) {
		return postgresMaps.getNameByOid(oid);
	}
	public int getRelationNameSpaceID(String namespace) {
		return postgresMaps.getSchemaNameID(namespace);
	}
	public String getRelationRelkind(int relId) {
		return postgresMaps.getRelationRelkind(relId);
	}
	public int getRelationTypeID(int typeId) {
		return postgresMaps.getRelationIDByTypeID(typeId);
	}
	public int getUserAttribute(int relationID) {
		return lookUp.getNumbersOfAttributes(relationID, postgresMaps);
	}

	public void jniDebugMessage(String debug) {
		log.debug(debug);
	}
	public void jniErrorMessage(String error) {
		log.error(error);
	}
	public void jniInfoMessage(String info) {
		log.info(info);
	}
	public void throwsNativeException(String errorMessage) {
		log.error(errorMessage, new NativeException(errorMessage));
	}
	
	// Loads the JNI library
	static {
		// System.load("/Users/alex/workspace/JDBCInterface/lib/libJNITestC.dylib");
		// System.load("/Users/alex/workspace/JDBCInterface/lib/libJdbcInterface.jnilib");
		System.loadLibrary("jni_test");
	}

	

	
	
}