package org.gprom.jdbc.driver;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.sql.ResultSet;
import java.sql.SQLException;

public class TestOutput {
	public void Test(String s) throws SQLException {
		try (PrintWriter out = new PrintWriter(new BufferedWriter(
				new FileWriter("myfile.txt", true)))) {
			out.print(s);
			out.print("\n");

		} catch (IOException e) {
			
		}
		try (PrintWriter out = new PrintWriter(new BufferedWriter(
				new FileWriter("myfile.csv", true)))) { //I have also stored the data in csv since Tanu and Alex mentioned that they use CSV files for transforming data
			out.print(s);

			out.print("\n");

		} catch (IOException e) {
			
		}
	}
}



/* Two interfaces onr for queries 
//  make interfaces for every method
//
provenance witgh TUPLE VERSIONS OF (select)
*/