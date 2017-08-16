/**
 * 
 */
package org.gprom.jdbc.testing;

import static org.junit.Assert.*;

import java.util.ArrayList;
import java.util.Arrays;

import org.gprom.jdbc.pawd.*;
import org.gprom.jdbc.pawd.VersionGraphStore.Operation;
import org.gprom.jdbc.pawd.VersionGraphStore.Operation.OpType;
import org.junit.Before;
import org.junit.Test;

/**
 * @author Amer
 *
 */
public class EdgeTest {
	Edge edge1, edge2,edge3,edge33,edge4,edge5,edge6;
	/**
	 * @throws java.lang.Exception
	 */
	@Before
	public void setUp() throws Exception {
		
		Node R = new Node(false, "R");
		Node S = new Node(false, "S");
		Node T = new Node(false,"T");
		Node Rprime = new Node(false, "R'");
		Node G = new Node(false, "G");
		Node K = new Node(false, "K");
	//	Node R2 = new Node(false,"R2");
		Node So = new Node(false,"So");
		Node J = new Node(false,"J");
		Node F = new Node(false,"F");
		
		ArrayList<Node> NodeSet1 = new ArrayList<>( Arrays.asList(S,K));
		ArrayList<Node> NodeSet2 = new ArrayList<>( Arrays.asList(So));
		ArrayList<Node> NodeSet3 = new ArrayList<>( Arrays.asList(T,F,J));
		ArrayList<Node> NodeSet4 = new ArrayList<>( Arrays.asList(K));
		
		
		Operation op1 = new Operation("SELECT a, b * 2 AS b FROM $$1$$",OpType.Query);
		Operation op2 = new Operation("SELECT sum(a), b FROM $$1$$ GROUP BY b",OpType.Update);
		//sample set of edges
		edge1 = new Edge(R,S,op1);
		edge2 = new Edge(S,T,op2);
		edge3 = new Edge(NodeSet1,NodeSet2,op2);
		edge33 = new Edge(NodeSet1,NodeSet2,op2);
		edge4 = new Edge(So,G,op1);
		edge5 = new Edge(So,Rprime,op1);
		edge6 = new Edge(NodeSet3,NodeSet4,op2);
	}

	@Test
	public void test() {
		assertFalse(edge1.equals(edge2));
		assertTrue(edge1.equals(edge1));
		assertTrue(edge33.equals(edge3));
		assertTrue(edge3.equals(edge33));
		assertFalse(edge4.equals(edge5));
		assertFalse(edge5.equals(edge6));
		
	}

}
