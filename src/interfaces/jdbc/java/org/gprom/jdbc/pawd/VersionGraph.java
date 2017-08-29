/**
 * 
 */
package org.gprom.jdbc.pawd;

import java.util.ArrayList;
import java.util.Arrays;

/**
 * @author Amer
 *
 */
public class VersionGraph {
	
	/**
	 * 
	 */
	private static long idCounter = 0;
	
	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	
	public static String IDGenerator(){
	    return String.valueOf(idCounter++);
	}
	public static long getIdCounter() {
		return idCounter;
	}
	public static void setIdCounter(long idCounter) {
		VersionGraph.idCounter = idCounter;
	}
	
	ArrayList<Node> Nodes;
	ArrayList<Edge> Edges;
	ArrayList<VersionEdge> VersionEdges;
	String [] Configuration;
	
	
 
	/**
	 * @return the nodes
	 */
	public ArrayList<Node> getNodes() {
		return Nodes;
	}


	/**
	 * @param nodes the nodes to set
	 */
	public void setNodes(ArrayList<Node> nodes) {
		Nodes = nodes;
	}


	/**
	 * @return the edges
	 */
	public ArrayList<Edge> getEdges() {
		return Edges;
	}


	/**
	 * @param edges the edges to set
	 */
	public void setEdges(ArrayList<Edge> edges) {
		Edges = edges;
	}


	/**
	 * @return the configuration
	 */
	public String[] getConfiguration() {
		return Configuration;
	}

	/**
	 * @param configuation the configuration to set
	 */
	public void setConfiguation(String[] configuration) {
		Configuration = configuration;
	}
	//get the edge that has Node node in the end nodes
	public Edge getChildEdge(Node node){
		for(Edge e: Edges){
			if(e.getEndNodes().get(0).equals(node)){
				return e;
			}
		}
		return null;
	}
	public Edge getParentEdge(ArrayList<Node> parents ,Node child){
		for(Edge e: Edges){
			if(e.getStartNodes().equals(parents) && e.getEndNodes().get(0).equals(child) ){
				return e;
			}
		}
		return null;
	}
	//get the edges that has Node node in the starting nodes
	public ArrayList<Edge> getParentEdges(Node node){
		ArrayList<Edge> parentedges = new ArrayList<Edge>();
		for(Edge e: Edges){
			if(e.getStartNodes().get(0).equals(node)){
				parentedges.add(e);
			}
		}
		return parentedges;
	}

	public VersionGraph() {
		Nodes = new ArrayList<Node>();
		Edges = new ArrayList<Edge>();
		VersionEdges = new ArrayList<VersionEdge>();
		Configuration = null; 
		
	}
	public VersionGraph(ArrayList<Node> Nodes, ArrayList<Edge> Edges
			,ArrayList<VersionEdge> VersionEdges, String[] Configuration){
		this.Nodes= Nodes;
		this.Edges= Edges;
		this.VersionEdges= VersionEdges;
		this.Configuration= Configuration;
	}
	public VersionGraph(ArrayList<Node> Nodes, ArrayList<Edge> Edges,ArrayList<VersionEdge> VersionEdges){
		this.Nodes= Nodes;
		this.Edges= Edges;
		this.VersionEdges= VersionEdges;
		this.Configuration= null;
	}
	/**
	 * @return the versionEdges
	 */
	public ArrayList<VersionEdge> getVersionEdges() {
		return VersionEdges;
	}
	/**
	 * @param versionEdges the versionEdges to set
	 */
	public void setVersionEdges(ArrayList<VersionEdge> versionEdges) {
		VersionEdges = versionEdges;
	}
	public void AddEdge(Edge d) {
		Edges.add(d);
	}
	public void AddNode(Node t){
		Nodes.add(t);
	}
	public void AddVersionEdge(VersionEdge e){
		VersionEdges.add(e);
	}

	@Override
	public String toString() {
		return "VersionGraph ["
				+ "\nNodes=\n" + Arrays.toString(Nodes.toArray())
				+ "\n Edges=\n" + Arrays.toString(Edges.toArray())
				+ "\n VersionEdges=" + Arrays.toString(VersionEdges.toArray())
				+ "\n Configuration=" + Arrays.toString(Configuration)
				+ "\n]"
				+ "IdCounter = "+ idCounter;
	}
	@Override
	public boolean equals(Object other){
	    if (other == null) return false;
	    if (other == this) return true;
	    if (!(other instanceof VersionGraph))return false;
	    VersionGraph otherVG = (VersionGraph)other;
	    if (Configuration.equals(otherVG.getConfiguration())){
	    	if(Nodes.equals(otherVG.getNodes())){
		    	if(otherVG.getEdges().equals(Edges)){
	    			if(VersionEdges.equals(otherVG.getVersionEdges())){
	    				return true;
	    			}
	    		}
	    	}
	    }
	    return false;
	}
	@Override 
	public int hashCode(){
		int result = Nodes.hashCode();
		result = result *31 + Edges.hashCode();
		result = result *31 + VersionEdges.hashCode();
		result = result *31 + Configuration.hashCode();
		return result;
	}

}

