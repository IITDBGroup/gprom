/**
 * 
 *//*
package org.gprom.jdbc.driver;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Properties;
import java.util.Scanner;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;
import org.gprom.jdbc.driver.GProMConnection;
import org.gprom.jdbc.driver.GProMDriverProperties;
import org.gprom.jdbc.driver.AutoProvenanceReader;

import com.sun.jna.Native;

*//**
 * @author lord_pretzel
 * 
 *//*
public class AutoProvenanceReader {

	static Logger log = Logger.getLogger(AutoProvenanceReader.class);
public static TestOutput tst = new TestOutput();

	public void provenance(String s, Statement st, GProMConnection con)
			throws Exception {

		if (!s.toLowerCase().contains("provenance")) {

			if (s.toLowerCase().contains("update")
					|| s.toLowerCase().contains("delete")) {
				try {
					String l = s.replace("update", "select");
				} catch (Exception e) {
					String l = s.replace("delete", "select");
				}
			}

			String x = s.replace(";", "");
			ResultSet rs = st.executeQuery("PROVENANCE OF (" + x + ");");
			ResultSetMetaData rsmd = rs.getMetaData();

			int columnCount = rsmd.getColumnCount();

			// The column count starts from 1
			for (int i = 1; i < columnCount + 1; i++) {

				String name = rsmd.getColumnName(i);
				if (name.contains("_result_tid")
						|| name.toLowerCase().contains("prov")) {
				} else {
					System.out.println("Here------>>>>>>" + name);
				}
			}

			ResultSet rs1 = st.executeQuery(s);
			printResult(rs1);
			String x1 = s.replace(";", "");
			rs1 = st.executeQuery("PROVENANCE OF (" + x1 + ");");
			printResult(rs1);

			// test error
			try {
				rs = st.executeQuery("PROVENANCE OF (" + x + ");");
			} catch (Exception e) {
				System.out.printf("%s", e);
			}

			rs = st.executeQuery("PROVENANCE OF (" + x + ");");
			printResult(rs);

			log.error("statement shutdown");
			con.close();
		} else {
			ResultSet rs = st.executeQuery(s);
			ResultSetMetaData rsmd = rs.getMetaData();

			int columnCount = rsmd.getColumnCount();

			// The column count starts from 1
			for (int i = 1; i < columnCount + 1; i++) {

				String name = rsmd.getColumnName(i);
				if (name.contains("_result_tid")
						|| name.toLowerCase().contains("prov")) {
				} else {
					System.out.println("Here------>>>>>>" + name);
				}
			}

			rs = st.executeQuery(s);

			ResultSetMetaData rsmd1 = rs.getMetaData();

			int columnCount1 = rsmd.getColumnCount();

			// The column count starts from 1
			for (int i = 1; i < columnCount + 1; i++) {
				String name = rsmd.getColumnName(i);
				if (name.contains("_result_tid")
						|| name.toLowerCase().contains("prov")) {
				} else {
					System.out.println("Here------>>>>>>" + name);
				}
			}

			printResult(rs);

			// test error
			try {
				rs = st.executeQuery(s);
			} catch (Exception e) {
				System.out.printf("%s", e);
			}

			rs = st.executeQuery(s);
			printResult(rs);

			log.error("statement shutdown");
			con.close();
		}
	}

	private static void printResult(ResultSet rs) throws SQLException
			 {
		try (PrintWriter out = new PrintWriter(new BufferedWriter(
				new FileWriter("myfile.txt", true)))) {

			System.out
					.println("-------------------------------------------------------------------------------");
		
			tst.Test("-------------------------------------------------------------------------------");
			tst.Test("\n");
			for (int i = 1; i <= rs.getMetaData().getColumnCount(); i++)
				System.out.print(rs.getMetaData().getColumnLabel(i) + "\t|");
			for (int i = 1; i <= rs.getMetaData().getColumnCount(); i++)
				//out.print(rs.getMetaData().getColumnLabel(i) + "\t|");
			tst.Test(rs.getMetaData().getColumnLabel(i) + "\t|");
			
			System.out.println();
			tst.Test("\n");
			System.out
					.println("-------------------------------------------------------------------------------");
			tst.Test("-------------------------------------------------------------------------------");
			tst.Test("\n");
			while (rs.next()) {
				for (int i = 1; i <= rs.getMetaData().getColumnCount(); i++)
					System.out.print(rs.getString(i) + "\t|");
				for (int i = 1; i <= rs.getMetaData().getColumnCount(); i++)
				
					tst.Test(rs.getString(i) + "\t|");
				tst.Test("\n");
				System.out.println();
			}
			System.out.println();
			tst.Test("\n");

			System.out
					.println("-------------------------------------------------------------------------------");
			tst.Test("-------------------------------------------------------------------------------");
			System.out.println();
			tst.Test("\n");
			System.out.println();
			tst.Test("\n");
		
			
		
			
		} catch (IOException e) {
			// exception handling left as an exercise for the reader
		}
	}

}
*/