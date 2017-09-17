/**
 * 
 */
package org.gprom.jdbc.testing;

import org.gprom.jdbc.pawd.Node;
import org.junit.Before;
import org.junit.Test;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

/**
 * @author Amer
 *
 */
public class NodeTest {
	private Node R;
    private Node S;
	private int idCount = 0;
	
	@Before
	public void setUp() throws Exception {
		R = new Node( dummyIDgenerator(),false, "R");
		S = new Node( dummyIDgenerator(),false, "S");
	}
	private String dummyIDgenerator(){
		return(String.valueOf(idCount++));

	}
	@Test
	public void test() {
		assertTrue(R.equals(R));
		assertFalse(R.equals(S));
	}

}
