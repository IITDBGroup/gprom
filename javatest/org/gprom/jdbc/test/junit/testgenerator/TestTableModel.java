/**
 * 
 */
package org.gprom.jdbc.test.junit.testgenerator;

import static org.junit.Assert.assertEquals;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;

import org.apache.log4j.Logger;
import org.gprom.jdbc.test.testgenerator.dataset.MemDBTable;
import org.gprom.jdbc.test.testgenerator.dataset.Row;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import static org.junit.Assert.*;

/**
 * @author lord_pretzel
 *
 */
public class TestTableModel {

	Logger log = Logger.getLogger(TestTableModel.class);
	
	@BeforeClass
	public static void setup () throws SQLException, ClassNotFoundException {
	}

	@AfterClass
	public static void shutdown() throws SQLException {
	}

	@Test
	public void testRows () {
		Row r1,r2;
		r1 = new Row("1","2");
		r2 = new Row("1","2");
		assertEquals(r1.toString() + " == " + r2.toString(), r1,r2);
	}

	@Test
	public void testRowConstructors () {
		Row r1,r2,r3;
		List<String> val = new ArrayList<String> ();
		val.add("1");
		val.add(null);
		val.add("2");
		val.add(null);
		
		r1 = new Row("1",null,"2",null);
		r2 = new Row(r1);
		r3 = new Row(val);
		assertEquals(r1,r2);
		assertEquals(r1,r3);
		assertEquals(r1.toString() + " == " + r2.toString(), r1,r2);
	}
	
	@Test
	public void testTables () {
		MemDBTable t1 = new MemDBTable();
		MemDBTable t2 = new MemDBTable();
		
		t1.addColumn("a");
		t1.addColumn("b");
		t1.addRow(new Row("1", "1"));
		t1.addRow(new Row("1", "2"));
		
		t2.addColumn("a");
		t2.addColumn("b");
		t2.addRow(new Row("1", "1"));
		t2.addRow(new Row("1", "2"));
		assertEquals(t1.toString() + " == " + t2.toString(), t1, t2);
		
		t1 = new MemDBTable();
		t2 = new MemDBTable();
		
		t1.addColumn("a");
		t1.addColumn("b");
		t1.addColumn("c");
		t1.addColumn("d");
		t1.addRow(new Row("1", null, "2", null));
		
		t2.addColumn("a");
		t2.addColumn("b");
		t2.addColumn("c");
		t2.addColumn("d");
		t2.addRow(new Row("1", null, "2", null));
		assertEquals(t1.toString() + " == " + t2.toString(), t1, t2);
	}
}
