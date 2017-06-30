/**
 * 
 */
package org.gprom.jdbc.pawd;

import java.util.ArrayList;
import java.util.Arrays;

import org.gprom.jdbc.pawd.VersionGraphStore.Operation;
import org.gprom.jdbc.pawd.VersionGraphStore.Operation.OpType;
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
		Node R = new Node(false, "R");
		Node S = new Node(false, "S");
		Node Rprime = new Node(false, "R'");
		Node G = new Node(false, "G");
		Node K = new Node(false, "K");
		Node R2 = new Node(false,"R2");
		Node So = new Node(false,"So");
		Node T = new Node(false,"T");
		Node J = new Node(false,"J");
		Node F = new Node(false,"F");
		//System.out.println(node1);
		//construct different arraylist for Edge construction
		ArrayList<Node> NodeSet1 = new ArrayList<>( Arrays.asList(S,K));
		ArrayList<Node> NodeSet2 = new ArrayList<>( Arrays.asList(So));
		ArrayList<Node> NodeSet3 = new ArrayList<>( Arrays.asList(T,F,J));
		ArrayList<Node> NodeSet4 = new ArrayList<>( Arrays.asList(K));
//		ArrayList<Node> NodeSet5 = new ArrayList<>( Arrays.asList(node5));
		ArrayList<Node> NodeSetAll = new ArrayList<>(Arrays.asList(R,S,Rprime,K,R2,G,So,T,F,J));
		//construct sample operations for edge creation
		Operation op1 = new Operation("SELECT * FROM R",OpType.Query);
		Operation op2 = new Operation("UPDATE R SET x=0 *",OpType.Update);
		//sample set of edges
		Edge edge1 = new Edge(R,S,op1);
		Edge edge2 = new Edge(K,Rprime,op2);
		Edge edge3 = new Edge(NodeSet1,NodeSet2,op2);
		Edge edge4 = new Edge(So,G,op1);
		Edge edge5 = new Edge(So,Rprime,op1);
		Edge edge6 = new Edge(NodeSet3,NodeSet4,op2);
		//construct Arraylist of EDGES
		ArrayList<Edge> EdgeSetAll = new ArrayList<>( Arrays.asList(edge1,edge2,edge3,edge4,edge5,edge6));
		//System.out.println(EdgeSetAll);
		//construct the VersionEdges
	//	VersionEdge VE1 = new VersionEdge(node1,node2);
	//	VersionEdge VE2 = new VersionEdge(node1,node3);
		//construct arraylist of versionedges
		ArrayList<VersionEdge> VersionEdgeSetAll = new ArrayList<>();
		//create a version graph
		VersionGraph Graph1 = new VersionGraph(NodeSetAll, EdgeSetAll,VersionEdgeSetAll,null);
		VersionGraphStore myinterface = new JSONVersionGraphStore();
		myinterface.Configure(Graph1);
//		this will test the update method.
//		System.out.println(Graph1);
//		myinterface.UpdateCall(Graph1,Rprime);
		System.out.println(myinterface.getPath(Graph1, So));
		
//this will test how load and save will work
//		JSONObject ser = myinterface.Save(Graph1);
//		System.out.print(ser.toString(1));
//		System.out.println(myinterface.Load(ser));
		

	}

}
