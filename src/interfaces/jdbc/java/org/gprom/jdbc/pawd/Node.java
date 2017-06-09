/**
 * 
 */
package org.gprom.jdbc.pawd;

/**
 * @author Amer
 *
 */
public class Node {

	/**
	 * 
	 */
	String Id;
	boolean Materialized;
	String Description;
	
	/**
	 * @return the id
	 */
	public String getId() {
		return Id;
	}

	/**
	 * @param id the id to set
	 */
	public void setId(String id) {
		Id = id;
	}

	/**
	 * @return the materialized
	 */
	public boolean isMaterialized() {
		return Materialized;
	}

	/**
	 * @param materialized the materialized to set
	 */
	public void setMaterialized(boolean materialized) {
		Materialized = materialized;
	}

	/**
	 * @return the description
	 */
	public String getDescription() {
		return Description;
	}

	/**
	 * @param description the description to set
	 */
	public void setDescription(String description) {
		Description = description;
	}

	public Node() {
		Id = "0";//I am not sure how versions will be saved so I can increment IDs.
		Materialized = false ;
		Description = null ;
		// TODO Auto-generated constructor stub
	}

	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString() {
		return Id + "," + Materialized + ", " + Description;
	}

}
