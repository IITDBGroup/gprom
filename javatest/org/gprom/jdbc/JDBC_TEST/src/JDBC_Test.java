import java.sql.Connection;  
import java.sql.DriverManager;  
import java.sql.PreparedStatement;  
import java.sql.ResultSet;  
import java.sql.Statement;  
import java.sql.* ;  
 
//JDBC Connection Test
public class JDBC_Test {  

   //1521 is port number  
	//pdborcl is the database name
	//11g:1521:PDBORCL, 12c:1521/PDBORCL
    private static String url="jdbc:oracle:thin:@localhost:1521/PDBORCL";  
    //pdbadmin is username
    private static String user="pdbadmin";  
    //learnin is passward
    private static String password="learnin";  
    public static Connection conn;  
    public static PreparedStatement ps;  
    public static ResultSet rs;  
    public static Statement st ;  
    //connect db 
    public void getConnection(){  
        try {   
            Class.forName("oracle.jdbc.driver.OracleDriver");    
            conn=DriverManager.getConnection(url, user, password);  
              
        } catch (Exception e) {  
            // TODO: handle exception  
            e.printStackTrace();  
        }  
    }  
     //test oracle connection  
     public static void main(String[] args) {  
        JDBC_Test basedao=new JDBC_Test();  
        basedao.getConnection();  
        if(conn==null){  
            System.out.println("connect oracle database failed！");  
        }else{  
            System.out.println("connect oracle database succeed！");  
        }  
     }  
} 
