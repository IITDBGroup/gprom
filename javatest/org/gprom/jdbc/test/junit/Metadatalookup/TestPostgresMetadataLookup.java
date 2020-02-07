/**
 * 
 */
package org.gprom.jdbc.test.junit.Metadatalookup;

import static org.junit.Assert.*;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;

import org.gprom.jdbc.jna.GProMJavaInterface.DataType;
import org.gprom.jdbc.metadata_lookup.postgres.PostgresMetadataLookup;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

/**
 * @author lord_pretzel
 *
 */
public class TestPostgresMetadataLookup {

	static PostgresMetadataLookup p;
	static Connection c;
	
	@BeforeClass
	public static void setup () throws SQLException, ClassNotFoundException {
		Class.forName("org.postgresql.Driver");
		c = DriverManager.getConnection("jdbc:postgresql://localhost/postgres?user=postgres");
	 	p = new PostgresMetadataLookup(c);
	}
	
	@AfterClass
	public static void shutdown() throws SQLException {
		c.close();
	}
	
	
	@Test
	public void getAttrs () throws SQLException {
		List<String> attrs;
		List<String> exprAttrs = new ArrayList<String>();
		Statement st = c.createStatement();
		
		st.execute("DROP TABLE IF EXISTS testjdbctable;");
		st.execute("CREATE TABLE testjdbctable (a int, b int)");
				
		exprAttrs.add("a");
		exprAttrs.add("b");
		attrs = p.getAttributeNames("testjdbctable");
		assertEquals(attrs, exprAttrs);
		
		st.execute("DROP TABLE IF EXISTS testjdbctable;");
		st.close();
	}
	
	@Test
	public void getFuncAndOp () {
		DataType d;
		
		d = DataType.valueOf(p.getFuncReturnType("substr", new String[] {"DT_STRING", "DT_INT", "DT_INT"}, 3));
		assertEquals(d, DataType.DT_STRING);
		
		d = DataType.valueOf(p.getOpReturnType("+", new String[] {"DT_INT", "DT_INT"}, 2));
		assertEquals(d, DataType.DT_INT);
	}
	
	@Test
	public void checkTypeOfFunction () {		
		assertTrue(p.isAgg("sum") == 1);
		assertTrue(p.isAgg("substr") == 0);
		assertTrue(p.isAgg("xxxffff") == 0);
		assertTrue(p.isAgg("avg") == 1);
		assertTrue(p.isAgg("count") == 1);
		
		assertTrue(p.isWindow("sum") == 1);
		assertTrue(p.isWindow("substr") == 0);
		assertTrue(p.isWindow("xxxffff") == 0);
		assertTrue(p.isWindow("row_number") == 1);
		assertTrue(p.isWindow("first_value") == 1);
	}

	@Test
	public void getViewDef () throws SQLException {
		Statement st = c.createStatement();
		String viewQuery = "SELECT test__jdbctable.a FROM test__jdbctable";
		String expected = "SELECT test__jdbctable.a\n" + 
				"   FROM test__jdbctable;";
		String viewDef = "CREATE VIEW test__view AS (" + viewQuery + ");";
		String result;

		st.execute("DROP VIEW IF EXISTS test__view CASCADE;");
		st.execute("DROP TABLE IF EXISTS test__jdbctable CASCADE;");
		st.execute("CREATE TABLE test__jdbctable (a int, b int)");
		st.execute(viewDef);
		
		result = p.getViewDefinition("test__view");
		assertEquals(expected, result);
		
		st.execute("DROP VIEW IF EXISTS test__view CASCADE;");
		st.execute("DROP TABLE IF EXISTS test__jdbctable CASCADE;");
		
		st.close();
	}
	
}
