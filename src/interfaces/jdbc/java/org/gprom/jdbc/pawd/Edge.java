/**
 * 
 */
package org.gprom.jdbc.pawd;

import java.util.Arrays;

/**
 * @author Amer
 *
 */
public class Edge {
	
	String[] startNodes;
	String[] endNodes;
	String Transformation;
	

	/**
	 * @return the startNodes
	 */
	public String[] getStartNodes() {
		return startNodes;
	}


	/**
	 * @param startNodes the startNodes to set
	 */
	public void setStartNodes(String[] startNodes) {
		this.startNodes = startNodes;
	}


	/**
	 * @return the endNodes
	 */
	public String[] getEndNodes() {
		return endNodes;
	}


	/**
	 * @param endNodes the endNodes to set
	 */
	public void setEndNodes(String[] endNodes) {
		this.endNodes = endNodes;
	}


	/**
	 * @return the transformation
	 */
	public String getTransformation() {
		return Transformation;
	}


	/**
	 * @param transformation the transformation to set
	 */
	public void setTransformation(String transformation) {
		Transformation = transformation;
	}


	/**
	 * default constructor
	 */
	public Edge() {
		this.startNodes= null;
		this.endNodes = null;
		this.Transformation= null;
		// TODO Auto-generated constructor stub
	}




	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString() {
		return Arrays.toString(startNodes) + ", " + Arrays.toString(endNodes)
				+ ", " + Transformation;
	}


	/**
	 * Non-default constructor 
	 */
	public Edge(String[] startNodes, String[] endNodes, String Transformation){
		this.startNodes = startNodes;
		this.endNodes = endNodes;
		this.Transformation = Transformation;
	}

}
