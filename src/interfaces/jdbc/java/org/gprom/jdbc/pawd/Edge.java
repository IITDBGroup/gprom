/**
 * 
 */
package org.gprom.jdbc.pawd;

import java.util.ArrayList;

/**
 * @author Amer
 *
 */
public class Edge {
	
	ArrayList<Node> startNodes;
	ArrayList<Node> endNodes;
	String Transformation;
	

	/**
	 * @return the startNodes
	 */
	public ArrayList<Node> getStartNodes() {
		return startNodes;
	}


	/**
	 * @param startNodes the startNodes to set
	 */
	public void setStartNodes(ArrayList<Node> startNodes) {
		this.startNodes = startNodes;
	}


	/**
	 * @return the endNodes
	 */
	public ArrayList<Node> getEndNodes() {
		return endNodes;
	}


	/**
	 * @param endNodes the endNodes to set
	 */
	public void setEndNodes(ArrayList<Node> endNodes) {
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







	@Override
	public String toString() {
		return startNodes + ", " + endNodes + "," + Transformation;
	}


	/**
	 * Non-default constructor 
	 */
	public Edge(ArrayList<Node> startNodes, ArrayList<Node> endNodes, String Transformation){
		this.startNodes = startNodes;
		this.endNodes = endNodes;
		this.Transformation = Transformation;
	}

}
