/**
 * 
 */
package org.gprom.jdbc.pawd;

import java.util.ArrayList;

/**
 * @author Amer
 *
 */
public class AddEdge {
	/**
	 * @param args
	 */
	public static void AddEdgeMain(VersionGraph V, Edge d) {
		// TODO Auto-generated method stub
		ArrayList<Edge> NewEdges = new ArrayList<Edge>(); 
		NewEdges = V.getEdges();
		NewEdges.add(d);
		V.setEdges(NewEdges);
	}

}
