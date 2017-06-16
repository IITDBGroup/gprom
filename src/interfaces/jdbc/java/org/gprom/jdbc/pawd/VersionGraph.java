/**
 * 
 */
package org.gprom.jdbc.pawd;

import java.util.ArrayList;

/**
 * @author Amer
 *
 */
public class VersionGraph {
	
	/**
	 * 
	 */
	public static long idCounter = 0;
	
	public static synchronized String IDGenerator()
	{
	    return String.valueOf(idCounter++);
	}
	public static synchronized String IDCurrent(){
		return String.valueOf(idCounter);
	}
	public static synchronized void setidCounter(long count){
		idCounter= count;
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
	public void AddEdge(Edge d) {
		Edges.add(d);
	}
	public void AddNode(Node t){
		Nodes.add(t);
	}
	

}

