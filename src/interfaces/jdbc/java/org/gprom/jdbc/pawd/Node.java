/**
 * 
 */
package org.gprom.jdbc.pawd;
import java.text.SimpleDateFormat;
import java.util.Date;
/**
 * @author Amer
 *
 */
public class Node {
	
	/**
	 * 
	 */
	private String Id;
	boolean Materialized;
	private String Description;
	private Date Time;

	/**
	 * @return the time
	 */

	public Date getTime() {
		return Time;
	}
	/**
	 * @param time the Time to set
	 */
	public void setTime(Date time) {
		Time = time;
	}

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
	public boolean getMaterialized() {
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

	public Node(String ID, boolean Materialized, String Description) {
		Id = ID;
		this.Materialized = Materialized ;
		this.Description = Description ;
		Time = new Date();
	}

	public Node() {
		Id = "0";
		Materialized = false ;
		Description = null ;
		Time = new Date();
	}

	public Node( String ID, boolean Materialized, String Description,Date time){
		Id= ID;
		this.Materialized=Materialized;
		this.Description= Description;
		this.Time = time;
	}

	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString() {
		SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ssZ");
		return "[Id=" + Id + 
				", Materialized=" + Materialized + 
				", Description=" + Description + 
				", Time=" + sdf.format(Time)+ "]\n";
	}

	@Override
	public boolean equals(Object other){
	    if (other == null) return false;
	    if (other == this) return true;
	    if (!(other instanceof Node))return false;
	    Node otherNode = (Node)other;
	    return Id.equals(otherNode.getId())
	    	&& otherNode.getDescription().equals(Description)
	    	&& Materialized == otherNode.getMaterialized()
	    	&& otherNode.getTime().equals(Time);
	}
	
	@Override
	public int hashCode(){
		int result = Description.hashCode();
		result = result * 31 + Id.hashCode();
		result = result * 31 + Time.hashCode();
		result = result *31 + (Materialized ? 1 : 0);
		return result;
	}

}
