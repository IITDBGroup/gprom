/**
 * 
 */
package org.gprom.jdbc.test;

import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Properties;

import org.apache.logging.log4j.Level;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.apache.logging.log4j.core.appender.ConsoleAppender;
import org.apache.logging.log4j.core.layout.PatternLayout;
import org.gprom.jdbc.driver.GProMConnection;
import org.gprom.jdbc.driver.GProMDriverProperties;
import org.gprom.jdbc.jna.NativeGProMLibException;
import org.gprom.jdbc.utility.PropertyConfigurator;

import com.sun.jna.Native;

/**
 * @author lord_pretzel
 *
 */
public class SQLiteJDBCTest {

	static Logger log; 
	
	public static void main (String[] args) throws Exception {
		String log4jFile = "blackboxtests/log4jtest.properties";
		
		
		if (args.length == 1)
			log4jFile = args[0];
		
		
		
		PropertyConfigurator.configureDefaultConsoleLogger(Level.DEBUG);
//		PropertyConfigurator.configureWithDefaultAsFallback(log4jFile);
	
		log = LogManager.getLogger(SQLiteJDBCTest.class);

		String driverURL = "org.sqlite.JDBC";
		String url = "jdbc:gprom:sqlite:examples/test.db";
		GProMConnection con = null;
		Native.setProtected(true);
		log.error(url);
		try{
			log.error("made it this far");
			Class.forName("org.gprom.jdbc.driver.GProMDriver");
			Class.forName(driverURL);
//			w.setLogLevel(2);
		} catch(ClassNotFoundException e){
			e.printStackTrace();
			System.err.println("Install the needed driver first");
			System.exit(-1);
		}
		try{
			log.error("made it this far");
			Properties info = new Properties();
			info.setProperty(GProMDriverProperties.JDBC_METADATA_LOOKUP_NAME, "TRUE");
			log.error("made it this far");
			con = (GProMConnection) DriverManager.getConnection(url,info);
			log.error("made it this far");
		} catch (SQLException e){
			e.printStackTrace();
			System.err.println("Something went wrong while connecting to the database.");
			System.exit(-1);
		}
		System.out.println("Connection was successfully");

		con.getW().setLogLevel(4);
		con.getW().setBoolOption("pi_cs_use_composable", false);
		con.getW().setBoolOption("optimize_operator_model", false);
		con.getW().setBoolOption("aggressive_model_checking", true);
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
//		ResultSet rs = st.executeQuery("SELECT a FROM r;");
		ResultSet rs;
		try {
			rs = st.executeQuery("PROVENANCE OF (SELECT a FROM r);");
			printResult(rs);
		}
		catch (NativeGProMLibException e) {
			log.error("###############################\n" + e.getMessage() + "\n###############################\n");
		}
		
//		
		rs = st.executeQuery("SELECT sum(a) AS X FROM R;");
		printResult(rs);
		
//		rs = st.executeQuery("SELECT sum(a) AS x FROM R;");
//		printResult(rs);
//
//		rs = st.executeQuery("PROVENANCE OF (SELECT sum(a) AS x FROM R);");
//		printResult(rs);
		
//		rs = st.executeQuery("SELECT \"a\" FROM \"o\";");
//		printResult(rs);
//		
//		
//		// test error
//		try {
//		rs = st.executeQuery("PROVENANCE OF (SELECT * FRO R);");
//		}
//		catch (Exception e) {
//			System.out.printf("%s", e);
//		}
//		
//		rs = st.executeQuery("PROVENANCE OF (SELECT * FROM R);");
//		printResult(rs);
		
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
