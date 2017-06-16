/**
 * 
 */
package org.gprom.jdbc.pawd;

import java.util.ArrayList;
import java.util.List;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;

import org.json.JSONArray;
import org.json.JSONException;

/**
 * @author Amer
 *
 */
public class VersionGraphManger implements VersionGraphStore {
	

	//helper method for putting arrays of JSON objects together
	private JSONArray concatArray(JSONArray... arrs)
	        throws JSONException {
	    JSONArray result = new JSONArray();
	    for (JSONArray arr : arrs) {
	        for (int i = 0; i < arr.length(); i++) {
	            result.put(arr.get(i));
	        }
	    }
	    return result;
	}
	
	public void Save(VersionGraph V) throws JSONException{
		JSONArray GraphNodesArray= new JSONArray(V.getNodes());
		JSONArray GraphEdgesArray= new JSONArray(V.getEdges());
	    JSONArray GraphConfigArray= new JSONArray(V.getConfiguration());
	    JSONArray GraphIDArray = new JSONArray(Long.toString(VersionGraph.idCounter));
	    JSONArray GraphJSONArray =
	    		concatArray(GraphConfigArray,GraphEdgesArray,GraphNodesArray, GraphIDArray);
	    System.out.println(GraphJSONArray); 
	}
	
	public VersionGraph Load(JSONArray GraphJSONArray){
		String GraphJSONArrayString = GraphJSONArray.toString();
		ArrayList<Node> Nodes = new Gson().fromJson(GraphJSONArrayString, new TypeToken<List<Node>>(){}.getType());
		ArrayList<Edge> Edges = new Gson().fromJson(GraphJSONArrayString, new TypeToken<List<Edge>>(){}.getType());
		String[] Configuration = new Gson().fromJson(GraphJSONArrayString, new TypeToken<List<String>>(){}.getType());
		VersionGraph V = new VersionGraph(Nodes,Edges,Configuration);
		//I am not sure about this, it needs further testing.
		String c= new Gson().fromJson(GraphJSONArrayString, new TypeToken<String>(){}.getType());
		VersionGraph.setidCounter(Long.parseLong(c));
		return V;
	}
	
	public void Configure(VersionGraph V){
		List<String> config = new ArrayList<>();
		for(Node t: V.getNodes()){
			if (t.Materialized){
				config.add(t.getId());
			}
		}
		V.setConfiguation((String[]) config.toArray());
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
