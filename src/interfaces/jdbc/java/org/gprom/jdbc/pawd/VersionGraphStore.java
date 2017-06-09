/**
 * 
 */
package org.gprom.jdbc.pawd;
import java.util.ArrayList;
/**
 * @author Amer
 *
 */
public class VersionGraphStore {

	/**
	 * 
	 */
	public static void VersionGraphStoreMain(VersionGraph V){
		//we have not yet determined how/where we want a graph version to be stored.
		ArrayList<Node> Nodes = V.getNodes(); 
		ArrayList<Edge> Edges = V.getEdges(); 
		String[] Configuration = V.getConfiguration();
	}
}
