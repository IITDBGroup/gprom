/**
 * 
 */
package org.gprom.jdbc.testing;

import static org.junit.Assert.*;

import java.util.ArrayList;
import java.util.Arrays;

import org.gprom.jdbc.pawd.Edge;
import org.gprom.jdbc.pawd.JSONVersionGraphStore;
import org.gprom.jdbc.pawd.Node;
import org.gprom.jdbc.pawd.VersionEdge;
import org.gprom.jdbc.pawd.VersionGraph;
import org.gprom.jdbc.pawd.VersionGraphStore;
import org.gprom.jdbc.pawd.VersionGraphStore.Operation;
import org.gprom.jdbc.pawd.VersionGraphStore.Operation.OpType;
import org.json.JSONObject;
import org.junit.Before;
import org.junit.Test;

/**
 * @author Amer
 *
 */
public class VersionGraphTest {
	VersionGraph VG1, VG2;

	/**
	 * @throws java.lang.Exception
	 */
	@Before
	public void setUp() throws Exception {
		//sample set of nodes
		Node R = new Node(false, "R");
		Node S = new Node(false, "S");
		Node T = new Node(false,"T");
		Node Rprime = new Node(false, "R'");
//		Node G = new Node(false, "G");
//		Node K = new Node(false, "K");
		Node So = new Node(false,"So");
//		Node J = new Node(false,"J");
//		Node F = new Node(false,"F");
		//Node R2 = new Node(false,"R2");
		//construct different arraylist for Edge construction
//		ArrayList<Node> NodeSet1 = new ArrayList<>( Arrays.asList(S,K));
//		ArrayList<Node> NodeSet2 = new ArrayList<>( Arrays.asList(So));
//		ArrayList<Node> NodeSet3 = new ArrayList<>( Arrays.asList(T,F,J));
//		ArrayList<Node> NodeSet4 = new ArrayList<>( Arrays.asList(K));
		//ArrayList<Node> NodeSet5 = new ArrayList<>( Arrays.asList(R));
		ArrayList<Node> NodeSetAll = new ArrayList<>(Arrays.asList(R,S,T,So,Rprime));
		//construct sample operations for edge creation
		Operation op1 = new Operation("SELECT a, b * 2 AS b FROM $$1$$",OpType.Query);
		Operation op2 = new Operation("SELECT sum(a), b FROM $$1$$ GROUP BY b",OpType.Update);
		//sample set of edges
		Edge edge1 = new Edge(R,S,op1);
		Edge edge2 = new Edge(S,T,op2);
//		Edge edge3 = new Edge(NodeSet1,NodeSet2,op2);
//		Edge edge4 = new Edge(So,G,op1);
//		Edge edge5 = new Edge(So,Rprime,op1);
//		Edge edge6 = new Edge(NodeSet3,NodeSet4,op2);
		//construct Arraylist of EDGES
		ArrayList<Edge> EdgeSetAll = new ArrayList<>( Arrays.asList(edge1,edge2));//,edge3,edge4,edge5,edge6));
		//construct the VersionEdges
		VersionEdge VE1 = new VersionEdge(R,Rprime);
		VersionEdge VE2 = new VersionEdge(S,So);
		ArrayList<VersionEdge> VersionEdgeSetAll = new ArrayList<>( Arrays.asList(VE1,VE2));
		//create a version graph
		VG1 = new VersionGraph(NodeSetAll, EdgeSetAll,VersionEdgeSetAll,null);
	}

	@Test
	public void testLoadSave() {
		VersionGraphStore myinterface = new JSONVersionGraphStore();
		myinterface.Configure(VG1);
		JSONObject ser = myinterface.Save(VG1);
		VG2 = myinterface.Load(ser);
		assertTrue(VG1.equals(VG1));
		assertTrue(VG1.equals(VG2));
		
	}
	@Test
	public void testCompose() {
		Node R = new Node(false, "R");
		Node S = new Node(false, "S");
//		Node Rprime = new Node(false, "R'");
//		Node G = new Node(false, "G");
//		Node K = new Node(false, "K");
//		Node R2 = new Node(false,"R2");
//		Node So = new Node(false,"So");
		Node T = new Node(false,"T");
//		Node J = new Node(false,"J");
//		Node F = new Node(false,"F");
		//System.out.println(node1);
		//construct different arraylist for Edge construction
//		ArrayList<Node> NodeSet1 = new ArrayList<>( Arrays.asList(S,K));
//		ArrayList<Node> NodeSet2 = new ArrayList<>( Arrays.asList(So));
//		ArrayList<Node> NodeSet3 = new ArrayList<>( Arrays.asList(T,F,J));
//		ArrayList<Node> NodeSet4 = new ArrayList<>( Arrays.asList(K));
//		ArrayList<Node> NodeSet5 = new ArrayList<>( Arrays.asList(node5));
		ArrayList<Node> NodeSetAll = new ArrayList<>(Arrays.asList(R,S,T));
		//construct sample operations for edge creation
		Operation op1 = new Operation("SELECT a, b * 2 AS b FROM $$1$$",OpType.Query);
		Operation op2 = new Operation("SELECT sum(a), b FROM $$1$$ GROUP BY b",OpType.Update);
		//sample set of edges
		Edge edge1 = new Edge(R,S,op1);
		Edge edge2 = new Edge(S,T,op2);
//		Edge edge3 = new Edge(NodeSet1,NodeSet2,op2);
//		Edge edge4 = new Edge(So,G,op1);
//		Edge edge5 = new Edge(So,Rprime,op1);
//		Edge edge6 = new Edge(NodeSet3,NodeSet4,op2);
		//construct Arraylist of EDGES
		ArrayList<Edge> EdgeSetAll = new ArrayList<>( Arrays.asList(edge1,edge2));//edge3,edge4,edge5,edge6));
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
		String finallia = myinterface.Compose(Graph1, T);
		System.out.println(finallia);
	}

}
