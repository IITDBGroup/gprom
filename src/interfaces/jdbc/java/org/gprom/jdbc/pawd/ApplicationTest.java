/**
 * 
 */
package org.gprom.jdbc.pawd;


import org.json.JSONException;
import org.json.JSONObject;
import sun.misc.IOUtils;

import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.Writer;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;


/**
 * @author Amer
 *
 */

public class ApplicationTest {


	/**
	 * @param args
	 *
	 */
	public static void main(String[] args) throws IOException,JSONException {
		Node R = new Node(false, "R");
		Node S = new Node(false, "S");
		Node T = new Node(false,"T");
		Node Rprime = new Node(false, "R'");
//		Node G = new Node(false, "G");
//		Node K = new Node(false, "K");
		Node So = new Node(false,"So");
//		Node J = new Node(false,"J");
//		Node F = new Node(false,"F");
		//Node R2 = new Node(false,"R2");
		//construct different arraylist for Edge construction
//		ArrayList<Node> NodeSet1 = new ArrayList<>( Arrays.asList(S,K));
//		ArrayList<Node> NodeSet2 = new ArrayList<>( Arrays.asList(So));
//		ArrayList<Node> NodeSet3 = new ArrayList<>( Arrays.asList(T,F,J));
//		ArrayList<Node> NodeSet4 = new ArrayList<>( Arrays.asList(K));
		//ArrayList<Node> NodeSet5 = new ArrayList<>( Arrays.asList(R));
		ArrayList<Node> NodeSetAll = new ArrayList<>(Arrays.asList(R,S,T,So,Rprime));
		//construct sample operations for edge creation
		VersionGraphStore.Operation op1 = new VersionGraphStore.Operation("SELECT a, b * 2 AS b FROM $$1$$", VersionGraphStore.Operation.OpType.Query);
		VersionGraphStore.Operation op2 = new VersionGraphStore.Operation("SELECT sum(a), b FROM $$1$$ GROUP BY b", VersionGraphStore.Operation.OpType.Update);
		//sample set of edges
		Edge edge1 = new Edge(R,S,op1);
		Edge edge2 = new Edge(S,T,op2);
//		Edge edge3 = new Edge(NodeSet1,NodeSet2,op2);
//		Edge edge4 = new Edge(So,G,op1);
//		Edge edge5 = new Edge(So,Rprime,op1);
//		Edge edge6 = new Edge(NodeSet3,NodeSet4,op2);
		//construct Arraylist of EDGES
		ArrayList<Edge> EdgeSetAll = new ArrayList<>( Arrays.asList(edge1,edge2));//,edge3,edge4,edge5,edge6));
		//construct the VersionEdges
		VersionEdge VE1 = new VersionEdge(R,Rprime);
		VersionEdge VE2 = new VersionEdge(S,So);
		ArrayList<VersionEdge> VersionEdgeSetAll = new ArrayList<>( Arrays.asList(VE1,VE2));
		String[] conf = {"2","3"};

		//create a version graph
		VersionGraph VG1 = new VersionGraph(NodeSetAll, EdgeSetAll,VersionEdgeSetAll,conf);
		System.out.println("Graph has been created");
		JSONObject myGraph = JSONVersionGraphSerializer.getInstance().Save(VG1);
		System.out.println("JSONObject has been created");
		try (FileWriter file = new FileWriter("C:\\Users\\Amer\\Desktop\\file1.txt")) {
			file.write(myGraph.toString());
			System.out.println("Successfully Copied JSON Object to File...");
			System.out.println("\nJSON Object: " + myGraph);
		}
		String filename = "C:\\Users\\Amer\\Desktop\\file1.txt";
		System.out.println("loading file");
		String content = new String(Files.readAllBytes(Paths.get(filename)));
		System.out.println("file loaded");
		JSONObject newGraph =  new JSONObject(content);
		System.out.println("JSON Object Created");
		VersionGraph loadedVG =JSONVersionGraphSerializer.getInstance().Load(newGraph);
		System.out.println("JSON Object loaded");
		System.out.println(loadedVG.toString());

	}
	
	
	
	
	
	
	
	
	
	
	
	


}
