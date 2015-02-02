package org.gprom.jdbc.container;

import java.util.HashMap;


import org.apache.log4j.Logger;


public class DBCatalogTopContainer {

	private static Logger log = Logger.getLogger(DBCatalogTopContainer.class);
	private HashMap<Integer, String> universalOidMap;
	private HashMap<String, Integer> universalNameMap;
	private HashMap<String, Integer> aggregateMap;
	private String[] aggregateList = {
							"count",
							"max",
							"min",
							"avg",
							"sum"
	};
	private RelationHolder relationHolder;
	private TypeHolder typeHolder;
	
	//private HashMap<Class<DBCatalogTopContainer>, HashMap<String,Integer>> universalNameMap;
	
	public DBCatalogTopContainer(){
		universalOidMap = new HashMap<Integer, String>();
		universalNameMap = new HashMap<String, Integer>();
		aggregateMap = new HashMap<String, Integer>();
		for(int i = 0; i < aggregateList.length; i++){
			aggregateMap.put(aggregateList[i].toUpperCase(), putObjectInMap(aggregateList[i].toUpperCase()));
		}
	}
	
	public void createOtherContainer(DBCatalogTopContainer topContainer){
		relationHolder = new RelationHolder(topContainer);
		typeHolder = new TypeHolder(topContainer);
	}
	/**
	 * Returns a catalog object by an OID
	 * @param oid the unique OID of a catalog object
	 * @return the catalog object or null
	 */
	public String getObjectByOID(int oid){
		return (String) universalOidMap.get(oid);
	}
	
	/**
	 * Returns a catalog object by a name
	 * @param name the name of the catalog object
	 * @param typ the typ of the catalog object
	 * @return the catalog typ object or null
	 */
	public int getOIDByName(String name){
		return universalNameMap.get(name);
	}
	
	public void putObjectInMapByOid(int oid, String name){
		universalOidMap.put(oid, name);
		universalNameMap.put(name, oid);
	}
	
	/**
	 * Puts an object, if it doesn't exists, in the top map and returns an universal OID for this object
	 * @param name the string representation of the object
	 * @return the oid for the object
	 */
	public int putObjectInMap(String name){
		name = name.trim();
		if(!universalOidMap.containsValue(name)){
			int oid = getNewOID();
			putObjectInMapByOid(oid, name);
			return oid;
		}else{
			return universalNameMap.get(name);
		}
	}
	
	/**
	 * Gets an unique OID for putting in a map
	 * @return an OID < 10000
	 */
	public int getNewOID(){
		boolean found = false;
		int id = 0;
		while(!found){
			double a = Math.random();
			id = (int) Math.round(a * 100000);
			if(getObjectByOID(id) == null){
				found = true;
			}
		}
		return id;
	}
	
	public boolean checkForAggregateFunction(String name){
		if(aggregateMap.containsKey(name.toUpperCase())){
			return true;
		}else{
			return false;
		}
	}
	//Getter and Setter Methods
	public HashMap<Integer, String> getUniversalOidMap() {
		return universalOidMap;
	}
	public void setUniversalOidMap(HashMap<Integer, String> universalOidMap) {
		this.universalOidMap = universalOidMap;
	}
	
	public RelationHolder getRelationHolder() {
		return relationHolder;
	}
	
	public TypeHolder getTypeHolder() {
		return typeHolder;
	}

}
