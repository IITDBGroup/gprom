/**
 * 
 */
package org.gprom.jdbc.pawd;


import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.gprom.jdbc.pawd.VersionGraphStore.Operation;
import org.gprom.jdbc.pawd.VersionGraphStore.Operation.Materialization;
import org.gprom.jdbc.pawd.VersionGraphStore.Operation.OpType;
import org.json.JSONException;
import org.stringtemplate.v4.ST;

/**
 * @author Amer
 *
 */
public class ApplicationTest {

	/**
	 * @param args
	 * @throws JSONException 
	 */
	public static void main(String[] args) throws JSONException {
		VersionGraph Graph1 ;
		Node T;
		Map <Node, Materialization> MaterializationPlan = new HashMap<Node, Materialization>();
		Node R = new Node(false, "R");
		Node S = new Node(false, "S");
		T = new Node(false,"T");
		Node J = new Node(false,"J");
		//construct different arraylist for Edge construction
		ArrayList<Node> NodeSetAll = new ArrayList<>(Arrays.asList(J,R,S,T));
		ArrayList<Node> startNodes = new ArrayList<>(Arrays.asList(S,J));
		ArrayList<Node> endNodes = new ArrayList<>(Arrays.asList(T));
		//construct sample operations for edge creation
		Operation op1 = new Operation("SELECT a, b * 2 AS b FROM $$1$$ ",OpType.Query);
//		Operation op2 = new Operation("SELECT sum(a), b FROM $$1$$ GROUP BY b",OpType.Update);
//		Operation op3 = new Operation ("SELECT b FROM $$1$$",OpType.Query);
		Operation op2 = new Operation("SELECT sum(a), b FROM ($$1$$ NATURAL JOIN $$2$$) GROUP BY b",OpType.Query);
		//sample set of edges
		Edge edge2 = new Edge(startNodes,endNodes,op2);
		Edge edge1 = new Edge(R,S, op1);
//		Edge edge2 = new Edge(S,T,op2);
//		Edge edge3 = new Edge(T,J,op3);
		//construct Arraylist of EDGES
		ArrayList<Edge> EdgeSetAll = new ArrayList<>( Arrays.asList(edge1,edge2));//edge3,edge4,edge5,edge6));
		//System.out.println(EdgeSetAll);
		//construct arraylist of versionedges
		ArrayList<VersionEdge> VersionEdgeSetAll = new ArrayList<>();
		//create a version graph
		Graph1 = new VersionGraph(NodeSetAll, EdgeSetAll,VersionEdgeSetAll,null);


		//setting up materialization plan
		System.out.println(Graph1);
		MaterializationPlan.put(R,Materialization.notMaterialized );
		MaterializationPlan.put(S,Materialization.notMaterialized );
		MaterializationPlan.put(T,Materialization.notMaterialized);
		MaterializationPlan.put(J,Materialization.notMaterialized);
		String q = Compose(Graph1, T,MaterializationPlan);
		System.out.println(q);

	}
	
	
	public static String Compose(VersionGraph V, Node startNode, Map<Node,Materialization> MPlan){
		Edge currentEdge = V.getChildEdge(startNode);
		String [] parentsSQL = null;
		ArrayList<Node> parents = null;
		if(currentEdge!=null){
			int num_of_parents = currentEdge.getStartNodes().size();
			parentsSQL = new String[num_of_parents] ;
			parents = new ArrayList<Node>();
			//this block of code is temporary
			for(int i=0;i<num_of_parents;i++){
				System.out.println("looping "+i);
				System.out.println("Edge is "+ currentEdge);
				Node current = currentEdge.getStartNodes().get(i);
				System.out.println("current Node "+ current);
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
	public static String ConstructFromMany(VersionGraph V, ArrayList<Node> parents, String[] parentsSQL,Node child){
		System.out.println("Construction from MANY");
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
		System.out.println("replacement of ConstructMany : "+replacement);
		return sb.toString();
	}
	public static String Construct(VersionGraph V, Node start, Node end,String parentQ) {
		Edge ce = V.getChildEdge(start);
		if(ce == null) {
//			System.out.println("Construct start "+ start.getDescription());
//			System.out.println("Construct end "+ end.getDescription());
//			System.out.println("the return"+end.getDescription());
			return start.getDescription();
		}
		String q = ce.getTransformation().getCode();
		String pattern = "(\\${2}\\d\\${2})";
		Pattern r = Pattern.compile(pattern);
		Matcher m = r.matcher(q);
		StringBuffer sb = new StringBuffer();
		String replacement;
		m.find();
		int index = Integer.parseInt(q.substring(m.start()+2, m.end()-2));
		if(start.equals(end)){
			replacement ="("+ parentQ+")";
		}
		else{
			start = ce.getStartNodes().get(index-1);
			replacement= "("+Construct(V,start,end,parentQ)+")";
		}
		m.appendReplacement(sb, replacement);
		
		//System.out.println(m.find());
		m.appendTail(sb);
//		System.out.println("replacement"+replacement);
		return sb.toString();
	}
	//materialize a node
	//set the materialized value
	//add a new table to JDBC
	public static String Materialize(String sql, Node node) {
			node.setMaterialized(true);
		//	JDBCConnect conn= new JDBCConnect();
			String newName = "REL_"+node.getDescription();
			ST query = new ST("CREATE TABLE <NodeID> AS SELECT * FROM (<NodeDesc>)");
			query.add("NodeID", newName);
			query.add("NodeDesc", sql);
			String q = query.render();
			//conn.RunUpdate(q);
			System.out.println(q);
			return newName;
	}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	public static void matt(String docToProcess ) {
		String pattern = "(\\${2}\\d*\\${2})";
		Pattern r = Pattern.compile(pattern);
		String replacement = "BYE";
		Matcher m = r.matcher(docToProcess);
		StringBuffer sb = new StringBuffer();
		int index;
		while (m.find()) {
			index = Integer.parseInt(docToProcess.substring(m.start()+2, m.end()-2));
			m.appendReplacement(sb, replacement);
			System.out.println(index);
		}
		m.appendTail(sb);
		System.out.println(sb.toString());
	}


}
