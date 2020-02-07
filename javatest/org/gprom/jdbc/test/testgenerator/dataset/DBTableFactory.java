/**
 * 
 */
package org.gprom.jdbc.test.testgenerator.dataset;

import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.gprom.jdbc.utility.LoggerUtil;

import java.sql.ResultSetMetaData;



/**
 * @author lord_pretzel
 *
 */
public class DBTableFactory {

	Logger log = LogManager.getLogger(DBTableFactory.class);
	
	public static DBTableFactory inst = new DBTableFactory();
	
	private DBTableFactory () {
		
	}
	
	public static DBTableFactory getInst() {
		return inst;
	}
	
	public DBTable tableFromString(String s) {
		MemDBTable t = new MemDBTable();
		String[] lines;
		String[] columns;
		String[] rows;
		int numColumns;
		String value;		
		
		/* split lines */
		lines = removeEmptyLines(s.split("\n"));
		
		/* get columns */
		columns = lines[0].split("\\|");
		numColumns = columns.length;
		
		/* output columns */
		for (int i = 0; i < numColumns; i++) {
			t.addColumn(columns[i].trim());
		}
		
		/* get lines and output them */
		for (int i = 2; i < lines.length; i++) {
			List<String> row = new ArrayList<String> ();
			rows = lines[i].split("\\|");
			/* replace escapted '|' characters */
			for(int j = 0; j < numColumns; j++) {
				rows[j] = rows[j].replaceAll("\\$MID\\$", "|");
			}
			
			for (int j = 0; j < rows.length; j++) {
				if (rows[j].trim().equals("(null)")) {
					row.add(null);
				}
				else if (rows[j].trim().equals("EMPTYSTRING")) {
					row.add("");
				}
				else {
					value = rows[j].trim();
					value = escapeXml(value);
					if(value.startsWith("@"))
						value = value.replace('@', ' ');
					row.add(value);
				}
			}
			t.addRow(new Row(row));
		}
		
		return t;
	}
	
	public DBTable tableFromQuery(Connection c, String q) throws SQLException {
		MemDBTable t = new MemDBTable();
		Statement s = c.createStatement();
		ResultSet r = null;
		
		try {
			r = s.executeQuery(q);
			ResultSetMetaData m = r.getMetaData();
			
			// add columns
			for (int i = 1; i <= m.getColumnCount(); i++) {
				t.addColumn(m.getColumnName(i));
			}
			
			// add rows
			while(r.next()) {
				List<String> values = new ArrayList<String> ();
				for(int i = 1; i <= m.getColumnCount(); i++) {
					values.add(r.getString(i));
				}
				t.addRow(new Row(values));
			}
			r.close();
		} catch (Throwable e) {
			LoggerUtil.logDebugException(e, log);
			System.out.println(e);
			throw(e);
		} finally {
			if (r != null)
				r.close();
			if (s != null)			
				s.close();
			log.debug("closed statement and result set");
		}
		
		return t;
	}
	
	protected String[] removeEmptyLines(String[] in) {
		String[] result;
		List<String> buf = new ArrayList<String> ();
		
		for(String line: in) {
			if (!line.trim().equals(""))
				buf.add(line.trim());
		}
		
		result = buf.toArray(new String[] {} );
		
		log.debug("after remove strings: " + Arrays.toString(result));
		
		return result;		
	}

	private String escapeXml (String xml) {
		StringBuilder result;
		char[] chars;
		
		result = new StringBuilder();
		chars = xml.toCharArray();
		
		for(int i = 0; i < chars.length; i++) {
			if (chars[i] == '<')
				result.append("&lt;");
			else if (chars[i] == '>')
				result.append("&gt;");
			else
				result.append(chars[i]);
		}		
		
		return result.toString();
	}
}
