/**
 * 
 */
package org.gprom.jdbc.pawd;
import java.util.ArrayList;

/**
 * @author Amer
 *
 */
public class AddNode {

	/**
	 * @param args
	 */
	public static void AddNodeMain(VersionGraph V, Node t){
		ArrayList<Node> NewNodes = new ArrayList<Node>(); 
		NewNodes = V.getNodes();
		NewNodes.add(t);
		V.setNodes(NewNodes);
	}

}
