/**
 * 
 */
package org.gprom.jdbc.test.junit.Metadatalookup;

import static org.gprom.jdbc.utility.LoggerUtil.logException;

import java.io.File;
import java.sql.Connection;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.core.LoggerContext;
import org.gprom.jdbc.metadata_lookup.oracle.OracleMetadataLookup;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

/**
 * @author lord_pretzel
 *
 */
public class TestHiveMetadataLookup {

	static Logger log;
	
	static OracleMetadataLookup p;
	static Connection c;
	
	static {
		System.setProperty("log4j.configurationFile", "blackboxtests/log4jtest.properties");
		log = LogManager.getLogger(TestHiveMetadataLookup.class);
	}
	
	@BeforeClass
	public static void setup () throws SQLException, ClassNotFoundException {
//				LoggerContext context = (org.apache.logging.log4j.core.LoggerContext) LogManager.getContext(false);
//		File file = new File("blackboxtests/log4jtest.properties");
//		 
//		// this will force a reconfiguration
//		context.setConfigLocation(file.toURI());
//		PropertyConfigurator.configure("javalib/log4j.properties");
		//TODO adapt to Hive
//		String username = "fga_user";
//		String password = "fga";
//		String host = "ligeti.cs.iit.edu";
//		String port = "1521";
//		String sid = "orcl";
//		String url = "jdbc:oracle:thin:" + username + "/" + password + 
//				"@(DESCRIPTION=(ADDRESS_LIST=(ADDRESS=(PROTOCOL=TCP)(HOST=" + host + ")(PORT=" + port + ")))(CONNECT_DATA=(SID=" + sid +")))";

//		Class.forName("oracle.jdbc.OracleDriver");		
//		c = DriverManager.getConnection(url);
	 	p = new OracleMetadataLookup(c);
	}
	
	@AfterClass
	public static void shutdown() throws SQLException {
		c.close();
	}

	@Test
	public void tableExists () throws SQLException {
		log.debug("*********");
		Statement st = c.createStatement();
		
		createTestTable(st);
		
//		assertTrue(p.tableExists("TESTJDBCTABLE") == 1);
//		assertTrue(p.tableExists("OTHERNONEXISTINGTABLEXXX") == 0);
		dropTestTable(st);
		st.close();
	}
	
	@Test
	public void isAgg () {
//		assertTrue(p.isAgg("SUM") == 1);
	}
	
	@Test
	public void getAttrs () throws SQLException {
		List<String> attrs;
		List<String> exprAttrs = new ArrayList<String>();
		Statement st = c.createStatement();
		
		createTestTable(st);
				
		exprAttrs.add("A");
		exprAttrs.add("B");
//		attrs = p.getAttributeNames("TESTJDBCTABLE");
//		assertEquals(attrs, exprAttrs);
		
		dropTestTable(st);

		st.close();
	}

	private void dropTestTable(Statement st) {
		try {
			st.execute("DROP TABLE testjdbctable");
		}
		catch (Exception e) {
			logException(e,log);
		}
	}

	private void createTestTable(Statement st) throws SQLException {
		dropTestTable(st);
		st.execute("CREATE TABLE testjdbctable (A int, B int)");
	}
	
	@Test
	public void getDataTypes () throws SQLException {
		List<String> attrs;
		List<String> exprAttrs = new ArrayList<String>();
		Statement st = c.createStatement();
		
		createTestTable(st);
				
		exprAttrs.add("DT_INT");
		exprAttrs.add("DT_INT");
//		attrs = p.getAttributeDTs("TESTJDBCTABLE");
//		assertEquals(attrs, exprAttrs);
		
		dropTestTable(st);

		st.close();		
	}
	
	@Test
	public void getFunctionTypes ()  {
		String expected = "DT_FLOAT";
		String result;
		
//		result = p.getFuncReturnType("SUM", new String[] { "DT_FLOAT" }, 1);
//		assertEquals(expected, result);
	}

	
}
