/**
 * 
 */
package org.gprom.jdbc.test;

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
import org.gprom.jdbc.driver.GProMDriver;
import org.gprom.jdbc.driver.GProMDriverProperties;
import org.gprom.jdbc.driver.AutoProvenanceReader;

import com.sun.jna.Native;

/**
 * @author lord_pretzel
 * 
 */
public class Test {

	static Logger log = Logger.getLogger(Test.class);

	public static void main(String[] args) throws Exception {
		PropertyConfigurator.configureAndWatch("javalib/log4j.properties");
		String driverURL = "oracle.jdbc.OracleDriver";
		// String url = "jdbc:hsqldb:file:/Users/alex/db/mydb";
		String username = "fga_user";
		String password = "fga";
		String host = "ligeti.cs.iit.edu";
		String port = "1521";
		String sid = "orcl";
		String url = "jdbc:gprom:oracle:thin:" + username + "/" + password
				+ "@(DESCRIPTION=(ADDRESS_LIST=(ADDRESS=(PROTOCOL=TCP)(HOST="
				+ host + ")(PORT=" + port + ")))(CONNECT_DATA=(SID=" + sid
				+ ")))";
		GProMConnection con = null;
		Native.setProtected(true);
		log.error("Proetcted: " + Native.isProtected());
		try {
			Class.forName("org.gprom.jdbc.driver.GProMDriver");
			Class.forName(driverURL);
			// w.setLogLevel(2);
		} catch (ClassNotFoundException e) {
			e.printStackTrace();
			System.err.println("Install the needed driver first");
			System.exit(-1);
		}
		try {
			Properties info = new Properties();
			info.setProperty(GProMDriverProperties.JDBC_METADATA_LOOKUP, "TRUE");
			info.setProperty("user", username);
			info.setProperty("password", password);
			con = (GProMConnection) DriverManager.getConnection(url, info);
		} catch (SQLException e) {
			e.printStackTrace();
			System.err
					.println("Something went wrong while connecting to the database.");
			System.exit(-1);
		}
		System.out.println("Connection was successfully");

		con.getW().setLogLevel(0);
		con.getW().setBoolOption("pi_cs_use_composable", true);
		con.getW().setBoolOption("optimize_operator_model", true);
		con.getW().setBoolOption("aggressive_model_checking", true);
		log.error("log.level=" + con.getW().getIntOption("log.level"));
		log.error("log.active=" + con.getW().getBoolOption("log.active"));
		log.error("pi_cs_use_composable="
				+ con.getW().getBoolOption("pi_cs_use_composable"));
		log.error("optimize_operator_model="
				+ con.getW().getBoolOption("optimize_operator_model"));
		log.error("type of log.level: " + con.getW().typeOfOption("log.level"));
		log.error("type of log.activate: "
				+ con.getW().typeOfOption("log.active"));
		log.error("type of connection.port: "
				+ con.getW().typeOfOption("connection.port"));
		log.error("type of connection.user: "
				+ con.getW().typeOfOption("connection.user"));
		log.error("type of connection.passwd: "
				+ con.getW().typeOfOption("connection.passwd"));
		log.error("type of connection.host: "
				+ con.getW().typeOfOption("connection.host"));
		log.error("type of connection.db: "
				+ con.getW().typeOfOption("connection.db"));

		Statement st = con.createStatement();

		log.error("statement created");

	/*	Scanner in = new Scanner(System.in);                //Just uncomment this code to get user input
		System.out.println("Enter a string");
		String s = in.nextLine();*/
		String s = "select rowid, * from r where a='2';";
		GProMDriver test = new GProMDriver();
		test.provenance(s, st, con);

	}

}
