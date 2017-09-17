/**
 * 
 */
package org.gprom.jdbc.pawd;

import java.util.Date;

/**
 * @author Amer
 *
 */
public class VersionEdge {

	/**
	 * 
	 */
	 Node Original;
	 Node Derivative;
	 Date Time;
	public VersionEdge() {
		Original = null;
		Derivative = null;
		Time = new Date();
	}
	public VersionEdge(Node Original, Node Derivative, Date Time){
		this.Original= Original;
		this.Derivative= Derivative;
		this.Time = Time;
		
	}
	public VersionEdge(Node Original, Node Derivative){
		this.Original= Original;
		this.Derivative= Derivative;
		Time = new Date();
	}
	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString() {
		return "VersionEdge [\n Original=" + Original + " Derivative=" + Derivative + " Time=" + Time + "]\n";
	}
	/**
	 * @return the original
	 */
	public Node getOriginal() {
		return Original;
	}
	/**
	 * @param original the original to set
	 */
	public void setOriginal(Node original) {
		Original = original;
	}
	/**
	 * @return the derivative
	 */
	public Node getDerivative() {
		return Derivative;
	}
	/**
	 * @param derivative the derivative to set
	 */
	public void setDerivative(Node derivative) {
		Derivative = derivative;
	}
	/**
	 * @return the time
	 */
	public Date getTime() {
		return Time;
	}
	/**
	 * @param time the time to set
	 */
	public void setTime(Date time) {
		Time = time;
	}
	@Override
	public boolean equals(Object other){
	    if (other == null) return false;
	    if (other == this) return true;
	    if (!(other instanceof VersionEdge))return false;
	    VersionEdge otherVE = (VersionEdge) other;
        return otherVE.getOriginal().equals(Original)
                && otherVE.getDerivative().equals(Derivative)
                && otherVE.getTime().equals(Time);
	}
	@Override
	public int hashCode(){
		int result = Original.hashCode();
		result = result *31 + Derivative.hashCode();
		result = result *31 + Time.hashCode();
		return result;
	}
	
}
