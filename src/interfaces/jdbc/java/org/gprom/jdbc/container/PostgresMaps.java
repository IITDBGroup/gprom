package org.gprom.jdbc.container;

import java.util.HashMap;
import java.util.Map;

import org.apache.log4j.Logger;

import org.gprom.jdbc.structure.FormPGAttributeClass;
import org.gprom.jdbc.structure.FormPGOperator;
import org.gprom.jdbc.structure.PGClass;

/**
 * Hashing Class for the postgres system table queries
 * @author alex
 *
 */
public class PostgresMaps {
	private static Logger log = Logger.getLogger(PostgresMaps.class);
	private HashMap<Integer, PGClass> relationMap;
	private HashMap<String, FormPGAttributeClass> attributeMap;
	private HashMap<String, Integer> nameMap;
	private HashMap<Integer, String> idMap;
	private HashMap<Integer,FormPGOperator> operatorMap;
	
	public static String RELATION_ENDING = "_rel";
	public static String ATTRIBUTE_ENDING = "_att";
	public static String OPERATOR_ENDING = "_op";
	public static String SCHEMA_ENDING = "_sch";
	public PostgresMaps(){
		relationMap = new HashMap<Integer, PGClass>();
		attributeMap = new HashMap<String, FormPGAttributeClass>();
		nameMap = new HashMap<String, Integer>();
		idMap = new HashMap<Integer, String>();
		operatorMap = new HashMap<Integer, FormPGOperator>();
	}
	
	/**
	 * Gets an PGClass object for an oid
	 * @param oid of a relation
	 * @return PGClass
	 */
	public PGClass getPGClass(int oid){
		return relationMap.get(oid);
	}
	
	/**
	 * Hashes a PGClass object and the OID and the name of the relation
	 * @param pgClass the constructed pg_class of a relation
	 * @param oid id of a relation
	 * @param name the name of the relation
	 */
	public void hashRelation(PGClass pgClass, int oid, String name){
		if(!relationMap.containsKey(oid)){
			relationMap.put(oid,pgClass);
			nameMap.put(name+RELATION_ENDING,oid);
			idMap.put(oid, name);
		}
	}
	/**
	 * Gets an FormPGAttribute
	 * @param oid the relation id the attribute belongs to
	 * @param index the index of the attribute
	 * @return FormPGAttribute
	 */
	public FormPGAttributeClass getPGAttribute(int relationId,int index){
		return attributeMap.get(relationId+"_"+index);
	}
	
	/**
	* Hashes a PGAttribute object and the OID and the name of the attribute with the ending _att
	 * @param pgAttribute the constructed pg_attribute of an attribute
	 * @param index the index of the attribute in the relation
	 * @param relationId the id of the relation
	 */
	public void hashAttribute(FormPGAttributeClass pgAttribute, int index, int relationId){
		if(!attributeMap.containsKey(pgAttribute.getAttrelid()+"_"+index)){
			attributeMap.put(relationId+"_"+index, pgAttribute);
			//nameMap.put(name+ATTRIBUTE_ENDING, oid);
		}
	}
	
	/**
	 * Gets an Operator
	 * @param oid the id of the operator
	 */
	public FormPGOperator getOperator(int oid){
		return operatorMap.get(oid);
	}
	
	/**
	* Hashes an FormPGOperator object and the OID and the name of the operator with the ending _op
	 * @param pgOperator the constructed pg_operator of an operator
	 * @param oid id of an operator
	 * @param name the name of the operator
	 */
	public void hashOperator(FormPGOperator pgOperator, int oid, String name){
		if(!operatorMap.containsKey(oid)){
			operatorMap.put(oid, pgOperator);
			nameMap.put(name+OPERATOR_ENDING, oid);
			idMap.put(oid, name);
		}
	}
	
	/**
	 * Gets the id of a hashed value
	 * @param name the name of a relation, operator, attribute
	 * @param type the ending for the name relation:"_rel", attribute:"_att", operator:"_op", schema name: "_sch";
	 * @return the id of a hashed value
	 */
	public int getOidByName(String name, String type){
		return nameMap.get(name+type);
	}
	
	/**
	 * Gets the name of an id
	 * @param id
	 * @return the name of the id
	 */
	public String getNameByOid(int id){
		return idMap.get(id);
	}
	
	/**
	 * Hashes a name with the given id
	 * @param name some string to hash
	 * @param oid the id for the name
	 * @param typeName the type for the name relation:"_rel", attribute:"_att", operator:"_op, schema name: "_sch";
	 */
	public void hashName(String name, int oid, String typeName){
		nameMap.put(name+typeName, oid);
		idMap.put(oid, name);
	}
	
