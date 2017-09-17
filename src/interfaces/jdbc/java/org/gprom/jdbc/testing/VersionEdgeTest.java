/**
 * 
 */
package org.gprom.jdbc.testing;

import org.gprom.jdbc.pawd.Node;
import org.gprom.jdbc.pawd.VersionEdge;
import org.junit.Before;
import org.junit.Test;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

/**
 * @author Amer
 *
 */
public class VersionEdgeTest {
	private VersionEdge ve1;
    private VersionEdge ve2;
    private VersionEdge ve3;
	private int idCount;
	/**
	 * @throws java.lang.Exception
	 */
	@Before
	public void setUp() throws Exception {
		Node G = new Node(dummyIDgenerator(),false, "G");
		Node K = new Node(dummyIDgenerator(),false, "K");
		Node R = new Node(dummyIDgenerator(),false, "R");
		Node S = new Node(dummyIDgenerator(),false, "S");
		ve1 = new VersionEdge(G,K);
		ve2 = new VersionEdge(R,S);
		ve3 = new VersionEdge(R,K);
		
	}
	private String dummyIDgenerator(){
		return(String.valueOf(idCount++));

	}

	@Test
	public void test() {
		assertTrue(ve1.equals(ve1));
		assertFalse(ve3.equals(ve1));
		assertFalse(ve1.equals(ve2));
		assertFalse(ve1.equals(ve3));
		
	}

}
