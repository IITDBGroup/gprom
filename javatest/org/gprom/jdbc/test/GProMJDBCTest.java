/**
 * 
 */
package org.gprom.jdbc.test;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;
import org.gprom.jdbc.driver.GProMConnection;
import org.gprom.jdbc.jna.GProMWrapper;

import com.sun.jna.Native;

/**
 * @author lord_pretzel
 *
 */
public class GProMJDBCTest {

	static Logger log = Logger.getLogger(GProMJDBCTest.class);
	
	public static void main (String[] args) throws Exception {
		PropertyConfigurator.configureAndWatch("javalib/log4j.properties");
		String driverURL = "oracle.jdbc.OracleDriver";
		//String url = "jdbc:hsqldb:file:/Users/alex/db/mydb";
		String username = "fga_user";
		String password = "fga";
		String host = "ligeti.cs.iit.edu";
		String port = "1521";
		String sid = "orcl";
		String url = "jdbc:gprom:oracle:thin:" + username + "/" + password + 
				"@(DESCRIPTION=(ADDRESS_LIST=(ADDRESS=(PROTOCOL=TCP)(HOST=" + host + ")(PORT=" + port + ")))(CONNECT_DATA=(SID=" + sid +")))";
		GProMConnection con = null;
		Native.setProtected(true);
		log.error("Proetcted: " + Native.isProtected());
		try{
			Class.forName("org.gprom.jdbc.driver.GProMDriver");
			Class.forName(driverURL);
//			w.setLogLevel(2);
		} catch(ClassNotFoundException e){
			e.printStackTrace();
			System.err.println("Install the needed driver first");
			System.exit(-1);
		}
		try{
			con = (GProMConnection) DriverManager.getConnection(url,username,password);
		} catch (SQLException e){
			e.printStackTrace();
			System.err.println("Something went wrong while connecting to the database.");
			System.exit(-1);
		}
		System.out.println("Connection was successfully");

		con.getW().setBoolOption("pi_cs_use_composable", true);
		con.getW().setBoolOption("optimize_operator_model", true);
		con.getW().setLogLevel(4);
		log.error("log.level=" + con.getW().getIntOption("log.level"));
		log.error("log.active=" +  con.getW().getBoolOption("log.active"));
		log.error("pi_cs_use_composable=" +  con.getW().getBoolOption("pi_cs_use_composable"));
		log.error("optimize_operator_model=" +  con.getW().getBoolOption("optimize_operator_model"));
		log.error("type of log.level: " + con.getW().typeOfOption("log.level"));
		log.error("type of log.activate: " + con.getW().typeOfOption("log.active"));
		log.error("type of connection.port: " + con.getW().typeOfOption("connection.port"));
		log.error("type of connection.user: " + con.getW().typeOfOption("connection.user"));
		log.error("type of connection.passwd: " + con.getW().typeOfOption("connection.passwd"));
		log.error("type of connection.host: " + con.getW().typeOfOption("connection.host"));
		log.error("type of connection.db: " + con.getW().typeOfOption("connection.db"));
		
		Statement st = con.createStatement();
		
		log.error("statement created");
		ResultSet rs = st.executeQuery("PROVENANCE OF (SELECT sum(a) FROM r GROUP BY b);");
		printResult(rs);
		
		rs = st.executeQuery("PROVENANCE OF (SELECT * FROM R);");
		printResult(rs);
		
		// test error
		rs = st.executeQuery("PROVENANCE OF (SELECT * FRO R);");
		
		
		
		log.error("statement shutdown");
		con.close();
	}

	private static void printResult(ResultSet rs) throws SQLException {
		/********************************************************************************/
		System.out.println();
		System.out.println("-------------------------------------------------------------------------------");
		for(int i = 1; i <= rs.getMetaData().getColumnCount(); i++)
			System.out.print(rs.getMetaData().getColumnLabel(i) + "\t|");
		System.out.println();
		System.out.println("-------------------------------------------------------------------------------");
		
		while(rs.next()) {		
			for(int i = 1; i <= rs.getMetaData().getColumnCount(); i++)
				System.out.print(rs.getString(i) + "\t|");
			System.out.println();
		}
		System.out.println("-------------------------------------------------------------------------------");
		System.out.println();
		System.out.println();
	}
	
}
