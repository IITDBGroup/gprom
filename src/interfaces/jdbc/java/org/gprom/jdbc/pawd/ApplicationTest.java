/**
 * 
 */
package org.gprom.jdbc.pawd;


import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.json.JSONException;

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
		//sample set of nodes
//		Node R = new Node(false, "R");
//		Node S = new Node(false, "S");
//		Node Rprime = new Node(false, "R'");
//		Node G = new Node(false, "G");
//		Node K = new Node(false, "K");
//		Node R2 = new Node(false,"R2");
//		Node So = new Node(false,"So");
//		Node T = new Node(false,"T");
//		Node J = new Node(false,"J");
//		Node F = new Node(false,"F");
		//System.out.println(node1);
		//construct different arraylist for Edge construction
//		ArrayList<Node> NodeSet1 = new ArrayList<>( Arrays.asList(S,K));
//		ArrayList<Node> NodeSet2 = new ArrayList<>( Arrays.asList(So));
//		ArrayList<Node> NodeSet3 = new ArrayList<>( Arrays.asList(T,F,J));
//		ArrayList<Node> NodeSet4 = new ArrayList<>( Arrays.asList(K));
//		ArrayList<Node> NodeSet5 = new ArrayList<>( Arrays.asList(node5));
//		ArrayList<Node> NodeSetAll = new ArrayList<>(Arrays.asList(R,S,T));
		//construct sample operations for edge creation
//		Operation op1 = new Operation("SELECT a, b * 2 AS b FROM $$1$$",OpType.Query);
//		Operation op2 = new Operation("SELECT sum(a), b FROM $$1$$ GROUP BY b",OpType.Update);
		//sample set of edges
//		Edge edge1 = new Edge(R,S,op1);
//		Edge edge2 = new Edge(S,T,op2);
//		Edge edge3 = new Edge(NodeSet1,NodeSet2,op2);
//		Edge edge4 = new Edge(So,G,op1);
//		Edge edge5 = new Edge(So,Rprime,op1);
//		Edge edge6 = new Edge(NodeSet3,NodeSet4,op2);
		//construct Arraylist of EDGES
//		ArrayList<Edge> EdgeSetAll = new ArrayList<>( Arrays.asList(edge1,edge2));//edge3,edge4,edge5,edge6));
		//System.out.println(EdgeSetAll);
		//construct the VersionEdges
	//	VersionEdge VE1 = new VersionEdge(node1,node2);
	//	VersionEdge VE2 = new VersionEdge(node1,node3);
		//construct arraylist of versionedges
//		ArrayList<VersionEdge> VersionEdgeSetAll = new ArrayList<>();
		//create a version graph
//		VersionGraph Graph1 = new VersionGraph(NodeSetAll, EdgeSetAll,VersionEdgeSetAll,null);
//		VersionGraphStore myinterface = new JSONVersionGraphStore();
//		myinterface.Configure(Graph1);
//		String finallia = myinterface.Compose(Graph1, T);
//		System.out.println(finallia);
//		this will test the update method.
//		System.out.println(Graph1);
//		myinterface.UpdateCall(Graph1,Rprime);
//		System.out.println(myinterface.getPath(Graph1, So));
		String q = "SELECT a, b * 2 AS b FROM $$1$$ and $$139999$$";
		matt(q);

		

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
