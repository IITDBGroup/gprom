package org.gprom.jdbc.driver;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.sql.ResultSet;
import java.sql.SQLException;

public class TestOutput {
public void Test(String s) throws SQLException{
	try (PrintWriter out = new PrintWriter(new BufferedWriter(
			new FileWriter("myfile.txt", true)))) {
		out.print(s);
		
		
		
	} catch (IOException e) {
				// exception handling left as an exercise for the reader
			}
}
}
