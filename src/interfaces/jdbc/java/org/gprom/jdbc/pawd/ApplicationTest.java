/**
 * 
 */
package org.gprom.jdbc.pawd;

import java.util.ArrayList;
import java.util.Arrays;

import org.gprom.jdbc.pawd.VersionGraphStore.Operation;
import org.gprom.jdbc.pawd.VersionGraphStore.Operation.OpType;
import org.json.JSONException;
import org.json.JSONObject;

/**
 * @author Amer
 *
 */
public class ApplicationTest {

	/**
	 * @param args
	 * @throws JSONException 
	 */
	public static void main(String[] args) throws JSONException {
		//sample set of nodes
		Node node1 = new Node(false, "desc1");
		Node node2 = new Node(true, "desc2");
		Node node3 = new Node(true, "desc3");
		Node node4 = new Node(false, "desc4");
		//System.out.println(node1);
		//construct different arraylist for Edge construction
		ArrayList<Node> NodeSet1 = new ArrayList<>( Arrays.asList(node1, node2));
		ArrayList<Node> NodeSet2 = new ArrayList<>( Arrays.asList(node3, node4));
		ArrayList<Node> NodeSet3 = new ArrayList<>( Arrays.asList(node1, node3));
		ArrayList<Node> NodeSet4 = new ArrayList<>( Arrays.asList(node2, node4));
		ArrayList<Node> NodeSetAll = new ArrayList<>(Arrays.asList(node1,node2,node3,node4));
		//construct sample operations for edge creation
		Operation op1 = new Operation();
		op1.Code = "SELECT * FROM R";
		op1.Op= OpType.Query;
		Operation op2 = new Operation();
		op2.Code = "UPDATE R SET x=0 *";
		op2.Op= OpType.Update;
		//sample set of edges
		Edge edge1 = new Edge(NodeSet1,NodeSet2,op1);
		Edge edge2 = new Edge(NodeSet3,NodeSet4,op2);
		Edge edge3 = new Edge(NodeSet2,NodeSet4,op1);
		Edge edge4 = new Edge(NodeSet1,NodeSet4,op2);
		//construct Arraylist of EDGES
		ArrayList<Edge> EdgeSetAll = new ArrayList<>( Arrays.asList(edge1, edge2,edge3,edge4));
		//System.out.println(EdgeSetAll);
		//create a version graph
		VersionGraph Graph1 = new VersionGraph(NodeSetAll, EdgeSetAll,null);
		VersionGraphStore myinterface = new JSONVersionGraphStore();
		myinterface.Configure(Graph1);
		JSONObject ser = myinterface.Save(Graph1);
		System.out.print(ser.toString(1));
		myinterface.Load(ser);
		

	}

}