	/**
	 * Checks if a relation was hashed
	 * @param oid id of a relation
	 * @return true if the relation was already hashed
	 */
	public boolean hasRelation(int oid){
		return relationMap.containsKey(oid);
	}
		
	/**
	 * Checks if an attribute was hashed
	 * @param relationId the id of the relation
	 * @param index the index of the attribute
	 * @return true if the attribute was already hashed
	 */
	public boolean hasAttribute(int relationId, int index){
		return attributeMap.containsKey(relationId + "_"+index);
	}
	
	/**
	 * Checks if an operator was hashed
	 * @param oid id of an operator
	 * @return true if the operator was alreasy hashed
	 */
	public boolean hasOperator(int oid){
		return operatorMap.containsKey(oid);
	}
	
	/**
	 * Gets for a relation id the relation kind
	 * @param relationId the id of the relation
	 * @return v for view or r for relation
	 */
	public String getRelationRelkind(int relationId){
		if(hasRelation(relationId)){
			return Character.toString(getPGClass(relationId).getRelkind());
		}else{
			return null;
		}
	}
	
	/**
	 * Clears all values in the maps
	 */
	public void clearAllMaps(){
		relationMap.clear();
		attributeMap.clear();
		operatorMap.clear();
		nameMap.clear();
		log.info("All maps were cleared");
	}
	
	/**
	 *  Creates a new id for an operator
	 * @return a random id for the operator
	 */
	public int getNewIdForOperator(){
		boolean found = false;
		int id = 0;
		while(!found){
			double a = Math.random();
			id = (int) Math.round(a * 100000);
			if(!hasOperator(id)){
				found = true;
			}
		}
		return id;
	}
	/**
	 *  Creates a new id for the name map
	 * @return a random id for the name map
	 */
	public int getNewIdForNameMap(){
		boolean found = false;
		int id = 0;
		while(!found){
			double a = Math.random();
			id = (int) Math.round(a * 100000);
			if(!idMap.containsKey(id)){
				found = true;
			}
		}
		return id;
	}
	
	/**
	 * Gets for a name space the id
	 * 
	 * @param namespace
	 *            the name of the name space
	 * @param maps
	 *            the HashMaps
	 * @return the id of the name space
	 */
	public int getRelationNamespaceID(String namespace, PostgresMaps maps) {
		return maps.getOidByName(namespace, PostgresMaps.SCHEMA_ENDING);
	}
	
	/**
	 * Checks for a type Id the corresponding relation id
	 * 
	 * @param typeId
	 *            the type id of a relation
	 * @param maps
	 *            the HashMaps
	 * @return the id of the relation
	 */
	public int getRelationIDByTypeID(int typeId) {
		for (Map.Entry<Integer, PGClass> entry : getRelationMap()
				.entrySet()) {
			PGClass pgClass = entry.getValue();
			if (pgClass.getRelType() == typeId) {
				return pgClass.getRelfilenode();
			}
		}
		return 0;
	}
	
	
	/**
	 * Check if a name exists in the name hash map
	 * @param name the name to check
	 * @param type the type of the name or null
	 * @return true if the name exists
	 */
	public boolean checkForName(String name, String type){
		if(type != null){
			if(nameMap.containsKey(name + "_"+ type)){
				return true;
			}
		}else{
			if(nameMap.containsKey(name)){
				return true;
			}
		}
		return false;
	}
	
	/**
	 * Checks if the the id exits in the name map
	 * @param id the id of a name
	 * @return true if the name exists
	 */
	public boolean checkForIDInNameMap(int id){
		if(idMap.containsKey(id)){
			return true;
		}
		return false;
	}
	/**
	 * Gets the id for a function name
	 * @param funcName the name of the function
	 * @return the id of the function
	 */
	public int getFunctionId(String funcName){
		return nameMap.get(funcName);
	}
	
	public int getSchemaNameID(String schemaName){
		return nameMap.get(schemaName+"_"+SCHEMA_ENDING);
	}
	
	public String getSchemaNameByID(int schemaID){
		return idMap.get(schemaID);
	}
	
	
	//GETTER METHODS
	public HashMap<Integer, PGClass> getRelationMap() {
		return relationMap;
	}

	public HashMap<String, FormPGAttributeClass> getAttributeMap() {
		return attributeMap;
	}

	public HashMap<String, Integer> getNameMap() {
		return nameMap;
	}
	public HashMap<Integer, String> getIdMap() {
		return idMap;
	}
	public HashMap<Integer, FormPGOperator> getOperatorMap() {
		return operatorMap;
	}
	
	

}
