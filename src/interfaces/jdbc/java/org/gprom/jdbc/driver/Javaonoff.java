package org.gprom.jdbc.driver;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.PrintWriter;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Scanner;
import java.util.Set;

public class Javaonoff implements MyInterface {

	private static Javaonoff inst = new Javaonoff();
	static boolean check = checkJDBC.checkJDBC();

	private Javaonoff() {

	}

	public static Javaonoff getInstance() {
		return inst;
	}

	public void JavaLoggingParameter(String s) {
		FunctionsClass.javaLogging(s);
	}

	public void QueryLogger(String s) {
		if (check) {
			FunctionsClass.queryLogger(s);
		}
	}

	public void HashLogger(String k, String v) throws FileNotFoundException,
			ClassNotFoundException, IOException {
		if (check) {
			FunctionsClass.HashLogger(k, v);
		}
	}

	public void printResult(ResultSet rs, String s) throws SQLException {
		if (check) {
			FunctionsClass.printResult(rs, s);
		}
	}

	public void resultsLogger(String s) {
		if (check) {
			FunctionsClass.resultLogger(s);
		}
	}
	public void JavaLogger(String s){
		if(check){
			System.out.println(s);
		}
	}

}
