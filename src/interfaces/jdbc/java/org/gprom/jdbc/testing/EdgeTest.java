/**
 * 
 */
package org.gprom.jdbc.testing;

import org.gprom.jdbc.pawd.Edge;
import org.gprom.jdbc.pawd.Node;
import org.gprom.jdbc.pawd.Operation;
import org.gprom.jdbc.pawd.Operation.OpType;
import org.junit.Before;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

/**
 * @author Amer
 *
 */
public class EdgeTest {
	private Edge edge1;
	private Edge edge2;
	private Edge edge3;
	private Edge edge33;
	private Edge edge4;
	private Edge edge5;
	private Edge edge6;
	private int idCount = 0;
	/**
	 * @throws java.lang.Exception
	 */
	@Before
	public void setUp() throws Exception {
		
		Node R = new Node( dummyIDgenerator(),false, "R");
		Node S = new Node(dummyIDgenerator(),false, "S");
		Node T = new Node(dummyIDgenerator(),false,"T");
		Node Rprime = new Node(dummyIDgenerator(),false, "R'");
		Node G = new Node(dummyIDgenerator(),false, "G");
		Node K = new Node(dummyIDgenerator(),false, "K");
	//	Node R2 = new Node(false,"R2");
		Node So = new Node(dummyIDgenerator(),false,"So");
		Node J = new Node(dummyIDgenerator(),false,"J");
		Node F = new Node(dummyIDgenerator(),false,"F");
		
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
	private String dummyIDgenerator(){
		return(String.valueOf(idCount++));

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
