package org.gprom.jdbc.instrumentation;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.sql.ResultSet;
import java.sql.SQLException;

public interface MyInterface {
	public void QueryLogger(String s);
	public void JavaLoggingParameter(String s);
	public void HashLogger(String k, String v) throws FileNotFoundException,
	ClassNotFoundException, IOException;
	public void printResult(ResultSet rs, String s) throws SQLException;
	public void resultsLogger(String s);
	public void JavaLogger(String s);
}
