/**
 * 
 */
package org.gprom.jdbc.pawd;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.gprom.jdbc.pawd.VersionGraphStore.Operation.OpType;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;


/**
 * @author Amer
 *
 */
public class JSONVersionGraphStore implements VersionGraphStore {
	

	public JSONObject Save(VersionGraph V){
	    JSONObject GraphJSONObject = new JSONObject();
	    
		try
		{
			//adding nodes as JSON OBjects
		    JSONArray NodesArray= getJSONArray(V.getNodes());
			GraphJSONObject.put("Nodes", NodesArray);
		    //adding edges as JSON OBJECTS
		    JSONArray EdgesArray = new JSONArray();
		    for (Edge edge : V.getEdges())
		    {
		         JSONObject edgeJSON = new JSONObject();
		         JSONArray StartNodesJSON = getJSONArray(edge.getStartNodes());
		         edgeJSON.put("StartNodes", StartNodesJSON);
		         JSONArray EndNodesJSON = getJSONArray(edge.getEndNodes());
		         edgeJSON.put("EndNodes", EndNodesJSON);
		         JSONObject trans = new JSONObject();
		         trans.put("Code",edge.getTransformation().Code );
		         trans.put("Operation",edge.getTransformation().Op );
		         edgeJSON.put("Transformation", trans);
		         EdgesArray.put(edgeJSON);
		    }
			GraphJSONObject.put("Edges", EdgesArray);
			//adding configuration
		    GraphJSONObject.put("Configuration", V.getConfiguration());
		    //adding IDcounter
	        GraphJSONObject.put("counterID", VersionGraph.getIdCounter());
		    return GraphJSONObject;
		} catch (JSONException jse) {
		    jse.printStackTrace();
		    return null;
		}
	}
	//helper method to get an arraylist of nodes from a JSON Array
	public ArrayList<Node> getNodeArrayList(JSONArray nodesJSONArray) {
		ArrayList<Node> NodesList = new ArrayList<Node>();
		try{
			for(int i = 0 ; i <nodesJSONArray.length();i++){
			JSONObject newnode = nodesJSONArray.getJSONObject(i);
			String nodeID = newnode.getString("Id");
			Date t = (Date) newnode.get("Time");
			boolean mat = newnode.getBoolean("Materialized");
			String Desc = newnode.getString("Description");
			Node nodeclassobj = new Node(nodeID,mat,Desc,t );
			NodesList.add(nodeclassobj);
			}
			return NodesList;
		}catch (JSONException jse){
				jse.printStackTrace();
				return null;
			}
		}
	
	//helper method to parse an array of edges to a JSONArray
	public JSONArray getJSONArray(ArrayList<Node> Nodes){
		JSONArray NodesArray= new JSONArray();
		//adding nodes as JSON OBjects
		try{
			for (Node node : Nodes)
			{
				JSONObject nodeJSON = new JSONObject();
				nodeJSON.put("Id", node.getId());
				nodeJSON.put("Materialized", node.isMaterialized());
				nodeJSON.put("Description", node.getDescription());
				nodeJSON.put("Time", node.getTime());
				NodesArray.put(nodeJSON);
			}
		} catch (JSONException jse) {
			jse.printStackTrace();
			return null;
		}
		return NodesArray;
	}

	public VersionGraph Load(JSONObject GraphJSONObject){
		JSONArray nodes;
		JSONArray edges;
		String[] Configuaration;
		ArrayList<Edge> EdgesList =new ArrayList<Edge>();
		VersionGraph V;
		ArrayList<Node>NodesList;
		try {
			nodes = GraphJSONObject.getJSONArray("Nodes");
			edges = GraphJSONObject.getJSONArray("Edges");
			Configuaration = (String[]) GraphJSONObject.get("Configuration");
			long counter = GraphJSONObject.getLong("counterID");
			NodesList = getNodeArrayList(nodes);
			//adding edges
			for(int i = 0 ; i <edges.length();i++){
				JSONObject newedge = edges.getJSONObject(i);
				ArrayList<Node> startNodes = getNodeArrayList(newedge.getJSONArray("StartNodes"));
				ArrayList<Node> endNodes = getNodeArrayList(newedge.getJSONArray("EndNodes"));
				//it was necessary to break up the object like that to because 
				//when I tried with Gson builder it failed
				//to take care of the enum.
				String code = newedge.getJSONObject("Transformation").get("Code").toString();
				OpType operation = OpType.valueOf(newedge.getJSONObject("Transformation").get("Operation").toString());
				Operation trans = new Operation(code, operation);
				Edge edgeclassobj = new Edge(startNodes, endNodes,trans);
				EdgesList.add(edgeclassobj);
			}
			V = new VersionGraph(NodesList,EdgesList,Configuaration);
			VersionGraph.setIdCounter(counter);
			return V;
		} catch (JSONException e) {
			System.out.println("Error we failed");
			e.printStackTrace();
		}
		return null;
	}
	
	public void Configure(VersionGraph V){
		List<String> config = new ArrayList<>();
		for(Node t: V.getNodes()){
			if (t.Materialized){
				config.add(t.getId());
			}
		}
		V.setConfiguation( config.toArray(new String[0]));
	}
	
	public class ConnectionInfo{
		String Username;
		String Password;
		String URL;
		String Database;
		/**
		 * @return the username
		 */
		public String getUsername() {
			return Username;
		}
		/**
		 * @param username the username to set
		 */
		public void setUsername(String username) {
			Username = username;
		}
		/**
		 * @return the password
		 */
		public String getPassword() {
			return Password;
		}
		/**
		 * @param password the password to set
		 */
		public void setPassword(String password) {
			Password = password;
		}
		/**
		 * @return the uRL
		 */
		public String getURL() {
			return URL;
		}
		/**
		 * @param uRL the uRL to set
		 */
		public void setURL(String uRL) {
			URL = uRL;
		}
		/**
		 * @return the database
		 */
		public String getDatabase() {
			return Database;
		}
		/**
		 * @param database the database to set
		 */
		public void setDatabase(String database) {
			Database = database;
		}
	}

}
