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
		assertEquals("1","1");
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
	}
}
