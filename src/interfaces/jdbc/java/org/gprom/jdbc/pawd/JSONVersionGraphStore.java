/**
 * 
 */
package org.gprom.jdbc.pawd;
import org.stringtemplate.v4.*;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.EmptyStackException;
import java.util.List;
import java.util.Map;
import java.util.Stack;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.gprom.jdbc.pawd.VersionGraphStore.Operation.Materialization;
import org.gprom.jdbc.pawd.VersionGraphStore.Operation.OpType;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;


/**
 * @author Amer
 *
 */
public class JSONVersionGraphStore implements VersionGraphStore {//TODO make this JSONVersionGraphSerializer, because that is what it does
	
	//TODO remove later
//	public String genericSerializer (Object o) throws IllegalArgumentException, IllegalAccessException {
//		Class myclazz = o.getClass();
//		Field[] myfields = myclazz.getFields();
//		Field f = myfields[0];
//		Class fieldType = f.getType();
//		Object f1Values = f.get(o);
//	}
	
	//helper method converting a JSONobject to node
	public Node JSONtoNode(JSONObject newnode ){
		try{
			String nodeID = newnode.getString("Id");
			Date t = (Date) newnode.get("Time");
			boolean mat = newnode.getBoolean("Materialized");
			String Desc = newnode.getString("Description");
			Node node = new Node(nodeID,mat,Desc,t );
			return node;
		}
		catch(JSONException jse){
			jse.printStackTrace();
			return null;
		}
	}
	//helper method converting a node object to a JSOn Object
	public JSONObject NodetoJSON(Node node){
		JSONObject nodeJSON = new JSONObject();
		try {
			nodeJSON.put("Id", node.getId());
			nodeJSON.put("Materialized", node.isMaterialized());
			nodeJSON.put("Description", node.getDescription());
			nodeJSON.put("Time", node.getTime());
			return nodeJSON;
		} catch (JSONException e) {
			e.printStackTrace();
			return null;
		}


	}

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
			//adding VersionEdges
			JSONArray VEdgesArray = new JSONArray();
			for (VersionEdge VE: V.getVersionEdges()){
				JSONObject vedgeJSON = new JSONObject();
				//adding original node
				JSONObject nodeJSON = NodetoJSON(VE.getOriginal());
				vedgeJSON.put("Original", nodeJSON);
				//adding derived node
				nodeJSON = NodetoJSON(VE.getDerivative());
				vedgeJSON.put("Derivative", nodeJSON);
				//adding time
				vedgeJSON.put("Time", VE.getTime());
				VEdgesArray.put(vedgeJSON);
			}
			GraphJSONObject.put("VersionEdges", VEdgesArray);
			//adding configuration
			GraphJSONObject.put("Configuration", V.getConfiguration());
			//adding IDcounter
			GraphJSONObject.put("counterID", V.getIdCounter());
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
				Node nodeclassobj = JSONtoNode(newnode);
				NodesList.add(nodeclassobj);
			}
			return NodesList;
		}catch (JSONException jse){
			jse.printStackTrace();
			return null;
		}
	}

	//helper method to parse an array of edges to a JSONArray
	public JSONArray getJSONArray(List<Node> list){
		JSONArray NodesArray= new JSONArray();
		//adding nodes as JSON OBjects
		for (Node node : list){
			JSONObject nodeJSON = NodetoJSON(node);
			NodesArray.put(nodeJSON);
		}
		return NodesArray;
	}
	
	//helper method to combine graphs
	public VersionGraph Absorb(VersionGraph V,VersionGraph G) {
		ArrayList<Node> newNodes = new ArrayList<Node>();
		ArrayList<Edge> newEdges = new ArrayList<Edge>();
		newNodes.addAll(V.getNodes());
		newEdges.addAll(V.getEdges());
		if(G.getNodes() !=null  && G.getEdges()!= null) {
			newNodes.removeAll(G.getNodes());
			newNodes.addAll(G.getNodes());
			newEdges.removeAll(G.getEdges());
			newEdges.addAll(G.getEdges());
		}
		V.setEdges(newEdges);
		V.setNodes(newNodes);
		return V;
	}

	
	public ArrayList<VersionGraph> getPath(VersionGraph V,Node n){
		ArrayList<VersionGraph> Graphs =  new ArrayList<VersionGraph>();
		//System.out.println(
		//consider that you can filter out these Subgraphs by checking the materialized attribute
		//then return this list again
		//the formula for counting the number of possible materialization plans is exponential
		//it is in the = 2^n0 * 2^n1  where n is the number of nodes in a branch
		Graphs.add(genPathes(V,n));
				//);
		return Graphs;
	}
	public VersionGraph genPathes (VersionGraph V, Node n){
		Edge e = V.getChildEdge(n);
		if(e == null)
			return null;
		VersionGraph VG = new VersionGraph();
		VG.AddNode(n);
		VG.AddEdge(e);
		if(n.isMaterialized())
			return VG;
		Node j = e.getStartNodes().get(0);
		VersionGraph S = genPathes(V,j);
		if(S != null) 
			VG= Absorb(VG,S);
		else
			VG.AddNode(j);
		for(int i = 1;i<e.getStartNodes().size();i++){
			Node t = e.getStartNodes().get(i);
			VersionGraph k = genPathes(V,t);
			if (k != null) 
				VG = Absorb(VG,k);
			else 
				VG.AddNode(t);
		}
		return VG;
	}
	public String Compose(VersionGraph V, Node startNode, Map<Node,Materialization> MPlan){
		Stack<Node> fullStack = new Stack<Node>();
		String lastq= null;
		fullStack.push(startNode);
		while(V.getChildEdge(startNode) != null){
			Edge currentEdge = V.getChildEdge(startNode);
			int my_size = currentEdge.getStartNodes().size();
			//this block of code is temporary
			if(my_size>1){
				String [] parentsSQL = new String[my_size] ;
				ArrayList<Node> parents = new ArrayList<Node>();
				for(int i=0;i<my_size;i++){
//					System.out.println("looping "+i);
//					System.out.println("Edge is "+ currentEdge);
					Node current = currentEdge.getStartNodes().get(i);
//					System.out.println("current Node "+ current);
					parents.add(current);
					parentsSQL[i] = Compose(V,current,MPlan);
				}
				if(MPlan.get(startNode)== Materialization.isMaterialized){
					lastq = Materialize(ConstructFromMany(V,parents,parentsSQL), startNode);
				}else{
					lastq = ConstructFromMany(V,parents,parentsSQL);
				}
				return lastq;
			}
			//end of temp block
			startNode = currentEdge.getStartNodes().get(0);
			fullStack.push(startNode);
		}
		while (!fullStack.isEmpty()) {
			try {
				Node end = fullStack.pop();
				Node start = end;
				while (MPlan.get(start) == Materialization.notMaterialized) {
					try {
						start = fullStack.pop();
					}catch (EmptyStackException e) {
				       //  System.out.println("empty stack");
				         break;
					}
				}
//				System.out.println("this is lastq before = "+ lastq);
//				System.out.println("this is start "+ start.getDescription());
//				System.out.println("this is end "+ end.getDescription());
				if(MPlan.get(start)== Materialization.isMaterialized){
					lastq = Materialize(Construct(V,start,end,lastq), start);
				}else{
					lastq = Construct(V,start,end,lastq);
				}
//				System.out.println("this is lastq after = "+ lastq);
			} catch (EmptyStackException e) {
//		         System.out.println("empty stack");
		         break;
			}
		}
		return lastq;
	}
	//construct an sql query of a series of nodes from start --> end 
	public String ConstructFromMany(VersionGraph V, ArrayList<Node> parents, String[] parentsSQL){
//		System.out.println("Construction from MANY");
		Edge ce = V.getParentEdge(parents);
		String q = ce.getTransformation().getCode();
		String pattern = "(\\${2}\\d\\${2})";
		Pattern r = Pattern.compile(pattern);
		Matcher m = r.matcher(q);
		StringBuffer sb = new StringBuffer();
		String replacement = "";
		int index;
		while(m.find()){
			index = Integer.parseInt(q.substring(m.start()+2, m.end()-2));
			replacement= "("+parentsSQL[index-1]+")";
			m.appendReplacement(sb, replacement);
		}
		m.appendTail(sb);
//		System.out.println("replacement of ConstructMany : "+replacement);
		return sb.toString();
	}
	public String Construct(VersionGraph V, Node start, Node end,String parentQ) {
		Edge ce = V.getChildEdge(start);
		if(ce == null) {
//			System.out.println("Construct start "+ start.getDescription());
//			System.out.println("Construct end "+ end.getDescription());
//			System.out.println("the return"+end.getDescription());
			return start.getDescription();
		}
		String q = ce.getTransformation().getCode();
		String pattern = "(\\${2}\\d\\${2})";
		Pattern r = Pattern.compile(pattern);
		Matcher m = r.matcher(q);
		StringBuffer sb = new StringBuffer();
		String replacement;
		m.find();
		int index = Integer.parseInt(q.substring(m.start()+2, m.end()-2));
		if(start.equals(end)){
			replacement ="("+ parentQ+")";
		}
		else{
			start = ce.getStartNodes().get(index-1);
			replacement= "("+Construct(V,start,end,parentQ)+")";
		}
		m.appendReplacement(sb, replacement);
		
		//System.out.println(m.find());
		m.appendTail(sb);
//		System.out.println("replacement"+replacement);
		return sb.toString();
	}
	//materialize a node
	//set the materialized value
	//add a new table to JDBC
	public String Materialize(String sql, Node node) {
			node.setMaterialized(true);
			JDBCConnect conn= new JDBCConnect();
			String newName = "REL_"+node.getDescription();
			ST query = new ST("CREATE TABLE <NodeID> AS SELECT * FROM (<NodeDesc>)");
			query.add("NodeID", newName);
			query.add("NodeDesc", sql);
			String q = query.render();
			conn.RunUpdate(q);
			return newName;
	}
	public VersionGraph Load(JSONObject GraphJSONObject){
		JSONArray nodes;
		JSONArray edges;
		JSONArray vedges;
		String[] Configuaration;
		ArrayList<Edge> EdgesList =new ArrayList<Edge>();
		ArrayList<VersionEdge> VEdgesList = new ArrayList<VersionEdge>();
		VersionGraph V;
		ArrayList<Node>NodesList;
		try {
			nodes = GraphJSONObject.getJSONArray("Nodes");
			edges = GraphJSONObject.getJSONArray("Edges");
			vedges = GraphJSONObject.getJSONArray("VersionEdges");
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
			for(int i = 0 ;i <vedges.length();i++){
				JSONObject newVEdge= vedges.getJSONObject(i);
				JSONObject newnode = (JSONObject) newVEdge.get("Original");
				Node p = JSONtoNode(newnode);
				newnode = (JSONObject) newVEdge.get("Derivative");
				Node c = JSONtoNode(newnode);
				Date tt = (Date) newVEdge.get("Time");
				VersionEdge VE = new VersionEdge(p,c,tt);
				VEdgesList.add(VE);
			}
			V = new VersionGraph(NodesList,EdgesList,VEdgesList,Configuaration);
			V.setIdCounter(counter);
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
		V.setConfiguation(config.toArray(new String[0]));
	}
	public void UpdateCall(VersionGraph V, Node Rprime){
		Node R = V.getChildEdge(Rprime).getStartNodes().get(0);
		Update(V,R,Rprime);
	}
	public void Update(VersionGraph V, Node R, Node Rprime){
		Edge u1 = V.getChildEdge(Rprime);
		if(V.getParentEdges(R).isEmpty())
			return;
		//removing the edge we started with
		ArrayList<Edge> edges = V.getParentEdges(R);
		edges.remove(u1);
		for(Edge Q : edges){
			for(Node S: Q.getEndNodes()){
				Node newnode = new Node();
				String description = S.getDescription();
				newnode.setDescription(description +"'");
				//creating the new edge 
				ArrayList<Node> startnodes = new ArrayList<>(Arrays.asList(Rprime));
				//check for the care where we have multiple input nodes for a given edge
				startnodes.addAll(Q.startNodes);
				startnodes.remove(R);
				ArrayList<Node> endnodes = new ArrayList<>(Arrays.asList(newnode));
				Edge newedge = new Edge(startnodes,endnodes,Q.getTransformation());
				//creating the version edge
				VersionEdge newve= new VersionEdge(S,newnode);
				V.AddVersionEdge(newve);
				V.AddEdge(newedge);
				V.AddNode(newnode);
				Update(V, S ,newnode);
			}
		}
	}


}
