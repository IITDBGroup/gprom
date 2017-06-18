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
	
	public static synchronized String IDGenerator()
	{
	    return String.valueOf(idCounter++);
	}
	public static synchronized long getIdCounter() {
		return idCounter;
	}
	public static synchronized void setIdCounter(long idCounter) {
		VersionGraph.idCounter = idCounter;
	}
	
	ArrayList<Node> Nodes;
	ArrayList<Edge> Edges;
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

	public VersionGraph() {
		Nodes = new ArrayList<Node>();
		Edges = new ArrayList<Edge>();
		Configuration = null; 
	}
	public VersionGraph(ArrayList<Node> Nodes, ArrayList<Edge> Edges, String[] Configuration){
		this.Nodes= Nodes;
		this.Edges= Edges;
		this.Configuration= Configuration;
	}
	public VersionGraph(ArrayList<Node> Nodes, ArrayList<Edge> Edges){
		this.Nodes= Nodes;
		this.Edges= Edges;
		this.Configuration= null;
	}
	public void AddEdge(Edge d) {
		Edges.add(d);
	}
	public void AddNode(Node t){
		Nodes.add(t);
	}

	@Override
	public String toString() {
		return "VersionGraph ["
				+ "\nNodes=\n" + Nodes + "\n Edges=\n" + Edges + "\n Configuration=" + Arrays.toString(Configuration)
				+ "\n]"
				+ "IdCounter = "+ idCounter;
	}

}

