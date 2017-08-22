/**
 * 
 */
package org.gprom.jdbc.pawd;
import java.sql.*;
/**
 * @author Amer
 *
 */
public class JDBCConnect {
	// JDBC driver name and database URL
   static final String JDBC_DRIVER = "oracle.jdbc.driver.OracleDriver";  
   static final String DB_URL = "jdbc:oracle:thin:@ligeti.cs.iit.edu:1521:orcl";

   //  Database credentials
   static final String USER = "fga_user" ;
   static final String PASS = "fga";
   Connection conn = null;
   Statement stmt = null;
   public void RunUpdate(String sql){
	   try{
		      //STEP 2: Register JDBC driver
		      Class.forName(JDBC_DRIVER);
	
		      //STEP 3: Open a connection
		      System.out.println("Connecting to a selected database...");
		      conn = DriverManager.getConnection(DB_URL, USER, PASS);
		      System.out.println("Connected database successfully...");
		      //STEP 4: Execute a query
		      System.out.println("Updating...");
		      System.out.println(sql );
		      stmt = conn.createStatement();
		      stmt.executeUpdate(sql);
		      System.out.println("done and done");
	   }catch(SQLException se){
		      //Handle errors for JDBC
		      se.printStackTrace();
		   }catch(Exception e){
		      //Handle errors for Class.forName
		      e.printStackTrace();
		   }finally{
		      //finally block used to close resources
		      try{
		         if(stmt!=null)
		            conn.close();
		      }catch(SQLException se){
		      }// do nothing
		      try{
		         if(conn!=null)
		            conn.close();
		      }catch(SQLException se){
		         se.printStackTrace();
		      }
		   }//end finally try
	
	}
   public void RunQuery(String sql){
	   try{
		      //STEP 2: Register JDBC driver
		      Class.forName(JDBC_DRIVER);
	
		      //STEP 3: Open a connection
		      System.out.println("Connecting to a selected database...");
		      conn = DriverManager.getConnection(DB_URL, USER, PASS);
		      System.out.println("Connected database successfully...");
		      //STEP 4: Execute a query
		      System.out.println("Querying...");
		      System.out.println(sql );
		      stmt = conn.createStatement();
		      ResultSet rs = stmt.executeQuery(sql);
		      ResultSetMetaData rsmd = rs.getMetaData();
		      int columnsNumber = rsmd.getColumnCount();
		      while (rs.next()) {
		          for (int i = 1; i <= columnsNumber; i++) {
		              if (i > 1) System.out.print(",  ");
		              String columnValue = rs.getString(i);
		              System.out.print(columnValue + " " + rsmd.getColumnName(i));
		          }
		          System.out.println("");
		      }
		      System.out.println("done and done");
	   }catch(SQLException se){
		      //Handle errors for JDBC
		      se.printStackTrace();
		   }catch(Exception e){
		      //Handle errors for Class.forName
		      e.printStackTrace();
		   }finally{
		      //finally block used to close resources
		      try{
		         if(stmt!=null)
		            conn.close();
		      }catch(SQLException se){
		      }// do nothing
		      try{
		         if(conn!=null)
		            conn.close();
		      }catch(SQLException se){
		         se.printStackTrace();
		      }
		   }//end finally try
	
	}
}
