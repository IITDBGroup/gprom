/**
 * 
 */
package org.gprom.jdbc.pawd;

import java.util.ArrayList;
import java.util.List;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
//uncomment this once you want to pretty print the graph
//import com.google.gson.Gson;
//import com.google.gson.GsonBuilder;
//import com.google.gson.JsonElement;
//import com.google.gson.JsonParser;

/**
 * @author Amer
 *
 */
public class VersionGraphManger implements VersionGraphStore {
	

	public JSONObject Save(VersionGraph V){
	    JSONObject GraphJSONObject = new JSONObject();
		try
		{
			//adding nodes as JSON OBjects
		    JSONArray NodesArray= new JSONArray();
		    for (Node node : V.getNodes())
		    {
		         JSONObject nodeJSON = new JSONObject();
		         nodeJSON.put("Id", node.getId());
		         nodeJSON.put("Materialized", node.isMaterialized());
		         nodeJSON.put("Description", node.getDescription());
		         nodeJSON.put("Time", node.getTime());
		         NodesArray.put(nodeJSON);
		    }
			GraphJSONObject.put("Nodes", NodesArray);
		    //adding edges as JSON OBJECTS
		    JSONArray EdgesArray = new JSONArray();
		    for (Edge edge : V.getEdges())
		    {
		         JSONObject edgeJSON = new JSONObject();
		         edgeJSON.put("StartNodes", edge.getStartNodes());
		         edgeJSON.put("EndNodes", edge.getEndNodes());
		         edgeJSON.put("Transformation", edge.getTransformation());
		         EdgesArray.put(edgeJSON);
		    }
			GraphJSONObject.put("Edges", EdgesArray);
			//adding configuration
		    GraphJSONObject.put("Configuration", V.getConfiguration());
		    //adding IDcounter
	        GraphJSONObject.put("counterID", VersionGraph.getIdCounter());
//		    Gson gson = new GsonBuilder().setPrettyPrinting().create();
//		    JsonParser jp = new JsonParser();
//		    JsonElement je = jp.parse( GraphJSONObject.toString());
//		    String prettyJsonString = gson.toJson(je);
//		    System.out.println("Printing Graph Info" +prettyJsonString);
//		    System.out.println("hi"+ GraphJSONObject);
		    return GraphJSONObject;
		} catch (JSONException jse) {
		    jse.printStackTrace();
		    return null;
		}
	}
	
	public VersionGraph Load(JSONObject GraphJSONObject){
		//this still needs implementation
		//I need to fix the rest of this method
		VersionGraph V = new VersionGraph();
		return V;
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
