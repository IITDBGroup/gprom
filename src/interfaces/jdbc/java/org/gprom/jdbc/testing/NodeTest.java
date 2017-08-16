/**
 * 
 */
package org.gprom.jdbc.testing;

import static org.junit.Assert.*;

import org.gprom.jdbc.pawd.Node;
import org.junit.Before;
import org.junit.Test;

/**
 * @author Amer
 *
 */
public class NodeTest {
	protected Node R, S;
	
	@Before
	public void setUp() throws Exception {
		R = new Node(false, "R");
		S = new Node(false, "S");
	}

	@Test
	public void test() {
		assertTrue(R.equals(R));
		assertFalse(R.equals(S));
	}

}
