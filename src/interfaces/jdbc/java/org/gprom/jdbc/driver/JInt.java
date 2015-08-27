package org.gprom.jdbc.driver;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.sql.ResultSet;
import java.sql.SQLException;

interface MyInterface {
	abstract void QueryLogger(String s);

	abstract void JavaLoggingParameter(String s);

	abstract void HashLogger(String k, String v)throws FileNotFoundException, ClassNotFoundException, IOException;

	abstract void printResult(ResultSet rs, String s) throws SQLException ;

	abstract void resultsLogger(String s);
}
