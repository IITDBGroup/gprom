/**
 * 
 */
package org.gprom.jdbc.pawd;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.gprom.jdbc.pawd.VersionGraphStore.Operation;

/**
 * @author Amer
 *
 */
public class Edge {
	
	List<Node> startNodes;
	List<Node> endNodes;
	Operation transformation;
	

	/**
	 * @return the startNodes
	 */
	public List<Node> getStartNodes() {
		return startNodes;
	}


	/**
	 * @param startNodes the startNodes to set
	 */
	public void setStartNodes(List<Node> startNodes) {
		this.startNodes = startNodes;
	}


	/**
	 * @return the endNodes
	 */
	public List<Node> getEndNodes() {
		return endNodes;
	}


	/**
	 * @param endNodes the endNodes to set
	 */
	public void setEndNodes(List<Node> endNodes) {
		this.endNodes = endNodes;
	}


	/**
	 * @return the transformation
	 */
	public Operation getTransformation() {
		return transformation;
	}


	/**
	 * @param transformation the transformation to set
	 */
	public void setTransformation(Operation transformation) {
		this.transformation = transformation;
	}


	/**
	 * default constructor
	 */
	public Edge() {
		this.startNodes= null;
		this.endNodes = null;
		this.transformation= null;
	}



	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString() {
		return "Edge[\n startNodes=\n" + startNodes + "\n endNodes=\n" + endNodes + "\n Transformation=" + transformation + "]\n";
	}


	/**
	 * Non-default constructor 
	 */
	public Edge(List<Node> startNodes, List<Node> endNodes, Operation Transformation){
		this.startNodes = startNodes;
		this.endNodes = endNodes;
		this.transformation = Transformation;
	}
	
	public Edge(Node startNode, Node endNode, Operation Transformation){
		this.startNodes= new ArrayList<Node>( Arrays.asList(startNode));
		this.endNodes = new ArrayList<Node>(Arrays.asList(endNode));
		this.transformation= Transformation;
	}
	
	@Override
	public boolean equals(Object other){
	    if (other == null) return false;
	    if (other == this) return true;
	    if (!(other instanceof Edge))return false;
	    Edge otherEdge = (Edge)other;
	    if(otherEdge.getEndNodes().equals(endNodes)){
	    	if(otherEdge.getStartNodes().equals(startNodes)){
	    		if(transformation.equals(otherEdge.getTransformation())){
	    			return true;
	    		}
	    	}	
	    }
	    return false;
	}

	@Override
	public int hashCode() {
		int result = startNodes.hashCode();
		result = result * 31 + endNodes.hashCode();
		result = result * 31 + transformation.hashCode();
		
		return result;
	}
	
	
}
