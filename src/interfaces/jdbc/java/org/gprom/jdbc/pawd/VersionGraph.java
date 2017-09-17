/**
 * 
 */
package org.gprom.jdbc.pawd;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * @author Amer
 *
 */
public class VersionGraph {
	
	/**
	 * 
	 */
	private String id;
	private long nodeIDCounter = 0;
	private ArrayList <Node> Nodes;
	private ArrayList<Edge> Edges;
	private ArrayList<VersionEdge> VersionEdges;
	private String [] Configuration;

	public String getId() {
		return id;
	}

	public void setId(String id) {
		this.id = id;
	}
	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	
	public String nodeIDGenerator(){
	    return String.valueOf(nodeIDCounter++);
	}
	public long getNodeIDCounter() {
		return nodeIDCounter;
	}
	public void setNodeIDCounter(long nodeIDCounter) {
		this.nodeIDCounter = nodeIDCounter;
	}
	

	
 
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
	 * @param configuration the configuration to set
	 */
	public void setConfiguration(String[] configuration) {
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
		id = String.valueOf(GraphIdGenerator.getInstance().generateNextId());
		Nodes = new ArrayList<Node>();
		Edges = new ArrayList<Edge>();
		VersionEdges = new ArrayList<VersionEdge>();
		Configuration = null; 
		
	}
	public VersionGraph(String id, ArrayList<Node> Nodes, ArrayList<Edge> Edges
			,ArrayList<VersionEdge> VersionEdges, String[] Configuration){
		this.id = id;
		this.Nodes= Nodes;
		this.Edges= Edges;
		this.VersionEdges= VersionEdges;
		this.Configuration= Configuration;
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
		Configure();
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
				+ "IdCounter = "+ nodeIDCounter;
	}
	@Override
	public boolean equals(Object other){
	    if (other == null) return false;
	    if (other == this) return true;
	    if (!(other instanceof VersionGraph))return false;
	    VersionGraph otherVG = (VersionGraph)other;
		boolean nodes, edges, vedges , conf;
		conf = Arrays.equals(Configuration,otherVG.getConfiguration());
		nodes = Nodes.equals(otherVG.getNodes());
		edges = otherVG.getEdges().equals(Edges);
		vedges =VersionEdges.equals(otherVG.getVersionEdges());
	    return conf && nodes && edges && vedges;
	}
	@Override 
	public int hashCode(){
		int result = Nodes.hashCode();
		result = result *31 + Edges.hashCode();
		result = result *31 + VersionEdges.hashCode();
		result = result *31 + Configuration.hashCode();
		return result;
	}
	public void Absorb(VersionGraph V) {
		ArrayList<Node> newNodes = new ArrayList<Node>();
		ArrayList<Edge> newEdges = new ArrayList<Edge>();
		newNodes.addAll(V.getNodes());
		newEdges.addAll(V.getEdges());
		if(getNodes() !=null  && getEdges()!= null) {
			newNodes.removeAll(getNodes());
			newNodes.addAll(getNodes());
			newEdges.removeAll(getEdges());
			newEdges.addAll(getEdges());
		}
		this.setEdges(newEdges);
		this.setNodes(newNodes);
	}

    public void Configure(){
        List<String> config = new ArrayList<>();
        for(Node t: getNodes()){
            if (t.Materialized){
                config.add(t.getId());
            }
        }
        setConfiguration(config.toArray(new String[0]));
    }
}

