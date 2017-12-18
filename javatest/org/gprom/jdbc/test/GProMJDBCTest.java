/**
 * 
 */
package org.gprom.jdbc.test;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Properties;
import java.util.Scanner;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.gprom.jdbc.driver.GProMConnection;
import org.gprom.jdbc.driver.GProMDriverProperties;
import org.gprom.jdbc.jna.NativeGProMLibException;
import org.gprom.jdbc.utility.PropertyConfigurator;

import com.sun.jna.Native;

/**
 * @author lord_pretzel
 *
 */
public class GProMJDBCTest {

	static {
		System.setProperty("log4j.configurationFile", "blackboxtests/log4jtest.properties");
	}
	
	static Logger log = LogManager.getLogger(GProMJDBCTest.class);
	
	public static void main (String[] args) throws Exception {
		
		//PropertyConfigurator.configureAndWatch("blackboxtests/log4jtest.properties");
		String driverURL = "oracle.jdbc.OracleDriver";
//		String driverURL = "org.postgresql.Driver";
//		String url = "jdbc:hsqldb:file:/Users/alex/db/mydb";
//		String username = "fga_user";
		String username = "testuser";
		String password; // = "XXX";
//		String username = "postgres";
//		String password = "";
//		String host = "ligeti.cs.iit.edu";
		String host = "127.0.0.1";
		String port = "1521";
		String sid = "orcl";
		String url;
//		String url = "jdbc:gprom:postgresql://127.0.0.1:5432/testdb";
		
		//get password
		System.out.print("enter password: ");
		Scanner in = new Scanner(System.in);
		password = in.nextLine();
		// setup url
		url = "jdbc:gprom:oracle:thin:" + username + "/" + password + 
				"@(DESCRIPTION=(ADDRESS_LIST=(ADDRESS=(PROTOCOL=TCP)(HOST=" + host + ")(PORT=" + port + ")))(CONNECT_DATA=(SID=" + sid +")))";
		
		GProMConnection con = null;
		Native.setProtected(true);
		log.debug("XXXXXXXXXXXX");
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
			info.setProperty(GProMDriverProperties.JDBC_METADATA_LOOKUP, "TRUE");
			info.setProperty("user", username);
			info.setProperty("password", password);
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
		
		Connection wCon = con.getWrappedCon();
		Statement st = wCon.createStatement();
		Statement gSt = con.createStatement();
		
		log.error("statement created");
		
		ResultSet rs;
//		rs = st.executeQuery("SELECT a FROM r;");
		try {
//			rs = st.executeQuery("PROVENANCE OF (SELECT a FROM R);");
			System.out.println("***************");
			for(int i = 0; i < 100; i++) {
				rs = gSt.executeQuery("SELECT * FROM r WHERE a < " + i + ";");
				printResult(rs);
				rs = st.executeQuery("SELECT   A.VALUE   "
						+ "FROM V$SESSTAT A,     V$STATNAME B,     V$SESSION S   "
						+ "WHERE A.STATISTIC# = B.STATISTIC#     AND S.SID        = A.SID     "
						+ "AND B.NAME       = 'opened cursors current'     "
						+ "AND USERNAME     = 'TESTUSER'");
				printResult(rs);
				Thread.sleep(3000);
			}
		}
		catch (NativeGProMLibException e) {
			log.error("###############################\n" + e.getMessage() + "\n###############################\n");
		}
		
		gSt.close();
		st.close();
//		
//		rs = st.executeQuery("PROVENANCE OF (SELECT a FROM R);");
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
		rs.close();
	}
	
}
