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
public class SQLiteDatalogJDBCTest {

	static Logger log; 
	
	public static void main (String[] args) throws Exception {
		String log4jFile = "log4j2-test.xml";
		
		if (args.length == 1)
			log4jFile = args[0];
				
		PropertyConfigurator.configureHonoringProperties(log4jFile, "blackboxtests/log4j2.xml", "log4j2.xml");
	
		log = LogManager.getLogger(SQLiteDatalogJDBCTest.class);

		String driverURL = "org.sqlite.JDBC";
		String url = "jdbc:gprom:sqlite:examples/test.db";
		GProMConnection con = null;
		Native.setProtected(true);
		log.error(url);
		try{
			log.error("made it this far");
			Class.forName("org.gprom.jdbc.driver.GProMDriver");
			Class.forName(driverURL);
		} catch(ClassNotFoundException e){
			e.printStackTrace();
			System.err.println("Install the needed driver first");
			System.exit(-1);
		}
		try{
			Properties info = new Properties();
			info.setProperty(GProMDriverProperties.JDBC_METADATA_LOOKUP_NAME, "TRUE");
			
			con = (GProMConnection) DriverManager.getConnection(url,info);
			
			con.getW().setStringOption("plugin.parser", "dl");
			con.getW().setStringOption("plugin.analyzer", "dl");
			con.getW().setStringOption("plugin.translator", "dl");
			con.getW().reconfPlugins();
			con.getW().setLogLevel(4);
		} catch (SQLException e){
			e.printStackTrace();
			System.err.println("Something went wrong while connecting to the database.");
			System.exit(1);
		}
		System.out.println("Connection was successful");
		
		Statement st = con.createStatement();		
		log.error("statement created");
		
		ResultSet rs;
		try {
			rs = st.executeQuery("Q(X) :- R(X,Y).");
			printResult(rs);
		}
		catch (NativeGProMLibException e) {
			log.error("###############################\n" + e.getMessage() + "\n###############################\n");
		}
		
		log.error("statement shutdown");
		con.close();
	}

	private static void printResult(ResultSet rs) throws SQLException {
		System.out.println("********************************************************************************");
		
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
