/**
 *
 */
package org.gprom.jdbc.testing;

import org.gprom.jdbc.pawd.*;
import org.gprom.jdbc.pawd.Operation.OpType;
import org.junit.Before;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Properties;

import static org.junit.Assert.assertTrue;

/**
 * @author Amer
 *
 */
public class JSONVersionGraphLoadSave {
	private VersionGraph VG1;
    private VersionGraph VG2;

	/**
	 * @throws java.lang.Exception
	 */
	@Before
	public void setUp() throws Exception {
		VG1 = new VersionGraph();
		//sample set of nodes
		Node R = new Node(VG1.nodeIDGenerator(),false, "R");
		Node S = new Node(VG1.nodeIDGenerator(),false, "S");
		Node T = new Node(VG1.nodeIDGenerator(),false,"T");
		Node Rprime = new Node(VG1.nodeIDGenerator(),true, "R'");
//		Node G = new Node(false, "G");
//		Node K = new Node(false, "K");
		Node So = new Node(VG1.nodeIDGenerator(),true,"So");
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
		VG1.setEdges(EdgeSetAll);
		VG1.setVersionEdges(VersionEdgeSetAll);
		VG1.setNodes(NodeSetAll);
		VG1.Configure();
	}
	@Test
	public void test() throws Exception {
		JSONVersionGraphStore myStore = new JSONVersionGraphStore();
		Properties myProperty = new Properties();
		myProperty.setProperty("dir","C:\\Users\\Amer\\Desktop");
		myStore.initialize(myProperty);
		myStore.save(VG1);
		VG2 = myStore.load(VG1.getId());
		assertTrue(VG2.equals(VG1));


	}

}
