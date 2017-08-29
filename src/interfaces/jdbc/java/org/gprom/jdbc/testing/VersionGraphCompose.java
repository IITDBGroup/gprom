package org.gprom.jdbc.testing;


import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

import org.gprom.jdbc.pawd.Edge;
import org.gprom.jdbc.pawd.JDBCConnect;
import org.gprom.jdbc.pawd.Node;
import org.gprom.jdbc.pawd.VersionEdge;
import org.gprom.jdbc.pawd.VersionGraph;
import org.gprom.jdbc.pawd.VersionGraphManager;
import org.gprom.jdbc.pawd.VersionGraphStore.Operation;
import org.gprom.jdbc.pawd.VersionGraphStore.Operation.Materialization;
import org.gprom.jdbc.pawd.VersionGraphStore.Operation.OpType;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class VersionGraphCompose {
	VersionGraph Graph1 ;
	Node T;
	Map <Node, Materialization> MaterializationPlan = new HashMap<Node, Materialization>();
	@Before
	public void setUp() throws Exception {
		Node R = new Node(false, "R");
		Node S = new Node(false, "S");
		T = new Node(false,"T");
		Node J = new Node(false,"J");
		//construct different arraylist for Edge construction
		ArrayList<Node> NodeSetAll = new ArrayList<>(Arrays.asList(J,R,S,T));
		ArrayList<Node> startNodes = new ArrayList<>(Arrays.asList(R,J));
		ArrayList<Node> endNodes = new ArrayList<>(Arrays.asList(T));
		//construct sample operations for edge creation
		Operation op1 = new Operation("SELECT a, b * 2 AS b FROM $$1$$ ",OpType.Query);
//		Operation op2 = new Operation("SELECT sum(a), b FROM $$1$$ GROUP BY b",OpType.Update);
//		Operation op3 = new Operation ("SELECT b FROM $$1$$",OpType.Query);
		Operation op2 = new Operation("SELECT sum(a), b FROM ($$1$$ NATURAL JOIN $$2$$) GROUP BY b",OpType.Query);
		//sample set of edges
		Edge edge2 = new Edge(startNodes,endNodes,op2);
		Edge edge1 = new Edge(R,S, op1);
//		Edge edge2 = new Edge(S,T,op2);
//		Edge edge3 = new Edge(T,J,op3);
		//construct Arraylist of EDGES
		ArrayList<Edge> EdgeSetAll = new ArrayList<>( Arrays.asList(edge1,edge2));//edge3,edge4,edge5,edge6));
		//System.out.println(EdgeSetAll);
		//construct arraylist of versionedges
		ArrayList<VersionEdge> VersionEdgeSetAll = new ArrayList<>();
		//create a version graph
		Graph1 = new VersionGraph(NodeSetAll, EdgeSetAll,VersionEdgeSetAll,null);


		//setting up materialization plan
		//System.out.println(Graph1);
		MaterializationPlan.put(R,Materialization.isMaterialized );
		MaterializationPlan.put(S,Materialization.notMaterialized );
		MaterializationPlan.put(T,Materialization.isMaterialized);
		MaterializationPlan.put(J,Materialization.isMaterialized);
	}

	@After
	public void tearDown() throws Exception {
		JDBCConnect mine= new JDBCConnect();
		for (Map.Entry<Node, Materialization> entry : MaterializationPlan.entrySet()) {
			  if (entry.getValue().equals(Materialization.isMaterialized)) {
				  String tblName = entry.getKey().getDescription();
				  System.out.println("Deleting "+tblName);
				 // mine.RunUpdate("DROP TABLE REL_"+ tblName );
			  }
			}
	}

	@Test
	public void test() {
		VersionGraphManager myManager = new VersionGraphManager();
		myManager.Configure(Graph1);
		String q = myManager.Compose(Graph1, T,MaterializationPlan);
		System.out.println(q);
		JDBCConnect mine= new JDBCConnect();
		mine.RunQuery("Select * FROM ("+q+")");
	}

}
