package org.gprom.jdbc.test;

import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;
import org.gprom.jdbc.driver.*;
import org.gprom.jdbc.jna.*;

public class GProMTest {
	// Variable
	private static Logger log = Logger.getLogger(GProMTest.class);

	static {
		System.setProperty("jna.library.path","/Users/lord_pretzel/Documents/workspace/GProM/src/libgprom/.libs/");
		System.out.println(System.getProperty("jna.library.path") + "\n\n");
	}
	
	/**
	 * @param args
	 * @throws SQLException 
	 */
	public static void main(String[] args) throws SQLException {
		PropertyConfigurator.configureAndWatch("javalib/log4j.properties");
		String driverURL = "org.gprom.jdbc.driver.GProMDriver";
		GProMWrapper w = GProMWrapper.inst;
		
		w.init();
		w.setupFromOptions(new String[] { "-log", "-loglevel", "4", "-backend", "oracle"});
		w.setLogLevel(1);
		w.setBoolOption("log.active", true);
		log.error("log.level=" +  w.getIntOption("log.level"));
		log.error("log.active=" +  w.getBoolOption("log.active"));
		
		String res = w.gpromRewriteQuery("SELECT * FROM r;");
		log.error(res == null ? "NULL" : res);
		
		res = w.gpromRewriteQuery("PROVENANCE OF (SELECT * FROM r);");
		log.error(res == null ? "NULL" : res);
		
		w.shutdown();
		System.exit(1);
		
		
		// HSQLDB
		/*
		 * //String url = "jdbc:gprom:hsqldb:file:/Users/alex/db/mydb"; String
		 * url = "jdbc:gprom:hsqldb:file:/Users/alex/db/testdb"; String username
		 * = "SA"; String password = "";
		 */

		// POSTGRES
		// String url = "jdbc:gprom:postgresql://localhost/mydb";
		String url = "jdbc:gprom:postgresql://127.0.0.1/testdb";
		String username = "postgres";
		String password = "";

		// load driver class
		try {
			Class.forName(driverURL);
		} catch (ClassNotFoundException e) {
			log.error("Error loading the driver");
			log.error(e.getMessage());
			System.exit(-1);
		}
		// get the db connection
//		GProMConnection gpromConnection = null;
//		try {
//			gpromConnection = (GProMConnection) DriverManager.getConnection(url,
//					username, password);
//		} catch (SQLException e) {
//			log.error("Error getting a connection");
//			log.error(e.getMessage());
//			System.exit(-1);
//		}
		// create a new statement
//		GProMStatement std = gpromConnection.createGProMStatement();
//		ResultSet rs;
//		try {
			// rs = std.executeQuery("SELECT PROVENANCE * FROM weather");
			// rs =
			// std.executeQuery("SELECT PROVENANCE * FROM AVGTEMP_IN_CITY");
			// rs =
			// std.executeQuery("SELECT PROVENANCE* FROM place WHERE isnice != false");
			// rs =
			// std.executeQuery("SELECT PROVENANCE isnice, CASE isnice WHEN isnice THEN 'true' ELSE 'false' END FROM place");
//			 rs =
//			 std.executeQuery("SELECT PROVENANCE * FROM weather WHERE temp_lo > (SELECT AVG(temp_lo) FROM weather)");
			// rs =
			// std.executeQuery("SELECT PROVENANCE * FROM city WHERE EXISTS(SELECT 1 FROM place WHERE name = place.name)");
			// rs =
			// std.executeQuery("SELECT PROVENANCE * FROM city CROSS JOIN place");
			// rs =
			// std.executeQuery("SELECT PROVENANCE * FROM city INNER JOIN place ON place.name = city.name ");
			// rs =
			// std.executeQuery("SELECT PROVENANCE * FROM city INNER JOIN place ON (place.name = city.name) WHERE place.isnice = 't'");
			// rs =
			// std.executeQuery("SELECT PROVENANCE COUNT(city) FROM weather WHERE weather.temp_lo BETWEEN (SELECT PROVENANCE AVG(a.temp_lo) FROM a WHERE a.prcp = 0.3 GROUP BY a.temp_lo) AND -20");
			// rs =
			// std.executeQuery("SELECT PROVENANCE MAX(city) FROM weather GROUP BY city");
			// rs =
			// std.executeQuery("SELECT PROVENANCE place.name, city.country, MAX(point) as MAXPOINT FROM place LEFT JOIN city USING(name) GROUP BY place.name, city.country");
			// rs =
			// std.executeQuery("SELECT PROVENANCE city, SUM(temp_lo) FROM weather GROUP BY city HAVING AVG(temp_hi) > 3");
			// rs =
			// std.executeQuery("SELECT PROVENANCE city, SUM(temp_lo) FROM weather GROUP BY city HAVING city < 'a'");
			// rs =
			// std.executeQuery("SELECT PROVENANCE DISTINCT temp_lo AS value, temp_lo + temp_hi AS sum FROM weather");
			// rs =
			// std.executeQuery("SELECT PROVENANCE city, SUM(temp_lo) FROM weather GROUP BY city ORDER BY 1");
			// rs =
			// std.executeQuery("SELECT PROVENANCE temp_lo, temp_hi FROM weather GROUP BY temp_lo, temp_hi, city, prcp, date ORDER BY temp_lo + temp_hi DESC");
			// rs =
			// std.executeQuery("SELECT PROVENANCE sum(sump) FROM (SELECT sum(i.price) AS sump FROM item i, sales s WHERE i.id = s.item_id GROUP BY s.shop_id) AS sub");
//			 rs = std.executeQuery("SELECT PROVENANCE sum(i) * null FROM r");
			// rs =
			// std.executeQuery("SELECT PROVENANCE sum(sub.sump) AS sum FROM ( SELECT sum(i.price) AS sump FROM item i, sales s WHERE i.id = s.item_id GROUP BY s.shop_id) sub");
//			 rs =
//			 std.executeQuery("SELECT PROVENANCE sum(sub.sum),(id / 2) AS id FROM (SELECT sum(id) AS sum,(id / 2) AS id FROM aggr GROUP BY (id / 2)) AS sub GROUP BY sum, (id / 2)");
//			rs = std
//					.executeQuery("SELECT PROVENANCE r.i IN (SELECT * FROM s), (SELECT * FROM t WHERE t.i = r.i) AS two, r.i FROM r");
//			rs = std
//					.executeQuery("select provenance l_returnflag,l_linestatus,sum(l_quantity) as sum_qty,sum(l_extendedprice) as sum_base_price,sum(l_extendedprice * (1 - l_discount)) as sum_disc_price,sum(l_extendedprice * (1 - l_discount) * (1 + l_tax)) as sum_charge,avg(l_quantity) as avg_qty,avg(l_extendedprice) as avg_price,avg(l_discount) as avg_disc,count(*) as count_order from lineitem where l_shipdate <= ('1992-01-11'::date) - ('0 day'::interval) group by l_returnflag, l_linestatus order by l_returnflag, l_linestatus limit 1");
//			rs = std.executeQuery("SELECT PROVENANCE * FROM r");
//			while (rs.next()) {
//				log.info(rs.getString(1));
//			}
//		} catch (SQLException e) {
//			log.error("Error getting a result set from a query");
//			log.error(e.getMessage());
//			System.exit(-1);
//		}
	}

}
