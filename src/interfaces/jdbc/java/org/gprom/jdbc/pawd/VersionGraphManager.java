package org.gprom.jdbc.pawd;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.gprom.jdbc.pawd.VersionGraphStore.Operation.Materialization;
import org.stringtemplate.v4.ST;

public class VersionGraphManager {
	//helper function
	public VersionGraph Absorb(VersionGraph V,VersionGraph G) {
		ArrayList<Node> newNodes = new ArrayList<Node>();
		ArrayList<Edge> newEdges = new ArrayList<Edge>();
		newNodes.addAll(V.getNodes());
		newEdges.addAll(V.getEdges());
		if(G.getNodes() !=null  && G.getEdges()!= null) {
			newNodes.removeAll(G.getNodes());
			newNodes.addAll(G.getNodes());
			newEdges.removeAll(G.getEdges());
			newEdges.addAll(G.getEdges());
		}
		V.setEdges(newEdges);
		V.setNodes(newNodes);
		return V;
	}
	public ArrayList<Map<Node, Materialization>> getPath(VersionGraph V,Node n){
		ArrayList<Map<Node, Materialization>> Plans =  new ArrayList<Map<Node, Materialization>>();
		ArrayList<Node> Nodes = generateSubGraph(V,n).getNodes();
		
		//System.out.println(
		//consider that you can filter out these Subgraphs by checking the materialized attribute
		//then return this list again
		//the formula for counting the number of possible materialization plans is exponential
		//it is in the = 2^n0 * 2^n1  where n is the number of nodes in a branch
				//);
		return Plans;
	}
	public VersionGraph generateSubGraph (VersionGraph V, Node n){
		Edge e = V.getChildEdge(n);
		if(e == null)
			return null;
		VersionGraph VG = new VersionGraph();
		VG.AddNode(n);
		VG.AddEdge(e);
		if(n.isMaterialized())
			return VG;
		Node j = e.getStartNodes().get(0);
		VersionGraph S = generateSubGraph(V,j);
		if(S != null) 
			VG= Absorb(VG,S);
		else
			VG.AddNode(j);
		for(int i = 1;i<e.getStartNodes().size();i++){
			Node t = e.getStartNodes().get(i);
			VersionGraph k = generateSubGraph(V,t);
			if (k != null) 
				VG = Absorb(VG,k);
			else 
				VG.AddNode(t);
		}
		return VG;
	}
	public String Compose(VersionGraph V, Node startNode, Map<Node,Materialization> MPlan){
		Edge currentEdge = V.getChildEdge(startNode);
		String [] parentsSQL = null;
		ArrayList<Node> parents = null;
		if(currentEdge!=null){
			int num_of_parents = currentEdge.getStartNodes().size();
			parentsSQL = new String[num_of_parents] ;
			parents = new ArrayList<Node>();
			//this block of code is temporary
			for(int i=0;i<num_of_parents;i++){
				Node current = currentEdge.getStartNodes().get(i);
				parents.add(current);
				parentsSQL[i] = Compose(V,current,MPlan);
				}
		}
		if(MPlan.get(startNode)== Materialization.isMaterialized){
			return  Materialize(ConstructFromMany(V,parents,parentsSQL,startNode), startNode);
		}else{
			return  ConstructFromMany(V,parents,parentsSQL,startNode);
		}
	}
	
	//construct an sql query of a series of nodes from start --> end 
	public String ConstructFromMany(VersionGraph V, ArrayList<Node> parents, String[] parentsSQL,Node child){
		Edge ce = V.getParentEdge(parents, child);
		if(ce == null){
			return child.getDescription();
		}
		String q = ce.getTransformation().getCode();
		String pattern = "(\\${2}\\d\\${2})";
		Pattern r = Pattern.compile(pattern);
		Matcher m = r.matcher(q);
		StringBuffer sb = new StringBuffer();
		String replacement = "";
		int index;
		while(m.find()){
			index = Integer.parseInt(q.substring(m.start()+2, m.end()-2));
			replacement= "("+parentsSQL[index-1]+")";
			m.appendReplacement(sb, replacement);
		}
		m.appendTail(sb);
		return sb.toString();
	}
	
	//materialize a node
	//set the materialized value
	//add a new table to JDBC
	public String Materialize(String sql, Node node) {
			node.setMaterialized(true);
			JDBCConnect conn= new JDBCConnect();
			String newName = "REL_"+node.getDescription();
			ST query = new ST("CREATE TABLE <NodeID> AS SELECT * FROM (<NodeDesc>)");
			query.add("NodeID", newName);
			query.add("NodeDesc", sql);
			String q = query.render();
			//conn.RunUpdate(q);
			return newName;
	}
	
	public void update(VersionGraph V, Node Rprime){
		Node R = V.getChildEdge(Rprime).getStartNodes().get(0);
		updateCall(V,R,Rprime);
	}
	public void updateCall(VersionGraph V, Node R, Node Rprime){
		Edge u1 = V.getChildEdge(Rprime);
		if(V.getParentEdges(R).isEmpty())
			return;
		//removing the edge we started with
		ArrayList<Edge> edges = V.getParentEdges(R);
		edges.remove(u1);
		for(Edge Q : edges){
			for(Node S: Q.getEndNodes()){
				Node newnode = new Node();
				String description = S.getDescription();
				newnode.setDescription(description +"'");
				//creating the new edge 
				ArrayList<Node> startnodes = new ArrayList<>(Arrays.asList(Rprime));
				//check for the case where we have multiple input nodes for a given edge
				startnodes.addAll(Q.startNodes);
				startnodes.remove(R);
				ArrayList<Node> endnodes = new ArrayList<>(Arrays.asList(newnode));
				Edge newedge = new Edge(startnodes,endnodes,Q.getTransformation());
				//creating the version edge
				VersionEdge newve= new VersionEdge(S,newnode);
				V.AddVersionEdge(newve);
				V.AddEdge(newedge);
				V.AddNode(newnode);
				updateCall(V, S ,newnode);
			}
		}
	}
	
	public void Configure(VersionGraph V){
		List<String> config = new ArrayList<>();
		for(Node t: V.getNodes()){
			if (t.Materialized){
				config.add(t.getId());
			}
		}
		V.setConfiguation(config.toArray(new String[0]));
	}
}
