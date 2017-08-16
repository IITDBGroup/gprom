/**
 * 
 */
package org.gprom.jdbc.pawd;

import java.util.ArrayList;
import java.util.Arrays;

import org.gprom.jdbc.pawd.VersionGraphStore.Operation;

/**
 * @author Amer
 *
 */
public class Edge {
	
	ArrayList<Node> startNodes;
	ArrayList<Node> endNodes;
	Operation Transformation;
	

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
	public Operation getTransformation() {
		return Transformation;
	}


	/**
	 * @param transformation the transformation to set
	 */
	public void setTransformation(Operation transformation) {
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
		return "Edge[\n startNodes=\n" + startNodes + "\n endNodes=\n" + endNodes + "\n Transformation=" + Transformation + "]\n";
	}


	/**
	 * Non-default constructor 
	 */
	public Edge(ArrayList<Node> startNodes, ArrayList<Node> endNodes, Operation Transformation){
		this.startNodes = startNodes;
		this.endNodes = endNodes;
		this.Transformation = Transformation;
	}
	public Edge(Node startNode, Node endNode, Operation Transformation){
		this.startNodes= new ArrayList<>( Arrays.asList(startNode));
		this.endNodes = new ArrayList<>(Arrays.asList(endNode));
		this.Transformation= Transformation;
	}
	@Override
	public boolean equals(Object other){
	    if (other == null) return false;
	    if (other == this) return true;
	    if (!(other instanceof Edge))return false;
	    Edge otherEdge = (Edge)other;
	    if(otherEdge.getEndNodes().equals(endNodes)){
	    	if(otherEdge.getStartNodes().equals(startNodes)){
	    		if(Transformation.equals(otherEdge.getTransformation())){
	    			return true;
	    		}
	    	}	
	    }
	    return false;
	}

}
