/**
 * 
 */
package org.gprom.jdbc.testing;

import static org.junit.Assert.*;

import org.gprom.jdbc.pawd.Node;
import org.gprom.jdbc.pawd.VersionEdge;
import org.junit.Before;
import org.junit.Test;

/**
 * @author Amer
 *
 */
public class VersionEdgeTest {
	VersionEdge ve1,ve2,ve3;
	/**
	 * @throws java.lang.Exception
	 */
	@Before
	public void setUp() throws Exception {
		Node G = new Node(false, "G");
		Node K = new Node(false, "K");
		Node R = new Node(false, "R");
		Node S = new Node(false, "S");
		ve1 = new VersionEdge(G,K);
		ve2 = new VersionEdge(R,S);
		ve3 = new VersionEdge(R,K);
		
	}

	@Test
	public void test() {
		assertTrue(ve1.equals(ve1));
		assertFalse(ve3.equals(ve1));
		assertFalse(ve1.equals(ve2));
		assertFalse(ve1.equals(ve3));
		
	}

}
