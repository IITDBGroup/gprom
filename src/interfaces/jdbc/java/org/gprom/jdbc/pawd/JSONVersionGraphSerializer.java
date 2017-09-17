///**
// *
// */
//package org.gprom.jdbc.pawd;
//import org.gprom.jdbc.pawd.VersionGraphStore.Operation;
//import org.gprom.jdbc.pawd.VersionGraphStore.Operation.OpType;
//import org.json.JSONArray;
//import org.json.JSONException;
//import org.json.JSONObject;
//
//import java.util.ArrayList;
//import java.util.Date;
//import java.util.List;
//
//
///**
// * @author Amer
// *
// */
//
//public class JSONVersionGraphSerializer {
//	private static JSONVersionGraphSerializer ourInstance = new JSONVersionGraphSerializer();
//
//	public static JSONVersionGraphSerializer getInstance() {
//		return ourInstance;
//	}
//
//	private JSONVersionGraphSerializer() {
//	}
//
//
////	public String genericSerializer (Object o) throws IllegalArgumentException, IllegalAccessException {
////		Class myclazz = o.getClass();
////		Field[] myfields = myclazz.getFields();
////		Field f = myfields[0];
////		Class fieldType = f.getType();
////		Object f1Values = f.get(o);
////		return null;
////	}
//
//	//helper method converting a JSONobject to node
//	public Node JSONtoNode(JSONObject newnode ){
//		try{
//			String nodeID = newnode.getString("Id");
//			Date t = (Date) newnode.get("Time");
//			boolean mat = newnode.getBoolean("Materialized");
//			String Desc = newnode.getString("Description");
//			Node node = new Node(nodeID,mat,Desc,t );
//			return node;
//		}
//		catch(JSONException jse){
//			jse.printStackTrace();
//			return null;
//		}
//	}
//	//helper method converting a node object to a JSOn Object
//	public JSONObject NodetoJSON(Node node){
//		JSONObject nodeJSON = new JSONObject();
//		try {
//			nodeJSON.put("Id", node.getId());
//			nodeJSON.put("Materialized", node.getMaterialized());
//			nodeJSON.put("Description", node.getDescription());
//			nodeJSON.put("Time", node.getTime());
//			return nodeJSON;
//		} catch (JSONException e) {
//			e.printStackTrace();
//			return null;
//		}
//
//
//	}
//	public static String[] toStringArray(JSONArray array) {
//		if(array==null) {
//			System.out.println("the thing was null");
//			return null;
//		}
//		String[] arr=new String[array.length()];
//		for(int i=0; i<arr.length; i++) {
//			arr[i]=array.optString(i);
//		}
//		return arr;
//	}
//
//	public JSONObject Save(VersionGraph V){
//		JSONObject GraphJSONObject = new JSONObject();
//		try
//		{
//			//adding nodes as JSON OBjects
//			JSONArray NodesArray= getJSONArray(V.getNodes());
//			GraphJSONObject.put("Nodes", NodesArray);
//			//adding edges as JSON OBJECTS
//			JSONArray EdgesArray = new JSONArray();
//			for (Edge edge : V.getEdges())
//			{
//				JSONObject edgeJSON = new JSONObject();
//				JSONArray StartNodesJSON = getJSONArray(edge.getStartNodes());
//				edgeJSON.put("StartNodes", StartNodesJSON);
//				JSONArray EndNodesJSON = getJSONArray(edge.getEndNodes());
//				edgeJSON.put("EndNodes", EndNodesJSON);
//				JSONObject trans = new JSONObject();
//				trans.put("Code",edge.getTransformation().Code );
//				trans.put("Operation",edge.getTransformation().Op );
//				edgeJSON.put("Transformation", trans);
//				EdgesArray.put(edgeJSON);
//			}
//			GraphJSONObject.put("Edges", EdgesArray);
//			//adding VersionEdges
//			JSONArray VEdgesArray = new JSONArray();
//			for (VersionEdge VE: V.getVersionEdges()){
//				JSONObject vedgeJSON = new JSONObject();
//				//adding original node
//				JSONObject nodeJSON = NodetoJSON(VE.getOriginal());
//				vedgeJSON.put("Original", nodeJSON);
//				//adding derived node
//				nodeJSON = NodetoJSON(VE.getDerivative());
//				vedgeJSON.put("Derivative", nodeJSON);
//				//adding time
//				vedgeJSON.put("Time", VE.getTime());
//				VEdgesArray.put(vedgeJSON);
//			}
//			GraphJSONObject.put("VersionEdges", VEdgesArray);
//			//adding configuration
//			JSONArray configArray = null;
//			if(V.getConfiguration()!=null){
//				configArray = new JSONArray();
//				for(String s : V.getConfiguration()){
//					configArray.put(s);
//				}
//			}
//			GraphJSONObject.put("Configuration", configArray);
//			//adding IDcounter
//			GraphJSONObject.put("counterID", VersionGraph.getNodeIDCounter());
//			return GraphJSONObject;
//		} catch (JSONException jse) {
//			jse.printStackTrace();
//			return null;
//		}
//	}
//	//helper method to get an arraylist of nodes from a JSON Array
//	public ArrayList<Node> getNodeArrayList(JSONArray nodesJSONArray) {
//		ArrayList<Node> NodesList = new ArrayList<Node>();
//		try{
//			for(int i = 0 ; i <nodesJSONArray.length();i++){
//				JSONObject newnode = nodesJSONArray.getJSONObject(i);
//				Node nodeclassobj = JSONtoNode(newnode);
//				NodesList.add(nodeclassobj);
//			}
//			return NodesList;
//		}catch (JSONException jse){
//			jse.printStackTrace();
//			return null;
//		}
//	}
//
//	//helper method to parse an array of edges to a JSONArray
//	public JSONArray getJSONArray(List<Node> list){
//		JSONArray NodesArray= new JSONArray();
//		//adding nodes as JSON OBjects
//		for (Node node : list){
//			//JSONObject nodeJSON = new JSONObject(node);
//			JSONObject nodeJSON = NodetoJSON(node);
//			NodesArray.put(nodeJSON);
//		}
//		return NodesArray;
//	}
//
//
//
//
//
//	public VersionGraph Load(JSONObject GraphJSONObject){
//		JSONArray nodes;
//		JSONArray edges;
//		JSONArray vedges;
//		String[] Configuaration;
//		ArrayList<Edge> EdgesList =new ArrayList<Edge>();
//		ArrayList<VersionEdge> VEdgesList = new ArrayList<VersionEdge>();
//		VersionGraph V;
//		ArrayList<Node>NodesList;
//		try {
//			nodes = GraphJSONObject.getJSONArray("Nodes");
//			edges = GraphJSONObject.getJSONArray("Edges");
//			vedges = GraphJSONObject.getJSONArray("VersionEdges");
//			Configuaration = toStringArray(GraphJSONObject.getJSONArray("Configuration"));
//			long counter = GraphJSONObject.getLong("counterID");
//			NodesList = getNodeArrayList(nodes);
//			//adding edges
//			for(int i = 0 ; i <edges.length();i++){
//				JSONObject newedge = edges.getJSONObject(i);
//				ArrayList<Node> startNodes = getNodeArrayList(newedge.getJSONArray("StartNodes"));
//				ArrayList<Node> endNodes = getNodeArrayList(newedge.getJSONArray("EndNodes"));
//				//it was necessary to break up the object like that to because
//				//when I tried with Gson builder it failed
//				//to take care of the enum.
//				String code = newedge.getJSONObject("Transformation").get("Code").toString();
//				OpType operation = OpType.valueOf(newedge.getJSONObject("Transformation").get("Operation").toString());
//				Operation trans = new Operation(code, operation);
//				Edge edgeclassobj = new Edge(startNodes, endNodes,trans);
//				EdgesList.add(edgeclassobj);
//			}
//			for(int i = 0 ;i <vedges.length();i++){
//				JSONObject newVEdge= vedges.getJSONObject(i);
//				JSONObject newnode = (JSONObject) newVEdge.get("Original");
//				Node p = JSONtoNode(newnode);
//				newnode = (JSONObject) newVEdge.get("Derivative");
//				Node c = JSONtoNode(newnode);
//				Date tt = (Date) newVEdge.get("Time");
//				VersionEdge VE = new VersionEdge(p,c,tt);
//				VEdgesList.add(VE);
//			}
//			V = new VersionGraph(NodesList,EdgesList,VEdgesList,Configuaration);
//			VersionGraph.setNodeIDCounter(counter);
//			return V;
//		} catch (JSONException e) {
//			System.out.println("Error we failed");
//			e.printStackTrace();
//		}
//		return null;
//	}
//
//
//
//
//}
