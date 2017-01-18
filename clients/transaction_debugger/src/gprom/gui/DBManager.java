package gprom.gui;
import java.sql.*;

/**
 * this class is use to manager the database, connect and exectue sql and it
 * will use single design pattern
 * 
 * @author lingxue zheng
 *
 */
public class DBManager {
	/** the url to the database **/
	private String url = "jdbc:oracle:thin:@localhost:1521/PDBORCL";
	/** atabase jdbc **/
	private String driver = "oracle.jdbc.driver.OracleDriver";
	/** user name in database **/
	private String user = "pdbadmin";
	/** password in database **/
	private String password = "learnin";
	/** this is the single object of Connection **/
	private Connection connection = null;
	/** the Statement when execute sql **/
	Statement stmt;
	/** single pattern **/
	private static DBManager dbManager = null;

	/**
	 * constructer will be private in single pattern
	 */
	private DBManager() {
		try {
			Class.forName(driver);
		} catch (Exception e) {
			System.out.println("Fail loading Driver!");
			e.printStackTrace();
		}
		try {
			connection = DriverManager.getConnection(url, user, password);
			if (connection.isClosed())
				System.out.println("fail to connecting to the Database!");
		} catch (SQLException e) {
			System.out
					.println("Connection URL or username or password errors!");
			e.printStackTrace();
		}
	}

	/**
	 * java single desgin pattern get the unique object;
	 * 
	 * @return
	 */
	public static DBManager getInstance() {
		if (dbManager == null) {
			dbManager = new DBManager();
		}
		return dbManager;
	}

	/**
	 * get the database connection object
	 * 
	 * @return
	 */
	public Connection getConnection() {
		return connection;
	}

	/**
	 * exectue a query sql and get the result set
	 * 
	 * @param sql
	 * @return resultset object
	 */
	public ResultSet executeQuery(String sql) {
		ResultSet rs = null;
		if (stmt == null) {
			try {
				stmt = connection.createStatement();
			} catch (SQLException e) {
				e.printStackTrace();
				System.out.println("create Statement fail:" + e.toString());
			}
		}
		try {
			rs = stmt.executeQuery(sql);
		} catch (SQLException e) {
			e.printStackTrace();
		}
		return rs;
	}

	/**
	 * exectue a update sql
	 * 
	 * @param sql
	 * @return if execute successful return true or it will return false;
	 */
	@SuppressWarnings("finally")
	public boolean executeUpdate(String sql) {
		boolean v = false;
		try {
			v = stmt.executeUpdate(sql) > 0 ? true : false;
		} catch (SQLException e) {
			e.printStackTrace();
		} finally {
			return v;
		}
	}

	public void close() {
		if (stmt != null) {
			try {
				stmt.close(); // close statement
			} catch (SQLException e) {
				e.printStackTrace();
				System.out.println("close statement fail:" + e.toString());
			}
		}
		if (connection != null) {
			try {
				connection.close(); // close connection
				connection = null;
			} catch (SQLException e) {
				e.printStackTrace();
				System.out.println("close connection fail:" + e.toString());
			}
		}
		dbManager = null;
	}

//	public static void main(String[] args) throws Exception {
//
//		DBManager dbManager = DBManager.getInstance();
//		if (dbManager.getConnection() == null) {
//			System.out.println("connect oracle database failed！");
//		} else {
//			System.out.println("connect oracle database succeed！");
//		}
//
//		ResultSet result = dbManager.executeQuery("SELECT * FROM PERSONS");
//		while (result.next()) {
//			// dbManager.executeUpdate("insert into )
//			// String city = result.getString("CITY");
//			System.out.println("the result is " + city);
//			// int index = 0; // use to record the resultList length.
//			// while (result.next()){
//			// /**get all attributes in database table**/
//			// int quantity = result.getInt("quant");
//			// String customer = result.getString("cust");
//			// String product = result.getString("prod");
//			// int day = result.getInt("day");
//			// int month = result.getInt("month");
//			// int year = result.getInt("year");
//			// String state = result.getString("state");
//		}

//	}
}
