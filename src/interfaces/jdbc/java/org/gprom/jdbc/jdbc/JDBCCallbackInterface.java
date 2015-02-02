package org.gprom.jdbc.jdbc;

import org.gprom.jdbc.structure.FormPGAttributeClass;
import org.gprom.jdbc.structure.FormPGOperator;
import org.gprom.jdbc.structure.FormPGType;
import org.gprom.jdbc.structure.PGClass;
import org.gprom.jdbc.structure.RewriteRuleClass;

public interface JDBCCallbackInterface {

	/**
	 * Sends a debug massage
	 * @param debug the debug message
	 */
	public void jniDebugMessage(String debug);

	/**
	 * Sends an error message
	 * @param error the error message
	 */
	public void jniErrorMessage(String error);

	/**
	 * Sends an info message
	 * @param info the info message
	 */
	public void jniInfoMessage(String info);
	
	/**
	 * Try to resolve an unqualified relation name
	 * @param relation the relation name which has to be checked in the database
	 * @return an OID if the relation was found, or 0 if not found
	 */
	public int checkRelationName(String relation);
	
	
	/**
	 * Gets the number of attributes for a relation
	 * @param relationID the oid of the relation
	 * @return number of attributes
	 */
	public int getUserAttribute(int relationID);
	
	/**
	 * Gets the relation name by an OID
	 * @param oid the OID of the relation
	 * @return the name of the relation
	 */
	public String getRelationNameBy(int oid);
	
	/**
	 * Gets the relation name space OID
	 * @param namespace name of the name space
	 * @return the OID of the name space
	 */
	public int getRelationNameSpaceID(String namespace);
	
	/**
	 * Creates a PGClass object like a struct Form_pg_class
	 * @param oid the id of the relation
	 * @return a PBClass object
	 */
	public PGClass createPGClass(int oid);
	
	/**
	 * Creates a FormPGAttributeClass object like a struct Form_pg_attribute
	 * @param oid the id of the relation
	 * @param index the index of the attribute
	 * @return a FormPGAttributeClass object
	 */
	public FormPGAttributeClass createFormPGAttribute(int oid, int index);
	
	/**
	 * For a typeid returns the relation type id
	 * @param typeid the type id
	 * @return the relation type id or -1 if there isn't any
	 */
	public int getRelationTypeID(int typeid);
	
	/**
	 * For a operator name return its OID
	 * @param opername the operator name
	 * @param leftId the left type id of the operator
	 * @param rightId the right type id of the operator
	 * @return the oid of the operator
	 */
	public int getOperatorTypeID(String opername, int leftId, int rightId);
	
	/**
	 * Creates an FormPGOperator object like the struct Form_pg_operator
	 * @param oid the OID of the operator
	 * @param ltypeId type of the left argument
	 * @param rtypeId type of the right argument
	 * @param oprkind type of the operator
	 * @return an FormPGOperator object
	 */
	public FormPGOperator createFormPGOperator(int oid, String opername, int ltypeId, int rtypeId, char oprkind);
	
	/**
	 * Returns an OID for a type name
	 * @param typename
	 * @return OID for the typename
	 */
	public int getIDforType(String typename);
	
	/**
	 * Checks if the funcName exists
	 * @param funcName the function name which has to be checked
	 * @return
	 * 0 = FUNCDETAIL_NOTFOUND
	 * 1 = FUNCDETAIL_MULTIPLE
	 * 2 = FUNCDETAIL_NORMAL
	 * 3 = FUNCDETAIL_AGGREGATE
	 * 4 = FUNCDETAIL_COERCION
	 */
	public int checkFunction(String funcName);
	
	/**
	 * Gets the function name 
	 * @param oid
	 * @return the function name
	 */
	public String getFunctionName(int oid);
	
	/**
	 * Returns the OID for a function
	 * @param funcName
	 * @return the oid of the function
	 */
	public int getFunctionID(String funcName);
	
	/**
	 * Check for reserved keywords
	 * @param identifier
	 * @return true if the identifier is a reserved keyword
	 */
	public boolean checkKeyword(String identifier);
	
	/**
	 * Checks if array types are supported and returns the type of an array, if
	 * the db system doesn't support arrays return 0;
	 * @param oid the type id
	 * @return 0 if array types not supported, else the array type id
	 */
	public int getArrayTypeByID(int oid);
	
	/**
	 * Check if the schema exists and return an id for the schema
	 * @param schemaName
	 * @return id of the schema or 0 if no schema exists
	 */
	public int checkForSchema(String schemaName);
	
	/**
	 * Throws an error with an error message from the native side
	 * @param errorMessage
	 * @throws NativeException
	 */
	public void throwsNativeException(String errorMessage);
	
	/**
	 * Gets for a relation id the name space id
	 * @param id relation id
	 * @return id of the name space
	 */
	public int getNamespaceID(int relationId);
	
	/**
	 * Gets the name space name for an name space id
	 * @param namespaceId 
	 * @return the name of the name space
	 */
	public String getNamespaceName(int namespaceId);
	
	/**
	 * Creates for argument type id an Operator
	 * @param leftType type of the left argument
	 * @param rightType type of the right argument
	 * @return the right equality operator
	 */
	public FormPGOperator createEqualityOperator(int leftType, int rightType);
	
	/**
	 * Check for the type of a relation
	 * @param relId 
	 * @return v for a view or r for a relation
	 */
	public String getRelationRelkind(int relId);
	
	/**
	 * Creates for a type id a FormPGType
	 * @param typeId
	 * @return a FormPGType
	 */
	public FormPGType createFormPGType(int typeId);
	
	/**
	 * Creates for a view id a rule for transforming
	 * @param viewName
	 * @return the RewriteRule
	 */
	public RewriteRuleClass createRewriteRule(int viewName);
	
}
