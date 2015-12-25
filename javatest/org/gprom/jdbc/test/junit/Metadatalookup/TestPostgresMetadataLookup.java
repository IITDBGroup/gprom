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
	
}
