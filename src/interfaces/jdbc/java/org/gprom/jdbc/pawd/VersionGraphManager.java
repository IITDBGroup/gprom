package org.gprom.jdbc.pawd;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.gprom.jdbc.pawd.VersionGraphStore.Operation.Materialization;
import org.stringtemplate.v4.ST;

public class VersionGraphManager {
	//generates all combinations of nodes sets for Materialization plans
	private ArrayList<ArrayList<Node>> AllPlans(ArrayList<Node> input){
		int max_sequence_size = input.size();
		ArrayList <ArrayList<Node>> Plans = new ArrayList<ArrayList<Node>>();
		for(int i = 1; i<=max_sequence_size;i++){
			for(ArrayList<Node> com:genCombinationOfSizeK(i, input)){
				Plans.add(com);
			}
		}
		return Plans;
	}
	private ArrayList<ArrayList<Node>> genCombinationOfSizeK(int k, ArrayList<Node> input){
		// input array
        // sequence length   k
		ArrayList<ArrayList<Node>> subsets = new ArrayList<ArrayList<Node>>();
		int[] s = new int[k]; 
	    // here we'll keep indices 
		// pointing to elements in input array
		if (k <= input.size()) {
		    // first index sequence: 0, 1, 2, ...
		    for (int i = 0; (s[i] = i) < k - 1; i++);  
		    subsets.add(getSubset(input, s));
		    for(;;) {
		        int i;
		        // find position of item that can be incremented
		        for (i = k - 1; i >= 0 && s[i] == input.size() - k + i; i--); 
		        if (i < 0) {
		            break;
		        }
		        s[i]++;                    // increment this item
		        for (++i; i < k; i++) {    // fill up remaining items
		            s[i] = s[i - 1] + 1; 
		        }
		        subsets.add(getSubset(input, s));
		    }
		}
		return subsets;
	}
	

	// generate actual subset by index sequence
	private ArrayList<Node> getSubset(ArrayList<Node> input, int[] subset) {
		ArrayList<Node> result = new ArrayList<Node>(subset.length);
	    for (int i = 0; i < subset.length; i++) 
	        result.add(input.get(subset[i]));
	    return result;
	}
	//function will generate materialization plans for a given Graph starting at Node
	public ArrayList<Map<Node, Materialization>> genMaterializationPlans(VersionGraph V,Node n){
		ArrayList<Map<Node, Materialization>> AllPlans =  new ArrayList<Map<Node, Materialization>>();
		ArrayList<Node> input = generateSubGraph(V,n).getNodes();
		for(ArrayList<Node> plan:AllPlans(input)){
			Map<Node,Materialization> myPlan =  new HashMap<Node,Materialization>();
			for(Node t : input){
				myPlan.put(t,(plan.contains(t))? Materialization.isMaterialized: Materialization.notMaterialized);
			}
			AllPlans.add(myPlan);
		}
		return AllPlans;
	}
	
	
	private VersionGraph generateSubGraph (VersionGraph V, Node n){
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
			VG.Absorb(S);
		else
			VG.AddNode(j);
		for(int i = 1;i<e.getStartNodes().size();i++){
			Node t = e.getStartNodes().get(i);
			VersionGraph k = generateSubGraph(V,t);
			if (k != null) 
				VG.Absorb(k);
			else 
				VG.AddNode(t);
		}
		return VG;
	}
	//compose a qurey based on a materialization plan for a specific starting ndoe
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
	//helper function
	//construct an sql query of a series of nodes from start --> end 
	private String ConstructFromMany(VersionGraph V, ArrayList<Node> parents, String[] parentsSQL,Node child){
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
	//helper function
	//materialize a node
	//set the materialized value
	//add a new table to JDBC
	private String Materialize(String sql, Node node) {
			node.setMaterialized(true);
			//JDBCConnect conn= new JDBCConnect();
			String newName = "REL_"+node.getDescription();
			ST query = new ST("CREATE TABLE <NodeID> AS SELECT * FROM (<NodeDesc>)");
			query.add("NodeID", newName);
			query.add("NodeDesc", sql);
	//		String q = query.render();
			//conn.RunUpdate(q);
			return newName;
	}
	
	public void update(VersionGraph V, Node Rprime){
		Node R = V.getChildEdge(Rprime).getStartNodes().get(0);
		updateCall(V,R,Rprime);
	}
	private void updateCall(VersionGraph V, Node R, Node Rprime){
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
