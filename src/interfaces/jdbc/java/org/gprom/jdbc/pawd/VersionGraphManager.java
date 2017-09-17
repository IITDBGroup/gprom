package org.gprom.jdbc.pawd;

import org.stringtemplate.v4.ST;


import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

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
	//helper function for genMaterializationPlans
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
	//helper function generate actual subset by index sequence
	private ArrayList<Node> getSubset(ArrayList<Node> input, int[] subset) {
		ArrayList<Node> result = new ArrayList<Node>(subset.length);
	    for (int i = 0; i < subset.length; i++)
	        result.add(input.get(subset[i]));
	    return result;
	}

	/**
	 * @param V parent versionGraph
	 * @param n node we want all the materialization plans for
	 * */
	public ArrayList<Map<Node, Operation.Materialization>> genMaterializationPlans(VersionGraph V, Node n){
		ArrayList<Map<Node, Operation.Materialization>> AllPlans =  new ArrayList<Map<Node, Operation.Materialization>>();
		ArrayList<Node> input = generateSubGraph(V,n).getNodes();
		for(ArrayList<Node> plan:AllPlans(input)){
			Map<Node,Operation.Materialization> myPlan =  new HashMap<Node,Operation.Materialization>();
			for(Node t : input){
				myPlan.put(t,(plan.contains(t))? Operation.Materialization.isMaterialized: Operation.Materialization.notMaterialized);
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
		if(n.getMaterialized())
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
	public String Compose(VersionGraph V, Node startNode, Map<Node,Operation.Materialization> MPlan){
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
		if(MPlan.get(startNode)== Operation.Materialization.isMaterialized){
			return  Materialize(ConstructFromMany(V,parents,parentsSQL,startNode), startNode);
		}else{
			return  ConstructFromMany(V,parents,parentsSQL,startNode);
		}
	}
	//	helper function construct an sql query of a series of nodes from start --> end
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
		String replacement;
		int index;
		while(m.find()){
			index = Integer.parseInt(q.substring(m.start()+2, m.end()-2));
			replacement= "("+parentsSQL[index-1]+")";
			m.appendReplacement(sb, replacement);
		}
		m.appendTail(sb);
		return sb.toString();
	}
	//	helper function materialize a node set the materialized value add a new table to JDBC*/
	private String Materialize(String sql, Node node) {
		String newName = "REL_"+node.getDescription();
		if(!node.getMaterialized()){
			node.setMaterialized(true);
			JDBCConnect conn= new JDBCConnect();
			ST query = new ST("CREATE TABLE <NodeID> AS SELECT * FROM (<NodeDesc>)");
			query.add("NodeID", newName);
			query.add("NodeDesc", sql);
			String q = query.render();
			conn.RunUpdate(q);
		}
		return newName;
	}

	/**
	 *
	 * @param V VersionGraph of parent node
	 * @param Rprime the node that has been created after an update
	 *  this function will create a versionEdge connecting parent node R and its child Rprime
	 */
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


	/**
	 *
	 * @param V VersionGraph of parent node
	 * @param n node we want to generate cheapest materialization plan for
	 *
	 * @return cheapest materialization plan
	 */
	public Map<Node, Operation.Materialization> genCheapestMaterializationPlanTree (VersionGraph V, Node n){
		// TODO implement simple Tree for "Cheapest Materialization plan"

		return null;
	}

}
