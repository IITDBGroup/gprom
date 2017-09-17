/**
 *
 */
package org.gprom.jdbc.pawd;


import org.json.JSONException;

import java.io.IOException;
import java.lang.reflect.Field;
import java.util.Arrays;
import java.util.Properties;

/**
 * @author Amer
 *
 */

class ApplicationTest {


	/**
	 * @param args
	 *
	 */
	public static void main(String[] args) throws IOException, JSONException, IllegalAccessException {
        Properties myProp = new Properties();
        myProp.put("dir","C:\\Users\\Amer\\Desktop");
        JSONVersionGraphStore myStore = new JSONVersionGraphStore();
        myStore.initialize(myProp);
       // myProp.getProperty()

//
//		Node R = new Node(false, "R");
//		Node S = new Node(false, "S");
//		Node T = new Node(false,"T");
//		Node Rprime = new Node(true, "R'");
////		Node G = new Node(false, "G");
////		Node K = new Node(false, "K");
//		Node So = new Node(true,"So");
////		Node J = new Node(false,"J");
////		Node F = new Node(false,"F");
//		//Node R2 = new Node(false,"R2");
//		//construct different arraylist for Edge construction
////		ArrayList<Node> NodeSet1 = new ArrayList<>( Arrays.asList(S,K));
////		ArrayList<Node> NodeSet2 = new ArrayList<>( Arrays.asList(So));
////		ArrayList<Node> NodeSet3 = new ArrayList<>( Arrays.asList(T,F,J));
////		ArrayList<Node> NodeSet4 = new ArrayList<>( Arrays.asList(K));
//		//ArrayList<Node> NodeSet5 = new ArrayList<>( Arrays.asList(R));
//		ArrayList<Node> NodeSetAll = new ArrayList<>(Arrays.asList(R,S,T,So,Rprime));
//		//construct sample operations for edge creation
//		VersionGraphStore.Operation op1 = new VersionGraphStore.Operation("SELECT a, b * 2 AS b FROM $$1$$", VersionGraphStore.Operation.OpType.Query);
//		VersionGraphStore.Operation op2 = new VersionGraphStore.Operation("SELECT sum(a), b FROM $$1$$ GROUP BY b", VersionGraphStore.Operation.OpType.Update);
//		//sample set of edges
//		Edge edge1 = new Edge(R,S,op1);
//		Edge edge2 = new Edge(S,T,op2);
////		Edge edge3 = new Edge(NodeSet1,NodeSet2,op2);
////		Edge edge4 = new Edge(So,G,op1);
////		Edge edge5 = new Edge(So,Rprime,op1);
////		Edge edge6 = new Edge(NodeSet3,NodeSet4,op2);
//		//construct Arraylist of EDGES
//		ArrayList<Edge> EdgeSetAll = new ArrayList<>( Arrays.asList(edge1,edge2));//,edge3,edge4,edge5,edge6));
//		//construct the VersionEdges
//		VersionEdge VE1 = new VersionEdge(R,Rprime);
//		VersionEdge VE2 = new VersionEdge(S,So);
//		ArrayList<VersionEdge> VersionEdgeSetAll = new ArrayList<>( Arrays.asList(VE1,VE2));
//
//		//create a version graph
//		VersionGraph VG1 = new VersionGraph(NodeSetAll, EdgeSetAll,VersionEdgeSetAll,null);
//		VG1.Configure();
//		ObjectMapper mapper = new ObjectMapper();
//		String jsonInString = "";
//		try {
//			//Convert object to JSON string and save into file directly
//			mapper.writeValue(new File("C:\\Users\\Amer\\Desktop\\user.json"), VG1);
//			System.out.println("Converted to file");
//			//Convert object to JSON string
//			jsonInString = mapper.writeValueAsString(VG1);
//			JSONObject old = new JSONObject(jsonInString);
//			System.out.println("Converted to JSON String");
//
//			//Convert object to JSON string and pretty print
//			jsonInString = mapper.writerWithDefaultPrettyPrinter().writeValueAsString(VG1);
//			//System.out.println(jsonInString);
//
//
//		} catch (JsonGenerationException e) {
//			e.printStackTrace();
//		} catch (JsonMappingException e) {
//			e.printStackTrace();
//		} catch (IOException e) {
//			e.printStackTrace();
//		}

//		try {
//
//			// Convert JSON string from file to Object
//			VersionGraph user = mapper.readValue(new File("C:\\Users\\Amer\\Desktop\\user.json"), VersionGraph.class);
//			System.out.println("Converted JSON string from file to Object");
//
//			// Convert JSON string to Object
//			VersionGraph user1 = mapper.readValue(jsonInString, VersionGraph.class);
//			compare(VG1,user);
//			System.out.println(VG1.getNodes()+"\n\n");
//			System.out.println("\n\n"+ user.getNodes());
//
//			if(VG1.equals(user1))
//				System.out.println("hey this works");
//
//
//			//System.out.println("Converted JSON string to Object");
//
//		} catch (JsonGenerationException e) {
//			e.printStackTrace();
//		} catch (JsonMappingException e) {
//			e.printStackTrace();
//		} catch (IOException e) {
//			e.printStackTrace();
//		}

//		System.out.println("Graph has been created");
//		JSONObject myGraph = JSONVersionGraphSerializer.getInstance().Save(VG1);
//		System.out.println("JSONObject has been created");
//		try (FileWriter file = new FileWriter("C:\\Users\\Amer\\Desktop\\file1.txt")) {
//			//file.write(myGraph.);
//			System.out.println("Successfully Copied JSON Object to File...");
//			System.out.println("\nJSON Object: " + myGraph);
//		}
//		String filename = "C:\\Users\\Amer\\Desktop\\file1.txt";
//		System.out.println("loading file");
//		String content = new String(Files.readAllBytes(Paths.get(filename)));
//		System.out.println("file loaded");
//		JSONObject newGraph =  new JSONObject(content);
//		System.out.println("JSON Object Created");
//		VersionGraph loadedVG =JSONVersionGraphSerializer.getInstance().Load(newGraph);
//		System.out.println("JSON Object loaded");
//		System.out.println(loadedVG.toString());

	}
	public static String genericSerializer (Object o) throws IllegalArgumentException, IllegalAccessException {
		Class myclazz = o.getClass();
		System.out.println(myclazz);
		Field[] myfields = myclazz.getDeclaredFields();
		for(Field f: myfields){
			f.setAccessible(true);
			System.out.println("field f is "+f);
			Class fieldType = f.getType();
			System.out.println("field type is = "+fieldType);
			Object f1Values = f.get(o);
			System.out.println("field value is "+f1Values);
		}

		return null;
	}


	public static void compare(VersionGraph mine, Object other){
		VersionGraph otherVG = (VersionGraph)other;
		System.out.println("comparing now");
		boolean nodes, edges, vedges , conf;
		conf = Arrays.equals(mine.getConfiguration(),otherVG.getConfiguration());
		nodes = mine.getNodes().equals(otherVG.getNodes());
		edges = otherVG.getEdges().equals(mine.getEdges());
		vedges = mine.getVersionEdges().equals(otherVG.getVersionEdges());
		System.out.println("config is " + conf);
		System.out.println("nodes is " + nodes);
		System.out.println("edges is " + edges);
		System.out.println("vedges is " + vedges);
	}










}
